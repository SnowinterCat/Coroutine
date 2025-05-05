#include "luancher/u8main.hpp"

#include <iostream>
#include <coroutine>

struct TaskSuspendFinal {
    constexpr bool await_ready() const noexcept { return true; }
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

struct TaskPromise {
    using handle_type = std::coroutine_handle<TaskPromise>;

    auto initial_suspend() noexcept { return std::suspend_never(); }
    auto final_suspend() noexcept { return std::suspend_always(); }

    void unhandled_exception() {}
    void return_void() noexcept {}

    auto get_return_object() noexcept { return handle_type::from_promise(*this); }
};

struct Task {
    using promise_type = TaskPromise;
    using handle_type  = std::coroutine_handle<promise_type>;

    Task(handle_type handle) : _handle(handle) {}
    ~Task() { _handle ? _handle.destroy() : (void)0; }

    void resume() { _handle.done() ? (void)0 : _handle.resume(); }

private:
    handle_type _handle = nullptr;
};

Task counter()
{
    std::cout << "counter: called\n";
    for (size_t i = 0; i < 4; ++i) {
        co_await std::suspend_always{};
        std::cout << "counter:: resumed (#" << i << ")\n";
    }
    co_return;
}

int u8main([[maybe_unused]] int argc, [[maybe_unused]] const char *const *argv)
{
    std::cout << "hello world\n";

    Task counter1 = counter();
    std::cout << "main:    resuming counter\n";
    counter1.resume();
    counter1.resume();
    counter1.resume();
    counter1.resume();
    counter1.resume();
    std::cout << "main:    done\n";
    return 0;
}