#include <vector>
#include <string>
#include <memory>
#include <random>
#include <algorithm>
#include <map>
#include <ranges>
#include <chrono>
#include <ranges>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <future>
#include "src/player_classes.cpp"
#include <thread>


#define DEBUG_LOG true


class MafiaGame {
private:
    std::map<int, SmartPointer<Player>> alivePlayers;
    Logger logger;
    bool user_in_game;
    int userId;
    int round;

public:
    MafiaGame(int N, bool user_participates) : user_in_game(user_participates), userId(-1), round(1) {
        initializePlayers(N);
        if (user_in_game) {
            assignUserRole();
        }
        logInitialRoles();
    }

    void run() {
        logger.logRound(0, "Игра началась!");
        
        while (true) {
            logger.logRound(round, "Начало раунда " + std::to_string(round));

            nightPhase(round);
            if (checkGameEnd()) break;

            dayPhase(round);
            if (checkGameEnd()) break;

            round++;
        }
    }

private:
    void initializePlayers(int N) {
        std::random_device rd;
        std::mt19937 gen(rd());

        int k = 3;
        int mafia_count = N / k;
        if (mafia_count == 0) mafia_count = 1;

        // Добавляем мафию
        for (int i = 1; i <= mafia_count; ++i) {
            alivePlayers[i] = SmartPointer<Player>(new Mafia(i, i == 1 ? true : false)); // Первый - дон
        }

        // Добавляем комиссара
        alivePlayers[mafia_count + 1] = SmartPointer<Player>(new Commissioner(mafia_count + 1));

        // Добавляем маньяка
        alivePlayers[mafia_count + 2] = SmartPointer<Player>(new Maniac(mafia_count + 2));

        // Добавляем доктора
        alivePlayers[mafia_count + 3] = SmartPointer<Player>(new Doctor(mafia_count + 3));

        // Добавляем мирных жителей
        int civilians_count = N - mafia_count - 3;
        for (int i = 0; i < civilians_count; ++i) {
            alivePlayers[mafia_count + 4 + i] = SmartPointer<Player>(new Civilian(mafia_count + 4 + i));
        }
    }

    void assignUserRole() {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        std::vector<int> playerIds;
        for (const auto& [id, player] : alivePlayers) {
            playerIds.push_back(id);
        }

        std::shuffle(playerIds.begin(), playerIds.end(), gen);
        userId = playerIds[0];

        std::cout << "Вы играете за игрока с ID: " << userId << ". Ваша роль: " << alivePlayers[userId]->getRoleName() << std::endl;
    }

    void logInitialRoles() {
        for (const auto& [id, player] : alivePlayers) {
            logger.logRound(0, "Игрок " + std::to_string(id) + ": " + player->getRoleName());
        }
    }

    void userNightAction(int userId) {
        if (alivePlayers.find(userId) == alivePlayers.end()) {
            std::cout << "Вы не можете выполнить действие, так как ваш персонаж больше не жив." << std::endl;
            return;
        }
        
        std::cout << "Игрок жив. Ночной раунд: " << round << std::endl;

        Role role = alivePlayers[userId]->getRole();
        
        if (role == Role::CITIZEN) {
            std::cout << "Вы мирный, пропуск ночной фазы." << std::endl;
            return;
        }

        if (role == Role::COMMISSIONER) {
            handleCommissionerNightAction(userId);
            return;
        }

        if (role == Role::DOCTOR) {
            handleDoctorNightAction(userId);
            return;
        }

        std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
        displayAlivePlayers(userId);
        
        int targetId;
        std::cin >> targetId;

        if (validateTarget(targetId, userId)) {
            if (role == Role::MANIAC) alivePlayers[userId]->setKillTarget(targetId);
            else alivePlayers[userId]->setTarget(targetId);
            logger.logRound(round, "Игрок " + std::to_string(userId) + " выбрал цель: игрок " + std::to_string(targetId));
        }
    }

    void handleCommissionerNightAction(int userId) {
        displayAlivePlayers(userId);
        std::cout << "1 - Проверить игрока." << std::endl;
        std::cout << "2 - Выстрелить в игрока." << std::endl;
        
        int choice;
        std::cin >> choice;
        
        while (choice != 1 && choice != 2) {
            std::cout << "1 - Проверить игрока." << std::endl;
            std::cout << "2 - Выстрелить в игрока." << std::endl;
            std::cin >> choice;
        }

        std::cout << "Выберите ID игрока: " << std::endl;
        int target;
        std::cin >> target;

        if (!validateTarget(target, userId)) return;

        if (choice == 1) {
            if (alivePlayers[target]->getRole() == Role::MAFIA) {
                std::cout << "Выбранный игрок: мафия." << std::endl;
            } else {
                std::cout << "Выбранный игрок: мирный." << std::endl;
            }
        } else if (choice == 2) {
            alivePlayers[userId]->setKillTarget(target);
        }
    }

    void handleDoctorNightAction(int userId) {
        displayAlivePlayers(userId);
        std::cout << "Выберите ID игрока для исцеления: " << std::endl;
        
        int target;
        std::cin >> target;
        
        Doctor* doctor = dynamic_cast<Doctor*>(alivePlayers[userId].get());
        while (!validateTarget(target, userId) || target == doctor->getPreviousTarget()) {
            std::cout << "Неверный ID игрока. ID не должен быть таким же как в прошлом раунде и должен принадлежать списку." << std::endl;
            std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
            std::cin >> target;
        }

        doctor->setPreviousTarget(target);
        alivePlayers[userId]->setTarget(target);
    }

    void userVotePhase(int userId) {
        if (alivePlayers.find(userId) == alivePlayers.end()) {
            std::cout << "Вы не можете голосовать, так как ваш персонаж больше не жив." << std::endl;
            return;
        }
        
        std::cout << "Игрок жив. Дневной раунд: " << round << std::endl;
        std::cout << "Ваш голос! Выберите игрока для голосования (введите ID):" << std::endl;

        displayAlivePlayers(userId);

        int voteTarget;
        std::cin >> voteTarget;

        if (validateTarget(voteTarget, userId)) {
            alivePlayers[userId]->setTarget(voteTarget);
            logger.logRound(round, "Игрок " + std::to_string(userId) + " голосует за игрока " + std::to_string(voteTarget));
        }
    }

    void displayAlivePlayers(int excludeId = -1) {
        for (const auto& [id, player] : alivePlayers) {
            if (id != excludeId) {
                std::cout << "Игрок " << id << " (" << player->getRoleName() << ")" << std::endl;
            }
        }
    }

    bool validateTarget(int targetId, int userId) {
        if (targetId == userId) {
            std::cout << "Нельзя выбирать себя в качестве цели. Действие пропущено." << std::endl;
            return false;
        }
        
        if (alivePlayers.find(targetId) == alivePlayers.end()) {
            std::cout << "Некорректный выбор. Действие пропущено." << std::endl;
            return false;
        }
        
        return true;
    }

    void nightPhase(int round) {
        logger.logRound(round, "Ночной этап:\n");

        if(DEBUG_LOG){
                for (const auto& [id, player] : alivePlayers) {
                    logger.logRound(round, "Игрок " + std::to_string(id) + ": " + player->getRoleName());
                }
                logger.logRound(round, "");
            }
        std::vector<std::future<void>> futures;
        int victimMafia = -1; 
        int victimManiac = -1;
        int victimCom = -1;
        int donId = -1;
        int victimDoctor = -1;
        bool isDoctorKilled = false;
        SmartPointer<Player> don;

        // Находим дона и мафиози
        for (const auto& [id, player] : alivePlayers) {
            if (player->getRoleName() == "Дон мафии") {
                don = player;
                donId = id;
            }
        }

        // Параллельные действия игроков
        for (const auto& [id, player] : alivePlayers) {
            if (user_in_game && id == userId) {
                userNightAction(userId);
            } else {
                futures.push_back(std::async(std::launch::async, [&, id, player]() {
                    Role role_name = player->getRole();
                    if (role_name == Role::MAFIA || 
                        role_name == Role::MANIAC || role_name == Role::COMMISSIONER || role_name == Role::DOCTOR) {
                        PlayerAction action = player->act(alivePlayers, logger, round, true);
                        action.resume();
                    }
                }));
            }
        }
        

        for (auto& future : futures) {
            future.get();
        }
        futures.clear();
        // Действие Дона
        //Не вызываем действия дона, если игрок и есть Дон
        if (donId != -1 && (userId != donId)) {
            futures.push_back(std::async(std::launch::async, [&]() {
                PlayerAction action = alivePlayers[donId].get()->act(alivePlayers, logger, round, false);
                action.resume();
            }));
        }
        // Ждем завершения действия Дона
        for (auto& future : futures) {
            future.get();
        }
        // Получаем цели через традиционные циклы (без ranges)
        for (const auto& [id, player] : alivePlayers) {
            if (player->getRoleName() == "Дон мафии") {
                victimMafia = player->getKillTarget();
            }
            if (player->getRole() == Role::MANIAC) {
                victimManiac = player->getKillTarget();
            }
            if (player->getRole() == Role::COMMISSIONER) {
                victimCom = player->getKillTarget();
            }
            if (player->getRole() == Role::DOCTOR) {
                victimDoctor = player->getTarget();
            }
            player->setKillTarget(-1);
            player->setTarget(-1);
        }

        // Проверка, был ли убит доктор
        bool doctorAlive = false;
        int doctorId = -1;
        for (const auto& [id, player] : alivePlayers) {
            if (player->getRole() == Role::DOCTOR) {
                doctorAlive = true;
                doctorId = id;
                break;
            }
        }

        if (!doctorAlive) {
            isDoctorKilled = true;
        } else {
            isDoctorKilled = (victimMafia == doctorId || victimManiac == doctorId || victimCom == doctorId);
            isDoctorKilled = victimDoctor == doctorId;
        }

        logger.logRound(round, "");
        // Обработка убийств
        processKill(victimMafia, "мафией", victimDoctor);
        processKill(victimManiac, "маньяком", victimDoctor);
        processKill(victimCom, "комиссаром", victimDoctor);
    }

    void processKill(int victim, const std::string& killer, int protectedPlayer) {
        if (victim != -1 && victim != protectedPlayer && alivePlayers.find(victim) != alivePlayers.end()) {
            bool isUser = (user_in_game && victim == userId);
            std::string victimType = isUser ? "Игрок" : "Бот";
            alivePlayers.erase(victim);
            logger.logRound(round, victimType + " " + std::to_string(victim) + " был убит " + killer + ".");

            if (user_in_game) {
                if (isUser) {
                    std::cout << "Игрок был убит " << killer << " в " << round << " раунде!!!!!!" << std::endl;
                } else {
                    std::cout << "Бот " << std::to_string(victim) << " был убит " << killer << "." << std::endl;
                }
            }
        }
    }

    void dayPhase(int round) {
        logger.logRound(round, "Дневной этап:\n");
        if(DEBUG_LOG){
                for (const auto& [id, player] : alivePlayers) {
                    logger.logRound(round, "Игрок " + std::to_string(id) + ": " + player->getRoleName());
                }
                logger.logRound(round, "");
            }
        std::map<int, int> voteCount;
        std::vector<std::future<void>> futures;

        // Параллельное голосование
        for (const auto& [id, player] : alivePlayers) {
            futures.push_back(std::async(std::launch::async, [&, id, player]() {
                if (user_in_game && id == userId) {
                    userVotePhase(userId);
                } else {
                    PlayerAction action = player->vote(alivePlayers, logger, round);
                    action.resume();
                }
            }));
        }

        for (auto& future : futures) {
            future.get();
        }

        // Подсчет голосов
        for (const auto& [id, player] : alivePlayers) {
            int target = player->getTarget();
            //std::cout << id << ":" << target << std::endl;
            if (target != -1) {
                voteCount[target]++;
            }
        }

        // for (const auto& [id, player] : voteCount) {
        //     std::cout << id << ":" << player << std::endl;
            
        // }


        // Изгнание игрока с наибольшим количеством голосов
        if (!voteCount.empty()) {
            auto maxVotesIt = std::max_element(voteCount.begin(), voteCount.end(), 
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                });
            std::vector<int> chosen;
            for (const auto& [id, count]: voteCount) {
                if (count == maxVotesIt->second) {
                    chosen.push_back(id);
                    //std::cout << id << " ";
                }
            }
            std::cout << std::endl;
            int victim = choose_random(chosen);
            bool isUser = (user_in_game && victim == userId);
            std::string victimType = isUser ? "Игрок" : "Бот";
            
            logger.logRound(round, "");
            alivePlayers.erase(victim);
            logger.logRound(round, victimType + " " + std::to_string(victim) + " был изгнан после дневного голосования.");

            if (user_in_game) {
                if (isUser) {
                    std::cout << "Игрок был изгнан после дневного голосования в " << round << " раунде!!!!!!" << std::endl;
                } else {
                    std::cout << "Бот " << std::to_string(victim) << " был изгнан после дневного голосования." << std::endl;
                }
                std::cout << "=========================================================" << std::endl;
            }
        }
    }

    bool checkGameEnd() {
        int mafiaAlive = 0;
        int civiliansAlive = 0;
        int maniacAlive = 0;
        
        for (const auto& [id, player] : alivePlayers) {
            Role role = player->getRole();
            if (role == Role::MAFIA) {
                mafiaAlive++;
            } else if (role == Role::MANIAC) {
                maniacAlive++;
                civiliansAlive++;
            } else {
                civiliansAlive++;
            }
        }

        if (mafiaAlive >= civiliansAlive) {
            logger.logFinal("Мафия победила!");
            if (user_in_game) {
                std::cout << "Мафия победила!\n";
            }
            return true;
        }

        if (mafiaAlive == 0 && maniacAlive == 0) {
            logger.logFinal("Мирные жители победили!");
            if (user_in_game) {
                std::cout << "Мирные жители победили!\n";
            }
            return true;
        }

        if (maniacAlive == 1 && civiliansAlive == 2 && mafiaAlive == 0) {
            logger.logFinal("Маньяк победил!");
            if (user_in_game) {
                std::cout << "Маньяк победил!\n";
            }
            return true;
        }
        
        return false;
    }
};