/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
/* #undef ENABLE_NLS */

/* alsa library is present */
#define HAVE_ALSA 1

/* glibc has functions to provide stack backtrace */
#define HAVE_BACKTRACE 1

/* BPF IR decoder support enabled */
/* #undef HAVE_BPF */

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
/* #undef HAVE_DCGETTEXT */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Usage of DVBv5 remote enabled */
/* #undef HAVE_DVBV5_REMOTE */

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* whether we use libjpeg */
#define HAVE_JPEG 1

/* Define to 1 if you have the `klogctl' function. */
#define HAVE_KLOGCTL 1

/* Use libudev */
/* #undef HAVE_LIBUDEV */

/* whether to use libv4lconvert helpers */
#define HAVE_LIBV4LCONVERT_HELPERS 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Have ioctl with POSIX signature */
/* #undef HAVE_POSIX_IOCTL */

/* Define if you have POSIX threads libraries and header files. */
/* #undef HAVE_PTHREAD */

/* Have PTHREAD_PRIO_INHERIT. */
/* #undef HAVE_PTHREAD_PRIO_INHERIT */

/* qt has opengl support */
/* #undef HAVE_QTGL */

/* Define to 1 if you have the `secure_getenv' function. */
#define HAVE_SECURE_GETENV 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/klog.h> header file. */
#define HAVE_SYS_KLOG_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* V4L plugin support enabled */
/* #undef HAVE_V4L_PLUGINS */

/* Define to 1 or 0, depending whether the compiler supports simple visibility
   declarations. */
#define HAVE_VISIBILITY 1

/* Define to 1 if you have the `__secure_getenv' function. */
/* #undef HAVE___SECURE_GETENV */

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST 

/* ir-keytable preinstalled tables directory */
#define IR_KEYTABLE_SYSTEM_DIR "/lib/udev/rc_keymaps"

/* ir-keytable user defined tables directory */
#define IR_KEYTABLE_USER_DIR "/etc/rc_keymaps"

/* libdvbv5 domain */
#define LIBDVBV5_DOMAIN "libdvbv5"

/* libv4l1 private lib directory */
#define LIBV4L1_PRIV_DIR "/usr/lib/libv4l"

/* libv4l2 plugin directory */
#define LIBV4L2_PLUGIN_DIR "/usr/lib/libv4l/plugins"

/* libv4l2 private lib directory */
#define LIBV4L2_PRIV_DIR "/usr/lib/libv4l"

/* libv4lconvert private lib directory */
#define LIBV4LCONVERT_PRIV_DIR "/usr/lib/libv4l"

/* locale directory */
#define LOCALEDIR "/usr/share/locale"

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to 1 if `major', `minor', and `makedev' are declared in <mkdev.h>.
   */
/* #undef MAJOR_IN_MKDEV */

/* Define to 1 if `major', `minor', and `makedev' are declared in
   <sysmacros.h>. */
#define MAJOR_IN_SYSMACROS 1

/* Name of package */
#define PACKAGE "v4l-utils"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "v4l-utils"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "v4l-utils 1.20.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "v4l-utils"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.20.0"

/* Define to the type that is the result of default argument promotions of
   type mode_t. */
#define PROMOTED_MODE_T mode_t

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* v4l-utils version string */
#define V4L_UTILS_VERSION "1.20.0"

/* Version number of package */
#define VERSION "1.20.0"

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */
