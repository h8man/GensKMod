#ifndef _GDB_DEBUG_H_
#define _GDB_DEBUG_H_

#ifdef _DEBUG

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif /* _WIN32 */

#include <stdio.h>
#include <stdarg.h>

// extern "C" void __stdcall OutputDebugStringA(const char * str);

static void debug_printf(const char * format, ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    OutputDebugStringA(buffer);
}

#else

static void debug_printf(const char * format, ...)
{

}

#endif /* _DEBUG */

#endif _GDB_DEBUG_H_
