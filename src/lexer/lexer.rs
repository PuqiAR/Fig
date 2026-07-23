/*
    src/lexer/lexer.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::error::definitions::*;
use crate::error::diagnostic::Diagnostic;
use crate::token::TokenKind;
use crate::token;

pub struct SrcReader<'a> {
    source: &'a str,
    index: usize,

    line: usize,
    column: usize,
}

impl<'a> SrcReader<'a> {
    pub fn new(source: &'a str) -> Self {
        SrcReader {
            source,
            index: 0,
            line: 1,
            column: 1,
        }
    }

    pub fn read_char(&mut self) -> Option<char> {
        if self.index >= self.source.len() {
            return None;
        }

        let ch = self.source[self.index..].chars().next().unwrap();
        self.index += ch.len_utf8();

        match ch {
            '\n' => {
                self.line += 1;
                self.column = 1;
            }
            '\t' => {
                self.column += 4;
            }
            _ => {
                self.column += 1;
            }
        }

        Some(ch)
    }

    pub fn peek_char(&self) -> Option<char> {
        if self.index >= self.source.len() {
            return None;
        }
        Some(self.source[self.index..].chars().next().unwrap())
    }

    pub fn is_eof(&self) -> bool {
        self.index >= self.source.len()
    }
}

pub struct Lexer<'a> {
    file_name: &'a str,
    file_id: usize,
    reader: SrcReader<'a>,

    is_eof: bool,
}

impl<'a> Lexer<'a> {
    pub fn new(file_name: &'a str, file_id: usize, source: &'a str) -> Self {
        Lexer {
            file_name,
            file_id,
            reader: SrcReader::new(source),
            is_eof: false,
        }
    }

    pub fn next_token(&mut self) -> Result<token::Token, Box<dyn Diagnostic>> {
        if self.is_eof {
            panic!("Lexer has reached EOF, cannot read more tokens.");
        } else if self.reader.is_eof() {
            self.is_eof = true;
            return Ok(self.make_eof_token());
        }
        let line = self.reader.line;
        let column = self.reader.column;
        let ch = self.reader.read_char().unwrap();

        match ch {
            ' ' | '\t' | '\n' => self.next_token(),

            c if c.is_alphabetic() || c == '_' => self.parse_identifier(c),

            '"' => self.parse_string(line, column),

            'r' if self.reader.peek_char().is_some_and(|c| c == '"') => self.parse_raw_string(),

            /* 二进制、八进制、十六进制数字 严格 0b 0o 0x小写，具体数字部分字母大写 */
            '0' if self.reader.peek_char().is_some_and(|c| c == 'b') => self.parse_binint(),
            '0' if self.reader.peek_char().is_some_and(|c| c == 'o') => self.parse_octint(),
            '0' if self.reader.peek_char().is_some_and(|c| c == 'x') => self.parse_hexint(),

            c if c.is_ascii_digit() => self.parse_number(c),

            '/' if self.reader.peek_char().is_some_and(|c| c == '/') => {
                self.skip_line_comment();
                self.next_token()
            }
            '/' if self.reader.peek_char().is_some_and(|c| c == '*') => {
                self.parse_block_comment(line, column)
            }

            c if token::lookup_operator(&c.to_string()).is_some() => self.parse_operator(c),

            _ => Err(Box::new(
                unexpected_character::UnexpectedCharacterError::new(self.file_id, ch, line, column),
            )),
        }
    }

    fn parse_identifier(&mut self, first: char) -> Result<token::Token, Box<dyn Diagnostic>> {
        let mut tok = token::Token::new(
            TokenKind::Identifier,
            self.reader.index - first.len_utf8(),
            1,
        );
        let mut lexeme = String::from(first);
        while self
            .reader
            .peek_char()
            .is_some_and(|c| c.is_alphanumeric() || c == '_')
        {
            tok.length += 1;
            lexeme.push(self.reader.read_char().unwrap());
        }
        if let Some(keyword_type) = token::lookup_keyword(&lexeme) {
            Ok(token::Token::new(keyword_type, tok.index, tok.length))
        } else {
            Ok(tok)
        }
    }

    fn parse_string(
        &mut self,
        start_line: usize,
        start_column: usize,
    ) -> Result<token::Token, Box<dyn Diagnostic>> {
        let mut tok = token::Token::new(token::TokenKind::String, self.reader.index - 1, 1);
        let mut terminated = false;
        let mut end_line = start_line;
        let mut end_column = start_column + 1;

        while let Some(c) = self.reader.peek_char() {
            match c {
                '"' => {
                    end_line = self.reader.line;
                    end_column = self.reader.column + 1;
                    tok.length += 1;
                    self.reader.read_char();
                    terminated = true;
                    break;
                }
                '\\' => {
                    tok.length += 1;
                    self.reader.read_char();
                    if let Some(esc) = self.reader.read_char() {
                        tok.length += 1;
                        match esc {
                            'n' | 't' | 'r' | '\\' | '"' => {}
                            _ => {
                                return self.diag_newline(Box::new(
                                    invalid_escape_sequence::InvalidEscapeSequenceError::new(
                                        self.file_id,
                                        self.reader.line,
                                        self.reader.column - 2,
                                        esc.to_string(),
                                    ),
                                ));
                            }
                        }
                    } else {
                        break;
                    }
                }
                _ => {
                    end_line = self.reader.line;
                    end_column = self.reader.column;
                    tok.length += 1;
                    self.reader.read_char();
                }
            }
        }

        if !terminated {
            return self.diag_newline(Box::new(
                unterminated_string_literal::UnterminatedStringLiteralError::new(
                    self.file_id,
                    start_line,
                    start_column,
                    end_line,
                    end_column,
                ),
            ));
        }
        Ok(tok)
    }

    fn parse_raw_string(&mut self) -> Result<token::Token, Box<dyn Diagnostic>> {
        let line = self.reader.line;
        let column = self.reader.column;
        self.reader.read_char(); // 跳过 '"'
        match self.parse_string(line, column) {
            Ok(mut tok) => {
                tok.kind = token::TokenKind::RawString;
                Ok(tok)
            }
            Err(e) => Err(e),
        }
    }

    fn parse_number(&mut self, first: char) -> Result<token::Token, Box<dyn Diagnostic>> {
        let mut tok = token::Token::new(TokenKind::Number, self.reader.index - first.len_utf8(), 1);

        if first == '0' && self.reader.peek_char().is_some_and(|c| c.is_ascii_digit()) {
            return self.diag_newline(Box::new(
                invalid_number_literal::InvalidNumberLiteralError::new(
                    self.file_id,
                    self.reader.line,
                    self.reader.column - 1,
                    self.reader.column,
                ),
            ));
        }

        self.read_digits(&mut tok);

        if self.reader.peek_char().is_some_and(|c| c == '.') {
            tok.length += 1;
            self.reader.read_char();
            self.read_digits(&mut tok);
        }

        if self
            .reader
            .peek_char()
            .is_some_and(|c| c == 'e' || c == 'E')
        {
            tok.length += 1;
            self.reader.read_char();
            if self
                .reader
                .peek_char()
                .is_some_and(|c| c == '+' || c == '-')
            {
                tok.length += 1;
                self.reader.read_char();
            }
            match self.reader.peek_char() {
                Some('0') => {
                    return self.diag_newline(Box::new(
                        invalid_number_literal::InvalidNumberLiteralError::new(
                            self.file_id,
                            self.reader.line,
                            self.reader.column,
                            self.reader.column + 1,
                        ),
                    ));
                }
                Some(c) if c.is_ascii_digit() => {
                    self.read_digits(&mut tok);
                }
                _ => {
                    return self.diag_newline(Box::new(
                        invalid_number_literal::InvalidNumberLiteralError::new(
                            self.file_id,
                            self.reader.line,
                            self.reader.column,
                            self.reader.column + 1,
                        ),
                    ));
                }
            }
        }
        Ok(tok)
    }

    fn parse_binint(&mut self) -> Result<token::Token, Box<dyn Diagnostic>> {
        let start_column = self.reader.column - 1;
        let mut tok = token::Token::new(TokenKind::BinInt, self.reader.index - 1, 2);
        self.reader.read_char(); // 跳过 b

        while let Some(c) = self.reader.peek_char() {
            match c {
                '0' | '1' => {
                    tok.length += 1;
                    self.reader.read_char();
                }

                '2'..='9' => {
                    return self.diag_newline(Box::new(
                        invalid_number_literal::InvalidNumberLiteralError::new(
                            self.file_id,
                            self.reader.line,
                            start_column,
                            self.reader.column,
                        ),
                    ))
                }
                _ => {
                    break;
                }
            }
        }
        Ok(tok)
    }

    fn parse_octint(&mut self) -> Result<token::Token, Box<dyn Diagnostic>> {
        let start_column = self.reader.column - 1;
        let mut tok = token::Token::new(TokenKind::OctInt, self.reader.index - 1, 2);
        self.reader.read_char(); // 跳过 o

        while let Some(c) = self.reader.peek_char() {
            match c {
                '0'..='8' => {
                    tok.length += 1;
                    self.reader.read_char();
                }

                '9' => {
                    return self.diag_newline(Box::new(
                        invalid_number_literal::InvalidNumberLiteralError::new(
                            self.file_id,
                            self.reader.line,
                            start_column,
                            self.reader.column,
                        ),
                    ))
                }
                _ => {
                    break;
                }
            }
        }
        Ok(tok)
    }

    fn parse_hexint(&mut self) -> Result<token::Token, Box<dyn Diagnostic>> {
        let start_column = self.reader.column - 1;
        let mut tok = token::Token::new(TokenKind::HexInt, self.reader.index - 1, 2);
        self.reader.read_char(); // 跳过 x

        while let Some(c) = self.reader.peek_char() {
            match c {
                '0'..='9' | 'A'..='F' => {
                    tok.length += 1;
                    self.reader.read_char();
                }

                'G'..='Z' => {
                    return self.diag_newline(Box::new(
                        invalid_number_literal::InvalidNumberLiteralError::new(
                            self.file_id,
                            self.reader.line,
                            start_column,
                            self.reader.column,
                        ),
                    ))
                }

                _ => {
                    break;
                }
            }
        }
        Ok(tok)
    }

    fn parse_operator(&mut self, first: char) -> Result<token::Token, Box<dyn Diagnostic>> {
        let start = self.reader.index - 1; // first 一定是 ASCII，1 字节
        let rest = &self.reader.source[start..];

        for len in [2, 1] {
            if let Some(prefix) = rest.get(..len) {
                if let Some((kind, op_len)) = token::lookup_operator(prefix) {
                    for _ in 1..op_len {
                        self.reader.read_char();
                    }
                    return Ok(token::Token::new(kind, start, op_len as u32));
                }
            }
        }

        unreachable!("dispatcher already confirmed first char is an operator")
    }
    
    fn read_digits(&mut self, tok: &mut token::Token) {
        while self.reader.peek_char().is_some_and(|c| c.is_ascii_digit()) {
            tok.length += 1;
            self.reader.read_char();
        }
    }

    fn diag_newline(
        &mut self,
        diagnostic: Box<dyn Diagnostic>,
    ) -> Result<token::Token, Box<dyn Diagnostic>> {
        while !self.reader.is_eof() {
            let ch = self.reader.read_char().unwrap();
            if ch == '\n' {
                break;
            }
        }
        Err(diagnostic)
    }

    fn skip_line_comment(&mut self) {
        self.reader.read_char();
        while let Some(c) = self.reader.peek_char() {
            if c == '\n' {
                break;
            }
            self.reader.read_char();
        }
    }

    fn parse_block_comment(
        &mut self,
        start_line: usize,
        start_column: usize,
    ) -> Result<token::Token, Box<dyn Diagnostic>> {
        self.reader.read_char();
        let mut end_line = start_line;
        let mut end_column = start_column;
        while let Some(c) = self.reader.peek_char() {
            end_line = self.reader.line;
            end_column = self.reader.column;
            if c == '*' {
                self.reader.read_char();
                if self.reader.peek_char().is_some_and(|c| c == '/') {
                    self.reader.read_char();
                    return self.next_token();
                }
            } else {
                self.reader.read_char();
            }
        }
        self.diag_newline(Box::new(unterminated_comment::UnterminatedCommentError::new(
            self.file_id,
            start_line,
            start_column,
            end_line,
            end_column,
        )))
    }

    fn make_eof_token(&self) -> token::Token {
        token::Token::new(TokenKind::Eof, self.reader.index, 1)
    }

    fn diag_semicolon(
        &mut self,
        diagnostic: Box<dyn Diagnostic>,
    ) -> Result<token::Token, Box<dyn Diagnostic>> {
        while !self.reader.is_eof() {
            let ch = self.reader.read_char().unwrap();
            if ch == ';' {
                break;
            }
        }
        Err(diagnostic)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::core;
    use crate::core::source::{SourceFile, SourceManager};
    use crate::error::report::Reporter;
    use crate::token::TokenKind::*;
    use std::string::String as StdString;

    fn setup(src: &str) -> (SourceManager, Reporter) {
        let mut mgr = SourceManager::new();
        mgr.add_file(SourceFile::from_string("test", src.to_string()));
        (mgr, Reporter::new(core::Lang::zh_CN))
    }

    fn lex(source: &str) -> Vec<token::Token> {
        let (mgr, reporter) = setup(source);
        let mut lexer = Lexer::new("test", 0, source);
        let mut tokens = vec![];
        loop {
            match lexer.next_token() {
                Ok(tok) => {
                    let is_eof = tok.kind == Eof;
                    tokens.push(tok);
                    if is_eof {
                        break;
                    }
                }
                Err(e) => {
                    let mut buf = StdString::new();
                    reporter.report(e.as_ref(), &mgr, &mut buf);
                    println!("{buf}");
                    panic!("Lexer error in test");
                }
            }
        }
        tokens
    }

    fn kinds(tokens: &[token::Token]) -> Vec<token::TokenKind> {
        tokens.iter().map(|t| t.kind).collect()
    }

    #[test]
    fn keywords() {
        let tokens = lex("var const func if else return true false null");
        assert_eq!(
            kinds(&tokens),
            [Var, Const, Func, If, Else, Return, True, False, Null, Eof]
        );
    }

    #[test]
    fn identifiers() {
        let tokens = lex("foo _bar 你好 a1 b2_c3");
        let ks = kinds(&tokens);
        assert_eq!(ks[0], Identifier);
        assert_eq!(ks[1], Identifier);
        assert_eq!(ks[2], Identifier);
        assert_eq!(ks[3], Identifier);
        assert_eq!(ks[4], Identifier);
        assert_eq!(ks[5], Eof);
    }

    #[test]
    fn numbers() {
        let tokens = lex("42 3.14 5e2 0b1010 0o77 0xFF");
        assert_eq!(
            kinds(&tokens),
            [Number, Number, Number, BinInt, OctInt, HexInt, Eof]
        );
    }

    #[test]
    fn binint() {
        let tokens = lex("0b1010 0b0 0b11111111");
        assert_eq!(kinds(&tokens), [BinInt, BinInt, BinInt, Eof]);
    }

    #[test]
    fn octint() {
        let tokens = lex("0o777 0o0 0o1234567");
        assert_eq!(kinds(&tokens), [OctInt, OctInt, OctInt, Eof]);
    }

    #[test]
    fn hexint() {
        let tokens = lex("0xFF 0xDEAD 0xBEEF 0xA0");
        assert_eq!(kinds(&tokens), [HexInt, HexInt, HexInt, HexInt, Eof]);
    }

    #[test]
    fn binint_invalid_digit() {
        let (mgr, reporter) = setup("0b102");
        let mut lexer = Lexer::new("test", 0, "0b102");
        let mut buf = StdString::new();
        match lexer.next_token() {
            Err(e) => reporter.report(e.as_ref(), &mgr, &mut buf),
            _ => {}
        }
        println!("{buf}");
    }

    #[test]
    fn strings() {
        let tokens = lex(r#""hello" "world\n""#);
        assert_eq!(kinds(&tokens), [String, String, Eof]);
    }

    #[test]
    fn operators_single() {
        let tokens = lex("+ - * / % = < > & | ^ ~");
        assert_eq!(
            kinds(&tokens),
            [Plus, Minus, Star, Slash, Percent, Assign, Lt, Gt, Amp, Pipe, Caret, Tilde, Eof]
        );
    }

    #[test]
    fn operators_multi() {
        let tokens = lex("== != <= >= && || << >> ++ -- **");
        assert_eq!(
            kinds(&tokens),
            [EqEq, NotEq, LtEq, GtEq, AndAnd, OrOr, LtLt, GtGt, PlusPlus, MinusMinus, StarStar, Eof]
        );
    }

    #[test]
    fn operators_assign() {
        let tokens = lex("+= -= *= /= %= ^=");
        assert_eq!(
            kinds(&tokens),
            [PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign, CaretAssign, Eof]
        );
    }

    #[test]
    fn operators_longest_match() {
        // (x = 42) 中 = 不能和 == 混淆
        let tokens = lex("x = 42 x == 42");
        assert_eq!(
            kinds(&tokens),
            [Identifier, Assign, Number, Identifier, EqEq, Number, Eof]
        );
    }

    #[test]
    fn operators_pair() {
        let tokens = lex("() {} []");
        assert_eq!(
            kinds(&tokens),
            [LParen, RParen, LBrace, RBrace, LBracket, RBracket, Eof]
        );
    }

    #[test]
    fn operators_closing_only() {
        let tokens = lex(") } ]");
        assert_eq!(kinds(&tokens), [RParen, RBrace, RBracket, Eof]);
    }

    #[test]
    fn operators_delimiters() {
        let tokens = lex("() {} [] , ; : . -> => ? @ # $");
        assert_eq!(
            kinds(&tokens),
            [
                LParen, RParen,
                LBrace, RBrace,
                LBracket, RBracket,
                Comma, Semicolon, Colon, Dot,
                Arrow, FatArrow,
                Question, At, Hash, Dollar,
                Eof,
            ]
        );
    }

    #[test]
    fn unterminated_string_errors() {
        let source = "\"hello\n";
        let (mgr, reporter) = setup(source);
        let mut lexer = Lexer::new("test", 0, source);
        let mut buf = StdString::new();
        match lexer.next_token() {
            Err(e) => reporter.report(e.as_ref(), &mgr, &mut buf),
            _ => {}
        }
        println!("{buf}");
        let tok = lexer.next_token().unwrap();
        assert_eq!(tok.kind, Eof);
    }

    #[test]
    fn line_comment() {
        let tokens = lex("42 // this is a comment\n99");
        assert_eq!(kinds(&tokens), [Number, Number, Eof]);
    }

    #[test]
    fn block_comment() {
        let tokens = lex("42 /* block */ 99");
        assert_eq!(kinds(&tokens), [Number, Number, Eof]);
    }

    #[test]
    fn block_comment_multiline() {
        let tokens = lex("42 /* line 1\nline 2\n*/ 99");
        assert_eq!(kinds(&tokens), [Number, Number, Eof]);
    }

    #[test]
    fn block_comment_unterminated() {
        let source = "42 /* never closed";
        let (mgr, reporter) = setup(source);
        let mut lexer = Lexer::new("test", 0, source);
        let mut buf = StdString::new();
        while let Ok(tok) = lexer.next_token() {
            if tok.kind == Eof { break; }
        }
        // error was reported via diag_newline, which recovers to newline
        // verify the error system captured it (we don't assert output, just no panic)
    }

    #[test]
    fn comment_inside_code() {
        let source = r#"
var x = 1 // inline comment
/* multi
   line */
var y = 2
"#;
        let (mgr, reporter) = setup(source);
        let mut lexer = Lexer::new("test", 0, source);
        loop {
            match lexer.next_token() {
                Ok(tok) => {
                    if tok.kind == Eof { break; }
                }
                Err(_) => {}
            }
        }
    }

    #[test]
    fn edge_empty() {
        // 空源码 → 只有 Eof
        let tokens = lex("");
        assert_eq!(kinds(&tokens), [Eof]);
    }

    #[test]
    fn edge_whitespace_only() {
        // 纯空白
        let tokens = lex("   \n  \t  \n  ");
        assert_eq!(kinds(&tokens), [Eof]);
    }

    #[test]
    fn edge_single_chars() {
        let tokens = lex("+ - * / % = < > & | ^ ~ ! . , ; : ? @ # $ ( ) { } [ ]");
        assert_eq!(kinds(&tokens).len(), 28); // 27 tokens + Eof
    }

    #[test]
    fn edge_eof_after_token_start() {
        let (_, reporter) = setup("\"eof");
        let mut lexer = Lexer::new("test", 0, "\"eof");
        assert!(lexer.next_token().is_err());
        assert_eq!(lexer.next_token().unwrap().kind, Eof);
    }

    #[test]
    fn edge_consecutive_operators() {
        let tokens = lex("+-*/%&&||==!=<><=>=->=>::++--**+=-=*=/=%=^=");
        assert!(kinds(&tokens).len() > 20);
    }

    #[test]
    fn edge_operator_at_eof() {
        let tokens = lex("42+");
        assert_eq!(kinds(&tokens), [Number, Plus, Eof]);
    }

    #[test]
    fn edge_line_comment_at_eof() {
        let tokens = lex("42// no newline");
        assert_eq!(kinds(&tokens), [Number, Eof]);
    }

    #[test]
    fn edge_block_comment_at_eof() {
        let mut lexer = Lexer::new("test", 0, "42/* no close");
        let mut kinds = vec![];
        loop {
            match lexer.next_token() {
                Ok(tok) => {
                    kinds.push(tok.kind);
                    if tok.kind == Eof { break; }
                }
                Err(_) => {}
            }
        }
        assert_eq!(&kinds, &[Number, Eof]);
    }

    #[test]
    fn edge_zero_at_eof() {
        let tokens = lex("x >= 0");
        assert_eq!(kinds(&tokens), [Identifier, GtEq, Number, Eof]);
    }

    #[test]
    fn edge_empty_prefix_number() {
        let tokens = lex("0b 0x 0o");
        assert_eq!(kinds(&tokens), [BinInt, HexInt, OctInt, Eof]);
    }

    #[test]
    fn edge_empty_raw_string() {
        let mut lexer = Lexer::new("test", 0, r###"r"""###);
        let tok = lexer.next_token().unwrap();
        println!("{:?}", tok.kind);
    }

    #[test]
    fn edge_star_not_comment() {
        let tokens = lex("2 * 3");
        assert_eq!(kinds(&tokens), [Number, Star, Number, Eof]);
    }

    #[test]
    fn edge_slash_not_comment() {
        let tokens = lex("2 / 3");
        assert_eq!(kinds(&tokens), [Number, Slash, Number, Eof]);
    }

    #[test]
    fn edge_block_comment_star_only() {
        let source = "42 /* * * *";
        let mut lexer = Lexer::new("test", 0, source);
        let mut tokens = vec![];
        loop {
            match lexer.next_token() {
                Ok(tok) => {
                    let is_eof = tok.kind == Eof;
                    tokens.push(tok.kind);
                    if is_eof { break; }
                }
                Err(_) => {} // unterminated comment, recovered
            }
        }
        assert_eq!(&tokens, &[Number, Eof]);
    }

    #[test]
    fn stress_1000_lines() {
        let mut source = StdString::new();
        // 生成约 1000 行覆盖全 token 类型的源码
        for i in 0..60 {
            let hex = format!("0x{:X}", i * 7);
            let oct = format!("0o{:o}", i);
            let bin = format!("0b{:b}", i % 256);
            source.push_str(&format!(
                "var v{i} = {i}\n\
                 const c{i} = {i}.{i}\n\
                 let s{i} = \"str {i} 你好\"\n\
                 // line comment {i}\n\
                 func f{i}(a{i}, b{i}) {{\n\
                     /* block comment {i} */\n\
                     if a{i} != {i} {{\n\
                         return a{i} + b{i} * {i} - {hex} | {oct}\n\
                     }} else {{\n\
                         return c{i} >= {i} && v{i} <= {i} || f{i}(a{i}, b{i})\n\
                     }}\n\
                 }}\n\
                 v{i} += {i}\n\
                 v{i} **= 2\n\
                 let t{i} = true && !false\n\
                 let n{i} = !{i}.0 + 0xFF\n\
                 let arr{i} = [? # $ @ -> =>]\n\
             "
            ));
        }
        // 添加故意错误行（错误恢复验证）
        source.push_str("var bad = \"never close this\n");
        source.push_str("/* unterminated block comment\n");
        source.push_str("var after_comment = 42\n");

        let (mgr, reporter) = setup(&source);
        let mut lexer = Lexer::new("stress", 0, &source);
        let mut tokens = 0usize;
        let mut errors = 0usize;
        loop {
            match lexer.next_token() {
                Ok(tok) => {
                    tokens += 1;
                    if tok.kind == Eof { break; }
                }
                Err(e) => {
                    errors += 1;
                    let mut buf = StdString::new();
                    reporter.report(e.as_ref(), &mgr, &mut buf);
                    println!("{buf}");
                }
            }
        }
        let lines = source.lines().count();
        println!(
            "stress: {} lines => {} tokens, {} errors recovered",
            lines, tokens, errors
        );
        assert!(tokens > 2000, "expected >2000 tokens, got {tokens}");
        assert!(lines > 800, "expected >800 lines, got {lines}");
    }

    #[test]
    fn stress_test() {
        let source = r#"
var 你好 = "hello 世界"
const MAX = 0xFF
func fib(n) {
    if n <= 1 {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}
let x = 42 + 3.14
let y = x * 2 - 1
let flag = true && !false
let bits = 0b1010 | 0o77
let result = (x >= 0) && (x != 42)
x += 1
y -= 1
x **= 2
-> => ? @ # $
"#;
        let (mgr, reporter) = setup(source);
        let mut lexer = Lexer::new("test", 0, source);
        let mut count = 0usize;
        loop {
            match lexer.next_token() {
                Ok(tok) => {
                    let text: StdString = source[tok.index..]
                        .chars()
                        .take(tok.length as usize)
                        .collect();
                    count += 1;
                    println!("{:>3}  {:?} '{}'", count, tok.kind, text);
                    if tok.kind == Eof {
                        break;
                    }
                }
                Err(e) => {
                    let mut buf = StdString::new();
                    reporter.report(e.as_ref(), &mgr, &mut buf);
                    println!("{buf}");
                }
            }
        }
    }
}
