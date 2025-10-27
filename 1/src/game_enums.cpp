#include <string>

enum class Role {
    MAFIA,
    CITIZEN,
    COMMISSIONER,
    DOCTOR,
    MANIAC,
    BODYGUARD
};

enum class PlayerState {
    ALIVE,
    DEAD,
    HEALED
};

inline std::string roleToString(Role role) {
    switch (role) {
        case Role::MAFIA: return "Мафия";
        case Role::CITIZEN: return "Мирный житель";
        case Role::COMMISSIONER: return "Комиссар";
        case Role::DOCTOR: return "Доктор";
        case Role::MANIAC: return "Маньяк";
        case Role::BODYGUARD:  return "Телохранитель";
        default: return "Неизвестно";
    }
}