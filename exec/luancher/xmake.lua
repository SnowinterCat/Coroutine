target("luancher")
    set_kind("binary")
    set_targetdir("$(bindir)")

    add_deps("config")
    -- add_packages("spdlog")

    add_includedirs("include", {public = true})
    add_headerfiles("include/(**)")
    add_headerfiles("src/(**.hpp)", {install = false})
    add_files("src/**.cpp", {public = true})

    if has_config("luanch") then
        add_runenvs("LD_LIBRARY_PATH", "$(libdir)", ".")
    end
    on_load(function (target) 
        import("lua.auto", {rootdir = os.projectdir()})
        auto().library_autodefine(target)
    end)
    after_build(function (target) 
        import("lua.auto", {rootdir = os.projectdir()})
        auto().target_autoclean(target)
    end)
target_end()