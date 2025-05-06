#include "luancher/u8main.hpp"

#include <iostream>
#include <coroutine>
#include <expected>

template <typename T>
class Promise;

template <typename T>
class CoroutineHandle {
public:
    using promise_type = Promise<T>;
    using handle_type  = std::coroutine_handle<promise_type>;
    using value_type   = T;

    explicit CoroutineHandle(handle_type handle) : _handle(handle) {}
    ~CoroutineHandle() { _handle ? _handle.destroy() : (void)0; }

    CoroutineHandle(CoroutineHandle const &) = delete;
    CoroutineHandle(CoroutineHandle &&rhs) noexcept : _handle(std::exchange(rhs._handle, {})) {}

    CoroutineHandle &operator=(CoroutineHandle const &) = delete;
    CoroutineHandle &operator=(CoroutineHandle &&rhs) noexcept
    {
        if (this != &rhs) {
            std::swap(_handle, rhs._handle);
        }
        return *this;
    }

    // constexpr auto operator->() -> handle_type * { return &_handle; }
    constexpr auto operator->() const -> const handle_type * { return &_handle; }

private:
    handle_type _handle;
};

class PromiseBase {
public:
    PromiseBase()  = default;
    ~PromiseBase() = default;

    // exception
    constexpr void unhandled_exception() noexcept {}

    // suspend
    constexpr auto initial_suspend() noexcept { return std::suspend_always(); }
    constexpr auto final_suspend() noexcept { return std::suspend_always(); }

    PromiseBase(PromiseBase const &)        = default;
    PromiseBase(PromiseBase &&rhs) noexcept = default;

    PromiseBase &operator=(PromiseBase const &) = default;
    PromiseBase &operator=(PromiseBase &&rhs)   = default;
};

template <typename T>
class PromiseImpl : public PromiseBase {
public:
    using value_type = T;

    auto return_value(value_type value) -> void { _value = std::move(value); }
    template <typename U>
        requires(std::convertible_to<U, value_type>)
    auto return_value(U &&value) -> void
    {
        _value = std::forward<U>(value);
    }

    constexpr auto operator*() -> value_type & { return _value; }
    constexpr auto operator*() const -> const value_type & { return _value; }
    constexpr auto operator->() -> value_type * { return &_value; }
    constexpr auto operator->() const -> const value_type * { return &_value; }

private:
    value_type _value;
};

template <>
class PromiseImpl<void> : public PromiseBase {
public:
    auto return_void() const noexcept {}
    auto value() const noexcept {}
};

template <typename T>
class Promise final : public PromiseImpl<T> {
public:
    using handle_type = std::coroutine_handle<Promise>;

    auto get_return_object() -> CoroutineHandle<T>
    {
        return CoroutineHandle<T>(handle_type::from_promise(*this));
    }
};

CoroutineHandle<std::expected<int, std::error_code>> counter()
{
    std::cout << "counter: called\n";
    for (size_t i = 0; i < 4; ++i) {
        co_await std::suspend_always{};
        std::cout << "counter:: resumed (#" << i << ")\n";
    }
    co_return 114514;
    co_return std::unexpected(std::make_error_code(std::errc::io_error));
}

CoroutineHandle<void> func1()
{
    co_return;
}

int u8main([[maybe_unused]] int argc, [[maybe_unused]] const char *const *argv)
{
    std::cout << "hello world\n";
    auto counter1 = counter();
    std::cout << "main:    resuming counter\n";
    counter1->done() ? (void)0 : counter1->resume();
    counter1->done() ? (void)0 : counter1->resume();
    counter1->done() ? (void)0 : counter1->resume();
    std::cout << "here" << std::endl;
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