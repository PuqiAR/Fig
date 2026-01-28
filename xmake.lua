add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("run.autobuild", false)

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
    


add_files("src/Core/warning.cpp")
add_files("src/Core/runtimeTime.cpp")

add_files("src/Lexer/lexer.cpp")
add_files("src/Parser/parser.cpp")

add_files("src/Module/builtins.cpp")

add_files("src/Evaluator/Value/value.cpp")
add_includedirs("src")

add_defines("__FCORE_COMPILE_TIME=\"" .. os.date("%Y-%m-%d %H:%M:%S") .. "\"")

target("Fig")
    set_kind("binary")

    add_files("src/Evaluator/Core/*.cpp")
    add_files("src/VirtualMachine/VirtualMachine.cpp")
    add_files("src/Evaluator/evaluator.cpp")
    add_files("src/Repl/Repl.cpp")
    add_files("src/main.cpp")
    
    set_warnings("all")

target("vm_test_main")
    set_kind("binary")

    add_files("src/VirtualMachine/VirtualMachine.cpp")
    add_files("src/Bytecode/vm_test_main.cpp")
    
    set_warnings("all")

target("ir_test_main")
    set_kind("binary")

    add_files("src/IR/ir_test_main.cpp")
    
    set_warnings("all")