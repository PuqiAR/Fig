add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("run.autobuild", false)

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

set_languages("c++23")
add_includedirs("src")

add_defines("__FCORE_COMPILE_TIME=\"" .. os.date("%Y-%m-%d %H:%M:%S") .. "\"")

target("StringTest")
    add_files("src/Deps/String/StringTest.cpp")
    
target("LexerTest")
    add_files("src/Core/*.cpp")
    add_files("src/Token/Token.cpp")
    add_files("src/Error/Error.cpp")
    add_files("src/Lexer/Lexer.cpp")

    add_files("src/Lexer/LexerTest.cpp")

target("ParserTest")
    add_files("src/Core/*.cpp")
    add_files("src/Token/Token.cpp")
    add_files("src/Error/Error.cpp")
    add_files("src/Lexer/Lexer.cpp")

    add_files("src/Ast/Operator.cpp")
    add_files("src/Parser/ExprParser.cpp")
    add_files("src/Parser/StmtParser.cpp")
    add_files("src/Parser/Parser.cpp")

    add_files("src/Parser/ParserTest.cpp")

target("ObjectTest")
    add_files("src/Object/Object.cpp")
    add_files("src/Object/ObjectTest.cpp")

target("CompilerTest")
    add_files("src/Core/*.cpp")
    add_files("src/Token/Token.cpp")
    add_files("src/Error/Error.cpp")
    add_files("src/Lexer/Lexer.cpp")

    add_files("src/Ast/Operator.cpp")
    add_files("src/Parser/ExprParser.cpp")
    add_files("src/Parser/StmtParser.cpp")
    add_files("src/Parser/Parser.cpp")

    add_files("src/Object/Object.cpp")

    add_files("src/Compiler/ExprCompiler.cpp")
    add_files("src/Compiler/StmtCompiler.cpp")
    add_files("src/Compiler/Compiler.cpp")
    
    add_files("src/Compiler/CompileTest.cpp")

target("Fig")
    add_files("src/Core/*.cpp")
    add_files("src/Token/Token.cpp")
    add_files("src/Error/Error.cpp")
    add_files("src/Lexer/Lexer.cpp")

    add_files("src/Ast/Operator.cpp")
    add_files("src/Parser/ExprParser.cpp")
    add_files("src/Parser/StmtParser.cpp")
    add_files("src/Parser/Parser.cpp")

    add_files("src/Object/Object.cpp")

    add_files("src/Compiler/ExprCompiler.cpp")
    add_files("src/Compiler/StmtCompiler.cpp")
    add_files("src/Compiler/Compiler.cpp")

    add_files("src/VM/VM.cpp")
    add_files("src/main.cpp")