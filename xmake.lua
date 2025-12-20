add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("run.autobuild", false)

target("Fig")
    set_kind("binary")
    set_languages("c++2b") 
    
    set_plat("mingw")
    --set_toolchains("clang")

    add_cxxflags("-static")
    add_cxxflags("-stdlib=libc++")

    add_files("src/*.cpp")
    add_files("src/Value/function.cpp")
    
    add_includedirs("include")
    
    set_warnings("all")

    add_defines("__FCORE_COMPILE_TIME=\"" .. os.date("%Y-%m-%d %H:%M:%S") .. "\"")