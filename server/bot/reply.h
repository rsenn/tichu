#ifndef BOT_REPLY_H
#define BOT_REPLY_H

typedef void (*reply_callback)(char *prefix, int argc, char *argv[]);

struct reply {
  const char    *name;
  int            args;
  int            maxargs;
  reply_callback callback;
}; 

extern struct reply reply_table[];

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         reply_init     (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         reply_shutdown (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          reply_parse(char *msg);
  
#endif /* BOT_REPLY_H */
