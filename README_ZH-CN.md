# Fig 语言 - 现代脚本语言

[English](README.md "英文版本")

**Fig** 是一种静态类型、面向表达式的编程语言，专为清晰性、安全性和现代开发实践而设计。Fig 融合了 Go、Rust 和 JavaScript 的灵感，旨在提供高效的开发体验，同时保持强大的类型安全。

## 特性

### 🚀 核心语言特性
- **静态类型与类型推断** - 强类型系统，最少类型注解
- **现代控制流** - 完整的 `for` 循环支持，正确的作用域管理
- **一等公民函数** - Lambda 表达式和闭包
- **丰富的数据结构** - 结构体、列表、映射和元组
- **内存安全** - 无空指针异常，自动内存管理

### 🔧 技术亮点
- **三级循环作用域** - 迭代中正确的变量隔离
- **智能分号处理** - 使用 RAII 守卫的灵活语句终止
- **全面的错误处理** - 带有源码位置的详细错误信息
- **干净的 C++ 实现** - 现代 C++23，使用 RAII 和智能指针

### 📁 项目结构
    Fig/
    ├── src/
    │ ├── lexer.cpp # 词法分析
    │ ├── parser.cpp # 语法分析
    │ ├── evaluator.cpp # 解释器/执行引擎
    │ └── value.cpp # 类型系统实现
    ├── include/
    │ ├── argparse
    │ ├── magic_enum
    │ ├── Ast # 抽象语法树定义
    │ ├── value.hpp # 类型系统头文件
    │ ├── Value/ # 类型系统定义
    │ │ ├── ...
    │ ├── AstPrinter.hpp # AST 打印器
    │ ├── context(_forward).hpp # 环境/上下文系统
    │ ├── core.hpp # 核心信息
    │ ├── error.hpp # 异常系统
    │ ├── errorLog.hpp # 彩色打印错误日志
    │ ├── fig_string.hpp # Fig UTF-8 字符串
    │ ├── module.hpp # 包/模块系统
    │ ├── utils.hpp
    │ ├── token.hpp # Token 定义
    │ ├── lexer.hpp # 词法分析
    │ ├── parser.hpp # 语法分析和 AST
    │ ├── evaluator.hpp # 解释器和控制流
    │ ├── warning.hpp # 标准警告
    ├── ExampleCodes/ # 示例程序
    ├── .clang-format # Clang 格式化风格
    ├── test.fig # 测试文件（开发用）
    ├── LICENSE # Fig 项目许可证
    ├── xmake.lua # XMake 配置
    └── Logo/ # Fig 的 Logo

## 语言设计哲学
Fig 围绕几个核心原则设计：

1. **清晰优于巧妙** - 代码首先应该可读
2. **默认为安全** - 在编译时防止常见错误
3. **现代人机工程学** - 开发者体验很重要
4. **渐进式学习** - 入门简单，需要时功能强大