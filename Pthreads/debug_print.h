#ifdef DEBUG
  #ifndef DEBUG_PRINT
    #define DEBUG_PRINT
    void debug_print(const char* format, ...);
  #endif
#else
  #ifndef DEBUG_PRINT
    #define DEBUG_PRINT
    #define debug_print(...) 
  #endif
#endif
