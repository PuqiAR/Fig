/*
    src/core/build_info.rs
    Part of The Fig Project, under the MIT License.
    Copyright (c) 2026, PuqiAR (im@puqiar.top) All rights reserved.
    See LICENSE for details.
*/

use crate::core::colors;

pub struct BuildInfo
{
    pub version:    &'static str,
    pub git_hash:   &'static str,
    pub build_time: &'static str,
    pub platform:   String,
}

pub fn get() -> BuildInfo
{
    BuildInfo
    {
        version:    env!("CARGO_PKG_VERSION"),
        git_hash:   env!("GIT_HASH"),
        build_time: env!("BUILD_TIME"),
        platform:   format!(
            "{} [{} | {}]",
            std::env::consts::OS,
            env!("FIG_COMPILER_ID"),
            std::env::consts::ARCH
        ),
    }
}

pub fn print_header(sub_system: &str)
{
    let info = get();
    print!(
        "{}{} v{}  {}(Build {}  {}  {}){}\n\n",
        colors::BOLD,
        sub_system,
        info.version,
        colors::DIM,
        info.git_hash,
        info.build_time,
        info.platform,
        colors::RESET,
    );
}
