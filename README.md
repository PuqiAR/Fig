# Fig

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./Logo/LogoDark.svg">
  <img src="./Logo/Logo.svg" alt="Fig Logo" width="200">
</picture>

> **🔔 Main Repository: https://git.fig-lang.cn/PuqiAR/Fig**  
> *GitHub is only a mirror. Please submit issues and PRs to the main repository.*

[English](./README.md) | [中文](./README_zh-CN.md)

![License](https://img.shields.io/badge/license-MIT-blue)
![Status](https://img.shields.io/badge/status-0.5.0--alpha-yellow)
![C++](https://img.shields.io/badge/C++-99.5%25-blue)
![xmake](https://img.shields.io/badge/xmake-build-green)
![LLVM](https://img.shields.io/badge/LLVM-clang%2021.1.8-purple)
![Platform](https://img.shields.io/badge/Windows%20·%20Linux%20·%20macOS-lightgrey)
![Performance](https://img.shields.io/badge/fib(30)-~28ms%20(i5--13490f)-brightgreen)

**Fig** is a programming language that blends dynamic typing with optional static type annotations, built on reference-based value semantics. It offers the flexibility of scripting languages while providing optional type constraints for more robust code.

> ⚠️ **Fig is currently at version 0.5.0-alpha, and its syntax and APIs are subject to change.** Feel free to try it out and provide feedback, but do not use it in production environments.

```fig
// Get a feel for Fig
var flexible = 10       // dynamic type, can be any value
flexible = "hello"      // works

var fixed := 20         // fixed to Int type
// fixed = 3.14         // error! type mismatch

func greet(name) => "Hello, " + name + "!"
println(greet("Fig"))   // output: Hello, Fig!
```

## ✨ Key Features

### 🎭 Dynamic & Static Blending
Variables are dynamic by default (`Any` type), but you can lock their type using `:=` or `: Type` for static safety.

```fig
var any;                // type is Any
any = 42;               // now points to an Int
any = [1,2,3];          // now points to a List

var safe := 100;        // inferred as Int, can only be assigned Int values
safe = 200;             // ✅
// safe = "oops"        // ❌ compile error
```

### 🔗 Reference-Based Value Semantics
All objects are immutable; variables are just "names" that refer to objects. Multiple variables can share the same object, and modifications to complex types (like lists) are visible to all references—this is the essence of references.

```fig
var a := [1, 2, 3];
var b := a;             // b and a refer to the same list
a[0] = 99;              // modify the list content
println(b)              // output: [99, 2, 3] — b sees the change

// Need a true copy? Use deep copy (syntax may change)
var c := new List{a};    // deep copy of a
c[0] = 0;
println(a)              // still [99, 2, 3]
```

### 🏗 Object-Oriented Features (OOP)
Fig provides lightweight OOP support based on structs, with polymorphism via interfaces.

- **Structs as classes**: defined with `struct`, can have fields and methods.
- **Access control**: `public` keyword; fields are private by default.
- **Concise construction**:
  ```fig
  var p1 := new Point{1, 2};           // positional
  var p2 := new Point{x: 2, y: 3};     // named
  var x := 5, y := 10;
  var p3 := new Point{y, x};           // shorthand, auto field matching
  ```
- **Interfaces and implementations**:
  ```fig
  interface Drawable { draw() -> String; }
  struct Circle { radius: Double }
  impl Drawable for Circle {
      draw() => "⚪ radius: " + radius;
  }
  ```
- **No inheritance, polymorphism via interfaces** (currently implicit `this`, may change to explicit `self.xxx` in the future).

### 🧩 Functional Features
Functions are first-class citizens, with support for closures and concise arrow syntax.

```fig
func multiplier(factor) {
    return func (n) => n * factor;   // closure (upvalue)
}

var double = multiplier(2);
println(double(5))      // output: 10
```

### ⚡ High Performance
**Recursive fib(30) takes only 28ms** (i5-13490f, Windows 11, DDR4 2667, clang 21.1.8, libc++, C++23) — excellent performance among dynamic languages.

Fig achieves high performance through a series of low-level optimizations:
- **Register-based VM**: replaces the tree-walk interpreter, dramatically improving execution speed.
- **FastCall optimization**: global ordinary functions can be called directly as prototypes, avoiding runtime unwrapping overhead (traditional calls require checking if an object is a function and unwrapping it).
- **Window Slicing** (originated from Lua): a stack sliding window technique that efficiently manages function calls and closures (upvalues), reducing memory allocations.
- **Upvalue mechanism**: lightweight closure implementation, making functional programming performant.
- **Computed Goto**: leverages GCC/Clang extensions to speed up bytecode dispatch.
- **NaN-Boxing**: efficient storage and type tagging to boost dynamic typing performance.

#### Performance Comparison
Rough comparison of recursive fib(30) execution times (environment may vary, for reference only):

| Language Type | Language       | Time (ms) | Notes |
|---------------|----------------|-----------|-------|
| AOT compiled  | C (clang -O2)  | ~0.05     | Nanoseconds, as a speed reference |
| AOT compiled  | Rust (--release) | ~0.06   | Same as above |
| **Dynamic**   | **Fig**        | **~28**   | **Register VM + optimizations** |
| Dynamic       | Lua 5.4        | ~35       | Classic dynamic language |
| Dynamic       | Python 3.11    | ~450      | CPython |
| Dynamic       | Node.js 20     | ~60       | V8 engine |
| Dynamic       | Ruby 3.2       | ~800      | CRuby |

*Fig performs admirably among dynamic languages, approaching Lua and Node.js, and significantly outperforming Python and Ruby.*

## 🚀 Quick Start

### Installation

#### Option 1: Download Pre-built Binaries
Download the binary for your platform from the [main repository Releases](https://git.fig-lang.cn/PuqiAR/Fig/releases) or the [GitHub mirror](https://github.com/PuqiAR/Fig/releases), extract it, and add it to your PATH.

#### Option 2: Build from Source with xmake
```bash
# Clone the main repository (GitHub is a mirror)
git clone https://git.fig-lang.cn/PuqiAR/Fig.git
cd Fig

# Install xmake if you haven't: https://xmake.io

# Build (must use clang because computed goto requires compiler support)
xmake f --toolchain=clang
xmake

# After building, the binary is in the build/ directory
```

### Run Your First Fig Program
Create a file `hello.fig`:
```fig
import std.io;   // io module must be imported; println and others reside here
println("Hello, Fig!");
```
Run it:
```bash
./build/fig hello.fig
```
Or if you added `fig` to your PATH:
```bash
fig hello.fig
```

## 📊 Current Status & Roadmap

| Status | Module                     | Description |
|--------|----------------------------|-------------|
| ✅     | Frontend                   | Lexer, parser, AST, semantic analysis |
| ✅     | Core semantics             | Variables, functions, closures, structs, interfaces, type system |
| ✅     | Built-in types             | 13 built-in types (Any, Null, Int, String, Bool, Double, Function, StructType, StructInstance, List, Map, Module, InterfaceType) |
| 🚧     | Register VM                | Original tree-walk interpreter deprecated; VM in development |
| 🚧     | Garbage Collection         | Basic GC implemented, optimizing |
| 📝     | Standard Library           | IO preliminarily available; more libraries to come |
| 📝     | FFI                        | Foreign function interface (C ABI) |
| 📝     | LSP                        | Language Server Protocol support |

## 📚 Documentation & Community

- **Documentation**: Check the [`/docs`](./docs) directory (continuously updated)
- **Example Code**: [`/ExampleCodes`](./ExampleCodes) contains various usage demonstrations
- **Contributing**: Issues and PRs are welcome at the main repository
  - Report bugs
  - Discuss features
  - Improve documentation
- **License**: MIT © PuqiAR
- **Author**: PuqiAR · [im@puqiar.top](mailto:im@puqiar.top)
- **Links**:
  - [Main Repository](https://git.fig-lang.cn/PuqiAR/Fig)
  - [GitHub Mirror](https://github.com/PuqiAR/Fig)

---

**Fig** is evolving rapidly. If you're intrigued, give it a try or join us in shaping its future!