/*
    src/error/definitions/invalid_escape_sequence.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::error::diagnostic::{Diagnostic, ErrorType, Thrower};

#[derive(Debug)]
pub struct InvalidEscapeSequenceError {
    file_id: usize,
    start_line: usize,
    start_column: usize,
    escape_sequence: String,

    thrower: Option<Thrower>,
}

impl InvalidEscapeSequenceError {
    pub fn new(file_id: usize, start_line: usize, start_column: usize, escape_sequence: String) -> Self {
        InvalidEscapeSequenceError {
            file_id,
            start_line,
            start_column,
            escape_sequence,
            thrower: None,
        }
    }
    pub fn with_thrower(mut self, thrower: Thrower) -> Self {
        self.thrower = Some(thrower);
        self
    }
}

impl Diagnostic for InvalidEscapeSequenceError {
    fn error_type(&self) -> ErrorType {
        ErrorType::InvalidEscapeSequence
    }

    fn message(&self, lang: crate::core::Lang) -> String {
        match lang {
            crate::core::Lang::en_US => format!(
                "Invalid escape sequence: {} at line {}, column {}",
                self.escape_sequence, self.start_line, self.start_column
            ),
            crate::core::Lang::zh_CN => format!(
                "无效的转义序列: {} 在第 {} 行，第 {} 列",
                self.escape_sequence, self.start_line, self.start_column
            ),
        }
    }

    fn span(&self) -> crate::core::source::SourceRange {
        crate::core::source::SourceRange {
            file_id: self.file_id,
            start_line: self.start_line,
            start_column: self.start_column,
            end_line: self.start_line,
            end_column: self.start_column + self.escape_sequence.len(),
        }
    }
}
