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
 * $Id: io.h,v 1.40 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_IO_H
#define LIB_IO_H

#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/syscall.h>
#include <libchaos/queue.h>

#ifdef WIN32
#include <winsock.h>
#endif /* WIN32 */

#ifdef HAVE_SSL
#include <libchaos/ssl.h>
#endif /* HAVE_SSL */

#ifdef USE_POLL
#define io_multiplex io_poll
#else
#define io_multiplex io_select
#endif


#define io_valid(x) ((x) >= 0 && (x) < MAX_FDS)

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define MAX_FDS 1024

#define READ   0
#define WRITE  1

#define IO_READ_SIZE  4096    /* How many bytes we read at once */
#define IO_WRITE_SIZE 4096    /* How many bytes we write at once */
#define IO_LINE_SIZE  1024

#define IO_READ_FLAGS  (O_RDONLY | O_NONBLOCK)
#define IO_WRITE_FLAGS (O_WRONLY | O_NONBLOCK | O_CREAT)

#define IO_ERROR   0x01
#define IO_READ    0x02
#define IO_WRITE   0x04
#define IO_ACCEPT  0x08
#define IO_CONNECT 0x10

#define IO_CB_ERROR   0x00
#define IO_CB_READ    0x01
#define IO_CB_WRITE   0x02
#define IO_CB_ACCEPT  0x03
#define IO_CB_CONNECT 0x04
#define IO_CB_MAX     IO_CB_CONNECT + 1

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
enum {
  OFF = 0,
  ON = 1
};

enum {
  FD_NONE   = 0,
  FD_FILE   = 1,
  FD_SOCKET = 2,
  FD_PIPE   = 3
};
  
typedef void (io_callback_t)(int fd, void *, void *, void *, void *);

struct io {
  int                type;
  int                error;
  /* 
   * configuration flags
   * 
   *    sendq        - when set to 1, an io_write() will
   *                   not go directly to the file descriptor.
   *                   it'll be put into the queue and the
   *                   queue will be emptied on a write event.
   * 
   *    recvq        - when set to 1, an io_read() will not
   *                   directly read from a file descriptor.
   *                   it'll read from a queue which is
   *                   filled on read events.
   * 
   *    linebuf      - this works only when the recvq
   *                   is enabled.
   *                   when is is set to 1 then the
   *                   read callback for the socket
   *                   will only be called if there
   *                   is a line in the linebuffer.
   *                   otherwise its called simply
   *                   when there is data in the queue.
   * 
   *    waitdns      - call IO_CB_ACCEPT after completed
   *                   reverse DNS.
   */
  struct {
    int sendq:1;     /* queue incoming data */
    int recvq:1;     /* queue outgoing data */
    int linebuf:1;   /* linebuffer incoming data */
    int events:5;
  } control;
  
  /* 
   * event flags 
   * 
   *    err          - there was an error
   * 
   *    line         - there is a line in the queue
   * 
   *    timeout      - operation timed out
   */
  struct {
    int events;
    int err;       /* got error */
/*    int line:1;  */    /* we got a line */
/*    int timeout:1; */  /* operation timed out */
/*    int listening:1;*/
/*    int connecting:1;*/
/*    int connected:1;*/
    int closed;
    int onread;
    int onwrite;
    int dead;
  } status;
  
  int                ret;
  void              *args[4];  
  long               flags;    /* posix filedescriptor flags */
  struct fqueue      sendq;
  struct fqueue      recvq;
  struct stat        stat;
  int                index;    /* index on the pollfd list */
  char               note[64]; /* description string */
  struct sockaddr_in a_remote; /* remote address */
  struct sockaddr_in a_local;  /* local address we bound to */
  io_callback_t     *callbacks[IO_CB_MAX];
#ifdef HAVE_SSL
  SSL               *ssl;
  int                sslstate;
  int                sslerror;
  int                sslwhere; /* where ssl error happened */
#endif /* HAVE_SSL */
};

//#define io_valid(x) ((x) >= 0 && (x) < MAX_FDS && io_list[(x)].type)

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int       io_log;
extern struct io io_list[MAX_FDS];

#ifdef USE_POLL
extern size_t    io_count;
#else
extern int       io_top;
#endif /* USE_SELECT || USE_POLL */

/* -------------------------------------------------------------------------- *
 * Initialize I/O code.                                                       *
 * -------------------------------------------------------------------------- */
extern void  io_init          (void);

extern void  io_init_except   (int            fd0,
                               int            fd1, 
                               int            fd2);
  
/* -------------------------------------------------------------------------- *
 * Shutdown I/O code.                                                         *
 * -------------------------------------------------------------------------- */
extern void  io_shutdown      (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int   io_flush         (int            fd);
  
/* -------------------------------------------------------------------------- *
 * Put a file descriptor into non-blocking mode.                              *
 * -------------------------------------------------------------------------- */
extern int   io_nonblock      (int            fd);

/* -------------------------------------------------------------------------- *
 * Control the queue behaviour.                                               *
 *                                                                            *
 * Use ON/OFF flags for all arguments except fd.                              *
 *                                                                            *
 * fd            - File descriptor                                            *
 * recvq         - queue incoming data.                                       *
 * sendq         - queue outgoing data.                                       *
 * linebuf       - Only call read callback when there's a full line of data.  *
 *                                                                            *
 * Note that none of the queues can be disabled if they still contain data.   *
 * -------------------------------------------------------------------------- */
extern int   io_queue_control (int            fd,
                               int            recvq,
                               int            sendq, 
                               int            linebuf);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int   io_queued_read   (int            fd);
                               
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int   io_queued_write  (int            fd);
                               
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void  io_handle_fd     (int            fd);
                               
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- *
 * Register a file descriptor to the io_list.                                  *
 * -------------------------------------------------------------------------- */
extern int   io_new           (int            fd, 
                               int            type);
  
/* -------------------------------------------------------------------------- *
 * Open a file.                                                               *
 * -------------------------------------------------------------------------- */
extern int   io_open          (const char    *path,
                               long           flags, 
                               ...);

/* -------------------------------------------------------------------------- *
 * Shut a filedescriptor                                                      *
 * -------------------------------------------------------------------------- */
extern void  io_shutup        (int            fd);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int   io_push          (int           *fdptr);

/* -------------------------------------------------------------------------- *
 * Close an fd.                                                               *
 * -------------------------------------------------------------------------- */
extern void  io_close         (int            fd);
  
/* -------------------------------------------------------------------------- *
 * Write a description string.                                                *
 * -------------------------------------------------------------------------- */
extern void  io_note          (int            fd,
                               const char    *format, 
                               ...);
  
/* -------------------------------------------------------------------------- *
 * Register a I/O event callback.                                             *
 *                                                                            *
 * type            - on which type of event to call the callback              *
 *                   IO_CB_ERROR   - I/O error                                * 
 *                   IO_CB_READ    - incoming data                            *
 *                   IO_CB_WRITE   - outgoing data                            *
 *                   IO_CB_ACCEPT  - a client connecting                      *
 *                   IO_CB_CONNECT - an established or failed connection      *
 *                                                                            *
 * callback        - a callback in the form:                                  *
 *                                                                            *
 *                   void callback(int fd, void *arg)                         *
 *                                                                            *
 * userarg         - a user-defined pointer to pass to the callback           *
 *                                                                            *
 * timeout         - if the event doesn't occur after this miliseconds        *
 *                   then call the callback anyway.                           *
 *                                                                            * 
 * -------------------------------------------------------------------------- */
extern int   io_vregister     (int            fd,
                               int            type, 
                               void          *callback,
                               va_list        args);

extern int   io_register      (int            fd,
                               int            type, 
                               void          *callback,
                               ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int   io_unregister    (int            fd, 
                               int            type);
  
/* -------------------------------------------------------------------------- *
 * Read either from the fd directly or from its queue.                        *
 * -------------------------------------------------------------------------- */
extern int   io_read          (int            fd,
                               void          *buf, 
                               size_t         n);

/* -------------------------------------------------------------------------- *
 * Write either to the fd directly or to its queue.                           *
 * -------------------------------------------------------------------------- */
extern int   io_write         (int            fd,
                               const void    *buf,
                               size_t         n);

/* -------------------------------------------------------------------------- *
 * Read a line from queue.                                                    *
 * -------------------------------------------------------------------------- */
extern int   io_gets          (int            fd,
                               void          *buf, 
                               size_t         n);

/* -------------------------------------------------------------------------- *
 * Write a line to fd or queue.                                               *
 * -------------------------------------------------------------------------- */
extern int   io_puts          (int            fd, 
                               const char    *s, 
                               ...);  

/* -------------------------------------------------------------------------- *
 * Write a line to fd or queue.                                               *
 * -------------------------------------------------------------------------- */
extern int   io_vputs         (int            fd, 
                               const char    *s,
                               va_list        args);

/* -------------------------------------------------------------------------- *
 *                                                                            *
 * -------------------------------------------------------------------------- */
extern void     io_multi_start(struct fqueue *fifoptr);
extern uint32_t io_multi_write(struct fqueue *fifoptr,
                               const void    *buf, 
                               uint32_t       n);
extern void     io_multi_link (struct fqueue *fifoptr, 
                               int            fd);
extern void     io_multi_end  (struct fqueue *fifoptr);

/* -------------------------------------------------------------------------- *
 * Do a select() system call.                                                 *
 *                                                                            *
 * If timeout == NULL then return only when an event occurred.                *
 * else return after <timeout> miliseconds or when there was an event.        *
 * In the latter case the remaining time will be in *timeout.                 *
 * -------------------------------------------------------------------------- */
extern int   io_select        (int64_t       *remain,
                               int64_t       *timeout);
  
/* -------------------------------------------------------------------------- *
 * Do a poll() system call.                                                   *
 *                                                                            *
 * If timeout == NULL then return only when an event occurred.                *
 * else return after <timeout> miliseconds or when there was an event.        *
 * In the latter case the remaining time will be in *timeout.                 *
 * -------------------------------------------------------------------------- */
extern int   io_poll          (int64_t       *remain,
                               int64_t       *timeout);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void  io_wait          (void);
  
/* -------------------------------------------------------------------------- *
 * Handle pending I/O events                                                  *
 * -------------------------------------------------------------------------- */
extern void  io_handle        (void);
  
/* -------------------------------------------------------------------------- *
 * Move an fd.                                                                *
 * -------------------------------------------------------------------------- */
extern void  io_move          (int            from, 
                               int            to);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void  io_vset_args     (int            fd,
                               va_list        args);

extern void  io_set_args      (int            fd,
                               ...);

/* -------------------------------------------------------------------------- *
 * This function will set the necessary flags in the fd_sets/pollfds for the  *
 * requested events.                                                          *
 * -------------------------------------------------------------------------- */
extern void  io_set_events    (int            fd, 
                               int            events);

/* -------------------------------------------------------------------------- *
 * This function will unset the necessary flags in the fd_sets/pollfds for    *
 * the requested events.                                                      *
 * -------------------------------------------------------------------------- */
extern void  io_unset_events  (int            fd, 
                               int            events);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void  io_dump          (int            fd);

#endif /* LIB_IO_H */
