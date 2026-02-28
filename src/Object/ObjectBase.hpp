/*!
    @file src/Object/ObjectBase.hpp
    @brief 值表示定义 (NaN Boxing) uint64
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <bit>
#include <cstdint>

#include <Deps/Deps.hpp>

namespace Fig
{

    struct Object; // 前置声明

    /*
        正常来说直接 Value = std::uint64_t会更快
        但是这样会带来隐式转换的问题

        因此我们封装成一个类，这样速度不会损失很多。
        (release模式编译器会直接优化, 速度和uint64_t直接表示一样快)
    */
    class Value
    {
    private:
        std::uint64_t v_; // 唯一的物理成员 sizeof(Value) 永远是 8 字节。

        // --- 私有掩码常量 ---
        static constexpr std::uint64_t QNAN_MASK = 0x7ffc000000000000;
        static constexpr std::uint64_t SIGN_BIT  = 0x8000000000000000;

        // 专门给 Int32 预留的高位 Tag
        static constexpr std::uint32_t INT_TAG_HIGH = 0x7FFD0000;

        // 基础原语 Tag
        static constexpr std::uint64_t TAG_NULL  = 1;
        static constexpr std::uint64_t TAG_FALSE = 2;
        static constexpr std::uint64_t TAG_TRUE  = 3;

        // 私有底层构造：仅供内部组装使用
        constexpr explicit Value(uint64_t raw) : v_(raw) {}

    public:
        // 默认构造为 Null，保证未初始化变量也是安全的
        constexpr Value()
        {
            *this = GetNullInstance();
        }

        [[nodiscard]] static constexpr Value FromDouble(double d)
        {
            uint64_t raw = std::bit_cast<uint64_t>(d);
            // 清洗非法的 NaN
            if ((raw & QNAN_MASK) == QNAN_MASK)
                return Value(QNAN_MASK);
            return Value(raw);
        }

        [[nodiscard]] static constexpr Value FromInt(std::int32_t i)
        {
            // 移位构造，彻底阻断符号扩展漏洞
            return Value((static_cast<uint64_t>(INT_TAG_HIGH) << 32) | static_cast<uint32_t>(i));
        }

        [[nodiscard]] static constexpr Value &GetTrueInstance()
        {
            static Value trueInstance(QNAN_MASK | TAG_TRUE);
            return trueInstance;
        }
        [[nodiscard]] static constexpr Value &GetFalseInstance()
        {
            static Value falseInstance(QNAN_MASK | TAG_FALSE);
            return falseInstance;
        }

        [[nodiscard]] static constexpr Value &FromBool(bool b)
        {
            return (b ? GetTrueInstance() : GetFalseInstance());
        }

        [[nodiscard]] static constexpr Value &GetNullInstance()
        {
            static Value nullInstance(QNAN_MASK | TAG_NULL);
            return nullInstance;
        }

        [[nodiscard]] static Value FromObject(Object *ptr)
        {
            return Value(reinterpret_cast<uint64_t>(ptr) | SIGN_BIT | QNAN_MASK);
        }

        // 类型检查 (Is)

        [[nodiscard]] constexpr bool IsDouble() const
        {
            return (v_ & QNAN_MASK) != QNAN_MASK;
        }

        [[nodiscard]] constexpr bool IsInt() const
        {
            // 安全的高 32 位移位判定
            return static_cast<uint32_t>(v_ >> 32) == INT_TAG_HIGH;
        }

        [[nodiscard]] constexpr bool IsNumber() const
        {
            return IsDouble() || IsInt();
        }

        [[nodiscard]] constexpr bool IsNull() const
        {
            return v_ == (QNAN_MASK | TAG_NULL);
        }

        [[nodiscard]] constexpr bool IsBool() const
        {
            return (v_ | 1) == (QNAN_MASK | TAG_TRUE);
        }

        [[nodiscard]] constexpr bool IsObject() const
        {
            return (v_ & (SIGN_BIT | QNAN_MASK)) == (SIGN_BIT | QNAN_MASK);
        }
        
        // 提取数据 (Unbox / As)
        [[nodiscard]] constexpr double AsDouble() const
        {
            return std::bit_cast<double>(v_);
        }

        [[nodiscard]] constexpr int32_t AsInt() const
        {
            return static_cast<int32_t>(v_);
        }

        // 核心辅助：泛型数字提取。算术指令可以直接用这个，免去手写 if 分支
        // 若不是 int/double 会导致非常恐怖的问题
        [[nodiscard]] constexpr double CastToDouble() const
        {
            return IsInt() ? static_cast<double>(AsInt()) : AsDouble();
        }

        [[nodiscard]] constexpr bool AsBool() const
        {
            return v_ == (QNAN_MASK | TAG_TRUE);
        }

        [[nodiscard]] struct Object *AsObject() const
        {
            return reinterpret_cast<struct Object *>(v_ & ~(SIGN_BIT | QNAN_MASK));
        }

        // 重载

        // 暴露原生值用于硬核位运算或 Hash 计算
        [[nodiscard]] constexpr uint64_t Raw() const
        {
            return v_;
        }

        // 让 VM 的 OP_EQ 指令极简：`if (RA == RB)`
        [[nodiscard]] constexpr bool operator==(const Value &other) const
        {
            // IEEE 754 规定浮点数有 +0.0 == -0.0 的特殊规则，所以不直接比对raw bits而是转换成 double进行C++比对
            if (IsDouble() && other.IsDouble())
            {
                return AsDouble() == other.AsDouble();
            }
            // 直接比较 64 位整数内存，所以堆对象比较为地址(运算符重载由Compiler处理)
            return v_ == other.v_;
        }

        [[nodiscard]] constexpr bool operator!=(const Value &other) const
        {
            return !(*this == other);
        }

        // 类函数

        [[nodiscard]]
        constexpr String ToString() const;
    };

    /*
        C风格继承 + 手动分发
        禁止任何 virtual 达到最高效率
    */
    enum class ObjectType : uint8_t
    {
        String,
        Function,
        Struct,
        Instance,
    };

    struct StructObject /* : public Object */; // 结构体基类的定义，前向声明

    // Total 24 bytes size
    struct Object
    {
        Object    *next;             // 8 bytes: gc链表
        StructObject    *klass;            // 8 bytes: 一切皆对象，父类指针
        ObjectType type;             // 1 byte : 类型
        bool       isMarked = false; // 1 byte : gc标记
        // + 6 bytes padding

        constexpr bool isString() const
        {
            return type == ObjectType::String;
        }

        constexpr bool isFunction() const
        {
            return type == ObjectType::Function;
        }

        constexpr bool isStruct() const
        {
            return type == ObjectType::Struct;
        }

        constexpr bool isInstance() const
        {
            return type == ObjectType::Instance;
        }
    };
} // namespace Fig