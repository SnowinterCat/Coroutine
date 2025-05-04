#include <coroutine/config.h>
#include "luanch.hpp"
// 系统库
#if defined(_WIN32)
    #include <Windows.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <sys/wait.h>
#endif
// 标准库
#include <memory>
#include <filesystem>
#include <format>
#include <iostream>
// 三方库

// 项目内依赖

// 当前目录

////////////////////////////////////////////////////////////////////////////////
// declaration
////////////////////////////////////////////////////////////////////////////////

struct LibraryDeleter {
#if defined(_WIN32)
    void operator()(HMODULE library)
    {
        if (library) {
            FreeLibrary(library);
        }
    }
#else
    void operator()(void *library)
    {
        if (library) {
            dlclose(library);
        }
    }
#endif
};

/**
 * @brief 根据基础名查找动态库路径
 *
 * @param baseName
 * @param dir
 * @param errc
 * @return std::filesystem::path
 */
auto find_dynamic_library_path(const char *baseName, std::filesystem::path dir,
                               std::error_code &errc) -> std::filesystem::path;

#if defined(_WIN32)
#else
bool check_environment_value(const char *name, std::string_view value);
bool add_environment_value(const char *name, std::string_view value);
#endif

////////////////////////////////////////////////////////////////////////////////
// definition (header)
////////////////////////////////////////////////////////////////////////////////

int luanch_main(const char *baseName, int argc, const char *const *argv)
{
    typedef int (*U8MAIN)(int argc, const char *const *argv);
    U8MAIN u8Main = nullptr;
    auto   errc   = std::error_code();

    auto execDir = std::filesystem::weakly_canonical(argv[0]).parent_path();
    auto dllPath = find_dynamic_library_path(baseName, execDir, errc);
    if (errc) {
        std::cout << std::format("find_dynamic_library_path error, code: {}, info", errc.value(),
                                 errc.message())
                  << std::endl;
        return errc.value();
    }

    auto oldWorkPath = std::filesystem::current_path();
    auto newWorkPath = execDir.parent_path().parent_path() / "lib" /
                       std::filesystem::relative(execDir, execDir.parent_path());
    if (std::filesystem::current_path(newWorkPath, errc); errc) {
        std::cout << std::format("std::filesystem::current_path error, code: {}, info: {}",
                                 errc.value(), errc.message())
                  << std::endl;
        return errc.value();
    }

#if defined(_WIN32)
    auto lib = std::unique_ptr<std::remove_pointer_t<HMODULE>, LibraryDeleter>(
        LoadLibraryW(dllPath.c_str()));
    if (lib == nullptr) {
        std::cout << std::format("LoadLibraryW error, code: {}", GetLastError()) << std::endl;
        return GetLastError();
    }
    u8Main =
        reinterpret_cast<U8MAIN>(reinterpret_cast<size_t>(GetProcAddress(lib.get(), "u8main")));
    if (u8Main == nullptr) {
        std::cout << std::format("GetProcAddress error, code: {}", GetLastError()) << std::endl;
        return GetLastError();
    }
#else
    auto lib = std::unique_ptr<void, LibraryDeleter>(dlopen(dllPath.c_str(), RTLD_LAZY));
    if (lib == nullptr) {
        std::cout << std::format("dlopen error, info: {}", dlerror()) << std::endl;
        return -1;
    }
    u8Main = reinterpret_cast<U8MAIN>(dlsym(lib.get(), "u8main"));
    if (u8Main == nullptr) {
        std::cout << std::format("dlsym error, info: {}", dlerror()) << std::endl;
        return -1;
    }
#endif

    std::filesystem::current_path(execDir, errc);
    int result = u8Main(argc, argv);
    std::filesystem::current_path(oldWorkPath, errc);
    return result;
}

#if defined(_WIN32)
auto wargv_convert(int argc, const wchar_t *const *wargv, std::string &args,
                   std::vector<const char *> &argvector) -> const char *const *
{
    int len = 0;
    for (int i = 0; i < argc; ++i) {
        len += WideCharToMultiByte(CP_UTF8, NULL, wargv[i], -1, NULL, NULL, NULL, NULL);
    }
    args.resize(len);

    argvector.resize(argc);
    for (int i = 0, cur = 0; i < argc; ++i) {
        argvector[i] = &args[cur];
        cur += WideCharToMultiByte(CP_UTF8, NULL, wargv[i], -1, &args[cur], len - cur, NULL, NULL);
    }
    return argvector.data();
}
#else
bool check_add_environ_and_restart([[maybe_unused]] int argc, const char *const *argv,
                                   const char *name, std::string_view value)
{
    if (!check_environment_value(name, value)) {
        add_environment_value(name, value);
        int pid = vfork(), stat;
        if (pid) {
            return wait(&stat) != pid;
        }
        int errc = execve(argv[0], const_cast<char *const *>(argv), environ);
        std::cout << std::format("启动失败, code: {}", errc) << std::endl;
    }
    return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// definition (source)
////////////////////////////////////////////////////////////////////////////////

auto find_dynamic_library_path(const char *baseName, std::filesystem::path dir,
                               std::error_code &errc) -> std::filesystem::path
{
#if defined(_WIN32)
    auto fileName = std::string(baseName) + ".dll";
#elif defined(__linux__)
    auto fileName = std::string("lib") + baseName + ".so";
#elif defined(__APPLE__)
    auto fileName = std::string("lib") + baseName + ".dylib";
#else
    #error "not support system"
#endif
    if (std::filesystem::is_regular_file(dir / fileName, errc)) {
        return dir / fileName;
    }

    auto traitPath = std::filesystem::relative(dir, dir.parent_path());
    for (const auto &entry : std::filesystem::directory_iterator(dir.parent_path().parent_path())) {
        if (entry.is_directory() &&
            std::filesystem::is_regular_file(entry.path() / traitPath / fileName, errc)) {
            return entry.path() / traitPath / fileName;
        }
    }
    return {};
}

#if defined(_WIN32)
#else
bool check_environment_value(const char *name, std::string_view value)
{
    auto env = getenv(name);
    if (env) {
        auto strenv   = std::string(":").append(env).append(":");
        auto strvalue = std::string(":").append(value).append(":");
        return strenv.find(strvalue) != std::string::npos;
    }
    return false;
}

bool add_environment_value(const char *name, std::string_view value)
{
    auto env = getenv(name);
    if (env) {
        return setenv(name, std::string(env).append(":").append(value).c_str(), 1) == 0;
    }
    return setenv(name, value.data(), 1) == 0;
}
#endif
