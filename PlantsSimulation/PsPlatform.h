#pragma once
// Refactor (Phase 2.2): single location for platform-specific abstractions.
// Before this header, every file had its own #if __APPLE__ / #else / #endif
// blocks for include paths and string formatting. Now they include this header
// and use ps_sprintf(); include paths are normalized to forward slashes (MSVC
// accepts them since VS2005, so the Windows #else branch is no longer needed).

// ps_sprintf: bounds-checked formatted print on Windows, standard snprintf on Apple.
#if __APPLE__
#  define ps_sprintf(buf, len, fmt, ...) snprintf((buf), (len), (fmt), ##__VA_ARGS__)
#else
#  define ps_sprintf(buf, len, fmt, ...) sprintf_s((buf), (len), (fmt), ##__VA_ARGS__)
#endif
