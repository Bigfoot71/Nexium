/* NX_Log.cpp -- API definition for Nexium's log module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <SDL3/SDL_log.h>
#include <NX/NX_Log.h>

void NX_SetLogPriority(NX_LogLevel log)
{
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log);
}

void NX_Log(NX_LogLevel log, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
    va_end(args);
}

void NX_LogVA(NX_LogLevel log, const char* msg, va_list args)
{
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
}

void NX_LogT(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void NX_LogV(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void NX_LogD(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, msg, args);
    va_end(args);
}

void NX_LogI(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, msg, args);
    va_end(args);
}

void NX_LogW(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, msg, args);
    va_end(args);
}

void NX_LogE(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, msg, args);
    va_end(args);
}

void NX_LogF(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, msg, args);
    va_end(args);
}
