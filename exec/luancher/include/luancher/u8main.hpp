#pragma once
#include <coroutine/config.h>

#if defined(_WIN32)
    #define CORO_EXPORT_API __declspec(dllexport)
#endif

#if !defined(CORO_EXPORT_API)
    #if defined(__GNUC__) && (__GNUC__ >= 4)
        #define CORO_EXPORT_API __attribute__((visibility("default")))
    #else
        #define CORO_EXPORT_API
    #endif
#endif

extern "C" CORO_EXPORT_API int u8main(int argc, const char *const *argv);
