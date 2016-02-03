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
 * $Id: io.c,v 1.74 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <stdarg.h>

#include <libchaos/defs.h>

/*#if (!defined USE_SELECT) || (!defined USE_POLL)
#error "no i/o multiplexing method"
#endif*/

#ifdef HAVE_SSL
#include <libchaos/ssl.h>
#endif /* HAVE_SSL */

#include <libchaos/syscall.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>
#include <libchaos/io.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int       io_log;
struct io io_list[MAX_FDS];

#ifdef USE_POLL
size_t    io_count = 0;
#else
int       io_top   = -1;
#endif /* USE_SELECT || USE_POLL */

/* -------------------------------------------------------------------------- *
 * Local variables                                                            *
 * -------------------------------------------------------------------------- */

#ifdef USE_POLL
static struct pollfd io_pfds[MAX_FDS];
#else
static fd_set io_efds, io_efds_r;
static fd_set io_rfds, io_rfds_r;
static fd_set io_wfds, io_wfds_r;
#endif /* USE_SELECT || USE_POLL */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *io_types[] = 
{
  "none",
  "file",
  "sock",
  "pipe"
};

/* -------------------------------------------------------------------------- *
 * Register fd for use with select/poll.                                      *
 * -------------------------------------------------------------------------- */
static void io_add_fd(int fd)
{
#ifdef USE_POLL
  
  /* Assign the next available pollfd struct to the fd */
  io_list[fd].index = io_count;
  
  /* Initialize the pollfd struct */
  io_pfds[io_count].fd = fd;
  io_pfds[io_count].events = 0;
  io_pfds[io_count].revents = 0;
  
  /* Update pollfd count */
  io_count++;  
#else
  
  /* New fd is higher than io_top, update io_top */
  if(fd > io_top)
    io_top = fd;
  
#endif /* USE_SELECT || USE_POLL */
}

/* -------------------------------------------------------------------------- *
 * Remove pollfd struct from array.                                           *
 * -------------------------------------------------------------------------- */
static void io_remove_fd(int fd)
{
#ifdef USE_POLL
  
  size_t index = io_list[fd].index;
     
  if(index >= 0 && index < io_count)
  {
    /* Clear the pollfd struct */
    io_pfds[index].fd = -1;
    io_pfds[index].events = 0;
    io_pfds[index].revents = 0;
    
    /* 
     * If it's not the last pollfd struct we'll have to
     * move the following ones down. 
     */
    while(index + 1 < io_count)
    { 
      io_pfds[index] = io_pfds[index + 1];
      
      if(io_pfds[index].fd > 0)
        io_list[io_pfds[index].fd].index = index;
      
      index++;
    }
    
    io_list[fd].index = -1;
    
    /* Decrease pollfd count */
    io_count--;
  }
  
#else
  
  /* Oh, it's the topmost fd, we need to decrease io_top */
  if(fd == io_top)
  {
    /* Loop down the io_list to find new io_top */
    for(io_top = io_top - 1; io_top >= 0; io_top--)
    {
      /* Active fd, this is our new top */
      if(io_list[io_top].type)
        break;
    }
  }
  
#endif /* USE_SELECT || USE_POLL */
}

/* -------------------------------------------------------------------------- *
 * This function will set the necessary flags in the fd_sets/pollfds for the  *
 * requested events.                                                          *
 * -------------------------------------------------------------------------- */
void io_set_events(int fd, int events)
{
  io_list[fd].control.events |= events;

#ifdef USE_POLL

  /* Fill in the flags into the pollfd struct */
  if(events & IO_ERROR)
    io_pfds[io_list[fd].index].events |= POLLERR;
  if(events & IO_READ)
    io_pfds[io_list[fd].index].events |= POLLIN;
  if(events & IO_WRITE)
    io_pfds[io_list[fd].index].events |= POLLOUT;
    
#else
  
  /* If we use select, each flag modifies another fd_set */
  if(events & IO_ERROR)
    FD_SET(fd, &io_efds);
  if(events & IO_READ)
    FD_SET(fd, &io_rfds);
  if(events & IO_WRITE)
    FD_SET(fd, &io_wfds);
  
#endif /* USE_SELECT || USE_POLL */
}

/* -------------------------------------------------------------------------- *
 * This function will unset the necessary flags in the fd_sets/pollfds for    *
 * the requested events.                                                      *
 * -------------------------------------------------------------------------- */
void io_unset_events(int fd, int events)
{
  io_list[fd].control.events &= ~events;

#ifdef USE_POLL
  
  if(io_list[fd].index >= 0)
  {
    /* Remove the flags from the pollfd struct */
    if(events & IO_ERROR)
      io_pfds[io_list[fd].index].events &= ~POLLERR;
    if(events & IO_READ)
      io_pfds[io_list[fd].index].events &= ~POLLIN;
    if(events & IO_WRITE)
      io_pfds[io_list[fd].index].events &= ~POLLOUT;
  }
  
#else
  
  /* If we use select, each flag modifies another fd_set */
  if(events & IO_ERROR)
    FD_CLR(fd, &io_efds);
  if(events & IO_READ)
    FD_CLR(fd, &io_rfds);
  if(events & IO_WRITE)
    FD_CLR(fd, &io_wfds);
  
#endif /* USE_SELECT || USE_POLL */
}

/* -------------------------------------------------------------------------- *
 * Evaluates returned events                                                  *
 * -------------------------------------------------------------------------- */
static void io_set_revents(int fd)
{
  io_list[fd].status.events = 0;

#ifdef USE_POLL

  if(io_list[fd].index >= 0)
  {
    /* Get returned events from the corresponding pollfd struct */
    if(io_pfds[io_list[fd].index].revents & POLLERR)
      io_list[fd].status.events |= IO_ERROR;
    if(io_pfds[io_list[fd].index].revents & POLLIN)
      io_list[fd].status.events |= IO_READ;
    if(io_pfds[io_list[fd].index].revents & POLLOUT)
      io_list[fd].status.events |= IO_WRITE;
  }
  
#else
  
  /* Collect event info from all 3 fd_sets */
  if(FD_ISSET(fd, &io_efds_r))
    io_list[fd].status.events |= IO_ERROR;
  if(FD_ISSET(fd, &io_rfds_r))
    io_list[fd].status.events |= IO_READ;
  if(FD_ISSET(fd, &io_wfds_r))
    io_list[fd].status.events |= IO_WRITE;
  
# endif /* USE_SELECT || USE_POLL */
}

/* -------------------------------------------------------------------------- *
 * Read as long as the system call fills the buffer and queue the data.       *
 * -------------------------------------------------------------------------- */
int io_queued_read(int fd)
{
  int ret;
  size_t sz = 0;
  char buf[IO_READ_SIZE];
  
  do
  {
    switch(io_list[fd].type)
    {
      case FD_SOCKET:
#ifdef HAVE_SSL
        if(io_list[fd].ssl)
          ret = ssl_read(fd, buf, IO_READ_SIZE);
        else
#endif /* HAVE_SSL */
          ret = syscall_recv(fd, buf, IO_READ_SIZE, 0);
        break;
      case FD_FILE:
      case FD_PIPE:
      default:
        ret = syscall_read(fd, buf, IO_READ_SIZE);
        break;
    }
  
    /* We got data, add it to the queue */
    if(ret > 0)
      sz += queue_write(&io_list[fd].recvq, buf, ret);
  }
  while(ret == IO_READ_SIZE);

  io_list[fd].status.onread = 1;
  io_list[fd].status.onwrite = 0;
  
/*  if(ret == 0 || (io_list[fd].type == FD_FILE && ret >= 0 && ret < IO_READ_SIZE))
  {
    io_list[fd].error = 0;
    io_list[fd].status.err = 1;
    
    return -1;
  }*/
  
#ifdef HAVE_SSL
/*  if(io_list[fd].ssl)
  {
    if(io_list[fd].error)
      return -1;
    else 
      return 0;
  }
  else*/
  {
#endif /* HAVE_SSL */    
    if(ret <= 0)
    {
      io_list[fd].error = syscall_errno;
      syscall_errno = 0;
      io_list[fd].status.err = 1;
      
      if(io_list[fd].error == EAGAIN)
      {
        io_list[fd].status.err = 0;
        
        return 0;
      }
      
      return -1;
    }
    else
    {
      io_list[fd].error = 0;
      io_list[fd].status.err = 0;
    }  
#ifdef HAVE_SSL
  }
#endif /* HAVE_SSL */

  return ret/* == 0 ? -1 : sz*/;
}

/* -------------------------------------------------------------------------- *
 * Read as long as the system call fills the buffer and queue the data.       *
 * -------------------------------------------------------------------------- */
int io_queued_write(int fd)
{
  char buf[IO_WRITE_SIZE];
  size_t n;
  int ret = 0;

/*  do
  {*/
    /* Write max. IO_WRITE_SIZE bytes */
    n = io_list[fd].sendq.size > IO_WRITE_SIZE ?
        IO_WRITE_SIZE :
        io_list[fd].sendq.size;
    
    if(n == 0)
    {
      io_unset_events(fd, IO_WRITE);
      return 0;
    }
  
    /* Get the stuff from queue, but not remove it */
    queue_map(&io_list[fd].sendq, buf, n);
  
    /* Write to fd */
    switch(io_list[fd].type)
    {
      case FD_SOCKET:
#ifdef HAVE_SSL
        if(io_list[fd].ssl)
          ret = ssl_write(fd, buf, n);
        else
#endif /* HAVE_SSL */        
          ret = syscall_send(fd, buf, n, 0);
        break;
      default:
      case FD_FILE:
      case FD_PIPE:
        ret = syscall_write(fd, buf, n);
        break;
    }
  
    /* Only remove stuff from the queue that has been sent */
    if(ret > 0)
      queue_cut(&io_list[fd].sendq, ret);
/*  }
  while(ret == IO_WRITE_SIZE);*/

#ifdef HAVE_SSL
  if(io_list[fd].ssl)
  {
    if(io_list[fd].error)
      return -1;
    else 
      return 0;
  }
  else
  {
#endif /* HAVE_SSL */    
    if(ret < 0)
    {
      if(syscall_errno && syscall_errno != EWOULDBLOCK)
      {
        io_list[fd].error = syscall_errno;
        syscall_errno = 0;
        io_list[fd].status.err = 1;
        io_list[fd].status.onwrite = 1;
        
        return -1;
      }
      
      return 0;
    }
    else
    {
      io_list[fd].status.err = 0;
    }
#ifdef HAVE_SSL    
  }
#endif /* HAVE_SSL */

  /* Queue was emptied, do not schedule another event */
  if(!io_list[fd].sendq.size)
    io_unset_events(fd, IO_WRITE);

  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int io_flush(int fd)
{
  int ret;
  size_t bytes = 0;
  
  while(io_list[fd].sendq.size)
  {
    if((ret = io_queued_write(fd)) == -1)
      return -1;
    
    bytes += ret;
  }
  
  return bytes;
}

/* -------------------------------------------------------------------------- *
 * Shut a filedescriptor                                                      *
 * -------------------------------------------------------------------------- */
void io_shutup(int fd)
{
  int i;
  
  if(fd < 0) 
    return;
  
  if(!io_list[fd].status.closed)
  {
    if(io_list[fd].sendq.size &&
       !io_list[fd].status.err && 
       !io_list[fd].status.dead)
    {
      io_queued_write(fd);
      
      if(io_list[fd].sendq.size)
        queue_destroy(&io_list[fd].sendq);
      if(io_list[fd].recvq.size)
        queue_destroy(&io_list[fd].recvq);
    }
    
    io_remove_fd(fd);
    
    io_list[fd].status.closed = 1;
    
    for(i = 0; i < IO_CB_MAX; i++)
      io_list[fd].callbacks[i] = NULL;
  }
  
  if(io_list[fd].control.events)
    io_unset_events(fd, IO_READ|IO_WRITE|IO_ERROR);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int io_push(int *fdptr)
{
  if(*fdptr >= 0 && *fdptr < MAX_FDS)
  {
    io_shutup(*fdptr);
    
    *fdptr = -1;
  }
  
  return *fdptr;
}

/* -------------------------------------------------------------------------- *
 * Handle pending I/O events                                                  *
 * -------------------------------------------------------------------------- */
void io_handle_fd(int fd)
{
  io_list[fd].status.onwrite = 0;
  io_list[fd].status.onread = 0;  
  
  /* Do SSL handshakes if not done yet */
#ifdef HAVE_SSL
  if(io_list[fd].ssl && io_list[fd].sslstate) 
  {
    if(ssl_handshake(fd, &io_list[fd]))
    {
      io_list[fd].status.onread = 1;
      io_list[fd].status.onwrite = 1;
      io_list[fd].status.err = 1;
      
      if(io_list[fd].sslerror)
        io_list[fd].error = 666;
      else
        io_list[fd].error = 0;
      
      io_list[fd].status.closed = 1;
      
      if(io_list[fd].callbacks[IO_CB_READ])
        io_list[fd].callbacks[IO_CB_READ](fd, 
                                    io_list[fd].args[0], io_list[fd].args[1],
                                    io_list[fd].args[1], io_list[fd].args[2]);
      io_list[fd].status.closed = 0;
      io_close(fd);
      return;
    } 
  } 
#endif /* HAVE_SSL */
  
  /* There is data on the fd */
  if(!io_list[fd].status.err && !io_list[fd].status.closed &&
     (io_list[fd].status.events & IO_READ))
  {
    /* If this fd is queued then we read now and fill the queue */
    if(io_list[fd].control.recvq)
    {
      io_list[fd].ret = io_queued_read(fd);
      
      if(io_list[fd].ret < 0)
      {
        io_list[fd].status.closed = 1;
        io_list[fd].status.err = 1;
        io_list[fd].status.onread = 1;
        
//        io_queued_write(fd);
        io_remove_fd(fd);
        
        if(io_list[fd].control.events)
          io_unset_events(fd, IO_READ|IO_WRITE|IO_ERROR);
      }
    }
  }

  /* We can write to the fd :D */
  if(io_list[fd].status.events & IO_WRITE)
  {
    /* If this fd is queued then we try to write */
    if(io_list[fd].control.sendq && 
       !io_list[fd].status.err && !io_list[fd].status.dead)
    {
      io_list[fd].ret = io_queued_write(fd);
      
      if(io_list[fd].ret < 0)
      {
        io_list[fd].status.onwrite = 1;
        
        io_queued_write(fd);
        io_remove_fd(fd);
        
        if(io_list[fd].control.events)
          io_unset_events(fd, IO_READ|IO_WRITE|IO_ERROR);
      }
    }
    
    if(io_list[fd].status.err || io_list[fd].status.dead)
      io_list[fd].status.events |= IO_READ;
  }
    
  /* We had an error */
  if(io_list[fd].status.events & IO_ERROR)
  {
    /* Call error callback for this fd if present */
    if(io_list[fd].callbacks[IO_CB_ERROR])
      io_list[fd].callbacks[IO_CB_ERROR](fd, io_list[fd].args[0], io_list[fd].args[1],
                                         io_list[fd].args[1], io_list[fd].args[2]);
  }
  
  if(!io_list[fd].status.err && (io_list[fd].status.events & IO_WRITE))
  {
    if(io_list[fd].callbacks[IO_CB_WRITE])
      io_list[fd].callbacks[IO_CB_WRITE](fd, io_list[fd].args[0], io_list[fd].args[1],
                                         io_list[fd].args[1], io_list[fd].args[2]);
  }
  
  if(io_list[fd].status.err || io_list[fd].status.closed || (io_list[fd].status.events & IO_READ) || io_list[fd].recvq.size)
  {
    if(io_list[fd].callbacks[IO_CB_READ])
      io_list[fd].callbacks[IO_CB_READ](fd, io_list[fd].args[0], io_list[fd].args[1],
                                        io_list[fd].args[1], io_list[fd].args[2]);
  }
  
  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    io_close(fd);
    return;
  }
}

/* -------------------------------------------------------------------------- *
 * Initialize I/O code.                                                       *
 * -------------------------------------------------------------------------- */
void io_init(void)
{
  size_t i;
  
  io_log = log_source_register("i/o");
  
  memset(io_list, 0, MAX_FDS * sizeof(struct io));
  
  /* Close all fds */
  for(i = 0; i < MAX_FDS; i++)
  {
    syscall_close(i);    
    io_list[i].index = -1;
  }

#ifdef USE_POLL
  memset(io_pfds, 0, sizeof(struct pollfd) * MAX_FDS);
  io_count = 0;
#else
  FD_ZERO(&io_rfds);
  FD_ZERO(&io_wfds);
  FD_ZERO(&io_efds);
#endif /* USE_SELECT || USE_POLL */
  
  log(io_log, L_status, "Initialized [i/o] module.");
}

/* -------------------------------------------------------------------------- *
 * Initialize I/O code.                                                       *
 * -------------------------------------------------------------------------- */
void io_init_except(int fd0, int fd1, int fd2)
{
  size_t i;
  
  io_log = log_source_register("i/o");
  
  /* Close all fds */
  for(i = 0; i < MAX_FDS; i++)
    if(i != fd0 && i != fd1 && i != fd2)
      syscall_close(i);
  
  memset(io_list, 0, MAX_FDS * sizeof(struct io));

#ifdef USE_POLL
  memset(io_pfds, 0, sizeof(struct pollfd) * MAX_FDS);
#else
  FD_ZERO(&io_rfds);
  FD_ZERO(&io_wfds);
  FD_ZERO(&io_efds);
#endif /* USE_SELECT || USE_POLL */
}

/* -------------------------------------------------------------------------- *
 * Shutdown I/O code.                                                         *
 * -------------------------------------------------------------------------- */
void io_shutdown(void)
{
  size_t i;
  
  log(io_log, L_status, "Shutting down [i/o] module...");
  
  /* Close all fds */
  for(i = 0; i < MAX_FDS; i++) if(io_list[i].type)
  {
    queue_destroy(&io_list[i].sendq);
    queue_destroy(&io_list[i].recvq);
    io_close(i);
  }
  
  log_source_unregister(io_log);
}

/* -------------------------------------------------------------------------- *
 * Put a file descriptor into non-blocking mode.                              *
 * -------------------------------------------------------------------------- */
int io_nonblock(int fd)
{
  if(!io_list[fd].flags)
    io_list[fd].flags = fcntl(fd, F_GETFL);
  
  io_list[fd].flags |= O_NONBLOCK;
  
  return fcntl(fd, F_SETFL, io_list[fd].flags);
}

/* -------------------------------------------------------------------------- *
 * Control the queue behaviour.                                               *
 *                                                                            *
 * Use ON/OFF flags for all arguments except fd.                              *
 *                                                                            *
 * fd            - File descriptor                                            *
 * recvq         - Queue incoming data.                                       *
 * sendq         - Queue outgoing data.                                       *
 * linebuf       - Only call read callback when there's a full line of data.  *
 *                                                                            *
 * Note that none of the queues can be disabled if they still contain data.   *
 * -------------------------------------------------------------------------- */
int io_queue_control(int fd, int recvq, int sendq, int linebuf)
{
  if(fd < 0)
    return -1;
  
  /* Can't disable recvq when there is data */
  if(recvq == OFF && io_list[fd].recvq.size)
    return -1;
  
  /* Can't disable sendq when there is data */
  if(sendq == OFF && io_list[fd].recvq.size)
    return -1;
  
  /* Update queue control information */
  io_list[fd].control.recvq = recvq;
  io_list[fd].control.sendq = sendq;
  io_list[fd].control.linebuf = linebuf;
  
  /* If receive queue has been enabled then wait for read events */
  if(recvq == ON)
    io_set_events(fd, IO_READ);
  
  /* 
   * If send queue has been enabled and there is data in the queue
   * wait for write events.
   */
  if(sendq == ON && io_list[fd].sendq.size)
    io_set_events(fd, IO_WRITE);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Register a file descriptor to the io_list.                                  *
 * -------------------------------------------------------------------------- */
int io_new(int fd, int type)
{  
  if(fd < 0 || fd >= MAX_FDS)
    return -1;

  io_list[fd].status.dead = 0;
  memset(&io_list[fd], 0, sizeof(struct io));

/*  queue_zero(&io_list[fd].recvq);
  queue_zero(&io_list[fd].sendq);
*/
  io_list[fd].type = type;
  
  io_add_fd(fd);

  if(!io_list[fd].flags)
    io_nonblock(fd);
  
  return fd;
}
/* -------------------------------------------------------------------------- *
 * Open a file.                                                               *
 * -------------------------------------------------------------------------- */
int io_open(const char *path, long flags, ...)
{
  int         fd;
  int         ret;
  struct stat st;
  va_list     args;
  
  if(!(flags & O_CREAT) && (stat(path, &st) == -1))
    return -1;  
  
  va_start(args, flags);
  
  fd = syscall_open(path, flags, va_arg(args, long));
  
  va_end(args);
  
  if(fd == -1)
    return -1;
  
  if(fd >= MAX_FDS)
  {
    syscall_close(fd);
    return -1;
  }
  
  io_list[fd].stat = st;
  
  ret = io_new(fd, FD_FILE);
  
  if(ret >= 0)
  {
    io_note(fd, "%s", path);
  }
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Close an fd.                                                               *
 * -------------------------------------------------------------------------- */
void io_close(int fd)
{
  if(io_list[fd].status.dead)
    return;
  
  if(fd < 0 || !io_list[fd].type)
    return;
  
  if(io_list[fd].sendq.size && !io_list[fd].status.closed)
    io_queued_write(fd);
  
  io_shutup(fd);

  syscall_close(fd);
  
  if(io_list[fd].recvq.size)
    queue_destroy(&io_list[fd].recvq);
  
  if(io_list[fd].sendq.size)
    queue_destroy(&io_list[fd].sendq);

  io_remove_fd(fd);
  io_list[fd].index = -1;
  
#ifdef HAVE_SSL
  if(io_list[fd].ssl)
    ssl_close(fd);
#endif /* HAVE_SSL */

  memset(&io_list[fd], 0, sizeof(struct io));
  io_list[fd].status.dead = 1;
}

/* -------------------------------------------------------------------------- *
 * Write a description string.                                                *
 * -------------------------------------------------------------------------- */
void io_note(int fd, const char *format, ...)
{
  va_list args;
  
  if(fd < 0)
    return;
  
  va_start(args, format);
  
  vsnprintf(io_list[fd].note, 64, format, args);
  
  va_end(args);
}

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
int io_vregister(int fd, int type, void *callback, va_list args)
{  
  if(fd < 0 || type >= IO_CB_MAX || type < 0)
    return -1;
  
  switch(type)
  {
    case IO_CB_ERROR:
      io_set_events(fd, IO_ERROR);
      break;
    case IO_CB_READ:
      io_set_events(fd, IO_READ);
      break;
    case IO_CB_WRITE:
      io_set_events(fd, IO_WRITE);
      break;
    case IO_CB_ACCEPT:
      io_set_events(fd, IO_READ);
      break;
    case IO_CB_CONNECT:
      io_set_events(fd, IO_READ | IO_WRITE);
      break;
    default:
      log(io_log, L_fatal, "invalid callback type %02u", type);
      return -1;
  }
  
  io_list[fd].callbacks[type] = callback;
  
  io_vset_args(fd, args);
  
  return 0;
}

int io_register(int fd, int type, void *callback, ...)
{
  int     ret;
  va_list args;
  
  va_start(args, callback);
  
  ret = io_vregister(fd, type, callback, args);
  
  va_end(args);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int io_unregister(int fd, int type)
{
  switch(type)
  {
    case IO_CB_ERROR:
      io_unset_events(fd, IO_ERROR);
      break;
    case IO_CB_READ:
      io_unset_events(fd, IO_READ);
      break;
    case IO_CB_WRITE:
      io_unset_events(fd, IO_WRITE);
      break;
    default:
      log(io_log, L_fatal, "invalid callback type %02u", type);
      return -1;
  }
  
  io_list[fd].callbacks[type] = NULL;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Read either from the fd directly or from its queue.                        *
 * -------------------------------------------------------------------------- */
int io_read(int fd, void *buf, size_t n)
{
  /* Catch invalid arguments */
  if(fd < 0)
    return -1;
  
  if(n == 0)
    return 0;
  
  /* fd is queued, read from the receive queue */
  if(io_list[fd].control.recvq)
  {
    /* Receive queue is empty */
    if(io_list[fd].recvq.size == 0)
    {
      syscall_errno = EAGAIN;
      return -1;
    }
    
    return queue_read(&io_list[fd].recvq, buf, n);
  }
  
  /* Read directly from fd */
  switch(io_list[fd].type)
  {
    case FD_SOCKET:
#ifdef HAVE_SSL
      if(io_list[fd].ssl)
        return ssl_read(fd, buf, IO_READ_SIZE);
      else
#endif /* HAVE_SSL */
      return syscall_recv(fd, buf, n, 0);
    default:
    case FD_FILE:
    case FD_PIPE:
      return syscall_read(fd, buf, n);
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Write either to the fd directly or to its queue.                           *
 * -------------------------------------------------------------------------- */
int io_write(int fd, const void *buf, size_t n)
{
  int ret;
  
  /* Catch invalid arguments */
  if(fd < 0)
    return -1;
  
  if(n == 0)
    return 0;
  
  /* fd is queued, write to the send queue */
  if(io_list[fd].control.sendq)
  {
    ret = queue_write(&io_list[fd].sendq, buf, n);
    
    if(io_list[fd].sendq.size)
      io_set_events(fd, IO_WRITE);
  
    return ret;
  }
  
  /* Write directly to fd */
  switch(io_list[fd].type)
  {
    case FD_SOCKET:
#ifdef HAVE_SSL
      if(io_list[fd].ssl)
        return ssl_write(fd, buf, n);
      else
#endif /* HAVE_SSL */
        return syscall_send(fd, buf, n, 0);
    default:
    case FD_FILE:
    case FD_PIPE:
      return syscall_write(fd, buf, n);
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Read either from the fd directly or from its queue.                        *
 * -------------------------------------------------------------------------- */
int io_gets(int fd, void *buf, size_t n)
{  
  /* Catch invalid arguments */
  if(fd < 0)
    return 0;
  
  if(n == 0)
    return 0;
  
  /* fd is queued, read from the receive queue */
  if(io_list[fd].control.recvq)
  {
    /* Receive queue is empty */
    if(io_list[fd].recvq.lines == 0)
    {
      if(io_list[fd].type == FD_SOCKET)
      {
        io_list[fd].error = EAGAIN;
      }
      
      return 0;
    }
    
    return queue_gets(&io_list[fd].recvq, buf, n);
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Write a line to fd or queue.                                               *
 * -------------------------------------------------------------------------- */
int io_puts(int fd, const char *s, ...)
{
  size_t len;
  va_list args;
  char buf[IO_LINE_SIZE];
  
  /* Catch invalid arguments */
  if(fd < 0)
    return -1;
  
  va_start(args, s);

  len = vsnprintf(buf, IO_LINE_SIZE - 1, s, args);
  
  va_end(args);
  
  buf[len++] = '\n';
    
  return io_write(fd, buf, len);
}

/* -------------------------------------------------------------------------- *
 * Write a line to fd or queue.                                               *
 * -------------------------------------------------------------------------- */
int io_vputs(int fd, const char *s, va_list args)
{
  size_t len;
  char buf[IO_LINE_SIZE];
  
  /* Catch invalid arguments */
  if(fd < 0)
    return -1;
  
  len = vsnprintf(buf, IO_LINE_SIZE - 1, s, args);
  
  buf[len++] = '\n';
    
  return io_write(fd, buf, len);
}

/* -------------------------------------------------------------------------- *
 *                                                                            *
 * -------------------------------------------------------------------------- */
void io_multi_start(struct fqueue *fifoptr)
{
  queue_zero(fifoptr);
}

uint32_t io_multi_write(struct fqueue *fifoptr, const void *buf, uint32_t n)
{
  return queue_write(fifoptr, buf, n);
}

void io_multi_link(struct fqueue *fifoptr, int fd)
{
/*  if(io_valid(fd))
  {*/
    queue_link(fifoptr, &io_list[fd].sendq);
    
    if(io_list[fd].sendq.size)
      io_set_events(fd, IO_WRITE);
/*  }*/
}

void io_multi_end(struct fqueue *fifoptr)
{
  queue_destroy(fifoptr);
}

/* -------------------------------------------------------------------------- *
 * Do a select() system call.                                                 *
 *                                                                            *
 * If timeout == NULL then return only when an event occurred.                *
 * else return after <timeout> miliseconds or when there was an event.        *
 * In the latter case the remaining time will be in *timeout.                 *
 * -------------------------------------------------------------------------- */
#ifndef USE_POLL
int io_select(int64_t *remain, int64_t *timeout)
{
  struct timeval  tv;
  struct timeval *tp = NULL;
  int             ret;
  size_t          i;
  
  /* timeout is present, set timeout pointer */
  if(timeout) if(*timeout >= 0LL)
  {
    timer_to_timeval(&tv, timeout);
    tp = &tv;
  }
  
  /* Do the actual select() */
  io_rfds_r = io_rfds;
  io_wfds_r = io_wfds;
  io_efds_r = io_efds;
  
  ret = syscall_select(io_top + 1, &io_rfds_r, &io_wfds_r, &io_efds_r, tp);
  
  /* Update system time */
  timer_update();
  
  /* Now set the returned events */
  if(io_top >= 0) for(i = 0; i <= io_top; i++)
  {
    if(!io_list[i].type)
      continue;
    
    io_set_revents(i);
  }
  
  /* If there was timeout value then return remaining time */
  if(tp && remain)
    timer_to_msec(remain, tp);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Do a poll() system call.                                                   *
 *                                                                            *
 * If timeout == NULL then return only when an event occurred.                *
 * else return after <timeout> miliseconds or when there was an event.        *
 * In the latter case the remaining time will be in *timeout.                 *
 * -------------------------------------------------------------------------- */
#else
int io_poll(int64_t *remain, int64_t *timeout)
{
  int to = -1;
  uint64_t old;
  uint64_t deadline = timer_mtime;
  size_t i;
  int ret;

  old = timer_mtime;

  /* There is a timeout, set it */
  if(timeout) if(*timeout >= 0LL) 
  {
    to = (int)(*timeout);
    deadline = timer_mtime + *timeout + 10;
  }

  /* Do the actual poll() */
  ret = syscall_poll(io_count ? io_pfds : NULL, io_count, to);

  /* Update system time */
  timer_update();

  /* 
   * If a timeout was set then set the timeout pointer 
   * to the remaining time until the poll deadline.
   */
  if(timeout && remain)
  {
    *remain = deadline - timer_mtime;

    if(*remain < 0LL)
    {
      if(*remain < -POLL_WARN_DELTA)
        log(io_log, L_warning, 
            "Timing error: poll() returned too late (diff: %lli)",
            *remain);

      *remain = 0;
    }

    if(*remain > *timeout)
    {
      if(*remain > *timeout + POLL_WARN_DELTA)
        log(io_log, L_warning,
            "Timing error: poll() returned too early (diff: %lli)",
            *remain);
      
      *remain = *timeout;
    }
  }
  
  /* Now set the returned events */
  for(i = 0; i < io_count; i++)
    io_set_revents(io_pfds[i].fd);
  
  return ret;
}
#endif

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void io_wait(void)
{
  io_multiplex(NULL, NULL);
  io_handle();
}

/* -------------------------------------------------------------------------- *
 * Handle pending I/O events                                                  *
 * -------------------------------------------------------------------------- */
void io_handle(void)
{
#ifdef USE_POLL
  
  size_t i;
  
  for(i = 0; i < io_count; i++)
    io_handle_fd(io_pfds[i].fd);
  
#else

  int i;
  
  if(io_top >= 0) for(i = 0; i <= io_top; i++)
  {
    if(io_list[i].type)
      io_handle_fd(i);
  }
  
#endif /* USE_SELECT || USE_POLL */  
}

/* -------------------------------------------------------------------------- *
 * Move an fd.                                                                *
 * -------------------------------------------------------------------------- */
void io_move(int from, int to)
{  
  if(io_list[to].type)
  {
    log(io_log, L_warning, "Cannot move fd %i to %i: Destination busy", from, to);
    return;
  }
  
  if(syscall_dup2(from, to) != to)
  {
    log(io_log, L_warning, "Failed dup()ing fd %i", from);
    return;
  }
  
  io_set_events(to, io_list[from].control.events);
  
  memcpy(&io_list[to], &io_list[from], sizeof(struct io));
  
  io_close(from);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void io_vset_args(int fd, va_list args)
{
  io_list[fd].args[0] = va_arg(args, void *);
  io_list[fd].args[1] = va_arg(args, void *);
  io_list[fd].args[2] = va_arg(args, void *);
  io_list[fd].args[3] = va_arg(args, void *);
}

void io_set_args(int fd, ...)
{
  va_list args;
  
  va_start(args, fd);
  io_vset_args(fd, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void io_dump(int fd)
{
  if(fd < 0 || fd >= MAX_FDS)
  {
    dump(io_log, "[================ i/o summary =================]");
    
    for(fd = 0; fd < MAX_FDS; fd++)
    {
      if(!io_list[fd].type)
        continue;
      
      dump(io_log, " #%u: <%s> %-20s",
           fd, io_types[io_list[fd].type], io_list[fd].note);
    }
    
    dump(io_log, "[============= end of i/o summary =============]");    
  }
  else
  {
#ifdef HAVE_SSL    
    char ciphbuf[100];
    
    if(io_list[fd].ssl)
      ssl_cipher(fd, ciphbuf, sizeof(ciphbuf));
    else
      ciphbuf[0] = '\0';
#endif /* HAVE_SSL */
    dump(io_log, "[================ i/o dump =================]");
    
    dump(io_log, "          fd: #%u", fd);
    dump(io_log, "        type: %s", io_types[io_list[fd].type]);
    dump(io_log, "       error: %s", syscall_strerror(io_list[fd].error));
    dump(io_log, "     control:%s%s%s%s (%s%s%s)",
         (io_list[fd].control.sendq ||
          io_list[fd].control.recvq ||
          io_list[fd].control.linebuf) ? " " : "",
         io_list[fd].control.sendq ? "s" : "",
         io_list[fd].control.recvq ? "r" : "",
         io_list[fd].control.linebuf ? "l" : "",
         (io_list[fd].control.events & IO_ERROR) ? "e" : "",
         (io_list[fd].control.events & IO_READ) ? "r" : "",
         (io_list[fd].control.events & IO_WRITE) ? "w" : "");
    dump(io_log, "      status:%s%s%s%s%s%s (%s%s)",
         (io_list[fd].status.err ||
          io_list[fd].status.closed ||
          io_list[fd].status.onread ||
          io_list[fd].status.onwrite ||
          io_list[fd].status.dead) ? " " : "",
         io_list[fd].status.err ? "e" : "",
         io_list[fd].status.closed ? "c" : "",
         io_list[fd].status.onread ? "r" : "",
         io_list[fd].status.onwrite ? "w" : "",
         io_list[fd].status.dead ? "d" : "",
         (io_list[fd].status.events & IO_READ) ? "r" : "",
         (io_list[fd].status.events & IO_WRITE) ? "w" : "");
    dump(io_log, "         ret: %i", io_list[fd].ret);
    dump(io_log, "        args: %p, %p, %p, %p", 
         io_list[fd].args[0], io_list[fd].args[1],
         io_list[fd].args[2], io_list[fd].args[3]);
    dump(io_log, "       flags:%s%s%s%s%s%s (%p)",
         (io_list[fd].flags & O_NONBLOCK) == O_NONBLOCK ? " O_NONBLOCK" : "",
         (io_list[fd].flags & O_RDWR) == O_RDWR ? " O_RDWR" : "",
         (io_list[fd].flags & O_RDONLY) == O_RDONLY ? " O_RDONLY" : "",
         (io_list[fd].flags & O_WRONLY) == O_WRONLY ? " O_WRONLY" : "",
         (io_list[fd].flags & O_CREAT) == O_CREAT ? " O_CREAT" : "",
         (io_list[fd].flags & O_TRUNC) == O_TRUNC ? " O_TRUNC" : "",
         (io_list[fd].flags & 04777));
    dump(io_log, "       sendq: %u", io_list[fd].sendq.size);
    dump(io_log, "       recvq: %u", io_list[fd].recvq.size);
    dump(io_log, "        note: %s", io_list[fd].note);
    dump(io_log, "   callbacks: E: %p R: %p W: %p", 
         io_list[fd].callbacks[IO_CB_ERROR],
         io_list[fd].callbacks[IO_CB_READ],
         io_list[fd].callbacks[IO_CB_WRITE]);
#ifdef HAVE_SSL    
    dump(io_log, "         ssl: %p (%s)", 
         io_list[fd].ssl, ciphbuf[0] ? ciphbuf : "none");
    dump(io_log, "    sslstate: %s",
         (io_list[fd].sslstate == SSL_WRITE_WANTS_READ ? "w/r" :
         (io_list[fd].sslstate == SSL_READ_WANTS_WRITE ? "r/w" :
         (io_list[fd].sslstate == SSL_ACCEPT_WANTS_READ ? "a/r" :
         (io_list[fd].sslstate == SSL_ACCEPT_WANTS_WRITE ? "a/w" :
         (io_list[fd].sslstate == SSL_CONNECT_WANTS_READ ? "c/r" :
         (io_list[fd].sslstate == SSL_CONNECT_WANTS_WRITE ? "c/w" : "none")))))));
    dump(io_log, "    sslerror: %s",
         (io_list[fd].sslerror == SSL_ERROR_WANT_READ ? "r" :
         (io_list[fd].sslerror == SSL_ERROR_WANT_WRITE ? "w" :
         (io_list[fd].sslerror == SSL_ERROR_SYSCALL ? "s" :
         (io_list[fd].sslerror == SSL_ERROR_ZERO_RETURN ? "z" : "none")))));
    dump(io_log, "    sslwhere: %s",
         (io_list[fd].sslwhere == SSL_READ ? "r" :
         (io_list[fd].sslwhere == SSL_WRITE ? "w" :
         (io_list[fd].sslwhere == SSL_ACCEPT ? "a" :
         (io_list[fd].sslwhere == SSL_CONNECT ? "c" : "none")))));
#endif /* HAVE_SSL */    
    dump(io_log, "[============= end of i/o dump =============]");        
  }
}
