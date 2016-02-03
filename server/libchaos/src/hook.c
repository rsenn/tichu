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
 * $Id: hook.c,v 1.20 2005/01/17 19:09:50 smoli Exp $   
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/timer.h>
#include <libchaos/dlink.h>
#include <libchaos/hook.h>
#include <libchaos/mem.h>

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
struct sheap  hook_heap;
struct list   hook_list;
struct timer *hook_timer;

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
static struct hook *hook_find(void *function, int type, void *callback)
{
  struct node *node;
  struct hook *hook;
  
  dlink_foreach(&hook_list, node)
  {
    hook = (struct hook *)node;
    
    if(hook->function == function &&
       hook->type == type &&
       hook->callback == callback)
      return hook;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
void hook_init()
{
  dlink_list_zero(&hook_list);
  
  mem_static_create(&hook_heap, sizeof(struct hook), HOOK_BLOCK_SIZE);  
  mem_static_note(&hook_heap, "hook heap");
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void hook_shutdown(void)
{
  mem_static_destroy(&hook_heap);
}

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
struct hook *hook_register(void *function, int type, void *callback)
{
  struct hook *hook;
  
  if((hook = hook_find(function, type, callback)))
    return NULL;
  
  hook = mem_static_alloc(&hook_heap);
  
  hook->function = function;
  hook->type = type;
  hook->callback = callback;
  
  dlink_add_tail(&hook_list, &hook->node, hook);
  
  return hook;
}


/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
int hook_unregister(void *function, int type, void *callback)
{
  struct hook *hook;
  
  hook = hook_find(function, type, callback);
  
  if(hook == NULL)
    return -1;
  
  dlink_delete(&hook_list, &hook->node);
  
  mem_static_free(&hook_heap, hook);
  
  return 0;
}

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
int hooks_call(void *function, int type, ...)
{
  struct node  *node;
  struct node  *next;
  void         *arga[4];
  va_list       args;
  struct hook  *hook;
  int           ret = 0;
  
  va_start(args, type);
  
  arga[0] = va_arg(args, void *);
  arga[1] = va_arg(args, void *);
  arga[2] = va_arg(args, void *);
  arga[3] = va_arg(args, void *);
 
  dlink_foreach_safe(&hook_list, node, next)
  {
    hook = (struct hook *)node;
    
    if(hook->function == function && hook->type == type)
    {
      ret += hook->callback(arga[0], arga[1], arga[2], arga[3]);
    }
  }
  
  va_end(args);
  
  return ret;
}

