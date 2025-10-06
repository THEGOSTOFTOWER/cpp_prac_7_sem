#include <iostream>
#include "mafia_game.cpp"

int main() {
    try {
        int N; // Количество игроков
        std::cout << "Введите количество игроков (N > 4): ";
        std::cin >> N;
        
        if (N <= 4) {
            std::cout << "Количество игроков должно быть больше 4." << std::endl;
            return 1;
        }

        // Будет ли пользователь участвовать в игре
        char user_in_game_choice;
        bool user_in_game = false;
        std::cout << "Хотите ли вы участвовать в игре? (y/n): ";
        std::cin >> user_in_game_choice;
        
        if (user_in_game_choice == 'y' || user_in_game_choice == 'Y') {
            user_in_game = true;
        }

        // Создание и запуск игры
        MafiaGame game(N, user_in_game);
        game.run();

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}