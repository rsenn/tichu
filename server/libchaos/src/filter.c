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
 * $Id: filter.c,v 1.10 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#include <libchaos/defs.h>

#ifdef HAVE_SOCKET_FILTER

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/filter.h>
#include <libchaos/listen.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/io.h>

#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif /* HAVE_NET_ETHERNET_H */

#include <sys/ioctl.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int           filter_log; 
struct sheap  filter_heap;       /* heap containing filter blocks */
struct sheap  filter_rule_heap;  /* heap containing filter rules */
struct dheap  filter_prog_heap;  /* heap containing filter rules */
struct list   filter_list;       /* list linking filter blocks */
struct timer *filter_timer;
uint32_t      filter_id;
int           filter_dirty;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define IPHDR_PROTO  0x09 /* Byte-location of protocol type in IP header */
#define IPHDR_SRCIP  0x0c /* Byte-location of source IP in IP header */
#define IPHDR_DSTIP  0x10 /* Byte-location of source IP in IP header */
#define IPHDR_TOTLEN 2  

#define ETHDR_PROTO 0x0c

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_jmp         (struct filter *fptr, 
                                uint16_t       code, 
                                uint8_t        jt,
                                uint8_t        jf, 
                                uint32_t       k);
static void filter_begin       (struct filter *fptr);
static void filter_ins         (struct filter *fptr,
                                uint16_t       code, 
                                uint32_t       k);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_cleanup(void)
{
  struct filter      *fptr;
  struct filter_rule *frptr;
  struct node        *next;
  int                 reattach;
  
  dlink_foreach(&filter_list, fptr)
  {
    reattach = 0;
    
    dlink_foreach_safe(&fptr->rules, frptr, next)
    {
      if(frptr->ts && frptr->ts < timer_mtime)
      {
        dlink_delete(&fptr->rules, &frptr->node);
                                   
        log(filter_log, L_verbose, "Deleted socket filter for %s",
            net_ntoa(*(struct in_addr *)&frptr->address));

        mem_static_free(&filter_rule_heap, frptr);
                                                                
        reattach = 1;
      }
      
    }
      
    if(reattach)
    {
      filter_rule_compile(fptr);

      filter_reattach_all(fptr);
    }
      
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_begin(struct filter *fptr)
{
  if(fptr->prog.bf_insns)
    mem_dynamic_free(&filter_prog_heap, fptr->prog.bf_insns);
  
  fptr->prog.bf_len = 0;
  fptr->prog.bf_insns = NULL;
  
  /* This drops non-ip shit */
/*  filter_ins(fptr, BPF_LD|BPF_B|BPF_ABS, SKF_NET_OFF +   );
  filter_jmp(fptr, BPF_JMP|BPF_JEQ|BPF_K, 0, 1, 0x92); 
  filter_ins(fptr, BPF_RET + BPF_K, 0xffff);
  filter_ins(fptr, BPF_RET + BPF_K, 0x0);*/
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_entry(struct filter *fptr, int type, 
                         uint32_t data1, uint32_t data2)
{
  switch(type)
  {
/*    case FILTER_PROTO:
      filter_ins(fptr, BPF_LD|BPF_H|BPF_ABS, ETHER_HDR_LEN + IPHDR_PROTO);
      filter_jmp(fptr, BPF_JMP|BPF_JEQ|BPF_K, 1, 0, (data & 0xffff) | ((data & 0xffff) << 16));
      break;*/
    case FILTER_SRCIP:
      filter_ins(fptr, BPF_LD|BPF_W|BPF_ABS, SKF_NET_OFF + IPHDR_SRCIP);
      filter_jmp(fptr, BPF_JMP|BPF_JEQ|BPF_K, 0, 1, ntohl(data1));
      break;
    case FILTER_DSTIP:
      filter_ins(fptr, BPF_LD|BPF_W|BPF_ABS, SKF_NET_OFF + IPHDR_DSTIP);
      filter_jmp(fptr, BPF_JMP|BPF_JEQ|BPF_K, 0, 1, ntohl(data1));
      break;
    case FILTER_SRCNET:
      filter_ins(fptr, BPF_LD|BPF_W|BPF_ABS, SKF_NET_OFF + IPHDR_SRCIP);
      filter_ins(fptr, BPF_ALU|BPF_AND|BPF_K, ntohl(data2));
      filter_jmp(fptr, BPF_JMP|BPF_JEQ|BPF_K, 0, 1, ntohl(data1));
      break;
    case FILTER_DSTNET:
      filter_ins(fptr, BPF_LD|BPF_W|BPF_ABS, SKF_NET_OFF + IPHDR_DSTIP);
      filter_ins(fptr, BPF_ALU|BPF_AND|BPF_K, ntohl(data2));
      filter_jmp(fptr, BPF_JMP|BPF_JEQ|BPF_K, 0, 1, ntohl(data1));
      break;
  }
  
  filter_ins(fptr, BPF_RET + BPF_K, 0);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_end(struct filter *fptr)
{
  filter_ins(fptr, BPF_LD|BPF_H|BPF_ABS, SKF_NET_OFF + IPHDR_TOTLEN);
/*  filter_ins(fptr, BPF_ALU|BPF_ADD|BPF_K, ETHER_HDR_LEN);*/
  filter_ins(fptr, BPF_RET + BPF_A, 0);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_ins(struct filter *fptr, uint16_t code, uint32_t k)
{
  fptr->prog.bf_insns = 
    mem_dynamic_realloc(&filter_prog_heap, fptr->prog.bf_insns, 
                        sizeof(struct bpf_insn) * (fptr->prog.bf_len + 1));

  fptr->prog.bf_insns[fptr->prog.bf_len].code = code;
  fptr->prog.bf_insns[fptr->prog.bf_len].k = k;
  fptr->prog.bf_insns[fptr->prog.bf_len].jt = 0;
  fptr->prog.bf_insns[fptr->prog.bf_len].jf = 0;
  
  fptr->prog.bf_len++;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void filter_jmp(struct filter *fptr, uint16_t code, 
                       uint8_t jt, uint8_t jf, uint32_t k)
{
  fptr->prog.bf_insns = 
    mem_dynamic_realloc(&filter_prog_heap, fptr->prog.bf_insns, 
                        sizeof(struct bpf_insn) * (fptr->prog.bf_len + 1));

  fptr->prog.bf_insns[fptr->prog.bf_len].code = code;
  fptr->prog.bf_insns[fptr->prog.bf_len].jt = jt;
  fptr->prog.bf_insns[fptr->prog.bf_len].jf = jf;
  fptr->prog.bf_insns[fptr->prog.bf_len].k = k;
  
  fptr->prog.bf_len++;
}

/* -------------------------------------------------------------------------- *
 * Initialize filterer heap and add garbage collect timer.                    *
 * -------------------------------------------------------------------------- */
void filter_init(void)
{
  filter_log = log_source_register("filter");
  
  dlink_list_zero(&filter_list);
  
  filter_id = 0;
  filter_dirty = 0;
  
  mem_static_create(&filter_heap, sizeof(struct filter), FILTER_BLOCK_SIZE);
  mem_static_note(&filter_heap, "filter block heap");
  mem_static_create(&filter_rule_heap, sizeof(struct filter_rule), FILTER_BLOCK_SIZE);
  mem_static_note(&filter_rule_heap, "filter rule heap");
  mem_dynamic_create(&filter_prog_heap, FILTER_MAX_SIZE);

  filter_timer = timer_start(filter_cleanup, FILTER_INTERVAL);
  
  log(filter_log, L_status, "Initialized [filter] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy filterer heap and cancel timer.                                    *
 * -------------------------------------------------------------------------- */
void filter_shutdown(void)
{
  struct filter *fptr;
  struct filter *next;
  
  /* Report status */
  log(filter_log, L_status, "Shutting down [filter] module...");
  
  timer_cancel(&filter_timer);
  
  /* Remove all filter blocks */
  dlink_foreach_safe(&filter_list, fptr, next)
  {
    if(fptr->refcount)
      fptr->refcount--;

    filter_delete(fptr);
  }

  /* Destroy filter prog heap */
  mem_dynamic_destroy(&filter_prog_heap);
  
  /* Destroy filter rule heap */
  mem_static_destroy(&filter_rule_heap);
  
  /* Destroy filter block heap */
  mem_static_destroy(&filter_heap);
  
  /* Unregister log source */
  log_source_unregister(filter_log);
}

/* -------------------------------------------------------------------------- *
 * Collect filter block garbage.                                              *
 * -------------------------------------------------------------------------- */
int filter_collect(void)
{
  struct filter *cnptr;
  struct filter *next;
  size_t         n = 0;
  
  if(filter_dirty)
  {
    /* Report verbose */
    log(filter_log, L_verbose, "Doing garbage collect for [filter] module.");
    
    /* Free all filter blocks with a zero refcount */
    dlink_foreach_safe(&filter_list, cnptr, next)
    {
      if(!cnptr->refcount)
      {
        filter_delete(cnptr);
        
        n++;
      }
    }
  
    /* Collect garbage on filter_heap */
    mem_static_collect(&filter_heap);
    
    filter_dirty = 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_default(struct filter *filter)
{
  dlink_node_zero(&filter->node);
  
  strcpy(filter->name, "default");
  filter->id = 0;
  filter->refcount = 0;
  filter->hash = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct filter *filter_add(const char *name)
{
  struct filter *filter;
  
  filter = mem_static_alloc(&filter_heap);
  
  strlcpy(filter->name, name, sizeof(filter->name));
  filter->hash = strihash(filter->name);
  filter->id = filter_id++;
  filter->refcount = 1;

  dlink_add_tail(&filter_list, &filter->node, filter);
  
  log(filter_log, L_status, "Added filter block: %s", filter->name);
  
  return filter;
}     
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_delete(struct filter *filter)
{
  log(filter_log, L_status, "Deleting filter block: %s", filter->name);
  
  dlink_delete(&filter_list, (struct node *)filter);
  
  mem_static_free(&filter_heap, filter);
}

 /* -------------------------------------------------------------------------- *
  * Loose all references                                                       *
  * -------------------------------------------------------------------------- */
void filter_release(struct filter *fptr)
{
  filter_dirty = 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct filter *filter_pop(struct filter *fptr)
{
  if(fptr)
  {
    if(!fptr->refcount)
      log(filter_log, L_warning, "Poping deprecated filter: %s",
          fptr->name);
    
    fptr->refcount++;
  }

  return fptr;
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct filter *filter_push(struct filter **fptrptr)
{
  if(*fptrptr)
  {
    if(!(*fptrptr)->refcount)
    {
      log(filter_log, L_warning, "Trying to push deprecated filter %s",
          (*fptrptr)->name);
    }
    else
    {
      if(--(*fptrptr)->refcount == 0)
        filter_release(*fptrptr);
    }
        
    (*fptrptr) = NULL;
  }
    
  return *fptrptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_set_name(struct filter *filter, const char *name)
{
  strlcpy(filter->name, name, sizeof(filter->name));
  
  filter->hash = strihash(filter->name);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *filter_get_name(struct filter *filter)
{
  return filter->name;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct filter *filter_find_name(const char *name)
{
  struct node   *node;
  struct filter *filter;
  uint32_t       hash;
  
  hash = strihash(name);
  
  dlink_foreach(&filter_list, node)
  {
    filter = node->data;
    
    if(filter->hash == hash)
    {
      if(!stricmp(filter->name, name))
        return filter;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct filter *filter_find_id(uint32_t id)
{
  struct filter *fptr;
  
  dlink_foreach(&filter_list, fptr)
  {
    if(fptr->id == id)
      return fptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct filter_rule *filter_rule_find(struct filter *fptr, int type, int action,
                                     uint32_t data1, uint32_t data2)
{
  struct filter_rule *rule;
  
  dlink_foreach(&fptr->rules, rule)
  {
    if(rule->type == type && rule->action == action && rule->address == data1 && rule->netmask == data2)
      return rule;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_rule_add(struct filter *fptr, int type, int action, 
                     uint32_t data1, uint32_t data2, uint64_t lifetime)
{
  struct filter_rule *frptr;
  
  if(filter_rule_find(fptr, type, action, data1, data2))
    return;
  
  frptr = mem_static_alloc(&filter_rule_heap);
  
  frptr->type = type;
  frptr->action = action;
  frptr->address = data1;
  frptr->netmask = data2;
  
  if(lifetime)
    frptr->ts = timer_mtime + lifetime;
  else
    frptr->ts = 0;
  
  dlink_add_tail(&fptr->rules, &frptr->node, frptr);
  
  log(filter_log, L_verbose, "Added socket filter for %s",
      net_ntoa(*(struct in_addr *)&data1));
 }

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_rule_insert(struct filter *fptr, int type, int action, 
                        uint32_t data1, uint32_t data2, uint64_t lifetime)
{
  struct filter_rule *frptr;
  
  if(filter_rule_find(fptr, type, action, data1, data2))
    return;
  
  frptr = mem_static_alloc(&filter_rule_heap);
  
  frptr->type = type;
  frptr->action = action;
  frptr->address = data1;
  frptr->netmask = data2;
  
  if(lifetime)
    frptr->ts = timer_mtime + lifetime;
  else
    frptr->ts = 0;
  
  dlink_add_head(&fptr->rules, &frptr->node, frptr);
  
  log(filter_log, L_verbose, "Inserted socket filter for %s",
      net_ntoa(*(struct in_addr *)&data1));
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_rule_delete(struct filter *fptr, int type, int action, uint32_t data1, uint32_t data2)
{
  struct filter_rule *rule;
  struct node        *next;
  
  dlink_foreach_safe(&fptr->rules, rule, next)
  {
    if(rule->type == type && rule->action == action && rule->address == data1 && rule->netmask == data2)
      dlink_delete(&fptr->rules, &rule->node);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_rule_compile(struct filter *fptr)
{
  struct filter_rule *frptr;
  
/*  filter_end(fptr);*/
  filter_begin(fptr);
  
  dlink_foreach(&fptr->rules, frptr)
    filter_entry(fptr, frptr->type, frptr->address, frptr->netmask);
  
  filter_end(fptr);
}

/* -------------------------------------------------------------------------- *
 * Attach a filter to a socket                                                *
 * -------------------------------------------------------------------------- */
int filter_attach_socket(struct filter *fptr, int fd)
{
#ifdef LINUX_SOCKET_FILTER
  int error = 0;
  int errsize = sizeof(error);

  syscall_setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &fptr->prog, 
                      sizeof(fptr->prog));
  syscall_getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &errsize);
  
  return error;
#endif /* LINUX_SOCKET_FILTER */
#ifdef BSD_PACKET_FILTER 
  return ioctl(fd, BIOCSETF, &fptr->prog);
#endif /* BSD_PACKET_FILTER */
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Detach filter from socket                                                  *
 * -------------------------------------------------------------------------- */
int filter_detach_socket(struct filter *fptr, int fd)
{
#ifdef LINUX_SOCKET_FILTER
  struct sock_fprog dummy = { 0, NULL };    
  
  return syscall_setsockopt(fd, SOL_SOCKET, SO_DETACH_FILTER, &dummy, sizeof(dummy));
#endif /* LINUX_SOCKET_FILTER */
#ifdef BSD_SOCKET_FILTER
  struct bpf_program dummy = { 0, NULL };    
  
  return ioctl(fd, BIOCSETF, &dummy);
#endif /* BSD_PACKET_FILTER */
}

/* -------------------------------------------------------------------------- *
 * Attach a filter to a listener                                              *
 * -------------------------------------------------------------------------- */
int filter_attach_listener(struct filter *fptr, struct listen *liptr)
{
  struct node *nptr;
  
  nptr = dlink_node_new();
  
  dlink_add_tail(&fptr->listeners, nptr, liptr);
  
  return listen_attach_filter(liptr, fptr);
}

/* -------------------------------------------------------------------------- *
 * Detach filter from listener                                                *
 * -------------------------------------------------------------------------- */
int filter_detach_listener(struct filter *fptr, struct listen *liptr)
{
  struct node *nptr;
  
  nptr = dlink_find_delete(&fptr->listeners, liptr);
  
  if(nptr)
  {
    dlink_node_free(nptr);
  
    return listen_detach_filter(liptr);
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Reattach filter to a listener (After a recompile)                          *
 * -------------------------------------------------------------------------- */
int filter_reattach_listener(struct filter *fptr, struct listen *liptr)
{
  if(liptr->filter)
    filter_detach_socket(fptr, liptr->fd);
  
  return filter_attach_socket(fptr, liptr->fd);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void filter_reattach_all(struct filter *fptr)
{
  struct node   *nptr;
  struct listen *lptr = NULL;
  
  dlink_foreach_data(&fptr->listeners, nptr, lptr)
    filter_reattach_listener(fptr, lptr);
}

/* -------------------------------------------------------------------------- *
 * Dump filterers and filter heap.                                            *
 * -------------------------------------------------------------------------- */
void filter_dump(struct filter *fptr)
{
  if(fptr == NULL)
  {
    dump(filter_log, "[============== filter summary ===============]");
    
    dlink_foreach(&filter_list, fptr)
      dump(filter_log, " #%03u: [%u] %-20s",
           fptr->id, 
           fptr->refcount,
           fptr->name);
    
    dump(filter_log, "[========== end of filter summary ============]");
  }
  else
  {
    struct filter_rule *rule;
    uint32_t            i = 0;
    
    dump(filter_log, "[============== filter dump ===============]");
    dump(filter_log, "         id: #%u", fptr->id);
    dump(filter_log, "   refcount: %u", fptr->refcount);
    dump(filter_log, "       hash: %p", fptr->hash);
    dump(filter_log, "       name: %s", fptr->name);

    dump(filter_log, "  listeners: %u nodes", fptr->listeners.size);
    
    /* dump filter rules */
    dlink_foreach(&fptr->rules, rule)
    {
      char netmask[16];
      
      net_ntoa_r(*(struct in_addr *)&rule->netmask, netmask);
      
      dump(filter_log, "   rule #%u: %s %s %s/%s",
           i,
           rule->type == FILTER_PROTO ? "PROTO" :
           (rule->type == FILTER_SRCIP ? "SRCIP" : 
            (rule->type == FILTER_SRCNET ? "SRCNET" :
             (rule->type == FILTER_DSTIP ? "DSTIP" : "DSTNET"))),
           rule->action == FILTER_DENY ? "DENY" : "ACCEPT",
           net_ntoa(*(struct in_addr *)&rule->address), netmask);
      
      i++;
    }
    
    /* dump filter rules */
    if(fptr->prog.bf_len)
    {
      dump(filter_log, " ----------- filter program -------------");
      
      for(i = 0; i < fptr->prog.bf_len; i++)
      {
        struct bpf_insn *p;
        char operand[64];
        char *fmt, *op;
        int v;
        
        p = &fptr->prog.bf_insns[i];
        v = p->k;
        
        /* disassemble filter rule */
        switch (p->code)
        {
          default:
            op = "unimp";
            fmt = "%x";
            v = p->code;
            break;
                  
          case BPF_RET|BPF_K:
            op = "ret";
            fmt = "#%d";
            break;
                  
          case BPF_RET|BPF_A:
            op = "ret";
            fmt = "";
            break;
                  
          case BPF_LD|BPF_W|BPF_ABS:
            op = "ld";
            fmt = "[%d]";
            break;
                  
          case BPF_LD|BPF_H|BPF_ABS:
            op = "ldh";
            fmt = "[%d]";
            break;
                  
          case BPF_LD|BPF_B|BPF_ABS:
            op = "ldb";
            fmt = "[%d]";
            break;
                  
          case BPF_LD|BPF_W|BPF_LEN:
            op = "ld";
            fmt = "#pktlen";
          case BPF_LD|BPF_W|BPF_IND:
            op = "ld";
            fmt = "[x + %d]";
            break;
          
          case BPF_LD|BPF_H|BPF_IND:
            op = "ldh";
            fmt = "[x + %d]";
            break;
          
          case BPF_LD|BPF_B|BPF_IND:
            op = "ldb";
            fmt = "[x + %d]";
            break;
          
          case BPF_LD|BPF_IMM:
            op = "ld";
            fmt = "#%x";
            break;
            
          case BPF_LDX|BPF_IMM:
            op = "ldx";
            fmt = "#%x";
            break;
                  
          case BPF_LDX|BPF_MSH|BPF_B:
            op = "ldxb";
            fmt = "4*([%d]&0xf)";
            break;
                  
          case BPF_LD|BPF_MEM:
            op = "ld";
            fmt = "M[%d]";
            break;
                  
          case BPF_LDX|BPF_MEM:
            op = "ldx";
            fmt = "M[%d]";
            break;
                  
          case BPF_ST:
            op = "st";
            fmt = "M[%d]";
            break;
                  
          case BPF_STX:
            op = "stx";
            fmt = "M[%d]";
            break;
                  
          case BPF_JMP|BPF_JA:
            op = "ja";
            fmt = "%d";
            v = i + 1 + p->k;
            break;
            
          case BPF_JMP|BPF_JGT|BPF_K:
            op = "jgt";
            fmt = "#%x";
            break;
            
          case BPF_JMP|BPF_JGE|BPF_K:
            op = "jge";
            fmt = "#%x";
            break;
            
          case BPF_JMP|BPF_JEQ|BPF_K:
            op = "jeq";
            fmt = "#%x";
            break;
            
          case BPF_JMP|BPF_JSET|BPF_K:
            op = "jset";
            fmt = "#%x";
            break;
            
          case BPF_JMP|BPF_JGT|BPF_X:
            op = "jgt";
            fmt = "x";
            break;
            
          case BPF_JMP|BPF_JGE|BPF_X:
            op = "jge";
            fmt = "x";
            break;
            
          case BPF_JMP|BPF_JEQ|BPF_X:
            op = "jeq";
            fmt = "x";
            break;
            
          case BPF_JMP|BPF_JSET|BPF_X:
            op = "jset";
            fmt = "x";
            break;
          case BPF_ALU|BPF_ADD|BPF_X:
            op = "add";
            fmt = "x";
            break;
            
          case BPF_ALU|BPF_SUB|BPF_X:
            op = "sub";
            fmt = "x";
            break;
            
          case BPF_ALU|BPF_MUL|BPF_X:
            op = "mul";
            fmt = "x";
            break;
                  
          case BPF_ALU|BPF_DIV|BPF_X:
            op = "div";
            fmt = "x";
            break;
                  
          case BPF_ALU|BPF_AND|BPF_X:
            op = "and";
            fmt = "x";
            break;
                  
          case BPF_ALU|BPF_OR|BPF_X:
            op = "or";
            fmt = "x";
            break;
                  
          case BPF_ALU|BPF_LSH|BPF_X:
            op = "lsh";
            fmt = "x";
            break;
                  
          case BPF_ALU|BPF_RSH|BPF_X:
            op = "rsh";
            fmt = "x";
            break;
                  
          case BPF_ALU|BPF_ADD|BPF_K:
            op = "add";
            fmt = "#%d";
            break;
          case BPF_ALU|BPF_SUB|BPF_K:
            op = "sub";
            fmt = "#%d";
            break;
                  
          case BPF_ALU|BPF_MUL|BPF_K:
            op = "mul";
            fmt = "#%d";
            break;
                  
          case BPF_ALU|BPF_DIV|BPF_K:
            op = "div";
            fmt = "#%d";
            break;
                  
          case BPF_ALU|BPF_AND|BPF_K:
            op = "and";
            fmt = "#%x";
            break;
                  
          case BPF_ALU|BPF_OR|BPF_K:
            op = "or";
            fmt = "#%x";
            break;
                  
          case BPF_ALU|BPF_LSH|BPF_K:
            op = "lsh";
            fmt = "#%d";
            break;
                  
          case BPF_ALU|BPF_RSH|BPF_K:
            op = "rsh";
            fmt = "#%d";
            break;
                  
          case BPF_ALU|BPF_NEG:
            op = "neg";
            fmt = "";
            break;
                  
          case BPF_MISC|BPF_TAX:
            op = "tax";
            fmt = "";
            break;
                  
          case BPF_MISC|BPF_TXA:
            op = "txa";
            fmt = "";
            
          break;
        }
        
        snprintf(operand, sizeof(operand), fmt, v);
        dump(filter_log, 
             (BPF_CLASS(p->code) == BPF_JMP &&
              BPF_OP(p->code) != BPF_JA) ?
             "(%03d) %-8s %-16s jt %d\tjf %d" :
             "(%03d) %-8s %s",
             i, op, operand, i + 1 + p->jt, i + 1 + p->jf);
      }
    }
    
    dump(filter_log, "[========== end of filter dump ============]");    
  }
}

#endif /* HAVE_SOCKET_FILTER */
