# Fig Language - A Modern Scripting Language

[简体中文](README_ZH-CN.md "Chinese version")

**Fig** is a statically-typed, expression-oriented programming language designed for clarity, safety, and modern development practices. With features inspired by Go, Rust, and JavaScript, Fig aims to provide a productive development experience while maintaining strong type safety.

## Features

### 🚀 Core Language Features
- **Static typing with type inference** - Strong typing with minimal annotations
- **Modern control flow** - Full `for` loop support with proper scoping
- **First-class functions** - Lambda expressions and closures
- **Rich data structures** - Structs, lists, maps, and tuples
- **Memory safety** - No null pointer exceptions, automatic memory management

### 🔧 Technical Highlights
- **Three-level scoping for loops** - Proper variable isolation in iterations
- **Smart semicolon handling** - Flexible statement termination with RAII guards
- **Comprehensive error handling** - Detailed error messages with source locations
- **Clean C++ implementation** - Modern C++23 with RAII and smart pointers

### 📁 Project Structure
    Fig/
        ├── src/
        │ ├── lexer.cpp             # Lexical analysis
        │ ├── parser.cpp            # Syntax analysis
        │ ├── evaluator.cpp         # Interpreter/execution engine
        │ └── value.cpp             # Type system implementation        
        ├── include/
        │ ├── argparse
        │ ├── magic_enum 
        │ ├── Ast                   # Abstract syntax tree definitions
        │ ├── value.hpp             # Type system header
        │ ├── Value/                # Type system definitions
        │       ├── ...
        │ ├── AstPrinter.hpp        # Ast printer
        │ ├── context(_forward).hpp # Environment/Context system
        │ ├── core.hpp              # Core informations
        │ ├── error.hpp             # Exception system
        │ ├── errorLog.hpp          # Colored-Print error log
        │ ├── fig_string.hpp        # Fig UTF-8 string
        │ ├── module.hpp            # Package/Module system
        │ ├── utils.hpp
        │ ├── token.hpp             # Token definitions
        │ ├── lexer.hpp             # Lexical analysis
        │ ├── parser.hpp            # Syntax analysis and AST
        │ ├── evaluator.hpp         # Interpreter and Control flows
        │ ├── warning.hpp           # STD-Warnings
        ├── ExampleCodes/           # Sample programs
        ├── .clang-format           # Clang format styles
        ├── test.fig                # Test file (dev)
        ├── LICENSE                 # Fig project license
        ├── xmake.lua               # XMake configurations
        └── Logo/                   # Fig's Logo

## Language Philosophy
    Fig is designed around several core principles:

    Clarity over cleverness - Code should be readable first

    Safety by default - Prevent common errors at compile time

    Modern ergonomics - Developer experience matters

    Gradual learning - Simple to start, powerful when needed