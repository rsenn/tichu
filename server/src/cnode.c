#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/dlink.h>
#include <libchaos/hook.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/cnode.h>
#include <tichu/player.h>
#include <tichu/combo.h>
#include <tichu/game.h>
#include <tichu/card.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int          cnode_log;
struct sheap cnode_heap;
uint32_t     cnode_serial;

/* -------------------------------------------------------------------------- *
 * Initialize the cnode module                                                *
 * -------------------------------------------------------------------------- */
void cnode_init(void)
{
  cnode_log = log_source_register("cnode");

  cnode_serial = 0;
  
  mem_static_create(&cnode_heap, sizeof(struct cnode),
                    CNODE_BLOCK_SIZE);
  mem_static_note(&cnode_heap, "cnode heap");

  log(cnode_log, L_status, "Initialised [cnode] module.");
}

/* -------------------------------------------------------------------------- *
 * Shut down the cnode module                                                 *
 * -------------------------------------------------------------------------- */
void cnode_shutdown(void)
{
  log(cnode_log, L_status, "Shutting down [cnode] module...");
  
  mem_static_destroy(&cnode_heap);
  
  log_source_unregister(cnode_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct cnode *cnode_new(struct game *game, struct card *card)
{
  struct cnode *cnode;
  
  cnode = mem_static_alloc(&cnode_heap);
  
  cnode->game = game;
  cnode->card = card;
  cnode->node.data = card;
  cnode->value = card->value;
  
  return cnode;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void cnode_delete(struct cnode *cnode)
{
  mem_static_free(&cnode_heap, cnode);                  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void cnode_link(struct cnode *cnode, struct list *list)
{  
  dlink_add_head(list, &cnode->node, cnode);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void cnode_unlink(struct cnode *cnode, struct list *list)
{
  dlink_delete(list, &cnode->node);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct cnode *cnode_find_by_card(struct card *card, struct list *list)
{
  struct node  *nptr;
  struct cnode *cnode;
  
  dlink_foreach_data(list, nptr, cnode)
  {    
    if(cnode->card == card)
      return cnode;
  }
  return NULL;
}

struct cnode *cnode_find_by_name(const char *name, struct list *list)
{
  struct cnode *cnode;
  uint32_t      hash;
  
  hash = strihash(name);
  
  dlink_foreach(list, cnode)
  {
    if(cnode->card->hash == hash)
    {
      if(!stricmp(cnode->card->name, name))
        return cnode;
    }
  }
  
  return NULL;
}

