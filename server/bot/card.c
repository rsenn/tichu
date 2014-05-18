#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/connect.h>
#include <libchaos/listen.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>
#include <libchaos/sauth.h>
#include <libchaos/hook.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/ini.h>
#include <libchaos/io.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include "card.h"

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int             card_log;                /* player log source */
struct sheap    card_heap;               /* heap for struct player */
//struct timer   *card_timer;              /* timer for heap gc */
uint32_t        card_serial;
uint32_t        card_max;
struct list     card_list;               /* list with all of them */
uint64_t        card_seed;
struct ini     *card_ini;

/* -------------------------------------------------------------------------- *
 * Initialize card module                                                     *
 * -------------------------------------------------------------------------- */
void card_init(void)
{  
  card_log = log_source_register("card");
  
  /* Zero all card lists */
  dlink_list_zero(&card_list);
  
  /* Setup card heap & timer */
  mem_static_create(&card_heap, sizeof(struct card), CARD_BLOCK_SIZE);
  mem_static_note(&card_heap, "card heap");
  
  card_serial = 0;
  
  card_ini = ini_add(DATADIR "/cards.ini");

  ini_callback(card_ini, card_load);
  
  log(card_log, L_status, "Initialised [card] module.");
}

/* -------------------------------------------------------------------------- *
 * Shutdown card module                                                    *
 * -------------------------------------------------------------------------- */
void card_shutdown(void)
{
  struct card *card;
  struct card *next;
  
  log(card_log, L_status, "Shutting down [card] module.");
  
  /* Push all cards */
  dlink_foreach_safe(&card_list, card, next)
  {
    if(card->refcount)
      card->refcount--;
    
    card_delete(card);
  }
  
  /* Destroy static heap */
  mem_static_destroy(&card_heap);
  
  /* Unregister log source */
  log_source_unregister(card_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void card_load(struct ini *ini)
{
  struct card *card;
  struct ini_section *isptr;
    
  log(card_log, L_warning, "Loading %u cards from %s...", ini->sections.size, ini->name);
  
  /* Gehe durch die INI sections */
  for(isptr = ini_section_first(ini); isptr;
      isptr = ini_section_next(ini))
  {
    card = card_new(isptr->name);
      
    /* Lese punktzahl */
    ini_read_int(isptr, "points", &card->points);
    
    /* Lese wertigkeit */
    ini_read_int(isptr, "value", &card->value);
      
    /* Bearbeiten möglich? */
    ini_read_int(isptr, "choose", &card->choose);
  }
}

/* -------------------------------------------------------------------------- *
 * Garbage collect card blocks                                                *
 * -------------------------------------------------------------------------- */
void card_collect(void)
{
  mem_static_collect(&card_heap);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct card *card_by_mode(struct list *list, int mode)
{
  struct node *node;
  struct card *card;
  int cval = (mode == CARD_LEET ? 0 : 1000);
  struct card *ccard = NULL;
  
  log(card_log, L_status, "searching %s card", (mode == CARD_LEET ? "leetest" : "lamest"));
  log(card_log, L_status, "=========> have %i card(s) left", list->size);
  
  dlink_foreach_data(list, node, card)
  {
    if(!strcmp(card->name, "xp"))
    {
      log(card_log, L_status, "setting phoenix value to 0");
      card->value = 0;
    }
      
    if((mode == CARD_LEET) && card->value >= cval)
    {
      cval = card->value;
      ccard = card;
    }
    if((mode == CARD_LAME) && card->value <= cval)
    {
      cval = card->value;
      ccard = card;
    }    
    
    log(card_log, L_status, "card %s value %u", card->name, card->value);
  
  }
  
  return ccard;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct card *card_higher(struct list *list, int value)
{
  struct node *node;
  struct card *card;
  int cval = 1000;
  struct card *ccard = NULL;
  
  log(card_log, L_status, "searching card higher than %u", value);
  
  dlink_foreach_data(list, node, card)
  {
    if(!strcmp(card->name, "xp") && value <= 28)
    {
      log(card_log, L_status, "setting phoenix value to %i", value + 1);
      card->value = value + 1;
    }
      
    log(card_log, L_status, "card %s value %u", card->name, card->value);
    
    if(card->value > value && card->value <= cval)
    {
      cval = card->value;
      ccard = card;
    }
  }
  
  if(ccard)
    log(card_log, L_status, "found card %s value %u", ccard->name, ccard->value);
  else
    log(card_log, L_status, "no card found");
  
  return ccard;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void card_by_value(struct list *in, struct list *out, int value, int max)
{
  struct card *card;
  struct node *node;
  
  dlink_foreach_data(in, node, card)
  {
    if(card->value == value)
      dlink_add_tail(out, dlink_node_new(), card);
    
    if(out->size >= (unsigned)max)
      break;
  }
}

/* -------------------------------------------------------------------------- *
 * Create a new card block                                                    *
 * -------------------------------------------------------------------------- */
struct card *card_new(const char *name)
{
  struct card *card = NULL;
  
  /* Allocate and zero card block */
  card = mem_static_alloc(&card_heap);

  memset(card, 0, sizeof(struct card));
  
  /* Link it to the main list and to the appropriate typed list */
  dlink_add_tail(&card_list, &card->node, card);
  
  /* Initialise the block */
  card->refcount = 1;
  card->serial = card_serial++;
  strlcpy(card->name, name, sizeof(card->name));
  card->hash = strhash(card->name);
    
  if(card_list.size > card_max)
    card_max = card_list.size;
  
//  log(card_log, L_debug, "New card: %s", card->name);
  
  return card;
}
  
/* -------------------------------------------------------------------------- *
 * Delete a card block                                                     *
 * -------------------------------------------------------------------------- */
void card_delete(struct card *card)
{
  card_release(card);
  
  /* Unlink from main list */
  dlink_delete(&card_list, &card->node);
  
  debug(card_log, "Deleted card");
  
  /* Free the block */
  mem_static_free(&card_heap, card);
}  
 
/*********************************************************************************************************/
/* -------------------------------------------------------------------------- *
 * Loose all references of an card block                                   *
 * -------------------------------------------------------------------------- */
void card_release(struct card *card)
{
  hooks_call(card_release, HOOK_DEFAULT, card);
  
/*  if(io_valid(card->fds[1]))
  {
    io_unregister(card->fds[1], IO_CB_READ);
    io_unregister(card->fds[1], IO_CB_WRITE);
    io_close(card->fds[1]);
  }
  
  if(io_valid(card->fds[0]) && card->fds[1] != card->fds[0])
  {
    io_unregister(card->fds[0], IO_CB_READ);
    io_unregister(card->fds[0], IO_CB_WRITE);
    io_close(card->fds[0]);
  }

  card->fds[0] = -1;
  card->fds[1] = -1;

  class_push(&card->class);
*/  
/*  if(card->user)
  {
    user_delete(card->user);
    card->user = NULL;
  }*/

//  card->listen = NULL;
/*  card->connect = NULL;*/
/*  listen_push(&card->listen);
  connect_push(&card->connect);*/
//  timer_cancel(&card->ptimer);
  
  card->refcount = 0;
//  card->shut = 1;
/*  card_dirty++;*/
}
/*********************************************************************************************************/

/* -------------------------------------------------------------------------- *
 * Find a card by its name                                                 *
 * -------------------------------------------------------------------------- */
struct card *card_find_name(const char *name)
{
  struct card *card;
  uint32_t     hash;
  
  hash = strihash(name);
  
  dlink_foreach(&card_list, card)
  {
    if(card->hash == hash)
    {
      if(!stricmp(card->name, name))
        return card;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a card by its id                                                      *
 * -------------------------------------------------------------------------- */
struct card *card_find_id(uint32_t id)
{
  struct card *card;
  
  dlink_foreach(&card_list, card)
  {
    if(card->serial == id)
      return card;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void card_dump(struct card *card)
{
  if(card == NULL)
  {
    struct node *node;
    
    dump(card_log, "[============== card summary ===============]");
    
    dlink_foreach_data(&card_list, node, card)
      dump(card_log, " #%03u: [%u] %-20s", card->serial, card->refcount, card->name);
    
    dump(card_log, "[========== end of card summary ============]");
  }
  else
  {
    dump(card_log, "[============== card dump ===============]");
    dump(card_log, "            name : %s", card->name);
    dump(card_log, "           points: %u", card->points);
    dump(card_log, "             hash: %p", card->hash);
    dump(card_log, "         refcount: %u", card->refcount);
    dump(card_log, "[========== end of card dump ============]");
  }
}
