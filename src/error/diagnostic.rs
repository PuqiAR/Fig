/*
    src/error/error.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::core;
use crate::core::source::SourceRange;

use crate::error::hint;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Severity
{
    Warning,
    Error,
    Critical,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorType
{
    UnterminatedStringLiteral,
    UnexpectedCharacter,
    InvalidNumberLiteral,
    InvalidEscapeSequence,
    UnterminatedComment,
}

impl ErrorType
{
    pub fn severity(&self) -> Severity
    {
        match self
        {
            ErrorType::UnterminatedStringLiteral => Severity::Error,
            ErrorType::UnexpectedCharacter => Severity::Error,
            ErrorType::InvalidNumberLiteral => Severity::Error,
            ErrorType::InvalidEscapeSequence => Severity::Error,
            ErrorType::UnterminatedComment => Severity::Error,
        }
    }

    pub fn title(&self, lang: core::Lang) -> String
    {
        match self
        {
            ErrorType::UnterminatedStringLiteral =>
            {
                match lang
                {
                    core::Lang::en_US => "Unterminated string literal".to_string(),
                    core::Lang::zh_CN => "字符串字面量未终止".to_string(),
                }
            }
            ErrorType::UnexpectedCharacter =>
            {
                match lang
                {
                    core::Lang::en_US => "Unexpected character".to_string(),
                    core::Lang::zh_CN => "意外的字符".to_string(),
                }
            }
            ErrorType::InvalidNumberLiteral =>
            {
                match lang
                {
                    core::Lang::en_US => "Invalid number literal".to_string(),
                    core::Lang::zh_CN => "无效的数字字面量".to_string(),
                }
            }
            ErrorType::InvalidEscapeSequence =>
            {
                match lang
                {
                    core::Lang::en_US => "Invalid escape sequence".to_string(),
                    core::Lang::zh_CN => "无效的转义序列".to_string(),
                }
            }
            ErrorType::UnterminatedComment =>
            {
                match lang
                {
                    core::Lang::en_US => "Unterminated comment".to_string(),
                    core::Lang::zh_CN => "未终止的注释".to_string(),
                }
            }
        }
    }
}

pub struct Related
{
    pub span:    SourceRange,
    pub message: String,
}

/// 编译器源码位置，`thrower!()` 宏自动捕获。
#[derive(Debug, Clone, Copy)]
pub struct Thrower
{
    pub file: &'static str,
    pub line: u32,
}

#[macro_export]
macro_rules! thrower {
    () => {
        $crate::error::diagnostic::Thrower {
            file: file!(),
            line: line!(),
        }
    };
}

pub trait Diagnostic: std::fmt::Debug
{
    fn error_type(&self) -> ErrorType;
    fn message(&self, lang: core::Lang) -> String;
    fn span(&self) -> SourceRange;

    fn hints(&self) -> Vec<hint::Hint>
    {
        vec![]
    }

    fn related(&self, _lang: core::Lang) -> Vec<Related>
    {
        vec![]
    }

    fn thrower(&self) -> Option<Thrower>
    {
        None
    }
}
