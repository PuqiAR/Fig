/*
    src/core/colors.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

pub const RESET:  &str = "\x1b[0m";
pub const BOLD:   &str = "\x1b[1m";
pub const DIM:    &str = "\x1b[2m";

pub const RED:    &str = "\x1b[31m";
pub const GREEN:  &str = "\x1b[32m";
pub const YELLOW: &str = "\x1b[33m";
pub const CYAN:   &str = "\x1b[36m";

pub const BRIGHT_RED:  &str = "\x1b[91m";
pub const BRIGHT_CYAN: &str = "\x1b[96m";

pub const GRAY:        &str = "\x1b[90m";
pub const LIGHT_GRAY:  &str = "\x1b[37m";

pub const ORANGE:       &str = "\x1b[38;2;217;119;6m";
pub const PURPLE:       &str = "\x1b[38;2;168;85;247m";  // 亮紫色 error 主色
pub const CRITICAL_RED: &str = "\x1b[38;2;239;68;68m";  // critical
pub const ACCENT_BLUE:  &str = "\x1b[38;2;59;130;246m";

// 源码行号
pub const LINE_NO:      &str = "\x1b[38;2;59;130;246m";  // 蓝，和 error 区分
// 源码文本
pub const LIGHT_BLUE:   &str = "\x1b[38;2;96;165;250m";
pub const SOURCE_TEXT:  &str = "\x1b[38;2;180;180;180m"; // 浅灰，比 DIM 亮
