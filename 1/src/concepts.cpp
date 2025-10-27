#include <iostream>
#include <concepts>

// Концепт для проверки наличия метода act
template <typename T>
concept PlayerConcept = requires(T player,
                                 std::vector<size_t> ids,
                                 size_t value,
                                 std::vector<SmartPtr<Player>> players) {
    // Проверяю, что у ведущего классы, для которых определены методы vote и act в объектах

    // тип T имеет  vote
    { player.vote(ids, value) } -> std::same_as<Task>;
    // тип T имеет  act
    { player.act(ids, night_actions, players) } -> std::same_as<Task>;
};