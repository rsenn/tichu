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
 * $Id: child.c,v 1.41 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#include <sys/wait.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/child.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>
#include <libchaos/syscall.h>

/* -------------------------------------------------------------------------- *
 * Local variables                                                            *
 * -------------------------------------------------------------------------- */
int           child_log;          /* Log source */
struct sheap  child_heap;         /* Heap containing child blocks */
struct list   child_list;         /* List linking child blocks */
int           child_dirty;        /* we need a garbage collect */
uint32_t      child_id;           /* Next serial number */

/* -------------------------------------------------------------------------- *
 * Initialize child heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void child_init(void)
{
  child_log = log_source_register("child");
  
  /* Zero child block list */
  dlink_list_zero(&child_list);
  
  /* Setup child heap & timer */
  mem_static_create(&child_heap, sizeof(struct child), CHILD_BLOCK_SIZE);
  mem_static_note(&child_heap, "child block heap");
                                 
  log(child_log, L_status, "Initialized [child] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy child heap and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
void child_shutdown(void)
{
  struct child *ciptr;
  struct child *next;
  
  log(child_log, L_status, "Shutting down [child] module...");
  
  /* Remove all child blocks */
  dlink_foreach_safe(&child_list, ciptr, next)
  {
    if(ciptr->refcount)
      ciptr->refcount--;
    
    child_delete(ciptr);
  }

  /* Destroy static heap */
  mem_static_destroy(&child_heap);
  
  /* Unregister log source */
  log_source_unregister(child_log);
}

/* -------------------------------------------------------------------------- *
 * Garbage collect child blocks                                               *
 * -------------------------------------------------------------------------- */
void child_collect(void)
{
  struct child *ciptr;
  struct child *next;
  size_t        n = 0;
  
  /* Only collect if we pushed out some block */
  if(child_dirty)
  {
    dlink_foreach_safe(&child_list, ciptr, next)
    {
      if(!ciptr->refcount)
      {
        child_delete(ciptr);
        
        n++;
      }
    }
    
    mem_static_collect(&child_heap);
    
    log(child_log, L_status, "Collected %u blocks.", n);
    
    child_dirty = 0;
  }
}

/* -------------------------------------------------------------------------- *
 * Create a new child block                                                   *
 * -------------------------------------------------------------------------- */
struct child *child_new(const char *path, uint32_t channels,
                        const char *argv, uint64_t interval, 
                        int         autostart)
{
  struct child *child;
  char         *p;
  
  /* Allocate child block */
  child = mem_static_alloc(&child_heap);
  
  child_default(child);
  
  /* Externally initialised stuff */
  strlcpy(child->path, path, sizeof(child->path));
  
  child->chans = channels;
  child->interval = interval;
  child->autostart = autostart;
  
  strlcpy(child->argv, argv, sizeof(child->argv));
  
  /* Internally initialised stuff */
  child->pid = -1;
  child->status = CHILD_IDLE;
  child->id = child_id++;
  child->callback = NULL;

  p = strrchr(child->path, '/');
  
  if(p) 
  {
    if(p[1])
      strlcpy(child->name, &p[1], sizeof(child->path));
  }
  
  if(!child->name[0])
    strlcpy(child->name, child->path, sizeof(child->name));  
  
  /* Add to child list */
  dlink_add_tail(&child_list, &child->node, child);
 
  /* If we're auto-childing then initiate it now */
  if(child->autostart)
    child_launch(child);

  log(child_log, L_status, "Added child block: %s", child->name);
  
  return child;
}     

/* -------------------------------------------------------------------------- *
 * Update the externally initialised stuff of a child block.                  *
 * -------------------------------------------------------------------------- */
int child_update(struct child *child,    uint32_t channels, const char *argv,
                 uint64_t      interval, int      autostart)
{
  char *p;
 
  child->chans = channels;
  child->interval = interval;
  child->autostart = autostart;
  
  strlcpy(child->argv, argv, sizeof(child->argv));

  /* Update name if we haven't got one */
  if(!child->name[0])
  {
    p = strrchr(child->path, '/');

    if(p) 
    {
      if(p[1])
        strlcpy(child->name, &p[1], sizeof(child->path));
    }

    if(!child->name[0])
      strlcpy(child->name, child->path, sizeof(child->name));
  }
  
  log(child_log, L_status, "Updated child block: %s", child->name);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove and free a child block.                                             *
 * -------------------------------------------------------------------------- */
void child_delete(struct child *child)
{
  log(child_log, L_status, "Deleting child block: %s", child->name);
  
  /* Cancels timers, shuts down sockets and stuff */
  child_cancel(child);
  
  /* Remove from the list and free */
  dlink_delete(&child_list, (struct node *)child);
  
  mem_static_free(&child_heap, child);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct child *child_find(const char *path)
{
  uint32_t      chash;
  struct node   *node;
  struct child *child;
  
  chash = strhash(path);
  
  dlink_foreach(&child_list, node)
  {
    child = node->data;
    
    if(chash == child->chash)
    {
      if(!strcmp(child->path, path))
        return child;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct child *child_pop(struct child *ciptr)
{
  if(ciptr)
  {
    if(!ciptr->refcount)
      log(child_log, L_warning, "Poping deprecated child: %s",
          ciptr->name);
    
    ciptr->refcount++;
  }

  return ciptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct child *child_push(struct child **ciptrptr)
{
  if(*ciptrptr)
  {
    if(!(*ciptrptr)->refcount)
      log(child_log, L_warning, "Trying to push deprecated child: %s",
          (*ciptrptr)->name);
    else
      (*ciptrptr)->refcount--;
    
    if(!(*ciptrptr)->refcount)
      child_dirty = 1;
    
    (*ciptrptr) = NULL;
  }

  return *ciptrptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_callback(struct child *child, int type) 
{
#ifndef WIN32
  int status = WEXITSTATUS(child->exitcode);
   
  if(child->status == type)
    return;
  
  switch(type) 
  {
    case CHILD_DEAD: 
    {
      if(WIFEXITED(child->exitcode))
        log(child_log, L_status, "Child %s [%i] exited with code %i.", 
	    child->name, child->pid, status);
      else
	log(child_log, L_status, "Child %s [%i] crashed.",
	    child->name, child->pid);
      break;
    } 
    case CHILD_RUNNING:
    {
      log(child_log, L_status, "Child %s started [%i].",
	  child->name, child->pid);
      break;
    } 
  } 
#endif /* WIN32 */
  child->status = type;
   
  if(child->callback)
    child->callback(child, 
		    child->args[0], child->args[1], 
		    child->args[2], child->args[3]);
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifndef WIN32
static int child_check(struct child *child)
{
  int status = 0;
  int ret;
  
  if((ret = syscall_waitpid(child->pid, &status, WNOHANG)) == child->pid ||
     ret == -1)
  {
    child->exitcode = status;

    child_cancel(child);
  }
  else if(child->status != CHILD_RUNNING)
  {
    child_callback(child, CHILD_RUNNING);
  } 

  return 0;
}
#endif /* WIN32 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifndef WIN32
static int child_format(struct child *child)
{
  size_t   n;
  uint32_t i;
  char    *argv[CHILD_MAX_CHANNEL * 4 + 1];
  int      chan;
  char     args[1024];

  strlcpy(args, child->argv, sizeof(args));
  
  n = strtokenize(args, argv, CHILD_MAX_CHANNEL * 4);
  
  for(i = 0; i < n; i++)
  {
    if(argv[i][0] == '%')
    {
      chan = atoi(&argv[i][2]);
      
      if(argv[i][1] == 'r')
      {
        snprintf(child->arguments[i], 6, "%u", child->channels[chan][CHILD][READ]);
      }
      else if(argv[i][1] == 'w')
      {
        snprintf(child->arguments[i], 6, "%u", child->channels[chan][CHILD][WRITE]);
      }
    }
    else
    {
      strlcpy(child->arguments[i], argv[i], sizeof(child->arguments[i]));
    }
  }
  
  return 0;
}
#endif /* WIN32 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifndef WIN32
static int child_prepare(struct child *child)
{
  uint32_t i;
  int sp[2];
  
  for(i = 0; i < child->chans; i++)
  {
#ifdef HAVE_SOCKETPAIR
    if(syscall_socketpair(PF_UNIX, SOCK_STREAM, IPPROTO_IP, sp) == -1)
      return -1;
    
    child->channels[i][PARENT][READ] =
    child->channels[i][PARENT][WRITE] = sp[PARENT];
    child->channels[i][CHILD][READ] =
    child->channels[i][CHILD][WRITE] = sp[CHILD];
    
    io_new(child->channels[i][PARENT][READ], FD_SOCKET);
    
    io_note(child->channels[i][PARENT][READ],
            "channel %u to child %s", i, child->name);
#else
    if(syscall_pipe(sp) == -1)
      return -1;
    
    child->channels[i][PARENT][READ] = sp[READ];
    child->channels[i][CHILD][WRITE] = sp[WRITE];
    
    io_new(child->channels[i][PARENT][READ], FD_PIPE);
    
    io_note(child->channels[i][PARENT][READ],
            "channel %u to child %s (read)", i, child->name);
    
    if(syscall_pipe(sp) == -1)
      return -1;
    
    child->channels[i][CHILD][READ] = sp[READ];
    child->channels[i][PARENT][WRITE] = sp[WRITE];

    io_new(child->channels[i][PARENT][WRITE], FD_PIPE);
    
    io_note(child->channels[i][PARENT][WRITE],
            "channel %u to child %s (write)", i, child->name);
#endif /* HAVE_SOCKETPAIR */    

    log(child_log, L_verbose, 
        "Child %s channel #%u: PR = %i, PW = %i, CR = %i, CW = %i",
        child->name, i, 
        child->channels[i][PARENT][READ],
        child->channels[i][PARENT][WRITE],
        child->channels[i][CHILD][READ],
        child->channels[i][CHILD][WRITE]);
  }
  return 0;
}
#endif /* WIN32 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int child_launch(struct child *child)
{
#ifndef WIN32
  uint32_t i;
  
  child_prepare(child);
  
  child->pid = syscall_fork();
  
  if(child->pid == -1)
    return -1;
  
  if(child->pid == 0)
  {
    struct dlog *drain;
    struct node *next;
    char        *argv[CHILD_MAX_CHANNEL * 4 + 2];
    
    dlink_foreach_safe(&log_list, drain, next)
      log_drain_delete(drain);
    
    for(i = 0; i < CHILD_MAX_CHANNEL; i++)
    {
      if(child->channels[i][PARENT][READ] > -1)
        syscall_close(child->channels[i][PARENT][READ]);
#ifndef HAVE_SOCKETPAIR
      if(child->channels[i][PARENT][WRITE] > -1)
        syscall_close(child->channels[i][PARENT][WRITE]);
#endif /* HAVE_SOCKETPAIR */
      child->channels[i][PARENT][READ] = -1;
      child->channels[i][PARENT][WRITE] = -1;
    }
    
    child_format(child);
    
    argv[0] = child->name;
    
    for(i = 0; child->arguments[i][0] && i < CHILD_MAX_CHANNEL * 4; i++)
    {
      argv[i + 1] = child->arguments[i];
    }
    
    argv[i + 1] = NULL;
    
    syscall_execve(child->path, argv, NULL);
    
    syscall_exit(1);
  }
  else
  {
    for(i = 0; i < CHILD_MAX_CHANNEL; i++)
    {
      if(child->channels[i][CHILD][READ] > -1)
        syscall_close(child->channels[i][CHILD][READ]);
#ifndef HAVE_SOCKETPAIR
      if(child->channels[i][CHILD][WRITE] > -1)
        syscall_close(child->channels[i][CHILD][WRITE]);
#endif /* HAVE_SOCKETPAIR */
      child->channels[i][CHILD][READ] = -1;
      child->channels[i][CHILD][WRITE] = -1;
    }    
  }

  child->status = CHILD_RUNNING;
  
  child->timer = timer_start(child_check, CHILD_INTERVAL, child);
  
  timer_note(child->timer, "wait for child %s", child->name);
#endif /* WIN32 */
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_default(struct child *child)
{
  uint32_t i;
  
  dlink_node_zero(&child->node);
  
  child->pid = 0;
  child->path[0] = '\0';
  child->name[0] = '\0';
  child->interval = 0LLU;
  child->autostart = 0;
  child->args[0] = NULL;
  child->args[1] = NULL;
  child->args[2] = NULL;
  child->args[3] = NULL;
  child->id = 0;
  child->refcount = 1;
  child->timer = NULL;

  for(i = 0; i < CHILD_MAX_CHANNEL; i++)
  {
    child->channels[i][PARENT][READ] = -1;
    child->channels[i][PARENT][WRITE] = -1;
    child->channels[i][CHILD][READ] = -1;
    child->channels[i][CHILD][WRITE] = -1;
  }
  
  child->chash = strihash(child->path);
  child->nhash = strihash(child->name);
}
       
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_set_callback(struct child *child, void *callback)
{
  child->callback = callback;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_kill(struct child *child)
{
#ifndef WIN32
  if(child->pid > 0)
  {
    if(syscall_kill(child->pid, SIGTERM) > -1)
      log(child_log, L_status, "Killed child %s [%i]", child->name, child->pid);    
  } 
#endif /* WIN32 */
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_cancel(struct child *child)
{
#ifndef WIN32
  uint32_t i;
  
  log(child_log, L_status, "Cancelling child %s [%i]",
      child->name, child->pid);
  
  child_callback(child, CHILD_DEAD);
  
  for(i = 0; i < CHILD_MAX_CHANNEL; i++)
  {
    if(child->channels[i][PARENT][READ] > -1)
      io_shutup(child->channels[i][PARENT][READ]);
#ifndef HAVE_SOCKETPAIR
    if(child->channels[i][PARENT][WRITE] > -1)
      io_shutup(child->channels[i][PARENT][WRITE]);
#endif /* HAVE_SOCKETPAIR */    
    child->channels[i][PARENT][READ] = -1;
    child->channels[i][PARENT][WRITE] = -1;    
  }

  if(child->pid > 0)
  {
    int status;
    
    child_kill(child);
    
    syscall_waitpid(child->pid, &status, WNOHANG);
  }
  
  if(child->timer)
  {
    timer_remove(child->timer);
    child->timer = NULL;
  }
#endif /* WIN32 */
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_vset_args(struct child *child, va_list args)
{  
  child->args[0] = va_arg(args, void *);
  child->args[1] = va_arg(args, void *);
  child->args[2] = va_arg(args, void *);
  child->args[3] = va_arg(args, void *);
} 

void child_set_args(struct child *child, ...)
{
  va_list args;
    
  va_start(args, child);
  child_vset_args(child, args);
  va_end(args);
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void child_set_name(struct child *child, const char *name)
{
  strlcpy(child->name, name, sizeof(child->name));
  
  child->nhash = strihash(child->name);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *child_get_name(struct child *child)
{
  return child->name;
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct child *child_find_name(const char *name)
{
  struct child *child;
  struct node  *node;
  uint32_t      nhash;
  
  nhash = strihash(name);
  
  dlink_foreach(&child_list, node)
  {
    child = node->data;
    
    if(child->nhash == nhash)
    {
      if(!stricmp(child->name, name))
        return child;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct child *child_find_id(uint32_t id)
{
  struct child *child;
  struct node  *node;
  
  dlink_foreach(&child_list, node)
  {
    child = node->data;
    
    if(child->id == id)
      return child;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Dump childs and child heap.                                                *
 * -------------------------------------------------------------------------- */
void child_dump(struct child *cdptr)
{
  if(cdptr == NULL)
  {
    dump(child_log, "[============== child summary ===============]");
    
    dlink_foreach(&child_list, cdptr)
    {
      dump(child_log, " #%u: [%u] %-20s (%i)",
           cdptr->id, cdptr->refcount, cdptr->name, cdptr->pid);
    }

    dump(child_log, "[=========== end of child summary ===========]");
  }
  else
  {
    dump(child_log, "[=============== child dump ================]");
    dump(child_log, "         id: #%u", cdptr->id);
    dump(child_log, "   refcount: %u", cdptr->refcount);
    dump(child_log, "      chash: %p", cdptr->chash);
    dump(child_log, "      nhash: %p", cdptr->nhash);
    dump(child_log, "     status: %u", cdptr->status);
    dump(child_log, "        pid: %i", cdptr->pid);
    dump(child_log, "      timer: %i", cdptr->timer ? cdptr->timer->id : -1);
    
    dump(child_log, "      chans: %u", cdptr->chans);
    dump(child_log, "   channels: [%i:%i] [%i:%i] [%i:%i] [%i:%i]",
         cdptr->channels[0][PARENT][0], cdptr->channels[0][PARENT][1],
         cdptr->channels[1][PARENT][0], cdptr->channels[1][PARENT][1],
         cdptr->channels[2][PARENT][0], cdptr->channels[2][PARENT][1],
         cdptr->channels[3][PARENT][0], cdptr->channels[3][PARENT][1]);
    dump(child_log, "  autostart: %s", cdptr->autostart ? "on" : "off");
    dump(child_log, "   exitcode: %i", cdptr->exitcode);
    dump(child_log, "   interval: %llu", cdptr->interval);
    dump(child_log, "      start: %llu", cdptr->start);
    dump(child_log, "       name: %s", cdptr->name);
    dump(child_log, "       path: %s", cdptr->path);
    dump(child_log, "       argv: %s", cdptr->argv);
    dump(child_log, "  arguments: [ '%s' , '%s' , '%s', '%s', '%s', '%s', '%s', '%s' ]",
         cdptr->arguments[0], cdptr->arguments[1], cdptr->arguments[2], cdptr->arguments[3],
         cdptr->arguments[4], cdptr->arguments[5], cdptr->arguments[6], cdptr->arguments[7]);
    dump(child_log, "       args: %p, %p, %p, %p",
         cdptr->args[0], cdptr->args[1], cdptr->args[2], cdptr->args[3]);
    dump(child_log, "[============ end of child dump ============]");
  }
}
