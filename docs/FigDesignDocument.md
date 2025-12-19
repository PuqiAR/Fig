## `Fig Programming Language` <font face="Consolas"> DESIGN DOC </font>

---
### 关键词解释 Token

``` cpp
enum class TokenType : int8_t
    {
        Illegal = -1,
        EndOfFile = 0,

        Comments,

        Identifier,

        /* Keywords */
        And,       // and
        Or,        // or
        Not,       // not
        Import,    // import
        Function,  // fun
        Variable,  // var
        Const,     // const
        Final,     // final
        While,     // while
        For,       // for
        Struct,    // struct
        Interface, // interface
        Implement, // implement
        Public,    // public

        // TypeNull,   // Null
        // TypeInt,    // Int
        // TypeString, // String
        // TypeBool,   // Bool
        // TypeDouble, // Double

        /* Literal Types (not keyword)*/
        LiteralNumber, // number (int,float...)
        LiteralString, // FString
        LiteralBool,   // bool (true/false)
        LiteralNull,   // null (Null的唯一实例)

        /* Punct */
        Plus,       // +
        Minus,      // -
        Asterisk,   // *
        Slash,      // /
        Percent,    // %
        Caret,      // ^
        Ampersand,  // &
        Pipe,       // |
        Tilde,      // ~
        ShiftLeft,  // <<
        ShiftRight, // >>
        // Exclamation,      // !
        Question,         // ?
        Assign,      // =
        Less,        // <
        Greater,     // >
        Dot,         // .
        Comma,       // ,
        Colon,       // :
        Semicolon,   // ;
        SingleQuote, // '
        DoubleQuote, // "
        // Backtick,         // `
        // At,               // @
        // Hash,             // #
        // Dollar,           // $
        // Backslash,        // '\'
        // Underscore,       // _
        LeftParen,    // (
        RightParen,   // )
        LeftBracket,  // [
        RightBracket, // ]
        LeftBrace,    // {
        RightBrace,   // }
        // LeftArrow,        // <-
        RightArrow,       // ->
        // DoubleArrow,      // =>
        Equal,           // ==
        NotEqual,        // !=
        LessEqual,       // <=
        GreaterEqual,    // >=
        PlusEqual,       // +=
        MinusEqual,      // -=
        AsteriskEqual,   // *=
        SlashEqual,      // /=
        PercentEqual,    // %=
        CaretEqual,      // ^=
        DoublePlus,      // ++
        DoubleMinus,     // --
        DoubleAmpersand, // &&
        DoublePipe,      // ||
        Walrus,          // :=
        Power,           // **
    };
```

* `Illegal`
  非法Token：无法解析或语法错误
&nbsp;
* `EndOfFile`
  即：
  ```cpp 
  EOF
  ``` 
  文件终止符
&nbsp;
* `Comments`
  注释Token，包括单行和多行
&nbsp;
* `Identifier`
  标识符，用户定义的‘名字’
&nbsp;
* `And` -> `&&` 或 `and`
  逻辑与
&nbsp;
* `Or`  -> `||` 或 `or`
  逻辑或
&nbsp;
* `Not` -> `!`  或 `!`
  逻辑非
&nbsp;
* `Import` -> `import`
  导入关键字，用于导入包。 e.g
  ``` python
  import std.io
  ```
&nbsp;
* `Function` -> `function`
  定义函数，匿名也可
  ``` javascript
  function greeting() -> Null public
  {
    std.io.println("Hello, world!");
  }

  function intAdder() -> Function public
  {
    return function(n1: Int, n2: Int) => n1 + n2;
  }
  ```
  此处的 `public` 为公开标识
  不进行显示声明 `public` 默认为私有，即对象仅能在当前作用域访问
&nbsp;
* `Variable` -> `var`
  定义变量
  ``` dart
  var foobar;
  var defaultVal = 1145;
  var numberSpecific: Int;
  var numberDefault: Int = 91;

  foobar = "hello, world!";
  foobar = 13;

  defaultVal = "it can be any value";

  numberSpecific = 78;
  numberDefault = 0;
  ```
&nbsp;
* `Const` -> `const`
  定义`全过程`常量： 从词法分析到求值器内的生命周期都为常量，仅能**在生命周期内**赋值一次，使用时也只有一个唯一对象
  &nbsp;
  必须在源码中指定值

  ``` dart
  const Pi = 3.1415926; // recommended

  const name; // ❌ 错误
  ```

  定义后的常量，其值及类型均不可改变，故可省略类型标识。这是推荐的写法
  同时，也可作为结构体成员的修饰
  ``` cpp
  struct MathConstants
  {
    const Pi = 3.1415926;
  };
  ```
&nbsp;
* `Final` -> `final`
  定义`结构体运行时`常量：从运行期开始的常量，仅能**在运行时**被赋值一次， **仅修饰结构体成员**
  不存在 **final** 类型的外部常量
  &nbsp;
  定义后的常量，其值及类型均不可改变，故可省略类型标识。这是推荐的写法
  ``` cpp
  struct Person
  {
    final name: String
    final age: Int
  
    final sex: String = "gender" // ❌ 请使用 const 代替
  }
  ```
&nbsp;
* `While` -> `while`
  while循环，满足一个布尔类型条件循环执行语句

  ``` cpp
  while (ans != 27.19236)
  {
    ans = Int.parse(std.io.readline());
  }
  ```
&nbsp;
* `For` -> `for`
  for循环，拥有初始语句、条件、增长语句
  ``` cpp
  for (init; condition; increment)
  {
    statements...;
  }
  ```
&nbsp;
* `Struct` -> `struct`
  结构体，面对对象

  ``` cpp
  struct Person
  {
    public final name: String; // public, final
    public age: Int; // public
    sex: String;  // private normally;
    
    const ADULT_AGE = 18; // private, const

    fun printInfo()
    {
      std.io.println("name: {}, age: {}, sex: {}", name, age, sex);
    }
  };

  var person = Person {"Fig", 1, "IDK"};
  // or
  var person = Person {name: "Fig", age: 1, sex: "IDK"}; // can be unordered

  var name = "Fig";
  var age = 1;
  var sex = "IDK";

  var person = Person {name, age, sex};
  // = `var person = Person {name: name, age: age, sex: sex}`
  ```

