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
 * $Id: filter.h,v 1.7 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_FILTER_H
#define LIB_FILTER_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/net.h>
#include <libchaos/dlink.h>

#ifdef HAVE_SSL
#include <libchaos/ssl.h>
#endif /* HAVE_SSL */

/* -------------------------------------------------------------------------- *
 * Kernel headers                                                             *
 * -------------------------------------------------------------------------- */
#ifdef HAVE_LINUX_TYPES_H
#include <linux/types.h>
#endif
#ifdef HAVE_LINUX_FILTER_H
#include <linux/filter.h>
#endif
#ifdef HAVE_NET_BPF_H
#include <net/bpf.h>
#endif /* HAVE_NET_BPF_H */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

#ifdef LINUX_SOCKET_FILTER  
#define bf_len   len
#define bf_insns filter
#define bpf_insn sock_filter
#endif /* LINUX_SOCKET_FILTER */

#ifdef BSD_SOCKET_FILTER
#define SKF_NET_OFF ETHER_HDR_LEN
#endif /* BSD_SOCKET_FILTER */

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

#define FILTER_PROTO 0
#define FILTER_SRCIP 1
#define FILTER_DSTIP 2
#define FILTER_SRCNET 3
#define FILTER_DSTNET 4

#define FILTER_DENY   0
#define FILTER_ACCEPT 1

#define FILTER_LIFETIME (60 * 1000LLU)
#define FILTER_INTERVAL (120 * 1000LLU)

#define FILTER_MAX_INSTRUCTIONS 4096
#define FILTER_MAX_SIZE         (FILTER_MAX_INSTRUCTIONS * sizeof(struct bpf_insn))

/* -------------------------------------------------------------------------- *
 * filter block structure.                                                    *
 * -------------------------------------------------------------------------- */
struct filter_rule {
  struct node         node;
  int                 type;
  int                 action;
  uint32_t            address;
  uint32_t            netmask;
  uint64_t            ts;
};

struct filter {
  struct node         node;                 /* linking node for filter_list */
  uint32_t            id;
  uint32_t            refcount;             /* times this block is referenced */
  uint32_t            hash;
  
#ifdef LINUX_SOCKET_FILTER  
  struct sock_fprog   prog;
#endif /* LINUX_SOCKET_FILTER */
#ifdef BSD_SOCKET_FILTER  
  struct bpf_program  prog;
#endif /* BSD_SOCKET_FILTER */
  
  /* externally initialised */
  struct list         rules;
  struct list         sockets;      /* the sockets the filter is attached to */
  struct list         listeners;
  char                name[HOSTLEN + 1];    /* user-definable name */
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int           filter_log;
extern struct sheap  filter_heap;       /* heap containing filter blocks */
extern struct sheap  filter_rule_heap;  /* heap containing filter rules */
extern struct dheap  filter_prog_heap;  /* heap containing the actual filters */
extern struct list   filter_list;       /* list linking filter blocks */
extern struct timer *filter_timer;
extern uint32_t      filter_id;
extern int           filter_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize filterer heap and add garbage collect timer.                    *
 * -------------------------------------------------------------------------- */
extern void           filter_init            (void);

/* -------------------------------------------------------------------------- *
 * Destroy filterer heap and cancel timer.                                    *
 * -------------------------------------------------------------------------- */
extern void           filter_shutdown        (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int            filter_collect         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_default         (struct filter  *fptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct filter *filter_add             (const char     *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_delete          (struct filter  *fptr);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void           filter_release         (struct filter  *fptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct filter *filter_pop             (struct filter  *fptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct filter *filter_push            (struct filter **fptrptr);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct filter *filter_find            (const char     *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_set_name        (struct filter  *fptr,
                                              const char     *name);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char    *filter_get_name        (struct filter  *fptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct filter *filter_find_name       (const char     *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct filter *filter_find_id         (uint32_t        id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_rule_add        (struct filter  *fptr,
                                              int             type,
                                              int             action, 
                                              uint32_t        data1,  
                                              uint32_t        data2,
                                              uint64_t        lifetime);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_rule_insert     (struct filter  *fptr,
                                              int             type,
                                              int             action,
                                              uint32_t        data1,
                                              uint32_t        data2,
                                              uint64_t        lifetime);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_rule_delete     (struct filter  *fptr,
                                              int             type,
                                              int             action,
                                              uint32_t        data1,
                                              uint32_t        data2);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_rule_compile    (struct filter  *fptr);

/* -------------------------------------------------------------------------- *
 * Attach a filter to a socket                                                *
 * -------------------------------------------------------------------------- */
extern int            filter_attach_socket   (struct filter  *fptr, 
                                              int             fd);

/* -------------------------------------------------------------------------- *
 * Detach filter from socket                                                  *
 * -------------------------------------------------------------------------- */
extern int            filter_detach_socket   (struct filter  *fptr,
                                              int             fd);

/* -------------------------------------------------------------------------- *
 * Attach a filter to a listener                                              *
 * -------------------------------------------------------------------------- */
extern int            filter_attach_listener (struct filter  *fptr, 
                                              struct listen  *liptr);

/* -------------------------------------------------------------------------- *
 * Detach filter from listener                                                *
 * -------------------------------------------------------------------------- */
extern int            filter_detach_listener (struct filter  *fptr, 
                                              struct listen  *liptr);
      
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           filter_reattach_all    (struct filter  *fptr);
      
/* -------------------------------------------------------------------------- *
 * Dump filterers and filter heap.                                            *
 * -------------------------------------------------------------------------- */
extern void           filter_dump            (struct filter  *lptr);

#endif /* LIB_FILTER_H */
