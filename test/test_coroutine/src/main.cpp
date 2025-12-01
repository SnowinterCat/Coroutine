#include <print>
#include <coroutine>

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

    CoroutineHandle(CoroutineHandle const &)        = delete;
    CoroutineHandle(CoroutineHandle &&rhs) noexcept = default;

    CoroutineHandle &operator=(CoroutineHandle const &)        = delete;
    CoroutineHandle &operator=(CoroutineHandle &&rhs) noexcept = default;

    constexpr auto operator->() const -> const handle_type * { return &_handle; }

    //
    constexpr bool await_ready() const noexcept
    {
        this->_handle.resume();
        std::println("await_ready called");
        return false;
    }
    template <typename U>
    constexpr auto await_suspend(std::coroutine_handle<Promise<U>> caller) noexcept -> void
    {
        std::println("await_suspend called, caller address: {}",
                     static_cast<void *>(caller.address()));
    }
    constexpr auto await_resume() -> value_type
    {
        std::println("await_resume called");
        // _caller.resume();
        return _handle.promise().get();
    }

protected:
    handle_type _handle;
};

template <typename T>
class Promise {
public:
    using handle_type = std::coroutine_handle<Promise>;
    using value_type  = T;

    auto get_return_object() -> CoroutineHandle<value_type>
    {
        return CoroutineHandle<value_type>(handle_type::from_promise(*this));
    }

    // exception
    constexpr void unhandled_exception() noexcept {}

    // suspend
    constexpr auto initial_suspend() noexcept { return std::suspend_always(); }
    constexpr auto final_suspend() noexcept { return std::suspend_always(); }

    // return
    auto return_value(value_type value) -> void { _value = std::move(value); }
    template <typename U>
        requires(std::convertible_to<U, value_type>)
    auto return_value(U &&value) -> void
    {
        _value = std::forward<U>(value);
    }

    constexpr auto get() -> value_type & { return _value; }
    constexpr auto get() const -> const value_type & { return _value; }

protected:
    value_type _value;
};

CoroutineHandle<std::tuple<bool, int>> coro1()
{
    std::println("hello coroutine! This is coro1");
    co_await std::suspend_always{};
    std::println("This is coro1 after first co_await");
    co_return std::make_tuple(true, 1);
}

CoroutineHandle<std::tuple<bool, int>> coro2()
{
    bool flag   = false;
    int  number = 0;

    std::println("hello coroutine! This is coro2");
    std::tie(flag, number) = co_await coro1();
    std::println("flag: {}, number: {}", flag, number);
    co_return std::make_tuple(true, 2);
}

int main()
{
    auto handle = coro2();
    std::println("before resume!");
    {
        auto [flag, number] = handle->promise().get();
        std::println("isdone: {}, flag: {}, number: {}", handle->done(), flag, number);
    }

    for (size_t i = 0; i < 3; ++i) {
        if (!handle->done()) {
            handle->resume();
            auto [flag, number] = handle->promise().get();
            std::println("isdone: {}, flag: {}, number: {}", handle->done(), flag, number);
        }
    }
    return 0;
}