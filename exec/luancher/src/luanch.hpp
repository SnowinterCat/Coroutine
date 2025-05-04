#pragma once
#include <coroutine/config.h>

#include <string>
#include <vector>
#include <string_view>

/**
 * @brief
 *
 * @param baseName
 * @param argc
 * @param argv
 * @return int
 */
int luanch_main(const char *baseName, int argc, const char *const *argv);

#if defined(_WIN32)
/**
 * @brief
 *
 * @param argc
 * @param wargv
 * @param args
 * @param argvector
 * @return const char* const*
 */
auto wargv_convert(int argc, const wchar_t *const *wargv, std::string &args,
                   std::vector<const char *> &argvector) -> const char *const *;
#else
/**
 * @brief
 *
 * @param argc
 * @param argv
 * @param name
 * @param value
 * @return true/false
 */
bool check_add_environ_and_restart(int argc, const char *const *argv, const char *name,
                                   std::string_view value);
#endif
