/*
    src/error/report.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use std::fmt::Write;

use crate::core;
use crate::core::colors as C;
use crate::core::source::SourceManager;
use crate::error::diagnostic::{Diagnostic, Related, Severity};
use crate::error::hint::Hint;

pub struct Reporter {
    lang: core::Lang,
}

impl Reporter {
    pub fn new(lang: core::Lang) -> Self {
        Self { lang }
    }

    pub fn report(&self, diag: &dyn Diagnostic, manager: &SourceManager, out: &mut impl Write) {
        let et = diag.error_type();
        let severity = et.severity();
        let span = diag.span();
        let file = manager.get(span.file_id);

        let (severity_color, severity_label, severity_icon) = match severity {
            Severity::Warning => (C::ORANGE, self.txt_warning(), "\u{26A0}"),
            Severity::Error => (C::PURPLE, self.txt_error(), "\u{2717}"),
            Severity::Critical => (C::CRITICAL_RED, self.txt_critical(), "\u{2620}"),
        };

        writeln!(
            out,
            "{} {}{}{}[E{}]{} {}",
            severity_icon,
            C::BOLD,
            severity_color,
            severity_label,
            et as usize,
            C::RESET,
            et.title(self.lang),
        )
        .unwrap();

        if let Some(f) = file {
            writeln!(
                out,
                "  {}{}-->{} {}:{}:{}",
                C::DIM,
                C::GRAY,
                C::RESET,
                f.path.display(),
                span.start_line,
                span.start_column,
            )
            .unwrap();
        }

        self.print_source_context(diag, manager, out);

        let hints = diag.hints();
        for hint in &hints {
            let label = self.fmt_hint(hint);
            writeln!(
                out,
                "  {}={}{} {}",
                C::DIM,
                C::RESET,
                self.txt_suggestion(),
                label
            )
            .unwrap();
        }

        if let Some(t) = diag.thrower() {
            writeln!(
                out,
                "  {}@ {}:{}{}",
                C::DIM,
                t.file, t.line,
                C::RESET,
            ).unwrap();
        }

        let related = diag.related(self.lang);
        if !related.is_empty() {
            for rel in &related {
                self.print_related(rel, manager, out);
            }
        }

        writeln!(out).unwrap();
    }

    fn print_source_context(
        &self,
        diag: &dyn Diagnostic,
        manager: &SourceManager,
        out: &mut impl Write,
    ) {
        let span = diag.span();
        let severity = diag.error_type().severity();
        let file = match manager.get(span.file_id) {
            Some(f) => f,
            None => return,
        };

        let line_color = match severity {
            Severity::Warning => C::ORANGE,
            Severity::Error => C::PURPLE,
            Severity::Critical => C::CRITICAL_RED,
        };

        let total_lines = file.line_offsets().len();
        let context_lines = 3;
        let line_start = span.start_line.saturating_sub(context_lines).max(1);
        let line_end = (span.end_line + 2).min(total_lines);

        let max_line_width = format!("{}", line_end).len();

        writeln!(out, "    {}|{}", C::DIM, C::RESET).unwrap();

        for line_no in line_start..=line_end {
            let offset = file.line_offsets()[line_no - 1];
            let line_text = file.get_line(offset);
            let highlight = line_no >= span.start_line && line_no <= span.end_line;

            if highlight {
                writeln!(
                    out,
                    " {} {}{}{}{} |{} {}",
                    padding(max_line_width, line_no),
                    C::BOLD,
                    line_color,
                    line_no,
                    C::RESET,
                    C::SOURCE_TEXT,
                    line_text,
                )
                .unwrap();
            } else {
                writeln!(
                    out,
                    " {} {}{}{} |{} {}",
                    padding(max_line_width, line_no),
                    C::DIM,
                    line_no,
                    C::RESET,
                    C::SOURCE_TEXT,
                    line_text,
                )
                .unwrap();
            }

            if line_no == span.start_line {
                let (caret_line, carets_end) =
                    self.build_caret_line(span, severity, file, line_no, max_line_width);
                writeln!(out, "{caret_line}").unwrap();

                let msg = diag.message(self.lang);
                let msg_color = match severity {
                    Severity::Warning => C::ORANGE,
                    Severity::Error => C::PURPLE,
                    Severity::Critical => C::CRITICAL_RED,
                };
                let arrow_indent = carets_end.saturating_sub(3);
                writeln!(
                    out,
                    "{} {}{}╰─{} {}  {}",
                    " ".repeat(arrow_indent),
                    C::DIM,
                    C::GRAY,
                    C::RESET,
                    msg_color,
                    msg,
                )
                .unwrap();
            }

            if severity == Severity::Critical && line_no == span.end_line {
                let text_before = take_chars(line_text, span.end_column.saturating_sub(1));
                let dw = display_width(&text_before);
                let span_dw = display_width(&take_chars_range(
                    line_text,
                    span.start_column.saturating_sub(1),
                    span.end_column,
                ));
                let ww = char_width('\u{FE4D}').max(1);
                let wave_line = "\u{FE4D}".repeat((span_dw + ww - 1) / ww);
                let pad = " ".repeat(max_line_width + 5 + dw);
                writeln!(
                    out,
                    "{} {}{}{}{}",
                    pad,
                    C::BRIGHT_RED,
                    C::BOLD,
                    wave_line,
                    C::RESET
                )
                .unwrap();
            }
        }

        writeln!(out, "    {}|{}", C::DIM, C::RESET).unwrap();
    }

    /// 返回 (caret行字符串, caret结束的显示列位置)
    fn print_related(
        &self,
        rel: &Related,
        manager: &SourceManager,
        out: &mut impl Write,
    )
    {
        let file = match manager.get(rel.span.file_id) {
            Some(f) => f,
            None => return,
        };

        writeln!(
            out,
            "  {}={} \u{1F4A1} {}{}{}: {}",
            C::DIM,
            C::RESET,
            C::LIGHT_BLUE, self.txt_note(), C::RESET,
            rel.message,
        ).unwrap();

        writeln!(
            out,
            "  {}{}-->{} {}:{}:{}",
            C::DIM,
            C::GRAY,
            C::RESET,
            file.path.display(),
            rel.span.start_line,
            rel.span.start_column,
        ).unwrap();

        let line_no = rel.span.start_line;
        let offset = file.line_offsets()[line_no - 1];
        let line_text = file.get_line(offset);
        let max_line_width = format!("{}", line_no).len();

        writeln!(out, "    {}|{}", C::DIM, C::RESET).unwrap();

        writeln!(
            out,
            " {} {}{}{} |{} {}",
            padding(max_line_width, line_no),
            C::DIM, line_no,
            C::RESET,
            C::SOURCE_TEXT,
            line_text,
        ).unwrap();

        let col_start = rel.span.start_column;
        let col_end = rel.span.end_column.min(chars_count(line_text) + 1);
        let len = col_end.saturating_sub(col_start);
        if len > 0 {
            let dw_before = display_width(&take_chars(line_text, col_start.saturating_sub(1)));
            let indent = " ".repeat(max_line_width + 5 + dw_before);
            let dw_error = display_width(&take_chars_range(line_text, col_start.saturating_sub(1), col_start.saturating_sub(1) + len));
            let wave_w = char_width('\u{FE4D}').max(1);
            let n_wave = (dw_error + wave_w - 1) / wave_w;
            let wave = "\u{FE4D}".repeat(n_wave);
            writeln!(out, "{}{}{}{}{}", indent, C::PURPLE, C::BOLD, wave, C::RESET).unwrap();
        }

        writeln!(out, "    {}|{}", C::DIM, C::RESET).unwrap();
        writeln!(out).unwrap();
    }

    fn build_caret_line(
        &self,
        span: core::source::SourceRange,
        severity: Severity,
        file: &core::source::SourceFile,
        line_no: usize,
        max_line_width: usize,
    ) -> (String, usize) {
        let line_offset = file.line_offsets()[line_no - 1];
        let line_text = file.get_line(line_offset);

        let col_start = if line_no == span.start_line {
            span.start_column
        } else {
            1
        };
        let col_end = if line_no == span.end_line {
            span.end_column.min(chars_count(line_text) + 1)
        } else {
            chars_count(line_text) + 1
        };

        let len = col_end.saturating_sub(col_start);
        if len == 0 {
            return (String::new(), 0);
        }

        let text_before_caret = take_chars(line_text, col_start.saturating_sub(1));
        let dw_before = display_width(&text_before_caret);
        let indent_width = max_line_width + 5 + dw_before;
        let indent = " ".repeat(indent_width);

        let error_text = take_chars_range(
            line_text,
            col_start.saturating_sub(1),
            col_start.saturating_sub(1) + len,
        );
        let dw_error = display_width(&error_text);
        let wave_w = char_width('\u{FE4D}').max(1);
        let n_wave = (dw_error + wave_w - 1) / wave_w;
        let wave = "\u{FE4D}".repeat(n_wave);

        let color = match severity {
            Severity::Warning => C::ORANGE,
            Severity::Error => C::PURPLE,
            Severity::Critical => C::CRITICAL_RED,
        };

        let line = format!("{}{}{}{}{}", indent, color, C::BOLD, wave, C::RESET);
        let end_col = indent_width + n_wave * wave_w;
        (line, end_col)
    }

    pub fn report_all(
        &self,
        diags: &[&dyn Diagnostic],
        manager: &SourceManager,
        out: &mut impl Write,
    ) {
        let mut warnings = 0u32;
        let mut errors = 0u32;

        for diag in diags {
            match diag.error_type().severity() {
                Severity::Warning => warnings += 1,
                _ => errors += 1,
            }
            self.report(*diag, manager, out);
        }

        writeln!(
            out,
            "{}{}{}",
            C::BOLD,
            self.fmt_summary(errors, warnings),
            C::RESET,
        )
        .unwrap();
    }

    // i18n

    fn txt_warning(&self) -> &str {
        match self.lang {
            core::Lang::zh_CN => "警告",
            core::Lang::en_US => "warning",
        }
    }
    fn txt_error(&self) -> &str {
        match self.lang {
            core::Lang::zh_CN => "错误",
            core::Lang::en_US => "error",
        }
    }
    fn txt_critical(&self) -> &str {
        match self.lang {
            core::Lang::zh_CN => "严重错误",
            core::Lang::en_US => "critical error",
        }
    }
    fn txt_suggestion(&self) -> &str {
        match self.lang {
            core::Lang::zh_CN => "建议",
            core::Lang::en_US => "suggestion",
        }
    }
    fn txt_note(&self) -> &str {
        match self.lang {
            core::Lang::zh_CN => "提示",
            core::Lang::en_US => "note",
        }
    }

    fn fmt_hint(&self, hint: &Hint) -> String {
        match hint {
            Hint::Insertion { content, .. } => match self.lang {
                core::Lang::zh_CN => format!(": 插入 `{}`", content),
                core::Lang::en_US => format!(": insert `{}`", content),
            },
            Hint::Deletion { length, .. } => match self.lang {
                core::Lang::zh_CN => format!(": 删除 {} 个字符", length),
                core::Lang::en_US => format!(": delete {} character(s)", length),
            },
            Hint::Replacement { content, .. } => match self.lang {
                core::Lang::zh_CN => format!(": 替换为 `{}`", content),
                core::Lang::en_US => format!(": replace with `{}`", content),
            },
        }
    }

    fn fmt_summary(&self, errors: u32, warnings: u32) -> String {
        match self.lang {
            core::Lang::zh_CN => format!("{} 个错误，{} 个警告", errors, warnings),
            core::Lang::en_US => format!("{} error(s), {} warning(s)", errors, warnings),
        }
    }
}

// 工具函数

fn padding(max: usize, n: usize) -> String {
    let w = format!("{}", n).len();
    " ".repeat(max.saturating_sub(w))
}

fn chars_count(s: &str) -> usize {
    s.chars().count()
}

fn take_chars(s: &str, n: usize) -> String {
    s.chars().take(n).collect()
}

fn take_chars_range(s: &str, start: usize, end: usize) -> String {
    s.chars()
        .skip(start)
        .take(end.saturating_sub(start))
        .collect()
}

/// 终端显示宽度：ASCII = 1，CJK 等宽字符 = 2，Tab = 4
fn char_width(c: char) -> usize {
    if c == '\t' {
        return 4;
    }
    if c.is_ascii() {
        return 1;
    }

    let cp = c as u32;
    // 宽字符区段
    if (0x1100..=0x115F).contains(&cp)   // Hangul Jamo
        || (0x2329..=0x232A).contains(&cp) // <>
        || (0x2E80..=0xA4CF).contains(&cp) // CJK Radicals .. CJK
        || (0xA960..=0xA97C).contains(&cp) // Hangul Jamo Extended-A
        || (0xAC00..=0xD7A3).contains(&cp) // Hangul Syllables
        || (0xF900..=0xFAFF).contains(&cp) // CJK Compatibility
        || (0xFE10..=0xFE6F).contains(&cp) // CJK Compatibility Forms
        || (0xFF01..=0xFF60).contains(&cp) // Fullwidth Forms
        || (0xFFE0..=0xFFE6).contains(&cp) // Fullwidth Signs
        || (0x1F300..=0x1F64F).contains(&cp) // Emoticons
        || (0x20000..=0x2FFFF).contains(&cp) // CJK Extension B+
        || (0x30000..=0x3FFFF).contains(&cp) // CJK Extension G+
        || cp >= 0x1F000
    // Emoji / Supplemental
    {
        return 2;
    }

    1
}

fn display_width(s: &str) -> usize {
    s.chars().map(char_width).sum()
}

// 测试

#[cfg(test)]
mod tests {
    use super::*;
    use crate::core::source::SourceFile;
    use crate::error::definitions::unterminated_string_literal::UnterminatedStringLiteralError;

    #[test]
    fn display_width_ascii() {
        assert_eq!(display_width("hello"), 5);
    }

    #[test]
    fn display_width_cjk() {
        assert_eq!(display_width("你好世界"), 8); // 4 chars × 2 width
    }

    #[test]
    fn display_width_mixed() {
        assert_eq!(display_width("var 你好"), 3 + 1 + 4); // "var" + " " + "你好" = 3 + 1 + 4 = 8
    }

    #[test]
    fn show_unterminated_string_zh_cn() {
        let source = "var x = \"hello\nvar y = \"world\n";
        let mut mgr = SourceManager::new();
        mgr.add_file(SourceFile::from_string("test.fig", source.to_string()));

        let err = UnterminatedStringLiteralError::new(0, 2, 9, 2, 14).with_thrower(crate::thrower!());
        let reporter = Reporter::new(core::Lang::zh_CN);
        let mut buf = String::new();
        reporter.report(&err, &mgr, &mut buf);
        println!("{buf}");
    }

    #[test]
    fn show_cjk_source() {
        let source = "让 你好 = \"你好世界\n";
        let mut mgr = SourceManager::new();
        mgr.add_file(SourceFile::from_string("test.fig", source.to_string()));

        let err = UnterminatedStringLiteralError::new(0, 1, 8, 1, 15).with_thrower(crate::thrower!());
        let reporter = Reporter::new(core::Lang::zh_CN);
        let mut buf = String::new();
        reporter.report(&err, &mgr, &mut buf);
        println!("{buf}");
    }

    #[test]
    fn show_unterminated_string_en_us() {
        let source = "var x = \"hello\n";
        let mut mgr = SourceManager::new();
        mgr.add_file(SourceFile::from_string("test.fig", source.to_string()));

        let err = UnterminatedStringLiteralError::new(0, 1, 9, 1, 14).with_thrower(crate::thrower!());
        let reporter = Reporter::new(core::Lang::en_US);
        let mut buf = String::new();
        reporter.report(&err, &mgr, &mut buf);
        println!("{buf}");
    }
}
