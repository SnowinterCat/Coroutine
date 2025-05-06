#pragma once
#include <coroutine/config.h>
// system header
// standard header
#include <coroutine>
#include <utility>
// 3rd header
// library header

AUTHOR_NAMESPACE_BEGIN
CORO_BEGIN

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

CORO_END
AUTHOR_NAMESPACE_END