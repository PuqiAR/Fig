/*
    src/core/mod.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

pub mod source;
pub mod colors;
pub mod build_info;

#[derive(Debug, Clone, Copy)]
#[allow(non_camel_case_types)]
pub enum Lang
{
    en_US,
    zh_CN,
}