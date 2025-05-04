#include <coroutine/config.h>

#include <filesystem>

#include "luanch.hpp"

#if defined(_WIN32)
int wmain(int argc, const wchar_t *const *wargv)
#else
int main(int argc, const char *const *argv)
#endif
{
    std::setlocale(LC_ALL, ".UTF-8");
#if defined(_WIN32)
    auto        args      = std::string();
    auto        argvector = std::vector<const char *>();
    const auto *argv      = wargv_convert(argc, wargv, args, argvector);
#else
    if (!check_add_environ_and_restart(argc, argv, "LD_LIBRARY_PATH", ".")) {
        return 0;
    }
#endif
#if defined(CORO_LUANCH)
    return luanch_main(CORO_LUANCH, argc, argv);
#else
    return luanch_main(std::filesystem::path(argv[0]).stem().string().c_str(), argc, argv);
#endif
}
