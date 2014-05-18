/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003-2005  Roman Senn <smoli@paranoya.ch>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 * 
 * $Id: syscall.h,v 1.33 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include <libchaos/defs.h>

#if 0 //(defined __linux) && (defined __i386__)

/*extern const char *syscall_strerror(int errno);
extern int syscall_errno;*/

#ifndef __USE_LARGEFILE64
# define __USE_LARGEFILE64 1
#else
# undef __USE_LARGEFILE64
# define __USE_LARGEFILE64 1
#endif 

#define _SIGNAL_H

/*#undef errno
#define errno syscall_errno*/

#include <stdarg.h>
#include <stdint.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/resource.h>
#undef WNOHANG
#undef WUNTRACED
#include <linux/net.h>
#undef WUNTRACED
#include <linux/unistd.h>
#undef errno
#include <linux/errno.h>
/*#undef errno
#define errno syscall_errno*/

#ifndef __NR__exit
#define __NR__exit __NR_exit
#endif /* __NR__exit */

#define psyscall0(type,  name) \
        type name()

#define psyscall1(type,  name, \
                  type1, arg1) \
        type name(type1  arg1)

#define psyscall2(type,  name, \
                  type1, arg1, \
                  type2, arg2) \
        type name(type1  arg1, \
                  type2  arg2)

#define psyscall3(type,  name, \
                  type1, arg1, \
                  type2, arg2, \
                  type3, arg3) \
        type name(type1  arg1, \
                  type2  arg2, \
                  type3  arg3)

#define psyscall4(type,  name, \
                  type1, arg1, \
                  type2, arg2, \
                  type3, arg3, \
                  type4, arg4) \
        type name(type1  arg1, \
                  type2  arg2, \
                  type3  arg3, \
                  type4  arg4)

#define psyscall5(type,  name, \
                  type1, arg1, \
                  type2, arg2, \
                  type3, arg3, \
                  type4, arg4, \
                  type5, arg5) \
        type name(type1  arg1, \
                  type2  arg2, \
                  type3  arg3, \
                  type4  arg4, \
                  type5  arg5)

#define psyscall6(type,  name, \
                  type1, arg1, \
                  type2, arg2, \
                  type3, arg3, \
                  type4, arg4, \
                  type5, arg5, \
                  type6, arg6) \
        type name(type1  arg1, \
                  type2  arg2, \
                  type3  arg3, \
                  type4  arg4, \
                  type5  arg5, \
                  type6  arg6)

#define psocketcall2(type,  name, \
                     type1, arg1, \
                     type2, arg2) \
        type name   (type1  arg1, \
                     type2  arg2)

#define _socketcall2(num, \
                     type,  name, \
                     type1, arg1, \
                     type2, arg2) \
        psocketcall2(type,  name, \
                     type1, arg1, \
                     type2, arg2) \
{ \
uint32_t __res, args[] = { (uint32_t)arg1, (uint32_t)arg2 }; \
__asm__ volatile("int $0x80" \
                 : "=a" (__res) \
                 : "0" (__NR_socketcall), \
                   "b" ((uint32_t)num), \
                   "c" ((uint32_t)args)); \
__syscall_return(type, __res); \
}
                     
#define psocketcall3(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3) \
        type name   (type1  arg1, \
                     type2  arg2, \
                     type3  arg3)

#define _socketcall3(num, \
                     type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3) \
        psocketcall3(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3) \
{ \
uint32_t __res, args[] = { (uint32_t)arg1, (uint32_t)arg2, (uint32_t)arg3 }; \
__asm__ volatile("int $0x80" \
                 : "=a" (__res) \
                 : "0" (__NR_socketcall), \
                   "b" ((uint32_t)num), \
                   "c" ((uint32_t)args)); \
__syscall_return(type, __res); \
}

#define psocketcall4(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4) \
        type name   (type1  arg1, \
                     type2  arg2, \
                     type3  arg3, \
                     type4  arg4)

#define _socketcall4(num, \
                     type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4) \
        psocketcall4(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4) \
{ \
uint32_t __res, args[] = { (uint32_t)arg1, (uint32_t)arg2, (uint32_t)arg3, (uint32_t)arg4 }; \
__asm__ volatile("int $0x80" \
                 : "=a" (__res) \
                 : "0" (__NR_socketcall), \
                   "b" ((uint32_t)num), \
                   "c" ((uint32_t)args)); \
__syscall_return(type, __res); \
}

#define psocketcall5(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4, \
                     type5, arg5) \
        type name   (type1  arg1, \
                     type2  arg2, \
                     type3  arg3, \
                     type4  arg4, \
                     type5  arg5)

#define _socketcall5(num, \
                     type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4, \
                     type5, arg5) \
        psocketcall5(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4, \
                     type5, arg5) \
{ \
uint32_t __res, args[] = { (uint32_t)arg1, (uint32_t)arg2, (uint32_t)arg3, (uint32_t)arg4, (uint32_t)arg5 }; \
__asm__ volatile("int $0x80" \
                 : "=a" (__res) \
                 : "0" (__NR_socketcall), \
                   "b" ((uint32_t)num), \
                   "c" ((uint32_t)args)); \
__syscall_return(type, __res); \
}

#define poldmmapcall(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4, \
                     type5, arg5, \
                     type6, arg6) \
        type name   (type1  arg1, \
                     type2  arg2, \
                     type3  arg3, \
                     type4  arg4, \
                     type5  arg5, \
                     type6  arg6)

#define _oldmmapcall(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4, \
                     type5, arg5, \
                     type6, arg6) \
        poldmmapcall(type,  name, \
                     type1, arg1, \
                     type2, arg2, \
                     type3, arg3, \
                     type4, arg4, \
                     type5, arg5, \
                     type6, arg6) \
{ \
uint32_t __res, args[] = { (uint32_t)arg1, (uint32_t)arg2, (uint32_t)arg3, \
                           (uint32_t)arg4, (uint32_t)arg5, (uint32_t)arg6 }; \
__asm__ volatile("int $0x80" \
                 : "=a" (__res) \
                 : "0" (__NR_mmap), \
                   "b" ((uint32_t)args)); \
__syscall_return(type, __res); \
}

#if (defined USE_SELECT)
#undef __NR_select
#define __NR_select             __NR__newselect
#define __NR_syscall_select       __NR_select
#elif (defined USE_POLL)
#define __NR_syscall_poll         __NR_poll
#endif /* USE_SELECT || USE_POLL */

#define __NR_syscall_close        __NR_close
#define __NR_syscall_exit         __NR_exit
#define __NR_syscall_fork         __NR_fork
#define __NR_syscall_setsid       __NR_setsid
#define __NR_syscall_pipe         __NR_pipe
#define __NR_syscall_gettimeofday __NR_gettimeofday
#define __NR_syscall_kill         __NR_kill
#define __NR_syscall_read         __NR_read
#define __NR_syscall_write        __NR_write
#define __NR_syscall__newselect   __NR__newselect
#define __NR_syscall_mmap         __NR_mmap
#define __NR_syscall_munmap       __NR_munmap
#define __NR_syscall_mprotect     __NR_mprotect
#define __NR_syscall_stat         __NR_stat
#define __NR_syscall_fstat        __NR_fstat
#define __NR_syscall_socketcall   __NR_socketcall
#define __NR_syscall_dup2         __NR_dup2
#define __NR_syscall_signal       __NR_signal
#define __NR_syscall_execve       __NR_execve
#define __NR_syscall_poll         __NR_poll
#define __NR_syscall_waitpid      __NR_waitpid
#define __NR_syscall_chdir        __NR_chdir
#define __NR_syscall_setrlimit    __NR_setrlimit
#define __NR_syscall_getpid       __NR_getpid
#define __NR_syscall_unlink       __NR_unlink
#define __NR_syscall_readlink     __NR_readlink

#undef PIC

/* -------------------------------------------------------------------------- *
 * int fork()                                                                 *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall0(int,              syscall_fork);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall0(int,              syscall_fork);

/* -------------------------------------------------------------------------- *
 * int setsid()                                                               *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall0(int,              syscall_setsid);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall0(int,              syscall_setsid);

/* -------------------------------------------------------------------------- *
 * int getpid()                                                               *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall0(pid_t,            syscall_getpid);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall0(pid_t,            syscall_getpid);

/* -------------------------------------------------------------------------- *
 * int close(int fd)                                                          *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall1(int,              syscall_close,     /* non-negative on error */
          int,              fd);             /* fd to close */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall1(int,              syscall_close,     /* non-negative on error */
          int,              fd);             /* fd to close */

/* -------------------------------------------------------------------------- *
 * int unlink(const char *path)                                               *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall1(int,              syscall_unlink, 
          const char *,     path);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall1(int,              syscall_unlink,
          const char *,     path);

/* -------------------------------------------------------------------------- *
 * int pipe(int filedes[2])                                                   *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall1(int,              syscall_pipe, 
          int *,            filedes); 
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall1(int,              syscall_pipe,
          int *,            filedes);

/* -------------------------------------------------------------------------- *
 * void _exit(int status)                                                     *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall1(int,              syscall_exit,
          int,              status);         /* process exit status */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall1(int,              syscall_exit,
          int,              status);         /* process exit status */

/* -------------------------------------------------------------------------- *
 * int chdir(const char *path)                                                *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall1(int,               syscall_chdir,
          const char *,      path);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall1(int,               syscall_chdir,
          const char *,      path);

/* -------------------------------------------------------------------------- *
 * int kill(pid_t pid, int sig);                                              *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall2(int,                  syscall_kill,  /* -1 on error */
          pid_t,                pid,
          int,                  sig);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,                  syscall_kill,  /* -1 on error */
          pid_t,                pid,
          int,                  sig);

/* -------------------------------------------------------------------------- *
 * int gettimeofday(struct timeval *tv, struct timezone *tz);                 *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall2(int,                  syscall_gettimeofday,/* -1 on error */
          struct timeval *,     tv,          /* unixtime in microsecs */
          struct timezone *,    tz);         /* timezone information */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,                  syscall_gettimeofday,/* -1 on error */
          struct timeval *,     tv,          /* unixtime in microsecs */
          struct timezone *,    tz);         /* timezone information */

/* -------------------------------------------------------------------------- *
 * int munmap(void *start, size_t length)                                     *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall2(int,                  syscall_munmap,/* -1 on error */
          void *,               start,
          size_t,               length);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,                  syscall_munmap,      /* -1 on error */
          void *,               start,
          size_t,               length);

/* -------------------------------------------------------------------------- *
 * int stat(const char *file_name, struct stat *buf)                          *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall2(int,                  syscall_stat,/* -1 on error */
          const char *,         file_name,
          struct stat  *,       buf);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,                  syscall_stat,/* -1 on error */
          const char *,         file_name,
          struct stat *,        buf);

/* -------------------------------------------------------------------------- *
 * int fstat(int fd, struct stat *buf)                                        *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall2(int,                  syscall_fstat,/* -1 on error */
          int,                  fd,
          struct stat *,        buf);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,                  syscall_fstat,/* -1 on error */
          int,                  fd,
          struct stat *,        buf);

/* -------------------------------------------------------------------------- *
 * int dup2(int oldfd, int newfd)                                             *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern 
psyscall2(int,               syscall_dup2,  /* -1 on error */
          int,               oldfd,
          int,               newfd);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,               syscall_dup2,  /* -1 on error */
          int,               oldfd,
          int,               newfd);

/* -------------------------------------------------------------------------- *
 * int signal(int signum, sighandler_t handler)                               *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern 
psyscall2(int,               syscall_signal,        /* -1 on error */
          int,               signum,
          void *,            handler);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,               syscall_signal,        /* -1 on error */
          int,               signum,
          void *,            handler);

/* -------------------------------------------------------------------------- *
 * int setrlimit(int resource, const struct rlimit *rlim)                     *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern 
psyscall2(int,                   syscall_setrlimit,     /* -1 on error */
          int,                   resource,
          const struct rlimit *, rlim);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall2(int,                   syscall_setrlimit,     /* -1 on error */
          int,                   resource,
          const struct rlimit *, rlim);

/* -------------------------------------------------------------------------- *
 * int execve(const char *filename, char *const argv[], char *const envp[]);  *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(int,               syscall_execve,
          const char *,      filename,         
          char *const *,     argv,
          char *const *,     envp);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(int,               syscall_execve,
          const char *,      filename,         
          char *const *,     argv,
          char *const *,     envp);

/* -------------------------------------------------------------------------- *
 * int waitpid(pid_t pid, int *status, int options);                          *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(int,               syscall_waitpid,
          pid_t,             pid,
          int *,             status,
          int,               options);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(int,               syscall_waitpid,
          pid_t,             pid,
          int *,             status,
          int,               options);

/* -------------------------------------------------------------------------- *
 * ssize_t read(int fd, void *buf, size_t count)                              *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(ssize_t,              syscall_read,  /* 0 on EOF, -1 on error */
          int,                  fd,          /* source fd */
          void *,               buf,         /* destination buffer */
          size_t,               count);      /* byte count */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(ssize_t,              syscall_read,  /* 0 on EOF, -1 on error */
          int,                  fd,          /* source fd */
          void *,               buf,         /* destination buffer */
          size_t,               count);      /* byte count */

/* -------------------------------------------------------------------------- *
 * ssize_t write(int fd, const void *buf, size_t count)                       *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(ssize_t,              syscall_write, /* -1 on error, else bytes written */
          int,                  fd,          /* destination fd */
          const void *,         buf,         /* source buffer */
          size_t,               count);      /* byte count */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(ssize_t,              syscall_write, /* -1 on error, else bytes written */
          int,                  fd,          /* destination fd */
          const void *,         buf,         /* source buffer */
          size_t,               count);      /* byte count */

/* -------------------------------------------------------------------------- *
 * int poll(struct pollfd *ufds, unsigned int nfds, int mtimeout)             *
 * -------------------------------------------------------------------------- */
#ifdef USE_POLL
#ifndef PIC
extern
psyscall3(int,                  syscall_poll,  /* -1 on error, else active fds */
          struct pollfd *,      ufds,        /* pollfd array */
          unsigned long int,    nfds,        /* size of array */
          int,                  mtimeout);   /* milisecond timeout */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(int,                  syscall_poll,  /* -1 on error, else active fds */
          struct pollfd *,      ufds,        /* pollfd array */
          unsigned long int,    nfds,        /* size of array */
          int,                  mtimeout);   /* milisecond timeout */
#endif /* USE_POLL */
/* -------------------------------------------------------------------------- *
 * int open(const char *pathname, int flags, ...)                             *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(int,                  syscall_open,  /* -1 on error, else valid fd */
          const char *,         pathname,    /* path to file to open */
          int,                  flags,       /* flags for new fd */
          ...,);                             /* modes if file gets created */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
psyscall3(int,                  syscall_open,  /* -1 on error, else valid fd */
          const char *,         pathname,    /* path to file to open */
          int,                  flags,       /* flags for new fd */
          ...,)                              /* modes if file gets created */
#ifdef __i386__
{
  int     res;
  va_list args;
  
  va_start(args, flags);
  
  __asm__ volatile ("int $0x80"
                    : "=a" (res)
                    : "0" ((long)__NR_open),
                      "b" ((long)pathname),
                      "c" ((long)flags),
                      "d" (va_arg(args, long)));
  va_end(args);
  
  if((unsigned long)res >= (unsigned long)-125)
  {
    errno = -res;
    res = -1;
  }

  return (int)res;
};
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * int fcntl64(int fd, int cmd, ...)                                          *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(int,                  syscall_fcntl,
          int,                  fd,
          int,                  cmd,
          ...,);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
psyscall3(int,                  syscall_fcntl,
          int,                  fd,
          int,                  cmd,
          ...,)
#ifdef __i386__
{
  int res;
  long arg;
  
  arg = (&cmd)[1];

  __asm__ volatile ("int $0x80"
                    : "=a" (res)
                    : "0" ((long)__NR_fcntl),
                      "b" ((long)fd),
                      "c" ((long)cmd),
                      "d" ((long)arg));

  if((unsigned long)res >= (unsigned long)-125)
  {
    errno = -res;
    res = -1;
  }

  return (int)res;
};
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * int readlink(const char *addr, char *buf, size_t n)                        *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(int,                  syscall_readlink,   /* -1 on error */
          const char *,         path,
          char *,               buf,
          size_t,               n);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(int,                  syscall_readlink,   /* -1 on error */
          const char *,         path,
          char *,               buf,
          size_t,               n);

/* -------------------------------------------------------------------------- *
 * int mprotect(const void *addr, size_t len, int prot)                       *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psyscall3(int,                  syscall_mprotect,/* -1 on error */
          const void *,         addr,
          size_t,               len,
          int,                  prot);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall3(int,                  syscall_mprotect,/* -1 on error */
          const void *,         addr,
          size_t,               len,
          int,                  prot);

/* -------------------------------------------------------------------------- *
 * int select(int, fd_set *, fd_set *, fd_set *, struct timeval *)            *
 * -------------------------------------------------------------------------- */
#ifdef USE_SELECT
#ifndef PIC
extern
psyscall5(int,                  syscall_select,
          int,                  n,           /* highest fd + 1 */
          fd_set *,             readfds,     /* fd_set for read events */
          fd_set *,             writefds,    /* fd_set for write events */
          fd_set *,             exceptfds,   /* fd_set for exceptions */
          struct timeval *,     timeout);    /* timeout value */
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_syscall5(int,                  syscall_select,
          int,                  n,           /* highest fd + 1 */
          fd_set *,             readfds,     /* fd_set for read events */
          fd_set *,             writefds,    /* fd_set for write events */
          fd_set *,             exceptfds,   /* fd_set for exceptions */
          struct timeval *,     timeout);    /* timeout value */
#endif /* USE_SELECT */
/* -------------------------------------------------------------------------- *
 * void *mmap(void *start, size_t length, int prot,                           *
 *            int flags, int fd, off_t offset)                                *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
poldmmapcall(void *,               syscall_mmap,
             void *,               start,
             size_t,               length,
             int,                  prot,
             int,                  flags,
             int,                  fd,
             off_t,                offset);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_oldmmapcall(void *,               syscall_mmap,
             void *,               start,
             size_t,               length,
             int,                  prot,
             int,                  flags,
             int,                  fd,
             off_t,                offset);

/* -------------------------------------------------------------------------- *
 * int socket(int domain, int type, int protocol);                            *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall3(int,               syscall_socket,
             int,               domain,
             int,               type,
             int,               protocol);            
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall3(SYS_SOCKET,
             int,               syscall_socket,
             int,               domain,
             int,               type,
             int,               protocol);            

/* -------------------------------------------------------------------------- *
 * int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)          *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern 
psocketcall3(int,                     syscall_bind,
             int,                     sockfd,
             const struct sockaddr *, my_addr,
             socklen_t,               addrlen);
#endif /* PIC */
             
#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall3(SYS_BIND,
             int,                     syscall_bind,
             int,                     sockfd,
             const struct sockaddr *, my_addr,
             socklen_t,               addrlen);
             
/* -------------------------------------------------------------------------- *
 * int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)     *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall3(int,                     syscall_connect,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t,               addrlen);             
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall3(SYS_CONNECT,
             int,                     syscall_connect,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t,               addrlen);             

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall3(int,                     syscall_getsockname,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t *,             addrlen);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall3(SYS_GETSOCKNAME,
             int,                     syscall_getsockname,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t *,             addrlen);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern 
psocketcall3(int,                     syscall_getpeername,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t *,             addrlen);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall3(SYS_GETPEERNAME,
             int,                     syscall_getpeername,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t *,             addrlen);

/* -------------------------------------------------------------------------- *
 * int listen(int sockfd, int backlog)                                        *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall2(int,               syscall_listen,
             int,               sockfd,
             int,               backlog);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall2(SYS_LISTEN,
             int,               syscall_listen,
             int,               sockfd,
             int,               backlog);

/* -------------------------------------------------------------------------- *
 * int accept(int s, struct sockaddr *addr, socklen_t *addrlen)               *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall3(int,               syscall_accept,
             int,               s,
             struct sockaddr *, addr,
             socklen_t *,       addrlen);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall3(SYS_ACCEPT,
             int,               syscall_accept,
             int,               s,
             struct sockaddr *, addr,
             socklen_t *,       addrlen);

/* -------------------------------------------------------------------------- *
 * int send(int s, const void *msg, size_t len, int flags)                    *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall4(int,               syscall_send,
             int,               s,
             const void *,      msg,
             size_t,            len,
             int,               flags);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall4(SYS_SEND,
             int,               syscall_send,
             int,               s,
             const void *,      msg,
             size_t,            len,
             int,               flags);

/* -------------------------------------------------------------------------- *
 * int recv(int s, void *buf, size_t len, int flags)                          *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall4(int,               syscall_recv,
             int,               s,
             void *,            buf,
             size_t,            len,
             int,               flags);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall4(SYS_RECV, 
             int,               syscall_recv,
             int,               s,
             void *,            buf,
             size_t,            len,
             int,               flags);

/* -------------------------------------------------------------------------- *
 * int socketpair(int d, int type, int protocol, int sv[2]);                  *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall4(int,               syscall_socketpair,
             int,               d,
             int,               type,
             int,               protocol,
             int *,             sv);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall4(SYS_SOCKETPAIR,
             int,               syscall_socketpair,
             int,               d,
             int,               type,
             int,               protocol,
             int *,             sv);

/* -------------------------------------------------------------------------- *
 * int getsockopt(int, int, int, void *, socklen_t *);                        *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall5(int,               syscall_getsockopt,
             int,               s,
             int,               level,
             int,               optname,
             void *,            optval,
             socklen_t *,       optlen);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall5(SYS_GETSOCKOPT,
             int,               syscall_getsockopt,
             int,               s,
             int,               level,
             int,               optname,
             void *,            optval,
             socklen_t *,       optlen);

/* -------------------------------------------------------------------------- *
 * int setsockopt(int, int, int, void *, socklen_t *);                        *
 * -------------------------------------------------------------------------- */
#ifndef PIC
extern
psocketcall5(int,               syscall_setsockopt,
             int,               s,
             int,               level,
             int,               optname,
             const void *,      optval,
             socklen_t,         optlen);
#endif /* PIC */

#ifndef PIC
extern inline
#else
static inline
#endif
_socketcall5(SYS_SETSOCKOPT,
             int,               syscall_setsockopt,
             int,               s,
             int,               level,
             int,               optname,
             const void *,      optval,
             socklen_t,         optlen);

//#define errno syscall_errno

#else /* USE_IA32_LINUX_INLINE */

#include <errno.h>

#define syscall_close        close
#define syscall_exit         exit
#define syscall_gettimeofday gettimeofday
#define syscall_read         read
#define syscall_write        write
#define syscall_select       select
#define syscall_mmap2        mmap2
#define syscall_mprotect     mprotect
#define syscall_munmap       munmap
#define syscall__stat64      stat64
#define syscall_fstat64      fstat64
#define syscall_socket       socket
#define syscall_bind         bind
#define syscall_accept       accept
#define syscall_send         send
#define syscall_recv         recv
#define syscall_poll         poll
#define syscall_kill         kill
#define syscall_open         open
#define syscall_exit         exit
#define syscall_pipe         pipe
#define syscall_fork         fork
#define syscall_setsockopt   setsockopt
#define syscall_getsockopt   getsockopt
#define syscall_getpeername  getpeername
#define syscall_getsockname  getsockname
#define syscall_signal       signal
#define syscall_dup2         dup2
#define syscall_mmap         mmap
#define syscall_waitpid      waitpid
#define syscall_execve       execve
#define syscall_connect      connect
#define syscall_setsid       setsid
#define syscall_socketpair   socketpair
#define syscall_listen       listen
#define syscall_fcntl        fcntl
#define syscall_stat         stat
#define syscall_chdir        chdir
#define syscall_setrlimit    setrlimit
#define syscall_getpid       getpid
#define syscall_unlink       unlink
#define syscall_readlink     readlink

#define syscall_strerror     strerror

#include <errno.h>
#define syscall_errno        errno

#endif /* USE_IA32_LINUX_INLINE */

#endif /* LIB_SYSCALL_H */
