#include <stdlib.h>
#include <stdio.h>

#include "messages.h"

lasvm_message_t lasvm_message_level = LASVM_INFO;

lasvm_message_proc_t  *lasvm_message_proc = 0;


static void defaultproc(lasvm_message_t level, const char *fmt, va_list ap){
  if (level <= lasvm_message_level)
    vprintf(fmt, ap);
  if (level <= LASVM_ERROR) // start of ifdef LUSH
    {
      extern void run_time_error(const char *s);
      run_time_error("lasvm error");
    } // endif LUSH
  if (level <= LASVM_ERROR)
    abort();
}

void lasvm_error(const char *fmt, ...){ 
  lasvm_message_proc_t *f = lasvm_message_proc;
  va_list ap; 
  va_start(ap,fmt);
  if (! f) 
    f = defaultproc;
  (*f)(LASVM_ERROR,fmt,ap);
  va_end(ap); 
  abort();
}

void lasvm_warning(const char *fmt, ...){ 
  lasvm_message_proc_t *f = lasvm_message_proc;
  va_list ap; 
  va_start(ap,fmt);
  if (! f) 
    f = defaultproc;
  (*f)(LASVM_WARNING,fmt,ap);
  va_end(ap); 
}

void lasvm_info(const char *fmt, ...){ 
  lasvm_message_proc_t *f = lasvm_message_proc;
  va_list ap; 
  va_start(ap,fmt);
  if (! f) 
    f = defaultproc;
  (*f)(LASVM_INFO,fmt,ap);
  va_end(ap); 
}

void lasvm_debug(const char *fmt, ...){ 
  lasvm_message_proc_t *f = lasvm_message_proc;
  va_list ap; 
  va_start(ap,fmt);
  if (! f) 
    f = defaultproc;
  (*f)(LASVM_DEBUG,fmt,ap);
  va_end(ap); 
}

void lasvm_assertfail(const char *file,long line){
  lasvm_error("Assertion failed: %s:%ld\n", file, line);
}