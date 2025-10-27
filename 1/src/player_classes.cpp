#include <random>
#include <vector>
#include <set>
#include <algorithm>
#include "player.cpp"
#include <concepts>

// Концепт для проверки наличия метода act
template <typename T>
concept HasActMethod = requires(T obj) {
    { obj.act() }; // Проверка на наличие метода act
};

template <typename T>
T choose_random(const std::vector<T>& candidates) {
    if (candidates.empty()) return T{};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, candidates.size() - 1);
    return candidates[dist(gen)];
}


class Civilian : public Player {
public:

    Civilian(int id, bool is_user = false)
        : Player(id, Role::CITIZEN, is_user) {}
    
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                    Logger& logger, int round, bool is_mafia_round) override {
        // Мирные жители ночью не действуют
        co_return;
    }
    
    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                     Logger& logger, int round) override {
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            logger.logRound(round, "Мирный житель " + std::to_string(id) + 
                           " выбрал игрока " + std::to_string(target) + " на голосовании.");
        } else {
            target = -1;
        }
        co_return;
    }
    
    std::string getRoleName() const override { 
        return "Мирный житель"; 
    }
};

// Мафия
class Mafia : public Player {
public:
    bool is_don;
    Mafia(int id, bool is_don, bool is_user = false)
        : Player(id, Role::MAFIA, is_user), is_don(is_don) {}
    
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                    Logger& logger, int round, bool is_mafia_round) override {
        if (is_mafia_round) {
                // Обычная мафия выбирает жертву из не-мафиози
                auto targets = getNonMafiaTargets(alive_players);
                if (!targets.empty()) {
                    kill_target = choose_random(targets);
                    logger.logRound(round, "Мафия " + std::to_string(id) + 
                                   " выбрала игрока " + std::to_string(kill_target) + " для убийства.");
                } else {
                    logger.logRound(round, "Мафия " + std::to_string(id) + " не нашла подходящих целей.");
                }
            } else {
                // Дон мафии собирает и анализирует выборы мафиози
                processDonAction(alive_players, logger, round);
            }
        co_return;
    }
    
    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                     Logger& logger, int round) override {
        auto targets = getNonMafiaTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            std::string role_name = is_don ? "Дон мафии" : "Мафия";
            logger.logRound(round, role_name + " " + std::to_string(id) + 
                           " выбрала игрока " + std::to_string(target) + " на голосовании.");
        } else {
            target = -1;
            logger.logRound(round, getRoleName() + " " + 
                           std::to_string(id) + " не может проголосовать.");
        }
        co_return;
    }
    
    std::string getRoleName() const override { 
        return is_don ? "Дон мафии" : "Мафия"; 
    }
    
    bool isDon() const { return is_don; }

private:
    void processDonAction(const std::map<int, SmartPointer<Player>>& alive_players, 
                         Logger& logger, int round) {
        // Собираем цели всех мафиози
        std::map<int, int> target_votes;
        
        for (const auto& [player_id, player] : alive_players) {
            if (player->getRole() == Role::MAFIA && player->getKillTarget() != -1) {
                int kill_target = player->getKillTarget();
                target_votes[kill_target]++;
                
                // logger.logRound(round, "Дон мафии видит, что мафия " + std::to_string(player_id) + 
                //                " хочет убить игрока " + std::to_string(kill_target));
            }
        }
        
        if (!target_votes.empty()) {
            // Находим цель с максимальным количеством голосов
            int max_votes = 0;
            std::vector<int> most_voted_targets;
            
            for (const auto& [target_id, vote_count] : target_votes) {
                if (vote_count > max_votes) {
                    max_votes = vote_count;
                    most_voted_targets = {target_id};
                } else if (vote_count == max_votes) {
                    most_voted_targets.push_back(target_id);
                }
            }
            
            // Выбираем случайную цель из наиболее популярных
            if (!most_voted_targets.empty()) {
                kill_target = choose_random(most_voted_targets);
                logger.logRound(round, "Дон мафии " + std::to_string(id) + 
                               " выбрал игрока " + std::to_string(kill_target) + 
                               " на основе голосов мафии (" + std::to_string(max_votes) + " голосов)");
            }
        } else {
            // Если мафиози не выбрали цели, дон выбирает сам
            auto targets = getNonMafiaTargets(alive_players);
            if (!targets.empty()) {
                kill_target = choose_random(targets);
                logger.logRound(round, "Дон мафии " + std::to_string(id) + 
                               " самостоятельно выбрал игрока " + std::to_string(kill_target) + " для убийства.");
            } else {
                logger.logRound(round, "Дон мафии " + std::to_string(id) + " не нашел подходящих целей.");
            }
        }
    }
};

// Комиссар
class Commissioner : public Player {
private:
    std::set<int> known_mafia;
    std::set<int> known_civilians;
    
public:
    Commissioner(int id, bool is_user = false)
        : Player(id, Role::COMMISSIONER, is_user) {}
    
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                    Logger& logger, int round, bool is_mafia_round) override {
        if (previous_target != -1) {
            kill_target = previous_target;
            previous_target = -1;
            logger.logRound(round, "Комиссар знает, что игрок " + std::to_string(kill_target) + " мафия. И стреляет в него.");
            co_return;
        }
        // Комиссар проверяет игроков ночью
        std::vector<int> check_targets;
        for (const auto& [player_id, player] : alive_players) {
            if (player_id != id && 
                known_mafia.find(player_id) == known_mafia.end() &&
                known_civilians.find(player_id) == known_civilians.end()) {
                check_targets.push_back(player_id);
            }
        }
        
        if (!check_targets.empty()) {
            int checked_player = choose_random(check_targets);
            auto player_role = alive_players.at(checked_player)->getRole();
            
            if (player_role == Role::MAFIA) {
                known_mafia.insert(checked_player);
                previous_target = checked_player;
                logger.logRound(round, "Комиссар " + std::to_string(id) + 
                                " обнаружил, что игрок " + std::to_string(checked_player) + " - мафия!");
            } else {
                known_civilians.insert(checked_player);
                logger.logRound(round, "Комиссар " + std::to_string(id) + 
                                " проверил игрока " + std::to_string(checked_player) + " - он не мафия.");
            }
        }
        co_return;
    }
    
    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                     Logger& logger, int round) override {
        // Комиссар голосует против известной мафии, если она жива
        for (int mafia_id : known_mafia) {
            if (alive_players.find(mafia_id) != alive_players.end()) {
                target = mafia_id;
                logger.logRound(round, "Комиссар " + std::to_string(id) + 
                               " голосует против известной мафии " + std::to_string(target));
                co_return;
            }
        }
        
        // Иначе случайный выбор
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            logger.logRound(round, "Комиссар " + std::to_string(id) + 
                           " выбрал игрока " + std::to_string(target) + " на голосовании.");
        } else {
            target = -1;
        }
        co_return;
    }
    
    std::string getRoleName() const override { 
        return "Комиссар"; 
    }
    
    const std::set<int>& getKnownMafia() const { return known_mafia; }
    const std::set<int>& getKnownCivilians() const { return known_civilians; }
};

// Маньяк
class Maniac : public Player {
public:
    Maniac(int id, bool is_user = false)
        : Player(id, Role::MANIAC, is_user) {}
    
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                    Logger& logger, int round, bool is_mafia_round) override {
        // Маньяк действует ночью
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            kill_target = choose_random(targets);
            logger.logRound(round, "Маньяк " + std::to_string(id) + 
                            " выбрал игрока " + std::to_string(kill_target) + " для убийства.");
        }
        co_return;
    }
    
    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                     Logger& logger, int round) override {
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            logger.logRound(round, "Маньяк " + std::to_string(id) + 
                           " выбрал игрока " + std::to_string(target) + " на голосовании.");
        } else {
            target = -1;
        }
        co_return;
    }
    
    std::string getRoleName() const override { 
        return "Маньяк"; 
    }
};

// Доктор
class Doctor : public Player {
public:
    Doctor(int id, bool is_user = false)
        : Player(id, Role::DOCTOR, is_user) {}
    
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                    Logger& logger, int round, bool is_mafia_round) override {
    
        // Доктор лечит ночью (не может лечить того же игрока дважды подряд)
        std::vector<int> heal_targets;
        for (const auto& [player_id, player] : alive_players) {
            if (player_id != previous_target) {
                heal_targets.push_back(player_id);
            }
        }
        
        if (!heal_targets.empty()) {
            target = choose_random(heal_targets);
            previous_target = target;
            logger.logRound(round, "Доктор " + std::to_string(id) + 
                            " лечит игрока " + std::to_string(target));
        } else {
            target = -1;
            logger.logRound(round, "Доктор " + std::to_string(id) + " не нашел, кого лечить.");
        }
        
        co_return;
    }
    
    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                     Logger& logger, int round) override {
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            logger.logRound(round, "Доктор " + std::to_string(id) + 
                           " выбрал игрока " + std::to_string(target) + " на голосовании.");
        } else {
            target = -1;
        }
        co_return;
    }
    
    std::string getRoleName() const override { 
        return "Доктор"; 
    }
};

class Bodyguard : public Player {
public:
    Bodyguard(int id, bool is_user = false) 
        : Player(id, Role::BODYGUARD, is_user) {}
    
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                    Logger& logger, int round, bool is_mafia_round) override {
        // Телохранитель выбирает игрока для защиты
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            logger.logRound(round, "Телохранитель " + std::to_string(id) + 
                                " защищает игрока " + std::to_string(target));
        }
    
        co_return;
    }

    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                     Logger& logger, int round) override {
        auto targets = getAliveTargets(alive_players);
        if (!targets.empty()) {
            target = choose_random(targets);
            logger.logRound(round, "Телохранитель " + std::to_string(id) + 
                               " выбрал игрока " + std::to_string(target) + " на голосовании.");
        }
        co_return;
    }

    std::string getRoleName() const override { 
        return "Телохранитель"; 
    }
    
    static constexpr bool is_valid_player = true;
};