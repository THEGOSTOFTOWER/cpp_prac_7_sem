#include <string>
#include "game_enums.cpp"
#include "smart_pointer.h"
#include "coroutines.cpp"
#include "Logger.cpp"
#include <vector>

class Player {
public:
    int id;
    PlayerState state;
    Role role;
    bool is_user;
    int target;
    int kill_target;
    int previous_target;

    Player(int id, Role role, bool is_user = false)
        : id(id), state(PlayerState::ALIVE), role(role), is_user(is_user), target(-1), kill_target(-1), previous_target(-1) {}
    
    virtual ~Player() = default;
    
    int getId() const { return id; }
    PlayerState getState() const { return state; }
    Role getRole() const { return role; }
    bool isUser() const { return is_user; }
    bool isAlive() const { return state == PlayerState::ALIVE; }

    int getTarget() const { return target; }
    int getKillTarget() const { return kill_target; }
    int getPreviousTarget() const { return previous_target; }

    void setState(PlayerState new_state) { state = new_state; }
    void setTarget(int t) { target = t; }
    void setKillTarget(int t) { kill_target = t; }
    void setPreviousTarget(int t) { previous_target = t; }
        
    virtual PlayerAction act(const std::map<int, SmartPointer<Player>>& alive_players, 
                           Logger& logger, int round, bool is_mafia_round) = 0;
    
    virtual PlayerAction vote(const std::map<int, SmartPointer<Player>>& alive_players,
                            Logger& logger, int round) = 0;
    virtual std::string getRoleName() const = 0;

    // Вспомогательные методы
    std::vector<int> getAliveTargets(const std::map<int, SmartPointer<Player>>& alive_players, 
                                   bool exclude_self = true) const {
        std::vector<int> targets;
        for (const auto& [player_id, player] : alive_players) {
            if (!exclude_self || player_id != id) {
                targets.push_back(player_id);
            }
        }
        return targets;
    }
    
    std::vector<int> getNonMafiaTargets(const std::map<int, SmartPointer<Player>>& alive_players) const {
        std::vector<int> targets;
        for (const auto& [player_id, player] : alive_players) {
            if (player_id != id && player->getRole() != Role::MAFIA) {
                targets.push_back(player_id);
            }
        }
        return targets;
    }
};