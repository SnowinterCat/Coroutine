target("config")
    set_kind("headeronly")

    add_includedirs("include", {public = true})
    add_headerfiles("include/(**)")
    -- add_headerfiles("src/(**.hpp)", {install = false})
    -- add_files("src/**.cpp")

    set_configdir("include/coroutine")
    if has_config("luanch") then
        set_configvar("CORO_LUANCH", get_config("luanch"))
    end
    add_configfiles("config.h.in", {public = true})
target_end()