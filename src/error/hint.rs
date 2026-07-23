/*
    src/error/hint.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

pub enum Hint
{
    Insertion { line: usize, column: usize, content: String },
    Deletion { line: usize, column: usize, length: usize },
    Replacement { line: usize, column: usize, length: usize, content: String },
}