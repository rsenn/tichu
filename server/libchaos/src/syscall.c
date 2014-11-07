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
 * $Id: syscall.c,v 1.32 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#include <libchaos/defs.h>

#if 0//(defined USE_IA32_LINUX_INLINE) //&& !(defined PIC)

#include <libchaos/syscall.h>

static const char *errno_table[] = {
/* esuccess         */ "success",
/* eperm            */ "operation not permitted",
/* enoent           */ "no such file or directory",
/* esrch            */ "no such process",
/* eintr            */ "interrupted system call",
/* eio              */ "i/o error",
/* enxio            */ "no such device or address",
/* e2big            */ "argument list too long",
/* enoexec          */ "exec format error",
/* ebadf            */ "bad file descriptor",
/* echild           */ "no child processes",
/* eagain           */ "resource temporarily unavailable",
/* enomem           */ "cannot allocate memory",
/* eacces           */ "permission denied",
/* efault           */ "bad address",
/* enotblk          */ "block device required",
/* ebusy            */ "device or resource busy",
/* eexist           */ "file exists",
/* exdev            */ "invalid cross-device link",
/* enodev           */ "no such device",
/* enotdir          */ "not a directory",
/* eisdir           */ "is a directory",
/* einval           */ "invalid argument",
/* enfile           */ "too many open files in system",
/* emfile           */ "too many open files",
/* enotty           */ "inappropriate ioctl for device",
/* etxtbsy          */ "text file busy",
/* efbig            */ "file too large",
/* enospc           */ "no space left on device",
/* espipe           */ "illegal seek",
/* erofs            */ "read-only file system",
/* emlink           */ "too many links",
/* epipe            */ "broken pipe",
/* edom             */ "numerical argument out of domain",
/* erange           */ "numerical result out of range",
/* edeadlk          */ "resource deadlock avoided",
/* enametoolong     */ "file name too long",
/* enolck           */ "no locks available",
/* enosys           */ "function not implemented",
/* enotempty        */ "directory not empty",
/* eloop            */ "too many levels of symbolic links",
/* ewouldblock      */ "pperation would block",
/* enomsg           */ "no message of desired type",
/* eidrm            */ "identifier removed",
/* echrng           */ "channel number out of range",
/* el2nsync         */ "level 2 not synchronized",
/* el3hlt           */ "level 3 halted",
/* el3rst           */ "level 3 reset",
/* elnrng           */ "link number out of range",
/* eunatch          */ "protocol driver not attached",
/* enocsi           */ "no CSI structure available",
/* el2hlt           */ "level 2 halted",
/* ebade            */ "invalid exchange",
/* ebadr            */ "invalid request descriptor",
/* exfull           */ "exchange full",
/* enoano           */ "no anode",
/* ebadrqc          */ "invalid request code",
/* ebadslt          */ "invalid slot",
/* edeadlock        */ "resource deadlock would occur",
/* ebfont           */ "bad font file format",
/* enostr           */ "device not a stream",
/* enodata          */ "no data available",
/* etime            */ "timer expired",
/* enosr            */ "out of streams resources",
/* enonet           */ "machine is not on the network",
/* enopkg           */ "package not installed",
/* eremote          */ "object is remote",
/* enolink          */ "link has been severed",
/* eadv             */ "advertise error",
/* esrmnt           */ "srmount error",
/* ecomm            */ "communication error on send",
/* eproto           */ "protocol error",
/* emultihop        */ "multihop attempted",
/* edotdot          */ "RFS specific error",
/* ebadmsg          */ "bad message",
/* eoverflow        */ "value too large for defined data type",
/* enotuniq         */ "name not unique on network",
/* ebadfd           */ "file descriptor in bad state",
/* eremchg          */ "remote address changed",
/* elibacc          */ "can not access a needed shared library",
/* elibbad          */ "accessing a corrupted shared library",
/* elibscn          */ ".lib section in a.out corrupted",
/* elibmax          */ "attempting to link in too many shared libraries",
/* elibexec         */ "cannot exec a shared library directly",
/* eilseq           */ "illegal byte sequence",
/* erestart         */ "interrupted system call should be restarted",
/* estrpipe         */ "streams pipe error",
/* eusers           */ "too many users",
/* enotsock         */ "socket operation on non-socket",
/* edestaddrreq     */ "destination address required",
/* emsgsize         */ "message too long",
/* eprototype       */ "protocol wrong type for socket",
/* enoprotoopt      */ "protocol not available",
/* eprotonosupport  */ "protocol not supported",
/* esocktnosupport  */ "socket type not supported",
/* eopnotsupp       */ "operation not supported on transport endpoint",
/* epfnosupport     */ "protocol family not supported",
/* eafnosupport     */ "address family not supported by protocol",
/* eaddrinuse       */ "address already in use",
/* eaddrnotavail    */ "cannot assign requested address",
/* enetdown         */ "network is down",
/* enetunreach      */ "network is unreachable",
/* enetreset        */ "network dropped connection because of reset",
/* econnaborted     */ "software caused connection abort",
/* econnreset       */ "connection reset by peer",
/* enobufs          */ "no buffer space available",
/* eisconn          */ "transport endpoint is already connected",
/* enotconn         */ "transport endpoint is not connected",
/* eshutdown        */ "cannot send after transport endpoint shutdown",
/* etoomanyrefs     */ "too many references: cannot splice",
/* etimedout        */ "connection timed out",
/* econnrefused     */ "connection refused",
/* ehostdown        */ "host is down",
/* ehostunreach     */ "no route to host",
/* ealready         */ "operation already in progress",
/* einprogress      */ "operation now in progress",
/* estale           */ "stale NFS file handle",
/* euclean          */ "structure needs cleaning",
/* enotnam          */ "not a XENIX named type file",
/* enavail          */ "no XENIX semaphores available",
/* eisnam           */ "is a named type file",
/* eremoteio        */ "remote I/O error",
/* edquot           */ "quota exceeded",
/* enomedium        */ "no medium found",
/* emediumtype      */ "wrong medium type"
};

/* -------------------------------------------------------------------------- *
 * main() will be called from w00t() in syscall.c                             *
 * -------------------------------------------------------------------------- */
extern int main(int argc, char **argv, char **envp);

/* -------------------------------------------------------------------------- *
 * The error number.                                                          *
 * -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- *
 * Return the location of the errno                                           *
 * -------------------------------------------------------------------------- */
/*int *__errno_location()
{
  return &errno;
}*/

/* -------------------------------------------------------------------------- *
 * Return error string for the specified errno                                *
 * -------------------------------------------------------------------------- */
const char *syscall_strerror(int err_no)
{
  return errno_table[err_no];
}

#if 0 //(defined __linux__) && (defined __i386__)

#undef errno
int syscall_errno = 0;

#if 0//ndef PIC
/* -------------------------------------------------------------------------- *
 * int fork()                                                                 *
 * -------------------------------------------------------------------------- */
_syscall0(int,               syscall_fork);

/* -------------------------------------------------------------------------- *
 * int setsid()                                                               *
 * -------------------------------------------------------------------------- */
_syscall0(int,               syscall_setsid);

/* -------------------------------------------------------------------------- *
 * int getpid()                                                               *
 * -------------------------------------------------------------------------- */
_syscall0(pid_t,             syscall_getpid);

/* -------------------------------------------------------------------------- *
 * int close(int fd)                                                          *
 * -------------------------------------------------------------------------- */
_syscall1(int,               syscall_close,
          int,               fd);           /* fd to close */

/* -------------------------------------------------------------------------- *
 * int unlink(const char *path)                                               *
 * -------------------------------------------------------------------------- */
_syscall1(int,               syscall_unlink,
          const char *,      path);

/* -------------------------------------------------------------------------- *
 * void _exit(int status)                                                     *
 * -------------------------------------------------------------------------- */
_syscall1(int,               syscall_exit,
          int,               status);       /* process exit status */

/* -------------------------------------------------------------------------- *
 * void pipe(int filedes[2])                                                  *
 * -------------------------------------------------------------------------- */
_syscall1(int,               syscall_pipe,
          int *,             filedes);

/* -------------------------------------------------------------------------- *
 * int chdir(const char *path)                                                *
 * -------------------------------------------------------------------------- */
_syscall1(int,               syscall_chdir,
          const char *,      path);

/* -------------------------------------------------------------------------- *
 * int gettimeofday(struct timeval *tv, struct timezone *tz);                 *
 * -------------------------------------------------------------------------- */
_syscall2(int,               syscall_gettimeofday,  /* -1 on error */
          struct timeval *,  tv,            /* unixtime in microsecs */
          struct timezone *, tz);           /* timezone information */

/* -------------------------------------------------------------------------- *
 * int munmap(void *start, size_t length)                                     *
 * -------------------------------------------------------------------------- */
_syscall2(int,               syscall_munmap,        /* -1 on error */
          void *,            start,
          size_t,            length);

/* -------------------------------------------------------------------------- *
 * int stat(const char *file_name, struct stat *buf)                          *
 * -------------------------------------------------------------------------- */
_syscall2(int,               syscall_stat,  /* -1 on error */
          const char *,      file_name,
          struct stat *,     buf);

/* -------------------------------------------------------------------------- *
 * int fstat(int fd, struct stat *buf)                                        *
 * -------------------------------------------------------------------------- */
_syscall2(int,               syscall_fstat, /* -1 on error */
          int,               fd,
          struct stat   *,   buf);

/* -------------------------------------------------------------------------- *
 * int dup2(int oldfd, int newfd)                                             *
 * -------------------------------------------------------------------------- */
_syscall2(int,               syscall_dup2,   /* -1 on error */
          int,               oldfd,
          int,               newfd);

/* -------------------------------------------------------------------------- *
 * int signal(int signum, sighandler_t)                                       *
 * -------------------------------------------------------------------------- */
_syscall2(int,               syscall_signal, /* -1 on error */
          int,               signum,
          void *,            handler);

/* -------------------------------------------------------------------------- *
 * int setrlimit(int resource, const struct rlimit *rlim)                     *
 * -------------------------------------------------------------------------- */
_syscall2(int,                   syscall_setrlimit,     /* -1 on error */
          int,                   resource,
          const struct rlimit *, rlim);


/* -------------------------------------------------------------------------- *
 * int execve(const char *filename, char *const argv[], char *const envp[]);  *
 * -------------------------------------------------------------------------- */
_syscall3(int,               syscall_execve,
          const char *,      filename,
          char *const *,     argv,
          char *const *,     envp);

/* -------------------------------------------------------------------------- *
 * int waitpid(pid_t pid, int *status, int options);                          *
 * -------------------------------------------------------------------------- */
_syscall3(int,               syscall_waitpid,
          pid_t,             pid,
          int *,             status,
          int,               options);

/* -------------------------------------------------------------------------- *
 * ssize_t read(int fd, void *buf, size_t count)                              *
 * -------------------------------------------------------------------------- */
_syscall3(ssize_t,           syscall_read,  /* 0 on EOF, -1 on error */
          int,               fd,            /* source fd */ 
          void *,            buf,           /* destination buffer */
          size_t,            count);        /* byte count */

/* -------------------------------------------------------------------------- *
 * ssize_t write(int fd, const void *buf, size_t count)                       *
 * -------------------------------------------------------------------------- */
_syscall3(ssize_t,           syscall_write, /* -1 on error, else bytes written */
          int,               fd,            /* destination fd */
          const void *,      buf,           /* source buffer */
          size_t,            count);        /* byte count */

/* -------------------------------------------------------------------------- *
 * int poll(struct pollfd *ufds, unsigned int nfds, int mtimeout)             *
 * -------------------------------------------------------------------------- */
_syscall3(int,                  syscall_poll,/* -1 on error, else active fds */
          struct pollfd *,      ufds,        /* pollfd array */
          unsigned long int,    nfds,        /* size of array */
          int,                  mtimeout);   /* milisecond timeout */

/* -------------------------------------------------------------------------- *
 * const char *pathname, int flags, ...)                                      *
 * -------------------------------------------------------------------------- */
psyscall3(int,               syscall_open,  /* -1 on error, else valid fd */
          const char *,      pathname,      /* path to file to open */
          int,               flags,         /* flags for new fd */
          ...,)                             /* modes if file gets created */
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
    syscall_errno = -res;
    res = -1;
  }
  
  return (int)res;
};
#else
#error "no open() syscall wrapper for non-x86"
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * int fcntl(int fd, int cmd, ...)                                            *
 * -------------------------------------------------------------------------- */
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
#else     
#error "no fcntl() syscall wrapper for non-x86"
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * int kill(pid_t pid, int sig);                                              *
 * -------------------------------------------------------------------------- */
_syscall2(int,                  syscall_kill,       /* -1 on error */
          pid_t,                pid,
          int,                  sig);

/* -------------------------------------------------------------------------- *
 * int mprotect(const void *addr, size_t len, int prot)                       *
 * -------------------------------------------------------------------------- */
_syscall3(int,                  syscall_mprotect,   /* -1 on error */
          const void *,         addr,
          size_t,               len,
          int,                  prot);

/* -------------------------------------------------------------------------- *
 * int readlink(const char *addr, char *buf, size_t n)                        *
 * -------------------------------------------------------------------------- */
_syscall3(int,                  syscall_readlink,   /* -1 on error */
          const char *,         path,
          char *,               buf,
          size_t,               n);

/* -------------------------------------------------------------------------- *
 * int select(int, fd_set *, fd_set *, fd_set *, struct timeval *)            *
 * -------------------------------------------------------------------------- */
#ifdef USE_SELECT
_syscall5(int,               syscall_select,/* -1 on error, else active fds */
          int,               n,             /* highest fd + 1 */
          fd_set *,          readfds,       /* fd_set for read events */
          fd_set *,          writefds,      /* fd_set for write events */
          fd_set *,          exceptfds,     /* fd_set for exceptions */
          struct timeval *,  timeout);      /* timeout value */
#endif /* USE_SELECT */
/* -------------------------------------------------------------------------- *
 * void *mmap(void *start, size_t length, int prot,                           *
 *            int flags, int fd, off_t offset)                                *
 * -------------------------------------------------------------------------- */
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
_socketcall3(SYS_SOCKET,
             int,               syscall_socket,       
             int,               domain,
             int,               type,
             int,               protocol);

/* -------------------------------------------------------------------------- *
 * int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)          *
 * -------------------------------------------------------------------------- */
_socketcall3(SYS_BIND,
             int,                     syscall_bind,               
             int,                     sockfd,
             const struct sockaddr *, my_addr,
             socklen_t,               addrlen);

/* -------------------------------------------------------------------------- *
 * int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)     *
 * -------------------------------------------------------------------------- */
_socketcall3(SYS_CONNECT,
             int,                     syscall_connect,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t,               addrlen);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
_socketcall3(SYS_GETSOCKNAME,
             int,                     syscall_getsockname,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t *,             addrlen);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
_socketcall3(SYS_GETPEERNAME,
             int,                     syscall_getpeername,
             int,                     sockfd,
             const struct sockaddr *, serv_addr,
             socklen_t *,             addrlen);

/* -------------------------------------------------------------------------- *
 * int listen(int sockfd, int backlog)                                        *
 * -------------------------------------------------------------------------- */
_socketcall2(SYS_LISTEN,
             int,               syscall_listen,
             int,               sockfd,
             int,               backlog);

/* -------------------------------------------------------------------------- *
 * int accept(int s, struct sockaddr *addr, socklen_t *addrlen)               *
 * -------------------------------------------------------------------------- */
_socketcall3(SYS_ACCEPT,
             int,               syscall_accept,
             int,               s,
             struct sockaddr *, addr,
             socklen_t *,       addrlen);

/* -------------------------------------------------------------------------- *
 * int send(int s, const void *msg, size_t len, int flags)                    *
 * -------------------------------------------------------------------------- */
_socketcall4(SYS_SEND,
             int,               syscall_send,
             int,               s,
             const void *,      msg,
             size_t,            len,
             int,               flags);

/* -------------------------------------------------------------------------- *
 * int recv(int s, void *buf, size_t len, int flags)                          *
 * -------------------------------------------------------------------------- */
_socketcall4(SYS_RECV,
             int,               syscall_recv,
             int,               s,
             void *,            buf,
             size_t,            len,
             int,               flags);

/* -------------------------------------------------------------------------- *
 * int socketpair(int d, int type, int protocol, int sv[2]);                  *
 * -------------------------------------------------------------------------- */
_socketcall4(SYS_SOCKETPAIR,
             int,               syscall_socketpair,
             int,               d,
             int,               type,
             int,               protocol,
             int *,             sv);

/* -------------------------------------------------------------------------- *
 * int getsockopt(int, int, int, void *, socklen_t *);                        *
 * -------------------------------------------------------------------------- */
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
_socketcall5(SYS_SETSOCKOPT,
             int,               syscall_setsockopt,
             int,               s,
             int,               level,
             int,               optname,
             const void *,      optval,
             socklen_t,         optlen);
#endif /* PIC */
void w00t(char *argv0)
{
/*  char **argv;
  char **envp;
  int argc;*/
  int ret;
/*  
  argv = &argv0;
  
  envp = argv;
  
  while(*envp) envp++;
  
  argc = (size_t)(envp - argv);
  
  envp++;
  */
  ret = main(0, NULL, NULL/*argc, argv, envp*/);
  
  syscall_exit(ret);
}


#endif /* USE_IA32_LINUX_INLINE */

#endif
