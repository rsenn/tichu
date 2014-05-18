/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/io.h>
#include <libchaos/ini.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/timer.h>
#include <libchaos/module.h>
#include <libchaos/listen.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/msg.h>
#include <tichu/game.h>
#include <tichu/class.h>
#include <tichu/player.h>
#include <tichu/card.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_dump(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_dump_help[] = {
  "DUMP <module> [handle]",
  "",
  "Dumps internal state information of the server.",
  "Use without the <module> argument for a list of valid modules.",
  NULL
};

static struct msg m_dump_msg = {
  "DUMP", 0, 3, MFLG_OPER,
  { m_dump, m_dump, m_dump },
  m_dump_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_dump_load(void)
{
  if(msg_register(&m_dump_msg) == NULL)
    return -1;
  
  return 0;
}

void m_dump_unload(void)
{
  msg_unregister(&m_dump_msg);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void m_dump_dlink(char *arg)
{
  dlink_dump();
}

static void m_dump_slog(char *arg)
{
  int id = -1;
  
  if(arg)
  {
    if(isdigit(*arg))
      id = atoi(arg);
    else
      id = log_source_find(arg);
  }
  
  log_source_dump(id);
}

static void m_dump_dlog(char *arg)
{
  struct dlog *dlptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
    {
      if(arg[1] == 'x')
        dlptr = log_drain_find_cb((void *)strtoul(arg, NULL, 0x10));
      else
        dlptr = log_drain_find_id(atoi(arg));
    }
    else
    {
      dlptr = log_drain_find_path(arg);
    }
  }
  
  log_drain_dump(dlptr);
}

static void m_dump_sheap(char *arg)
{
  struct sheap *shptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      shptr = mem_static_find(atoi(arg));
  }
  
  mem_static_dump(shptr);
}

static void m_dump_dheap(char *arg)
{
  struct dheap *dhptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      dhptr = mem_dynamic_find(atoi(arg));
  }
  
  mem_dynamic_dump(dhptr);
}

static void m_dump_io(char *arg)
{
  int fd = -1;
  
  if(arg)
  {
    if(isdigit(*arg))
      fd = atoi(arg);
  }
  
  io_dump(fd);
}

static void m_dump_ini(char *arg)
{
  struct ini *iptr = NULL;

  if(arg)
  {
    if(isdigit(*arg))
      iptr = ini_find_id(atoi(arg));
    else
      iptr = ini_find_name(arg);
  }
  
  ini_dump(iptr);
}

static void m_dump_card(char *arg)
{
  struct card *cptr = NULL;

  if(arg)
  {
    if(isdigit(*arg))
      cptr = card_find_id(atoi(arg));
    else
      cptr = card_find_name(&card_list, arg);
  }
  
  card_dump(cptr);
}

static void m_dump_timer(char *arg)
{
  struct timer *tptr = NULL;

  if(arg)
  {
    if(isdigit(*arg))
      tptr = timer_find_id(atoi(arg));
  }
  
  timer_dump(tptr);
}

/* dump players */
static void m_dump_player(char *arg)
{
  struct player *player = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      player = player_find_id(atoi(arg));
    else
      player = player_find_name(arg);
  }
  
  player_dump(player);
}

/* dump listeners */
static void m_dump_listen(char *arg)
{
  struct listen *lptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      lptr = listen_find_id(atoi(arg));
    else
      lptr = listen_find_name(arg);
  }
  
  listen_dump(lptr);
}

/* dump classes */
static void m_dump_class(char *arg)
{
  struct class *clptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      clptr = class_find_id(atoi(arg));
    else
      clptr = class_find_name(arg);
  }
  
  class_dump(clptr);
}

/* dump games */
static void m_dump_game(char *arg)
{
  struct game *game = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      game = game_find_id(atoi(arg));
    else
      game = game_find_name(arg);
  }
  
  game_dump(game);
}

/* dump modules */
static void m_dump_module(char *arg)
{
  struct module *mptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
    {
      mptr = module_find_id(atoi(arg));
    }
    else
    {
      if((mptr = module_find_name(arg)) == NULL)
        mptr = module_find_path(arg);
    }
  }
  
  module_dump(mptr);
}

/* dump msgs */
static void m_dump_message(char *arg)
{
  struct msg *mptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      mptr = msg_find_id(atoi(arg));
    else
      mptr = msg_find(arg);
  }
  
  msg_dump(mptr);
}

/* dump net */
static void m_dump_net(char *arg)
{
  struct protocol *nptr = NULL;
  
  if(arg)
  {
    if(isdigit(*arg))
      nptr = net_find_id(atoi(arg));
  }
  
  net_dump(nptr);
}

typedef void (dump_cb_t)(char *);
                         
static struct {
  const char *name;
  dump_cb_t  *cb;
  int        *sp;
} m_dump_table[] = {
  { "io",      m_dump_io,      &io_log       },
  { "ini",     m_dump_ini,     &ini_log      },
  { "msg",     m_dump_message, &msg_log      },
  { "net",     m_dump_net,     &net_log      },
  { "slog",    m_dump_slog,    &log_log      },
  { "dlog",    m_dump_dlog,    &log_log      },
  { "game",    m_dump_game,    &game_log     },
  { "sheap",   m_dump_sheap,   &mem_log      },
  { "dheap",   m_dump_dheap,   &mem_log      },
  { "class",   m_dump_class,   &class_log    },
  { "dlink",   m_dump_dlink,   &dlink_log    },
  { "timer",   m_dump_timer,   &timer_log    },
  { "listen",  m_dump_listen,  &listen_log   },
  { "module",  m_dump_module,  &module_log   },
  { "player",  m_dump_player,  &player_log   },
  { "card",    m_dump_card,    &card_log     },
  { NULL,    NULL,         NULL              }
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void m_dump_callback(uint64_t        flag,  int         lvl,
                            const char     *level, const char *source,
                            const char     *date,  const char *msg,
                            struct player  *player)
{
  player_send(player, "DUMP :%s", msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "DUMP"                                                           *
 * argv[1] - module                                                           *
 * argv[2] - handle                                                           *
 * -------------------------------------------------------------------------- */
static void m_dump(struct player *player, int argc, char **argv)
{
  size_t         i;
  struct dlog   *ldptr;
  
  log(msg_log, L_status, "inside m_dump(%p, %u, [ %s ])", player, argc, argv[0]);
  
  if(!argc)
  {
    uint32_t sz;
    
    player_send(player, "DUMP :modules available to dump:");
    
    sz = (sizeof(m_dump_table) / sizeof(m_dump_table[0])) - 1;
    
    for(i = 0; i < sz; i++)
/*      player_send(player, "DUMP :%-10s %-10s %-10s %-10s",
                  m_dump_table[i + 0].name,
                  m_dump_table[i + 1].name,
                  m_dump_table[i + 2].name,
                  m_dump_table[i + 3].name);
    
    if(sz - i == 3)
      player_send(player, "DUMP :%-10s %-10s %-10s",
                  m_dump_table[i + 0].name,
                  m_dump_table[i + 1].name,
                  m_dump_table[i + 2].name);
    else if(sz - i == 2)
      player_send(player, "DUMP :%-10s %-10s",
                  m_dump_table[i + 0].name,
                  m_dump_table[i + 1].name);
    else if(sz - i == 1)*/
      player_send(player, "DUMP :%-10s",
                  m_dump_table[i + 0].name);                  
  }
  else
  {
    for(i = 0; m_dump_table[i].name; i++)
    {
      if(!stricmp(m_dump_table[i].name, argv[1]))
      {
        ldptr = log_drain_callback(m_dump_callback,
                                   log_sources[*m_dump_table[i].sp].flag,
                                   L_debug, player);
        
        m_dump_table[i].cb(argv[2]);
        
        log_drain_delete(ldptr);
      }
    }
  }  
}
