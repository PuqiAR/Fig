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
.
├── ExampleCodes                # Example programs & performance tests
│   └── SpeedTest               # Performance benchmark samples
├── LICENSE                     # Project license
├── Logo                        # Project logo assets
├── README.md                   # English README
├── README_ZH-CN.md             # Chinese README
├── compile_flags.txt           # Compiler flags helper
├── fig-vscode                  # VSCode extension project
│   ├── node_modules            # Extension dependencies
│   ├── out                     # Built extension output
│   ├── src                     # Extension source code
│   └── syntaxes                # Syntax highlighting definition
├── src                         # Core Fig language source
│   ├── Ast                     # AST definitions
│   ├── Context                 # Runtime context
│   ├── Core                    # Core utilities (UTF8/string/etc.)
│   ├── Error                   # Error handling system
│   ├── Evaluator               # Interpreter / evaluator
│   ├── Lexer                   # Lexical analyzer
│   ├── Module                  # Modules and builtins
│   ├── Parser                  # Parser
│   ├── Token                   # Token definitions
│   ├── Utils                   # Utilities & helper headers
│   └── Value                   # Runtime type/value system
├── test.fig                    # Test script
└── xmake.lua                   # Xmake build config

## Language Philosophy
    Fig is designed around several core principles:

    Clarity over cleverness - Code should be readable first

    Safety by default - Prevent common errors at compile time

    Modern ergonomics - Developer experience matters

    Gradual learning - Simple to start, powerful when needed