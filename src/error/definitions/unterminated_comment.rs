/*
    src/error/definitions/unterminated_comment.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::error::diagnostic::{Diagnostic, ErrorType, Thrower};
use crate::core;
use crate::core::source::SourceRange;

#[derive(Debug)]
pub struct UnterminatedCommentError
{
    file_id: usize,
    start_line: usize,
    start_column: usize,
    end_line: usize,
    end_column: usize,
    thrower: Option<Thrower>,
}

impl UnterminatedCommentError
{
    pub fn new(file_id: usize, start_line: usize, start_column: usize, end_line: usize, end_column: usize) -> Self
    {
        UnterminatedCommentError
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

impl Diagnostic for UnterminatedCommentError
{
    fn error_type(&self) -> ErrorType
    {
        ErrorType::UnterminatedComment
    }

    fn message(&self, lang: core::Lang) -> String
    {
        match lang
        {
            core::Lang::en_US =>
            {
                format!("Unterminated block comment starting at line {}, column {}", self.start_line, self.start_column)
            }
            core::Lang::zh_CN =>
            {
                format!("未终止的多行注释，起始于第 {} 行，第 {} 列", self.start_line, self.start_column)
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