#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <tichu/player.h>
#include <tichu/class.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int           class_log;
struct sheap  class_heap;
struct list   class_list;
uint32_t      class_id;
struct timer *class_timer;

/* -------------------------------------------------------------------------- *
 * Initialize channel heaps and add garbage collect timer.                    *
 * -------------------------------------------------------------------------- */
void class_init(void)
{
  class_log = log_source_register("class");
  
  class_id = 0;
  
  dlink_list_zero(&class_list);
  
  mem_static_create(&class_heap, sizeof(struct class), CLASS_BLOCK_SIZE);
  mem_static_note(&class_heap, "class heap");

  log(class_log, L_status, "Initialized [class] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy class heap and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
void class_shutdown(void)
{
  struct class *clptr;
  struct class *next;
  
  log(class_log, L_status, "Shutting down [class] module...");
  
  dlink_foreach_safe(&class_list, clptr, next)
    class_delete(clptr);

  mem_static_destroy(&class_heap);
  
  log_source_unregister(class_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void class_default(struct class *clptr)
{
  dlink_node_zero(&clptr->node);
  
  strcpy(clptr->name, "default");
  clptr->ping_freq = 360;
  clptr->max_players = 1000;
  clptr->players_per_ip = 1;
  clptr->recvq = (1 << 16);
  clptr->sendq = (1 << 16);
  clptr->flood_trigger = 0;
  clptr->flood_interval = 0;
  clptr->throttle_trigger = 0;
  clptr->throttle_interval = 0;
  
  clptr->hash = strihash(clptr->name);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct class *class_add(const char *name,             uint64_t ping_freq,
                        uint32_t    max_players,      uint32_t players_per_ip, 
                        uint32_t    recvq,            uint32_t sendq,
                        uint32_t    flood_trigger,    uint64_t flood_interval,
                        uint32_t    throttle_trigger, uint64_t throttle_interval)
{
  struct class *clptr;
  
  /* allocate, zero and link class struct */
  clptr = mem_static_alloc(&class_heap);
  
  memset(clptr, 0, sizeof(struct class));
  
  dlink_add_tail(&class_list, &clptr->node, clptr);

  strlcpy(clptr->name, name, sizeof(clptr->name));  
  clptr->hash = strihash(clptr->name);

  clptr->ping_freq = ping_freq;
  clptr->max_players = max_players;
  clptr->players_per_ip = players_per_ip;
  clptr->recvq = recvq;
  clptr->sendq = sendq;
  clptr->id += class_id++;
  clptr->refcount = 1;
  clptr->flood_trigger = flood_trigger;
  clptr->flood_interval = flood_interval;
  clptr->throttle_trigger = throttle_trigger;
  clptr->throttle_interval = throttle_interval;
  
  log(class_log, L_status, "Added class block: %s", clptr->name);
  
  return clptr;
}     
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int class_update(struct class *clptr,            uint64_t ping_freq,
                 uint32_t      max_players,      uint32_t players_per_ip, 
                 uint32_t      recvq,            uint32_t sendq,
                 uint32_t      flood_trigger,    uint64_t flood_interval,
                 uint32_t      throttle_trigger, uint64_t throttle_interval)
{
  clptr->ping_freq = ping_freq;
  clptr->max_players = max_players;
  clptr->players_per_ip = players_per_ip;
  clptr->recvq = recvq;
  clptr->sendq = sendq;
  clptr->flood_trigger = flood_trigger;
  clptr->flood_interval = flood_interval;
  clptr->throttle_trigger = throttle_trigger;
  clptr->throttle_interval = throttle_interval;
  
  log(class_log, L_status, "Updated class block: %s", clptr->name);
  
  return 0;
}     
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void class_delete(struct class *clptr)
{
  log(class_log, L_status, "Deleting class block: %s", clptr->name);
  
  dlink_delete(&class_list, &clptr->node);
  
  mem_static_free(&class_heap, clptr);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct class *class_pop(struct class *clptr)
{
  if(clptr)
  {
    if(!clptr->refcount)
      log(class_log, L_warning, "Poping deprecated class: %s",
          clptr->name);
  
    clptr->refcount++;
  }
  
  return clptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct class *class_push(struct class **clptrptr)
{
  if(*clptrptr)
  {
    if(!(*clptrptr)->refcount)
    {
      log(class_log, L_warning, "Trying to push deprecated class: %s",
          (*clptrptr)->name);
    }
    else
    {
      (*clptrptr)->refcount--;
    }
    
    (*clptrptr) = NULL;
  }
  
  return *clptrptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct class *class_find_name(const char *name)
{
  struct class *clptr;
  struct node  *node;
  uint32_t      hash;
  
  hash = strihash(name);
  
  dlink_foreach(&class_list, node)
  {
    clptr = (struct class *)node;
    
    if(hash == clptr->hash)
    {
      if(!stricmp(clptr->name, name))
        return clptr;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct class *class_find_id(uint32_t id)
{
  struct class *clptr;
  
  dlink_foreach(&class_list, clptr)
  {
    if(id == clptr->id)
      return clptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void class_dump(struct class *clptr)
{
  if(clptr == NULL)
  {
    struct node *node;
    
    dump(class_log, "[============== class summary ===============]");
    
    dlink_foreach_data(&class_list, node, clptr)
      dump(class_log, " #%03u: [%u] %-20s (sendq: %u/recvq: %u)",
            clptr->id, 
            clptr->refcount,
            clptr->name,
            clptr->sendq,
            clptr->recvq);
    
    dump(class_log, "[========== end of class summary ============]");
  }
  else
  {
    dump(class_log, "[============== class dump ===============]");
    dump(class_log, "               id: #%u", clptr->id);
    dump(class_log, "         refcount: %u", clptr->refcount);
    dump(class_log, "             hash: %p", clptr->hash);
    dump(class_log, "             name: %s", clptr->name);
    dump(class_log, "        ping_freq: %llu", clptr->ping_freq);
    dump(class_log, "      max_players: %u", clptr->max_players);
    dump(class_log, "   players_per_ip: %u", clptr->players_per_ip);
    dump(class_log, "            recvq: %u", clptr->recvq);
    dump(class_log, "            sendq: %u", clptr->sendq);
    dump(class_log, "    flood_trigger: %u", clptr->flood_trigger);
    dump(class_log, "   flood_interval: %u", clptr->flood_interval);
    dump(class_log, " throttle_trigger: %u", clptr->throttle_trigger);
    dump(class_log, "throttle_interval: %u", clptr->throttle_interval);
    dump(class_log, "[========== end of class dump ============]");    
  }
}
