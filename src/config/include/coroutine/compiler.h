#pragma once
#include <version>

// C++ library
#ifdef _MSVC_STL_VERSION
    // msvc stl
    #define SWC_CORO_STL_MSVC
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS
    #endif
    #ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    #endif
#elif defined(_LIBCPP_VERSION)
    // libc++
    #define SWC_CORO_STL_LIBCXX
#elif defined(__GLIBCXX__)
    // libstdc++
    #define SWC_CORO_STL_STDCXX
#else
    #error "not support other stl"
#endif