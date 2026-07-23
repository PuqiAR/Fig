/*
    src/error/definitions/unterminated_string_literal.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::error::diagnostic::{Diagnostic, ErrorType, Thrower};
use crate::core;
use crate::core::source::SourceRange;

#[derive(Debug)]
pub struct UnterminatedStringLiteralError
{
    file_id: usize,
    start_line: usize,
    start_column: usize,
    end_line: usize,
    end_column: usize,
    thrower: Option<Thrower>,
}

impl UnterminatedStringLiteralError
{
    pub fn new(file_id: usize, start_line: usize, start_column: usize, end_line: usize, end_column: usize) -> Self
    {
        UnterminatedStringLiteralError
        {
            file_id,
            start_line,
            start_column,
            end_line,
            end_column,
            thrower: None,
        }
    }

    pub fn with_thrower(mut self, t: Thrower) -> Self
    {
        self.thrower = Some(t);
        self
    }
}

impl Diagnostic for UnterminatedStringLiteralError
{
    fn error_type(&self) -> ErrorType
    {
        ErrorType::UnterminatedStringLiteral
    }

    fn message(&self, lang: core::Lang) -> String
    {
        match lang
        {
            core::Lang::en_US =>
            {
                format!("Unterminated string literal starting at line {}, column {}", self.start_line, self.start_column)
            }
            core::Lang::zh_CN =>
            {
                format!("字符串字面量未终止，起始于第 {} 行，第 {} 列", self.start_line, self.start_column)
            }
        }
    }

    fn span(&self) -> SourceRange
    {
        SourceRange
        {
            file_id:      self.file_id,
            start_line:   self.start_line,
            start_column: self.start_column,
            end_line:     self.end_line,
            end_column:   self.end_column,
        }
    }

    fn thrower(&self) -> Option<Thrower>
    {
        self.thrower
    }
}