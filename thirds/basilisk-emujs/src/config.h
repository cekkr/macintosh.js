/* config.h for Basilisk II + Emscripten */

#ifndef CONFIG_H
#define CONFIG_H

/* Verifica presenza header ANSI C */
#define STDC_HEADERS 1

/* Definizioni per i tipi di dimensione fissa */
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_VOID_P 4

/* Define se hai questi header */
#define HAVE_STDINT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDLIB_H 1
#define HAVE_FCNTL_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_MMAN_H 1
#define TIME_WITH_SYS_TIME 1

/* General options */
#define VERSION "1.0"
#define PACKAGE "BasiliskII"
#define EMSCRIPTEN 1

/* SDL support */
#define HAVE_SDL 1
#define HAVE_SDL_AUDIO 1
#define HAVE_SDL_VIDEO 1

/* Addressing modes */
#define REAL_ADDRESSING 0
#define DIRECT_ADDRESSING 0
#define DIRECT_FIXED_ADDRESSING 0
#define FIXED_ADDRESSING 0
#define ADDRESSING_TEST_WORKS 1
#define ADDRESS_SPACE_24 1

/* Features disabled for web */
#define DISABLE_VOSF 1
#define DISABLE_XF86_DGA 1
#define DISABLE_XDND 1
#define DISABLE_GNOME 1
#define DISABLE_GTK 1

/* Networking */
#define HAVE_NETWORK 1
#define ETHERNET_DRIVER_WARN 0
#define BINCUE_WORKS 0
#define USE_FENV_H 0
#define USE_MACH_EXCEPTIONS 0

/* Thread support */
#define HAVE_PTHREADS 1
#define _REENTRANT 1
#define MULTIPROCESSING 0

/* System functions */
#define HAVE_MMAP 1
#define HAVE_MPROTECT 1
#define HAVE_SIGACTION 1
#define HAVE_SIGNAL_H 1
#define HAVE_NANOSLEEP 1
#define HAVE_STRTOUL 1
#define HAVE_STRTOULL 1
#define HAVE_GMTIME_R 1
#define HAVE_LOCALTIME_R 1
#define HAVE_CLOCK_GETTIME 1
#define HAVE_SBRK 0

/* Time handling */
#define HAVE_TIMER_CREATE 0
#define HAVE_TIMER_SETTIME 0

/* Define if <sys/stat.h> declares S_ISREG and S_ISSOCK */
#define HAVE_S_ISREG 1
#define HAVE_S_ISSOCK 1

/* JIT compiler options */
#define ENABLE_MON 1
#define ENABLE_JIT 0

/* BSD sockets */
#define HAVE_BSD_SOCKETS 1

/* Fork */
#define vfork fork

/* Include necessari header */
#include <stdint.h>
#include <sys/types.h>

/* Definizioni tipi fixed-size */
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

/* Definizioni tipi UAE */
typedef int8_t uae_s8;
typedef int16_t uae_s16;
typedef int32_t uae_s32;
typedef int64_t uae_s64;
typedef uint8_t uae_u8;
typedef uint16_t uae_u16;
typedef uint32_t uae_u32;
typedef uint64_t uae_u64;

#endif /* CONFIG_H */