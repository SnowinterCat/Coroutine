#pragma once

// define SWC_CORO_FORCE_EXPORT_API
#if defined(_WIN32)
    #define SWC_CORO_FORCE_EXPORT_API __declspec(dllexport)
#endif
#if !defined(SWC_CORO_FORCE_EXPORT_API)
    #if defined(__GNUC__) && (__GNUC__ >= 4)
        #define SWC_CORO_FORCE_EXPORT_API __attribute__((visibility("default")))
    #else
        #define SWC_CORO_FORCE_EXPORT_API
    #endif
#endif