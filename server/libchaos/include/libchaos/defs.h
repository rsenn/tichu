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
 * $Id: defs.h,v 1.45 2005/04/14 22:18:00 smoli Exp $
 */

#ifndef __DEFS_H
#define __DEFS_H

#ifdef WIN32
#define size_t unsigned int
//#define NULL (void *)0
#endif

#include <libchaos/config.h>

#undef CREATION

#undef PACKAGE_NAME
#undef PACKAGE_VERSION
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_RELEASE


#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
/*#undef WNOHANG*/
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif /* HAVE_SYS_MMAN_H */

#include <sys/stat.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#ifdef HAVE_SYS_TYPES_H

#ifndef _BSD_SIZE_T_
#define _BSD_SIZE_T_ unsigned int
#endif

# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif /* HAVE_SYS_RESOURCE_H */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef USE_SELECT
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#endif /* USE_SELECT */
#ifdef USE_POLL
#include <sys/poll.h>
#endif /* USE_POLL */

#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif /* HAVE_WINSOCK_H */


#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

/* hehe, nice hack */
#ifndef EVER
#define EVER ;;
#endif /* EVER */

/* max. path len */
#undef PATH_MAX
#define PATH_MAX     64

/* max. buf size */
#define BUFSIZE      1024

#define HOSTLEN      64
#define HOSTIPLEN    15
#define PROTOLEN     32
#define USERLEN      32

/* dynamic heap blocks are 128kb */
#define DYNAMIC_BLOCK_SIZE (1024 * 128)
                                
/* garbage collect heaps every 5 minutes */
#define GARBAGE_COLLECT_INTERVAL (300 * 1000LL)
#define GC_INTERVAL GARBAGE_COLLECT_INTERVAL
                            
/* timers per heap block */
#define TIMER_BLOCK_SIZE 16
 
/* queue chunks per heap block */
#define QUEUE_BLOCK_SIZE 256

/* dlink nodes per heap block */
#define DLINK_BLOCK_SIZE 256

/* log entries per heap block */
#define LOG_BLOCK_SIZE   16

/* mfile entries per heap block */
#define MFILE_BLOCK_SIZE 8

/* module entries per heap block */
#define MODULE_BLOCK_SIZE 96

/* listen entries per heap block */
#define LISTEN_BLOCK_SIZE 8

/* connect entries per heap block */
#define CONNECT_BLOCK_SIZE 8

/* net entries per heap block */
#define NET_BLOCK_SIZE 8

/* ini entries per heap block */
#define INI_BLOCK_SIZE 8

/* ini keys per heap block */
#define KEY_BLOCK_SIZE 256

/* ini keys per heap block */
#define SEC_BLOCK_SIZE 64

/* hook per heap block */
#define HOOK_BLOCK_SIZE 64

/* child per heap block */
#define CHILD_BLOCK_SIZE 2

/* sauth per heap block */
#define SAUTH_BLOCK_SIZE 64

/* ssl contexts per heap block */
#define SSL_BLOCK_SIZE 8

/* http clients per heap block */
#define HTTPC_BLOCK_SIZE 8

/* html parsers per heap block */
#define HTMLP_BLOCK_SIZE 8

/* filters per heap block */
#define FILTER_BLOCK_SIZE 8

/* gifs per heap block */
#define GIF_BLOCK_SIZE 8

/* images per heap block */
#define IMAGE_BLOCK_SIZE 8

/* graphs per heap block */
#define GRAPH_BLOCK_SIZE 8

/* cfgs per heap block */
#define CFG_BLOCK_SIZE 1

/* db connects per heap block */
#define DB_BLOCK_SIZE 4

/* ttf fonts per heap block */
#define TTF_BLOCK_SIZE 4

/* wav fonts per heap block */
#define WAV_BLOCK_SIZE 4

/*
 * Warn if system time differs more than +/-TIMER_WARN_DELTA 
 * from expected system time.
 */
#define TIMER_MAX_DRIFT  10000LL

/* 
 * Warn if a timer gets executed TIMER_WARN_DELTA 
 * miliseconds too early or too late.
 */
#define TIMER_WARN_DELTA  10LL

/*
 * Warn if expected return from poll() drifts 
 * from real time by more than these msecs.
 */
#define POLL_WARN_DELTA 10LL

#endif /* __DEFS_H */
