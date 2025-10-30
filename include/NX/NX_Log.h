/* NX_Log.h -- API declaration for Nexium's log module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_LOG_H
#define NX_LOG_H

#include "./NX_API.h"
#include <stdarg.h>

// ============================================================================
// MACROS DEFINITIONS
// ============================================================================

/**
 * @brief Optional log macro that can be disabled at compile-time.
 *
 * This macro is used internally by Nexium and can also be used by the user.
 * Logs are only generated if NX_DISABLE_LOG is not defined.
 *
 * @note If NX_DISABLE_LOG is defined, calls to NX_LOG are completely removed at compile-time.
 * @note The `level` parameter selects which log function to call:
 *       T = Trace, V = Verbose, D = Debug, I = Info, W = Warning, E = Error, F = Fatal
 */
#ifndef NX_DISABLE_LOG
#	define NX_LOG(level, fmt, ...) NX_Log##level(fmt, ##__VA_ARGS__)
#else
#   define NX_LOG(level, fmt, ...) do { } while (0)
#endif

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Logging levels.
 *
 * Used to control verbosity and priority of log messages.
 * Functions like `NX_Log` and `NX_SetLogPriority` use these levels.
 */
typedef enum NX_LogLevel {
    NX_LOG_INVALID,         ///< Invalid log level
    NX_LOG_TRACE,           ///< Trace-level messages for detailed debugging
    NX_LOG_VERBOSE,         ///< Verbose messages
    NX_LOG_DEBUG,           ///< Debug messages
    NX_LOG_INFO,            ///< Informational messages
    NX_LOG_WARN,            ///< Warning messages
    NX_LOG_ERROR,           ///< Error messages
    NX_LOG_FATAL,           ///< Fatal error messages
    NX_LOG_COUNT            ///< Number of log levels
} NX_LogLevel;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Set the minimum log priority.
 * @param log Log level to set as minimum (messages below this level will be ignored).
 */
NXAPI void NX_SetLogPriority(NX_LogLevel log);

/**
 * @brief Log a message with the specified log level.
 * @param log Log level for this message.
 * @param msg Null-terminated format string.
 */
NXAPI void NX_Log(NX_LogLevel log, const char* msg, ...);

/**
 * @brief Log a message using a va_list.
 * @param log Log level for this message.
 * @param msg Null-terminated format string.
 * @param args Variable argument list.
 */
NXAPI void NX_LogVA(NX_LogLevel log, const char* msg, va_list args);

/** @brief Log a trace message */
NXAPI void NX_LogT(const char* msg, ...);

/** @brief Log a verbose message */
NXAPI void NX_LogV(const char* msg, ...);

/** @brief Log a debug message */
NXAPI void NX_LogD(const char* msg, ...);

/** @brief Log an info message */
NXAPI void NX_LogI(const char* msg, ...);

/** @brief Log a warning message */
NXAPI void NX_LogW(const char* msg, ...);

/** @brief Log an error message */
NXAPI void NX_LogE(const char* msg, ...);

/** @brief Log a fatal error message */
NXAPI void NX_LogF(const char* msg, ...);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_LOG_H
