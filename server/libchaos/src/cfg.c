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
 * $Id: cfg.c,v 1.6 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#include <math.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/mem.h>
#include <libchaos/cfg.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/gif.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int                cfg_log; 
struct sheap       cfg_heap;       /* heap containing cfg blocks */
struct list        cfg_list;       /* list linking cfg blocks */
uint32_t           cfg_id;
int                cfg_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize cfg heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void cfg_init(void)
{
  cfg_log = log_source_register("cfg");
  
  dlink_list_zero(&cfg_list);
  
  cfg_id = 0;
  cfg_dirty = 0;
  
  mem_static_create(&cfg_heap, sizeof(struct cfg), CFG_BLOCK_SIZE);
  mem_static_note(&cfg_heap, "cfg block heap");

  log(cfg_log, L_status, "Initialized [cfg] module.");
}


/* -------------------------------------------------------------------------- *
 * Destroy cfg heap                                                         *
 * -------------------------------------------------------------------------- */
void cfg_shutdown(void)
{
  struct cfg *cfptr;
  struct cfg *next;
  
  /* Report status */
  log(cfg_log, L_status, "Shutting down [cfg] module...");
  
  /* Remove all cfg blocks */
  dlink_foreach_safe(&cfg_list, cfptr, next)
  {
    if(cfptr->refcount)
      cfptr->refcount--;

    cfg_delete(cfptr);
  }

  mem_static_destroy(&cfg_heap);
    
  /* Unregister log source */
  log_source_unregister(cfg_log);
}

/* -------------------------------------------------------------------------- *
 * Collect cfg block garbage.                                              *
 * -------------------------------------------------------------------------- */
int cfg_collect(void)
{
  struct cfg *cnptr;
  struct cfg *next;
  size_t         n = 0;
  
  if(cfg_dirty)
  {
    /* Report verbose */
    log(cfg_log, L_verbose, 
        "Doing garbage collect for [cfg] module.");
    
    /* Free all cfg blocks with a zero refcount */
    dlink_foreach_safe(&cfg_list, cnptr, next)
    {
      if(!cnptr->refcount)
      {
        cfg_delete(cnptr);
        
        n++;
      }
    }
  
    /* Collect garbage on cfg_heap */
    mem_static_collect(&cfg_heap);
    
    cfg_dirty = 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void cfg_default(struct cfg *cfptr)
{
  dlink_node_zero(&cfptr->node);
  
  strcpy(cfptr->name, "default");
  cfptr->id = 0;
  cfptr->refcount = 0;
  cfptr->hash = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct cfg *cfg_new(const char *name)
{
  struct cfg *cfptr;
  
  cfptr = mem_static_alloc(&cfg_heap);
  
  cfptr->id = cfg_id++;
  cfptr->refcount = 1;
  
  strlcpy(cfptr->name, name, sizeof(cfptr->name));
  
  cfptr->hash = strhash(cfptr->name);
  
  dlink_add_tail(&cfg_list, &cfptr->node, cfptr);

  dlink_list_zero(&cfptr->chain);
  
  log(cfg_log, L_status, "Added cfg block: %s",
      cfptr->name);
  
  return cfptr;
}
     
/* -------------------------------------------------------------------------- *
 * Load config file and all its dependants. Re-read them if necessary.        *
 * -------------------------------------------------------------------------- */
int cfg_load(struct cfg *cfptr, const char *path)
{
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void cfg_delete(struct cfg *cfptr)
{
  log(cfg_log, L_status, "Deleting cfg block: %s", cfptr->name);
 
  dlink_delete(&cfg_list, &cfptr->node);
  
  mem_static_free(&cfg_heap, cfptr);
}

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
void cfg_release(struct cfg *cfptr)
{
  cfg_dirty = 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void cfg_set_name(struct cfg *cfptr, const char *name)
{
  strlcpy(cfptr->name, name, sizeof(cfptr->name));
  
  cfptr->hash = strihash(cfptr->name);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *cfg_get_name(struct cfg *cfptr)
{
  return cfptr->name;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct cfg *cfg_find_name(const char *name)
{
  struct node   *node;
  struct cfg *cfptr;
  uint32_t       hash;
  
  hash = strihash(name);
  
  dlink_foreach(&cfg_list, node)
  {
    cfptr = node->data;
    
    if(cfptr->hash == hash)
    {
      if(!stricmp(cfptr->name, name))
        return cfptr;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct cfg *cfg_find_id(uint32_t id)
{
  struct cfg *cfptr;
  
  dlink_foreach(&cfg_list, cfptr)
  {
    if(cfptr->id == id)
      return cfptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Dump cfgs and cfg heap.                                                *
 * -------------------------------------------------------------------------- */
void cfg_dump(struct cfg *cfptr)
{
  if(cfptr == NULL)
  {
    dump(cfg_log, "[============== cfg summary ===============]");
    
    dlink_foreach(&cfg_list, cfptr)
      dump(cfg_log, " #%03u: [%u] %-20s",
           cfptr->id, 
           cfptr->refcount,
           cfptr->name);
    
    dump(cfg_log, "[========== end of cfg summary ============]");
  }
  else
  {
    dump(cfg_log, "[============== cfg dump ===============]");
    dump(cfg_log, "         id: #%u", cfptr->id);
    dump(cfg_log, "   refcount: %u", cfptr->refcount);
    dump(cfg_log, "       hash: %p", cfptr->hash);
    dump(cfg_log, "       name: %s", cfptr->name);

    dump(cfg_log, "[========== end of cfg dump ============]");    
  }
}
