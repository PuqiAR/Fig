/*
    src/token/mod.rs

    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TokenKind {
    Eof,

    // 标识符
    Identifier,

    // 字面量
    String,
    RawString, // r"raw string"
    Number,    // 123, 3.1415, 100e10 (等同 100e+10) ...
    BinInt,    // 0b1010
    OctInt,    // 0o77
    HexInt,    // 0xFF

    // 关键字
    Var,
    Const,
    Func,
    Struct,
    Interface,
    Impl,
    Enum,
    If,
    Else,
    For,
    While,
    Return,
    Break,
    Continue,
    Import,
    New,
    As,
    Is,
    True,
    False,
    Null,
    And,
    Or,
    Not,
    Public,

    // 分隔符
    LParen,    // (
    RParen,    // )
    LBrace,    // {
    RBrace,    // }
    LBracket,  // [
    RBracket,  // ]
    Comma,     // ,
    Dot,       // .
    Semicolon, // ;
    Colon,     // :
    Arrow,     // ->
    FatArrow,  // =>
    Question,  // ?
    At,        // @
    Hash,      // #
    Dollar,    // $

    // 运算符
    Plus,       // +
    Minus,      // -
    Star,       // *
    Slash,      // /
    Percent,    // %
    StarStar,   // **
    PlusPlus,   // ++
    MinusMinus, // --

    // 赋值运算符
    Assign,        // =
    PlusAssign,    // +=
    MinusAssign,   // -=
    StarAssign,    // *=
    SlashAssign,   // /=
    PercentAssign, // %=
    CaretAssign,   // ^=

    // 比较运算符
    EqEq,  // ==
    NotEq, // !=
    Lt,    // <
    Gt,    // >
    LtEq,  // <=
    GtEq,  // >=

    // 逻辑运算符
    AndAnd, // &&
    OrOr,   // ||

    // 位运算符
    Amp,   // &
    Pipe,  // |
    Caret, // ^
    Tilde, // ~
    Bang,  // !
    LtLt,  // <<
    GtGt,  // >>
}

/// Token 实例
#[derive(Debug, Clone)]
pub struct Token {
    pub kind: TokenKind,
    pub index: usize,
    pub length: u32,
    
}

impl Token
{
    pub fn new(kind: TokenKind, index: usize, length: u32) -> Self 
    {
        Token
        {
            kind,
            index,
            length
        }
    }
}


/// 运算符打表，按字符串长度降序保证最长匹配优先。
pub static OPERATORS: &[(&str, TokenKind)] = &[
    ("->", TokenKind::Arrow),
    ("=>", TokenKind::FatArrow),

    ("**", TokenKind::StarStar),
    ("*=", TokenKind::StarAssign),

    ("++", TokenKind::PlusPlus),
    ("+=", TokenKind::PlusAssign),

    ("--", TokenKind::MinusMinus),
    ("-=", TokenKind::MinusAssign),

    ("/=", TokenKind::SlashAssign),
    ("%=", TokenKind::PercentAssign),
    ("^=", TokenKind::CaretAssign),

    ("==", TokenKind::EqEq),
    ("!=", TokenKind::NotEq),
    ("<=", TokenKind::LtEq),
    (">=", TokenKind::GtEq),

    ("&&", TokenKind::AndAnd),
    ("||", TokenKind::OrOr),

    ("<<", TokenKind::LtLt),
    (">>", TokenKind::GtGt),

    ("+", TokenKind::Plus),
    ("-", TokenKind::Minus),
    ("*", TokenKind::Star),
    ("/", TokenKind::Slash),
    ("%", TokenKind::Percent),
    ("=", TokenKind::Assign),
    ("<", TokenKind::Lt),
    (">", TokenKind::Gt),
    ("&", TokenKind::Amp),
    ("|", TokenKind::Pipe),
    ("^", TokenKind::Caret),
    ("~", TokenKind::Tilde),
    ("!", TokenKind::Bang),

    ("(", TokenKind::LParen),
    (")", TokenKind::RParen),
    ("{", TokenKind::LBrace),
    ("}", TokenKind::RBrace),
    ("[", TokenKind::LBracket),
    ("]", TokenKind::RBracket),
    (",", TokenKind::Comma),
    (".", TokenKind::Dot),
    (";", TokenKind::Semicolon),
    (":", TokenKind::Colon),
    ("?", TokenKind::Question),
    ("@", TokenKind::At),
    ("#", TokenKind::Hash),
    ("$", TokenKind::Dollar),
];

/// 遍历 OPERATORS，返回第一个匹配的 (TokenKind, 长度)。
pub fn lookup_operator(prefix: &str) -> Option<(TokenKind, usize)> {
    for (s, kind) in OPERATORS {
        if prefix.starts_with(s) {
            return Some((*kind, s.len()));
        }
    }
    None
}

pub fn lookup_keyword(word: &str) -> Option<TokenKind> {
    match word {
        "var" => Some(TokenKind::Var),
        "const" => Some(TokenKind::Const),
        "func" => Some(TokenKind::Func),
        "struct" => Some(TokenKind::Struct),
        "interface" => Some(TokenKind::Interface),
        "impl" => Some(TokenKind::Impl),
        "enum" => Some(TokenKind::Enum),
        "if" => Some(TokenKind::If),
        "else" => Some(TokenKind::Else),
        "for" => Some(TokenKind::For),
        "while" => Some(TokenKind::While),
        "return" => Some(TokenKind::Return),
        "break" => Some(TokenKind::Break),
        "continue" => Some(TokenKind::Continue),
        "import" => Some(TokenKind::Import),
        "new" => Some(TokenKind::New),
        "as" => Some(TokenKind::As),
        "is" => Some(TokenKind::Is),
        "true" => Some(TokenKind::True),
        "false" => Some(TokenKind::False),
        "null" => Some(TokenKind::Null),
        "and" => Some(TokenKind::And),
        "or" => Some(TokenKind::Or),
        "not" => Some(TokenKind::Not),
        "public" => Some(TokenKind::Public),
        _ => None,
    }
}
