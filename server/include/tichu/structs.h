#ifndef SRC_STRUCTS_H
#define SRC_STRUCTS_H

#include "combo.h"

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
struct turn {  
  struct node      node;
  struct game     *game;
  struct player   *player;
  struct list      cards;
  struct combo     combo;
};

struct prick {
  struct node      node;
  struct game     *game;
  struct list      turns;
};

struct points {
  struct node        node;
  int                team1;
  int                team2;
};


/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int           structs_log;

extern struct sheap  turn_heap;

extern struct sheap  prick_heap;

extern struct sheap  points_heap;

/* -------------------------------------------------------------------------- *
 * Initialize the cnode module                                             *
 * -------------------------------------------------------------------------- */
extern void           structs_init        (void);

/* -------------------------------------------------------------------------- *
 * Shut down the structs module                                              *
 * -------------------------------------------------------------------------- */
extern void           structs_shutdown    (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct turn   *turn_new            (struct game *game,
                                          struct player *player,
                                          int type,
                                          int value);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct prick  *prick_new          (struct game *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct points *points_new         (struct game *game);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           turn_delete        (struct turn *turn);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           prick_delete       (struct prick *prick);


#endif /* SRC_STRUCTS_H */
