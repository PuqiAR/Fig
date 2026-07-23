/*
    src/error/definitions/unexpected_character.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::core;
use crate::core::source::SourceRange;
use crate::error::diagnostic::{Diagnostic, ErrorType, Thrower};

#[derive(Debug)]
pub struct UnexpectedCharacterError
{
    file_id: usize,
    character: char,
    line: usize,
    column: usize,
    thrower: Option<Thrower>,
}

impl UnexpectedCharacterError
{
    pub fn new(file_id: usize, character: char, line: usize, column: usize) -> Self
    {
        UnexpectedCharacterError
        {
            file_id,
            character,
            line,
            column,
            thrower: None,
        }
    }

    pub fn with_thrower(mut self, t: Thrower) -> Self
    {
        self.thrower = Some(t);
        self
    }
}

impl Diagnostic for UnexpectedCharacterError
{
    fn error_type(&self) -> ErrorType
    {
        ErrorType::UnexpectedCharacter
    }

    fn message(&self, lang: crate::core::Lang) -> String
    {
        let ch = match (self.character, lang) {
            (' ', crate::core::Lang::zh_CN) => "<空格>",
            (' ', _) => "<space>",
            ('\n', crate::core::Lang::zh_CN) => "<换行>",
            ('\n', _) => "<newline>",
            ('\t', crate::core::Lang::zh_CN) => "<制表符>",
            ('\t', _) => "<tab>",
            ('\r', crate::core::Lang::zh_CN) => "<回车>",
            ('\r', _) => "<carriage return>",
            (other, crate::core::Lang::zh_CN) =>
                return format!("意外的字符 '{}'，位于第 {} 行，第 {} 列", other, self.line, self.column),
            (other, _) =>
                return format!("Unexpected character '{}' at line {}, column {}", other, self.line, self.column),
        };
        match lang {
            crate::core::Lang::zh_CN =>
                format!("意外的字符 {}，位于第 {} 行，第 {} 列", ch, self.line, self.column),
            _ =>
                format!("Unexpected character {} at line {}, column {}", ch, self.line, self.column),
        }
    }

    fn span(&self) -> SourceRange
    {
        SourceRange
        {
            file_id:      self.file_id,
            start_line:   self.line,
            start_column: self.column,
            end_line:     self.line,
            end_column:   self.column + 1,
        }
    }

    fn thrower(&self) -> Option<Thrower>
    {
        self.thrower
    }
}