

option("libdir")
    add_deps("outputdir")
    set_showmenu(false)
    on_check(function (option)
        option:set_value(path.translate(
            vformat("$(outputdir)/lib/$(plat)-$(arch)-$(kind)-$(runtimes)", xmake)
        ))
    end)
option_end()

option("bindir")
    add_deps("outputdir")
    set_showmenu(false)
    on_check(function (option)
        option:set_value(path.translate(
            vformat("$(outputdir)/bin/$(plat)-$(arch)-$(kind)-$(runtimes)", xmake)
        ))
    end)
option_end()

option("testdir")
    add_deps("outputdir")
    set_showmenu(false)
    on_check(function (option)
        option:set_value(path.translate(
            vformat("$(outputdir)/test/$(plat)-$(arch)-$(kind)-$(runtimes)", xmake)
        ))
    end)
option_end()

option("plugindir")
    add_deps("outputdir")
    set_showmenu(false)
    on_check(function (option)
        option:set_value(path.translate(
            vformat("$(outputdir)/plugin/$(plat)-$(arch)-$(kind)-$(runtimes)", xmake)
        ))
    end)
option_end()

option("datadir")
    add_deps("outputdir")
    set_showmenu(false)
    on_check(function (option)
        option:set_value(path.translate(
            vformat("$(outputdir)/data", xmake)
        ))
    end)
option_end()