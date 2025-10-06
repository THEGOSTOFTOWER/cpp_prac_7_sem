#include <coroutine>
#include <functional>
#include <vector>
#include <memory>
#include <iostream>

struct PlayerAction {
    struct promise_type {
        PlayerAction get_return_object() {
            return PlayerAction{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    
    std::coroutine_handle<promise_type> handle;
    
    PlayerAction(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~PlayerAction() { 
        if (handle) handle.destroy(); 
    }
    
    // Запрещаем копирование
    PlayerAction(const PlayerAction&) = delete;
    PlayerAction& operator=(const PlayerAction&) = delete;
    
    // Разрешаем перемещение
    PlayerAction(PlayerAction&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    
    PlayerAction& operator=(PlayerAction&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
    
    void resume() {
        if (handle && !handle.done()) {
            handle.resume();
        }
    }
    
    bool done() const {
        return !handle || handle.done();
    }
};