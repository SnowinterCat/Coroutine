#include "luancher/u8main.hpp"

#include <iostream>
#include <coroutine>
#include <expected>

template <typename T>
struct TaskPromise;

template <typename T>
struct Task {
    using promise_type = TaskPromise<T>;
    using handle_type  = std::coroutine_handle<promise_type>;
    using value_type   = T;

    explicit Task(handle_type handle) : _handle(handle) {}
    ~Task() { _handle ? _handle.destroy() : (void)0; }

    // auto operator->() -> handle_type * { return &_handle; }
    auto operator->() const -> const handle_type * { return &_handle; }

private:
    handle_type _handle;
};

struct PromiseBase {
    // exception
    constexpr void unhandled_exception() noexcept {}

    // suspend
    constexpr auto initial_suspend() noexcept { return std::suspend_always(); }
    constexpr auto final_suspend() noexcept { return std::suspend_always(); }
};

template <typename T>
struct TaskPromiseImpl : public PromiseBase {
    using value_type = T;

    auto return_value(value_type value) -> void { _value = std::move(value); }
    template <typename U>
        requires(std::convertible_to<U, value_type>)
    auto return_value(U &&value) -> void
    {
        _value = std::forward<U>(value);
    }

private:
    value_type _value;
};

template <typename T, typename E>
struct TaskPromiseImpl<std::expected<T, E>> : public PromiseBase {
    using value_type = std::expected<T, E>;

    auto return_value(value_type value) -> void { _value = std::move(value); }

    template <typename U>
        requires(std::convertible_to<U, value_type>)
    auto return_value(U &&value) -> void
    {
        _value = std::forward<U>(value);
    }

    auto operator*() -> value_type & { return _value; }
    auto operator*() const -> const value_type & { return _value; }
    auto operator->() -> value_type * { return &_value; }
    auto operator->() const -> const value_type * { return &_value; }

private:
    value_type _value;
};

template <>
struct TaskPromiseImpl<void> : public PromiseBase {
public:
    auto return_void() const noexcept {}
    auto value() const noexcept {}
};

template <typename T>
struct TaskPromise final : public TaskPromiseImpl<T> {
    using handle_type = std::coroutine_handle<TaskPromise>;

    auto get_return_object() -> Task<T> { return Task<T>(handle_type::from_promise(*this)); }
};

Task<std::expected<int, std::error_code>> counter()
{
    std::cout << "counter: called\n";
    for (size_t i = 0; i < 4; ++i) {
        co_await std::suspend_always{};
        std::cout << "counter:: resumed (#" << i << ")\n";
    }
    co_return 114514;
    co_return std::unexpected(std::make_error_code(std::errc::io_error));
}

int u8main([[maybe_unused]] int argc, [[maybe_unused]] const char *const *argv)
{
    std::cout << "hello world\n";
    Task counter1 = counter();
    std::cout << "main:    resuming counter\n";
    counter1->done() ? (void)0 : counter1->resume();
    counter1->done() ? (void)0 : counter1->resume();
    counter1->done() ? (void)0 : counter1->resume();
    counter1->done() ? (void)0 : counter1->resume();
    counter1->done() ? (void)0 : counter1->resume();
    std::cout << "main:    done\n";
    if (counter1->done()) {
        if (*counter1->promise())
            std::cout << counter1->promise()->value() << std::endl;
        else
            std::cout << counter1->promise()->error().message() << std::endl;
    }
    return 0;
}