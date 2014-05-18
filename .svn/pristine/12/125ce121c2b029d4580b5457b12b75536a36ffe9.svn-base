#ifndef SRC_CLASS_H
#define SRC_CLASS_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct class {
  struct node node;
  uint32_t    id;
  uint32_t    refcount;
  uint32_t    hash;
  uint64_t    ping_freq;
  uint32_t    max_players;
  uint32_t    players_per_ip;
  uint32_t    recvq;
  uint32_t    sendq;
  uint32_t    flood_trigger;
  uint64_t    flood_interval;
  uint32_t    throttle_trigger;
  uint64_t    throttle_interval;
  char        name[TICHU_CLASSLEN + 1];
};

/* -------------------------------------------------------------------------- *
  * -------------------------------------------------------------------------- */
extern int           class_log;
extern struct sheap  class_heap;
extern struct timer *class_timer;
extern uint32_t      class_serial;
extern struct list   class_list;

/* -------------------------------------------------------------------------- *
 * Initialize class heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
extern void          class_init             (void);

/* -------------------------------------------------------------------------- *
 * Destroy class heap and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
extern void          class_shutdown         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          class_default          (struct class    *clptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct class *class_add              (const char    *name,
                                             uint64_t       ping_freq,
                                             uint32_t       max_clients,
                                             uint32_t       clients_per_ip,
                                             uint32_t       recvq,
                                             uint32_t       sendq,
                                             uint32_t       flood_trigger,
                                             uint64_t       flood_interval,
                                             uint32_t       throttle_trigger, 
                                             uint64_t       throttle_interval);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           class_update           (struct class  *clptr,
                                             uint64_t       ping_freq,
                                             uint32_t       max_clients,
                                             uint32_t       clients_per_ip,
                                             uint32_t       recvq,
                                             uint32_t       sendq,
                                             uint32_t       flood_trigger,
                                             uint64_t       flood_interval,
                                             uint32_t       throttle_trigger, 
                                             uint64_t       throttle_interval);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          class_delete           (struct class  *clptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct class *class_find_name        (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct class *class_find_id          (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct class *class_pop              (struct class  *clptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct class *class_push             (struct class **clptr);

/* -------------------------------------------------------------------------- *
 * Dump classes and class heap.                                               *
 * -------------------------------------------------------------------------- */
extern void          class_dump             (struct class  *clptr);
  
#endif /* SRC_CLASS_H */
