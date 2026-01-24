# Fig Language - A Modern Scripting Language

[Fig-Gitea](https://git.fig-lang.cn/PuqiAR/Fig)
Recommend view on Gitea Endpoint

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

### 🔧 Install
# Installation

## Prerequisites
**Xmake** version **3.0.0 or higher** must be installed on your system.

## Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/PuqiAR/Fig.git
```

2. Navigate to the project directory:

```bash
cd Fig
```

3. Build the project:

```bash
xmake build Fig
```

4. Run the program:

```bash
xmake run Fig [file]
```

Replace `[file]` with the path to your input file.

## Language Philosophy
    Fig is designed around several core principles:

    Clarity over cleverness - Code should be readable first

    Safety by default - Prevent common errors at compile time

    Modern ergonomics - Developer experience matters

    Gradual learning - Simple to start, powerful when needed

## Performance Summary

**Version:** 0.4.2-alpha (Tree-walker Interpreter)  
**Test Hardware:** i5-13490F, Windows 11

**Execution Times for Fibonacci(30):**
- Naive Recursion: **11.72s**
- Memoization: **0.93ms** (12,600× faster)
- Iteration: **0.37ms** (31,300× faster)  
- Tail Recursion: **0.40ms** (29,200× faster)

**Visual Comparison:**
```
Naive Recursion  : ████████████████████████████████████████ 11.72s
Memoization      : ▉ 0.93ms
Iteration        : ▍ 0.37ms
Tail Recursion   : ▎ 0.40ms
```

**Key Insight:** Algorithm choice dominates performance in this tree-walker implementation.

**Detailed Reports:** [English](./docs/benchmark_result/benchmark_result_en_0.4.2-alpha.md) | [中文](./docs/benchmark_result/benchmark_result_zh_0.4.2-alpha.md)


## Language Documents

see ./docs/en_US/...

We're looking for translators to help translate our project and make it accessible to more language communities.