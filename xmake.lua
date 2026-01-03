add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("run.autobuild", false)

target("Fig")
    set_kind("binary")
    set_languages("c++23")

    if is_plat("linux") then
        set_toolchains("clang")
        add_cxxflags("-stdlib=libc++")
        add_ldflags("-stdlib=libc++")
    elseif is_plat("windows") then
        set_toolchains("mingw")
        add_ldflags("-static")
    end
    
    add_ldflags("-Wl,--stack,268435456")


    add_files("src/main.cpp")
    add_files("src/Core/warning.cpp")
    add_files("src/Evaluator/evaluator.cpp")
    add_files("src/Lexer/lexer.cpp")
    add_files("src/Parser/parser.cpp")
    add_files("src/Value/value.cpp")

    add_includedirs("src")

    set_warnings("all")

    add_defines("__FCORE_COMPILE_TIME=\"" .. os.date("%Y-%m-%d %H:%M:%S") .. "\"")
