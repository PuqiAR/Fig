/*
   src/error/definitions/invalid_number_literal.rs
   Part of The Fig Project, under the MIT License.
   Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
   See LICENSE for details.
*/

use crate::core;
use crate::core::source::SourceRange;
use crate::error::diagnostic::{Diagnostic, ErrorType, Thrower};

#[derive(Debug)]
pub struct InvalidNumberLiteralError {
    file_id: usize,
    start_line: usize,
    start_column: usize,
    end_column: usize,
    thrower: Option<Thrower>,
}

impl InvalidNumberLiteralError {
    pub fn new(file_id: usize, start_line: usize, start_column: usize, end_column: usize) -> Self {
        InvalidNumberLiteralError {
            file_id,
            start_line,
            start_column,
            end_column,
            thrower: None,
        }
    }
    pub fn with_thrower(mut self, thrower: Thrower) -> Self {
        self.thrower = Some(thrower);
        self
    }
}

impl Diagnostic for InvalidNumberLiteralError {
    fn error_type(&self) -> ErrorType {
        ErrorType::InvalidNumberLiteral
    }

    fn message(&self, lang: core::Lang) -> String {
        match lang {
            core::Lang::en_US => {
                format!(
                    "Invalid number literal at line {}, column {}",
                    self.start_line, self.start_column,
                )
            }
            core::Lang::zh_CN => {
                format!(
                    "无效的数字字面量，位于第 {} 行，第 {} 列",
                    self.start_line, self.start_column,
                )
            }
        }
    }

    fn span(&self) -> SourceRange {
        SourceRange {
            file_id:      self.file_id,
            start_line:   self.start_line,
            start_column: self.start_column,
            end_line:     self.start_line,
            end_column:   self.end_column,
        }
    }

    fn thrower(&self) -> Option<Thrower> {
        self.thrower
    }
}