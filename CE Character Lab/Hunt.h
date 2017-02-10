#pragma once

#include "g_shared.h"

#define csONWATER          0x00010000

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)

#include <windows.h>

inline void delay( unsigned long ms )
{
  Sleep( ms );
}

#else  /* presume POSIX */

#include <unistd.h>

inline void delay( useconds_t ms )
{
  usleep( ms * 1000 );
}

#endif



