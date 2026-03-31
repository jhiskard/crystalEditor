#pragma once

#ifdef WB_TEST_NO_SPDLOG
#ifndef SPDLOG_TRACE
#define SPDLOG_TRACE(...) ((void)0)
#endif
#ifndef SPDLOG_DEBUG
#define SPDLOG_DEBUG(...) ((void)0)
#endif
#ifndef SPDLOG_INFO
#define SPDLOG_INFO(...) ((void)0)
#endif
#ifndef SPDLOG_WARN
#define SPDLOG_WARN(...) ((void)0)
#endif
#ifndef SPDLOG_ERROR
#define SPDLOG_ERROR(...) ((void)0)
#endif
#ifndef SPDLOG_CRITICAL
#define SPDLOG_CRITICAL(...) ((void)0)
#endif
#else
// SPDLOG_ACTIVE_LEVEL should be defined before first including <spdlog.h>
#ifdef DEBUG_BUILD
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#include <spdlog/spdlog.h>
#endif

// Log level (trace -> debug -> info -> warn -> error -> critical)
// - SPDLOG_TRACE: Trace message
// - SPDLOG_DEBUG: Debug message
// - SPDLOG_INFO: Information message
// - SPDLOG_WARN: Warning message
// - SPDLOG_ERROR: Error message
// - SPDLOG_CRITICAL: Critical message

// For debug build (Format: [timestamp] [level] [source:line] message)
// - SPDLOG_DEBUG, SPDLOG_INFO, SPDLOG_WARN, SPDLOG_ERROR and SPDLOG_CRITICAL can be used.
// - SPDLOG_TRACE is not used in debug build.

// For release build (Format: [timestamp] [level] message)
// - SPDLOG_INFO, SPDLOG_WARN, SPDLOG_ERROR and SPDLOG_CRITICAL can be used.
// - SPDLOG_DEBUG and SPDLOG_TRACE are not used in release build.
