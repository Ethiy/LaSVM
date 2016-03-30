#ifndef MESSAGES_H
#define MESSAGES_H


#include <cstdarg>


/* Message levels */

typedef enum { LASVM_ERROR, 
	       LASVM_WARNING, 
	       LASVM_INFO, 
	       LASVM_DEBUG }  lasvm_message_t;


/* lasvm_message_level ---
   Default routine will print every message 
   more severe than this.
*/

extern lasvm_message_t lasvm_message_level;

/* lasvm_message_proc ---
   The routine to handle messages.
   The default routine will print everything
   with message level less than lasvm_message_level
   and abort when message level is LASVM_ERROR.
*/

typedef void lasvm_message_proc_t(lasvm_message_t, const char*, va_list);
extern lasvm_message_proc_t  *lasvm_message_proc;


/* lasvm_error, lasvm_warning, lasvm_info, lasvm_debug ---
   Convenience functions to display messages and signal errors.
*/
extern void lasvm_error(const char *fmt, ...);
extern void lasvm_warning(const char *fmt, ...);
extern void lasvm_info(const char *fmt, ...);
extern void lasvm_debug(const char *fmt, ...);


/* ASSERT --- 
   Macro for quickly inserting debugging checks.
*/

#ifdef NDEBUG
# define ASSERT(x) /**/
#else
# define ASSERT(x) do { if (!(x))\
                     lasvm_assertfail(__FILE__,__LINE__);\
                   } while(0)
#endif

void lasvm_assertfail(const char *file,unsigned long line);

#endif
