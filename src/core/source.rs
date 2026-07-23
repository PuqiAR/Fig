/*
    src/core/source.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use std::path::PathBuf;

#[derive(Debug, Clone, Copy)]
pub struct SourcePosition
{
    pub file_id: usize,
    pub line:    usize,
    pub column:  usize,
}

#[derive(Debug, Clone, Copy)]
pub struct SourceRange
{
    pub file_id:      usize,
    pub start_line:   usize,
    pub start_column: usize,
    pub end_line:     usize,
    pub end_column:   usize,
}

// SourceFile / SourceManager

pub struct SourceFile
{
    pub path:   PathBuf,
    pub source: String,
    line_offsets: Vec<usize>,
}

impl SourceFile
{
    pub fn from_string(name: &str, source: String) -> Self
    {
        let line_offsets = Self::build_line_offsets(&source);
        Self { path: PathBuf::from(name), source, line_offsets }
    }

    pub fn from_path(path: PathBuf) -> Result<Self, std::io::Error>
    {
        let source = std::fs::read_to_string(&path)?;
        let line_offsets = Self::build_line_offsets(&source);
        Ok(Self { path, source, line_offsets })
    }

    pub fn read(path: PathBuf) -> Result<Self, std::io::Error>
    {
        let source = std::fs::read_to_string(&path)?;
        let line_offsets = Self::build_line_offsets(&source);
        Ok(Self { path, source, line_offsets })
    }

    fn build_line_offsets(source: &str) -> Vec<usize>
    {
        std::iter::once(0)
            .chain(
                source
                    .bytes()
                    .enumerate()
                    .filter(|(_, b)| *b == b'\n')
                    .map(|(i, _)| i + 1),
            )
            .collect()
    }

    pub fn line_offsets(&self) -> &[usize] { &self.line_offsets }

    pub fn offset_to_position(&self, offset: usize) -> (usize, usize)
    {
        let line = match self.line_offsets.binary_search(&offset) {
            Ok(line) => line,
            Err(line) => line.saturating_sub(1),
        };
        let col = offset - self.line_offsets[line] + 1;
        (line + 1, col)
    }

    pub fn get_line(&self, offset: usize) -> &str
    {
        let (line, _) = self.offset_to_position(offset);
        let start = self.line_offsets[line - 1];
        let end = self.source[start..]
            .find('\n')
            .map(|i| start + i)
            .unwrap_or(self.source.len());
        &self.source[start..end]
    }
}

pub struct SourceManager
{
    files: Vec<SourceFile>,
}

impl SourceManager
{
    pub fn new() -> Self
    {
        Self { files: Vec::new() }
    }

    pub fn add_file(&mut self, file: SourceFile) -> usize
    {
        let id = self.files.len();
        self.files.push(file);
        id
    }

    pub fn get(&self, id: usize) -> Option<&SourceFile>
    {
        self.files.get(id)
    }

    pub fn source(&self, id: usize) -> Option<&str>
    {
        self.files.get(id).map(|f| f.source.as_str())
    }
}
