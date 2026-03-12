# Fig

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./Logo/LogoDark.svg">
  <img src="./Logo/Logo.svg" alt="Fig Logo" width="200">
</picture>

> **🔔 主仓库：https://git.fig-lang.cn/PuqiAR/Fig**  
> *GitHub 仅为镜像，Issue 与 PR 请提交至主仓库*

[English](./README.md) | [中文](./README_zh-CN.md)

![License](https://img.shields.io/badge/license-MIT-blue)
![Status](https://img.shields.io/badge/status-0.5.0--alpha-yellow)
![C++](https://img.shields.io/badge/C++-99.5%25-blue)
![xmake](https://img.shields.io/badge/xmake-构建-green)
![LLVM](https://img.shields.io/badge/LLVM-clang%2021.1.8-purple)
![Platform](https://img.shields.io/badge/Windows%20·%20Linux%20·%20macOS-lightgrey)
![Performance](https://img.shields.io/badge/fib(30)-~28ms%20(i5--13490f)-brightgreen)

**Fig** 是一门动态类型与静态类型注解混合的编程语言，采用基于引用的值语义。它既保留了脚本语言的灵活性，又提供了可选的类型约束，让代码更健壮。

> ⚠️ **Fig 当前为 0.5.0-alpha 版本，语法和 API 仍可能发生变动。** 欢迎试用和反馈，但请勿用于生产环境。

```fig
// 试试 Fig 的感觉
var flexible = 10       // 动态类型，可以是任意值
flexible = "hello"      // 没问题

var fixed := 20         // 固定为 Int 类型
// fixed = 3.14         // 错误！类型不匹配

func greet(name) => "Hello, " + name + "!"
println(greet("Fig"))   // 输出: Hello, Fig!
```

## ✨ 核心特性

### 🎭 动态与静态的融合
变量默认是动态的（`Any` 类型），但你可以随时用 `:=` 或 `: Type` 锁定类型，享受静态检查的安全感。

```fig
var any;                // 类型为 Any
any = 42;               // 现在指向 Int
any = [1,2,3];          // 现在指向 List

var safe := 100;        // 推断为 Int，之后只能赋 Int 值
safe = 200;             // ✅
// safe = "oops"        // ❌ 编译错误
```

### 🔗 基于引用的值语义
所有对象都是不可变的，变量只是指向对象的“名字”。多个变量可以共享同一个对象，修改操作会创建新对象，但复杂类型（如列表）的修改会反映在所有引用上——这正是引用的含义。

```fig
var a := [1, 2, 3];
var b := a;             // b 和 a 指向同一个列表
a[0] = 99;              // 修改列表内容
println(b)              // 输出: [99, 2, 3]  —— 因为 b 也看到了变化

// 需要真正的副本？用深拷贝 (语法可能变动)
var c := new List{a};    // 深拷贝 a
c[0] = 0;
println(a)              // 仍是 [99, 2, 3]
```

### 🏗 面向对象特性 (OOP)
Fig 提供基于结构体的轻量面向对象支持，通过接口实现多态。

- **结构体即类**：用 `struct` 定义，可包含字段和方法
- **访问控制**：`public` 关键字，默认为私有
- **简洁构造**：
  ```fig
  var p1 := new Point{1, 2};           // 位置参数
  var p2 := new Point{x: 2, y: 3};     // 命名参数
  var x := 5, y := 10;
  var p3 := new Point{y, x};           // 变量名简写，自动匹配字段
  ```
- **接口与实现**：
  ```fig
  interface Drawable { draw() -> String; }
  struct Circle { radius: Double }
  impl Drawable for Circle {
      draw() => "⚪ 半径: " + radius;
  }
  ```
- **无继承，用接口实现多态**（当前为隐式 `this`，未来可能改为显式 `self.xxx`）

### 🧩 函数式特性
函数是一等公民，支持闭包和简洁的箭头语法。

```fig
func multiplier(factor) {
    return func (n) => n * factor;   // 闭包（upvalue）
}

var double = multiplier(2);
println(double(5))      // 输出: 10
```

### ⚡ 高性能实现
**递归 fib(30) 仅需 28ms** (i5-13490f, Windows 11, DDR4 2667, clang 21.1.8, libc++, C++23) —— 在动态语言中表现优异。

Fig 通过一系列底层优化实现高性能：
- **寄存器虚拟机**：替代树遍历解释器，执行效率大幅提升。
- **FastCall 优化**：全局普通函数直接调用原型（proto），避免运行时解包开销（传统调用需检查是否为函数对象并解包）。
- **Window Slicing**（源自 Lua）：栈滑动窗口技术，高效管理函数调用和闭包（upvalue），减少内存分配。
- **Upvalue 机制**：轻量闭包实现，让函数式编程无性能负担。
- **Computed Goto**：利用 GCC/Clang 扩展，加速字节码分发。
- **NaN-Boxing**：高效存储和类型判断，提升动态类型性能。

#### 性能对比
以下为递归计算 fib(30) 的粗略对比（不同环境可能存在差异，仅供参考）：

| 语言类型 | 语言 | 时间 (ms) | 备注 |
|--------|------|----------|------|
| AOT 编译 | C (clang -O2) | ~0.05 | 纳秒级，作为速度参照 |
| AOT 编译 | Rust (--release) | ~0.06 | 同上 |
| **动态语言** | **Fig** | **~28** | **寄存器 VM + 上述优化** |
| 动态语言 | Lua 5.4 | ~35 | 经典的动态语言 |
| 动态语言 | Python 3.11 | ~450 | CPython |
| 动态语言 | Node.js 20 | ~60 | V8 引擎 |
| 动态语言 | Ruby 3.2 | ~800 | CRuby |

*Fig 在动态语言阵营中表现出色，接近 Lua 和 Node.js 的水平，远超 Python 和 Ruby。*

## 🚀 快速上手

### 安装

#### 方法一：下载预编译二进制
从 [主仓库 Releases](https://git.fig-lang.cn/PuqiAR/Fig/releases) 或 [GitHub 镜像](https://github.com/PuqiAR/Fig/releases) 下载对应平台的二进制，解压后加入 PATH。

#### 方法二：使用 xmake 从源码编译
```bash
# 克隆主仓库（GitHub 为镜像）
git clone https://git.fig-lang.cn/PuqiAR/Fig.git
cd Fig

# 安装 xmake（如未安装）：https://xmake.io

# 编译（必须用 clang，因为 computed goto 需要编译器支持）
xmake f --toolchain=clang
xmake

# 编译完成后，二进制位于 build/ 目录下
```

### 运行第一个 Fig 程序
创建文件 `hello.fig`：
```fig
import std.io;   // io 模块必须导入，println 等函数位于其中
println("Hello, Fig!");
```
运行：
```bash
./build/fig hello.fig
```
或已加入 PATH：
```bash
fig hello.fig
```

## 📊 当前状态与路线图

| 状态 | 模块 | 说明 |
|------|------|------|
| ✅ | 前端 | 词法/语法分析、AST、语义分析 |
| ✅ | 核心语义 | 变量、函数、闭包、结构体、接口、类型系统 |
| ✅ | 内置类型 | 13 种内置类型（Any, Null, Int, String, Bool, Double, Function, StructType, StructInstance, List, Map, Module, InterfaceType） |
| 🚧 | 寄存器虚拟机 | 原树遍历解释器已废弃，VM 开发中 |
| 🚧 | 垃圾回收 | 基础 GC 已实现，正在优化 |
| 📝 | 标准库 | IO 已初步可用，更多库待完善 |
| 📝 | FFI | 外部函数接口（C ABI） |
| 📝 | LSP | 语言服务器协议支持 |

## 📚 文档与社区

- **文档**：查看 [`/docs`](./docs) 目录（持续更新）
- **示例代码**：[`/ExampleCodes`](./ExampleCodes) 包含各种用法演示
- **贡献**：欢迎提交 Issue 或 PR（主仓库）
  - 报告 bug
  - 讨论特性
  - 改进文档
- **许可证**：MIT © PuqiAR
- **作者**：PuqiAR · [im@puqiar.top](mailto:im@puqiar.top)
- **相关链接**：
  - [主仓库](https://git.fig-lang.cn/PuqiAR/Fig)
  - [GitHub 镜像](https://github.com/PuqiAR/Fig)

---

**Fig** 还在快速迭代中，如果你对它感兴趣，不妨试一试，或者加入我们一起塑造它的未来！