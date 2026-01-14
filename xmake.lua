add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("run.autobuild", false)

target("Fig")
    set_kind("binary")
    set_languages("c++23")

    add_ldflags("-static", {force = true})
    if is_plat("linux") then
        -- Linux: clang + libc++
        set_toolchains("clang")
        add_cxxflags("-stdlib=libc++")
        add_ldflags("-stdlib=libc++")
    elseif is_plat("windows") then
        -- 1. CI cross (Linux -> Windows)
        -- 2. local dev (Windows + llvm-mingw)
        set_toolchains("mingw") -- llvm-mingw
        add_ldflags("-Wl,--stack,268435456")
        -- set_toolchains("clang")
        -- static lib
        -- add_ldflags("-target x86_64-w64-mingw32", "-static")
        -- add_cxxflags("-stdlib=libc++")
        -- add_ldflags("-stdlib=libc++")
    end
    

    add_files("src/Evaluator/main.cpp")
    add_files("src/Core/warning.cpp")
    add_files("src/Evaluator/evaluator.cpp")
    add_files("src/Evaluator/Value/value.cpp")
    add_files("src/Lexer/lexer.cpp")
    add_files("src/Parser/parser.cpp")

    add_includedirs("src")
    add_includedirs("src/Evaluator")

    set_warnings("all")

    add_defines("__FCORE_COMPILE_TIME=\"" .. os.date("%Y-%m-%d %H:%M:%S") .. "\"")

-- -- Bytecode VM target
-- target("Figc")
--     set_kind("binary")
--     set_languages("c++23")

--     add_ldflags("-static", {force = true})
--     if is_plat("linux") then
--         -- Linux: clang + libc++
--         set_toolchains("clang")
--         add_cxxflags("-stdlib=libc++")
--         add_ldflags("-stdlib=libc++")
--     elseif is_plat("windows") then
--         -- 1. CI cross (Linux -> Windows)
--         -- 2. local dev (Windows + llvm-mingw)
--         set_toolchains("mingw") -- llvm-mingw
--         add_ldflags("-Wl,--stack,268435456")
--         -- set_toolchains("clang")
--         -- static lib
--         -- add_ldflags("-target x86_64-w64-mingw32", "-static")
--         -- add_cxxflags("-stdlib=libc++")
--         -- add_ldflags("-stdlib=libc++")
--     end
    
--     add_includedirs("src/Evaluator")
--     add_files("src/Evaluator/Value/value.cpp")
--     add_files("src/VirtualMachine/main.cpp")
--     add_files("src/Core/warning.cpp")
--     add_files("src/Lexer/lexer.cpp")
--     add_files("src/Parser/parser.cpp")

--     add_includedirs("src")
--     set_warnings("all")

--     add_defines("__FCORE_COMPILE_TIME=\"" .. os.date("%Y-%m-%d %H:%M:%S") .. "\"")
