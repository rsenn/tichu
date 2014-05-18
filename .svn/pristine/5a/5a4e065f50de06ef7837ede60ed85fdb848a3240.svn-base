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
#include <tichu/player.h>
#include <tichu/combo.h>
#include <tichu/structs.h>
#include <tichu/cnode.h>
#include <tichu/game.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int          structs_log;
uint32_t     structs_serial;

struct sheap turn_heap;
struct sheap prick_heap;
struct sheap points_heap;

/* -------------------------------------------------------------------------- *
 * Initialize the structs module                                                *
 * -------------------------------------------------------------------------- */
void structs_init(void)
{
  structs_log = log_source_register("structs");

  structs_serial = 0;
  
  mem_static_create(&turn_heap, sizeof(struct turn),
                    TURN_BLOCK_SIZE);
  mem_static_note(&turn_heap, "turn heap");

  mem_static_create(&prick_heap, sizeof(struct prick),
                    PRICK_BLOCK_SIZE);
  mem_static_note(&prick_heap, "prick heap");

  mem_static_create(&points_heap, sizeof(struct points),
                    POINTS_BLOCK_SIZE);
  mem_static_note(&points_heap, "points heap");

  log(structs_log, L_status, "Initialised [structs] module.");
}

/* -------------------------------------------------------------------------- *
 * Shut down the structs module                                                 *
 * -------------------------------------------------------------------------- */
void structs_shutdown(void)
{
  log(structs_log, L_status, "Shutting down [structs] module...");
  
  mem_static_destroy(&turn_heap);
  mem_static_destroy(&prick_heap);
  mem_static_destroy(&points_heap);

  log_source_unregister(structs_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct turn *turn_new(struct game *game, struct player *player, int type, int value)
{
  struct turn  *turn;
/*  struct node  *node, node2;
  struct cnode *cnode;*/
  
  turn = mem_static_alloc(&turn_heap);
  memset(turn, 0, sizeof(struct turn));
  dlink_list_zero(&turn->cards);
  
  turn->game = game;
  turn->player = player;
  turn->combo.type = type;
  turn->combo.value = value;
  turn->node.data = turn;
  
  dlink_add_tail(&game->prick->turns, &turn->node, turn);
  
  return turn;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct prick *prick_new(struct game *game)
{
  struct prick *prick;
  
  prick = mem_static_alloc(&prick_heap);
  memset(prick, 0, sizeof(struct prick));
  dlink_list_zero(&prick->turns);
  
  prick->game = game;
  prick->node.data = prick;
  
  dlink_add_tail(&game->pricks, &prick->node, prick);
  
  return prick;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct points *points_new(struct game *game)
{
  struct points *points;

  points = mem_static_alloc(&points_heap);
  memset(points, 0, sizeof(struct points));
  
  dlink_add_tail(&game->points, &points->node, points);
  
  return points;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void prick_delete(struct prick *prick)
{    
  /* Free the block */
  mem_static_free(&prick_heap, prick);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void turn_delete(struct turn *turn)
{    
  /* Free the block */
  mem_static_free(&turn_heap, turn);
}

