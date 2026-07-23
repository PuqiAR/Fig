use std::process::Command;

fn main() {
    let hash = Command::new("git")
        .args(["rev-parse", "--short=7", "HEAD"])
        .output()
        .ok()
        .and_then(|o| String::from_utf8(o.stdout).ok())
        .unwrap_or_default()
        .trim()
        .to_string();

    println!("cargo:rustc-env=GIT_HASH={}", hash);
    println!("cargo:rustc-env=BUILD_TIME={}", chrono_build_time());

    // 编译器 ID：macOS 上用 clang/LLVM，Linux 上用 GCC，Windows 上用 MSVC
    #[cfg(target_os = "macos")]
    { println!("cargo:rustc-env=FIG_COMPILER_ID=LLVM"); }
    #[cfg(target_os = "linux")]
    { println!("cargo:rustc-env=FIG_COMPILER_ID=GCC"); }
    #[cfg(target_os = "windows")]
    { println!("cargo:rustc-env=FIG_COMPILER_ID=MSVC"); }
    #[cfg(not(any(target_os = "macos", target_os = "linux", target_os = "windows")))]
    { println!("cargo:rustc-env=FIG_COMPILER_ID=unknown"); }
}

// chrono 还没引入，先用简单方式
fn chrono_build_time() -> String {
    // RFC 3339 without nanoseconds: "2026-07-23T13:14:00+08:00"
    // 但纯 std 拿不到时区，用环境变量 SOURCE_DATE_EPOCH 或 UTC
    let now = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs();

    let secs = now % 86400;
    let days = now / 86400;

    // 简单计算 UTC 时间，避免引入 chrono
    let hour = (secs / 3600) % 24;
    let min  = (secs / 60) % 60;
    let sec  = secs % 60;

    // 粗略日期计算（从 1970-01-01 开始）
    let (y, mo, d) = civil_from_days(days as i64);

    format!("{y:04}-{mo:02}-{d:02} {hour:02}:{min:02}:{sec:02} UTC")
}

// 从 Unix epoch 天数反算年月日
fn civil_from_days(days: i64) -> (i64, u32, u32) {
    let z = days + 719468;
    let era = if z >= 0 { z } else { z - 146096 } / 146097;
    let doe = (z - era * 146097) as u32;
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let y = yoe as i64 + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = doy - (153 * mp + 2) / 5 + 1;
    let m = if mp < 10 { mp + 3 } else { mp - 9 };
    let y = if m <= 2 { y + 1 } else { y };
    (y, m, d)
}
