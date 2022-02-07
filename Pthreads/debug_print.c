#include <stdio.h>
#include <stdarg.h>

#ifdef DEBUG
  #ifndef DEBUG_PRINT
    #define DEBUG_PRINT
    void debug_print(const char* format, ...) {
      va_list args;
      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);
    }
  #endif
#else
  #define debug_print(...) 
#endif
