/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/dlink.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/tichu.h>
#include <tichu/msg.h>
#include <tichu/player.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define M_HELP_INDEX     "INDEX"

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_help           (struct player *player, int argc, char **argv);
static void m_help_index     (struct player *player);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_help_help[] = {
  "HELP [topic]",
  "",
  "Displays help about the given topic.",
  NULL    
};

static struct msg m_help_msg = {
  "HELP", 0, 1, MFLG_PLAYER,
  { NULL, m_help, NULL, m_help },
  m_help_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_help_load(void)
{
  if(msg_register(&m_help_msg) == NULL)
    return -1;
  
  return 0;
}

void m_help_unload(void)
{
  msg_unregister(&m_help_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - prefix                                                           *
 * argv[1] - 'help'                                                           *
 * argv[2] - command                                                          *
 * -------------------------------------------------------------------------- */
static void m_help(struct player *player, int argc, char **argv)
{
  struct msg *mptr;
  uint32_t    i;

  if(argv[1] == NULL || !stricmp(argv[1], M_HELP_INDEX))
  {
    player_send(player, "HELP OK :Commands:");
    m_help_index(player);
    return;
  }
  
  if((mptr = msg_find(argv[1])) == NULL)
  {
    player_send(player, "HELP FAIL :Keine Hilfe für Befehl %s.", argv[1]);
    return;    
  }
  
/*  if(mptr->help == NULL || mptr->help[0] == NULL)
  {
    log(msg_log, L_fatal,
        "The developer was too lazy to write a help for %s. This must be sued.",
        mptr->cmd);
    tichu_shutdown();
  }*/
  
  for(i = 0; mptr->help[i]; i++)
  {
    player_send(player, "HELP OK :%s", mptr->help[i]);
  }
    
  player_send(player, "HELP END :Ende der Hilfe");
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void m_help_index(struct player *player)
{
  struct node *node;
  struct msg  *mptr;
  uint32_t     msgi = 0;
  char        *msgs[4];
  uint32_t     i;
 
  for(i = 0; i < MSG_HASH_SIZE; i++)
  {
    dlink_foreach_data(&msg_table[i], node, mptr)
    {
      msgs[msgi++] = mptr->cmd;
      
      if(msgi == 4)
      {
        player_send(player, "HELP OK :%-12s %-12s %-12s %s",
                    msgs[0], msgs[1], msgs[2], msgs[3]);
        msgi = 0;
      }
    }
  }
  
  if(msgi == 3)
  {
    player_send(player, "HELP OK :%-12s %-12s %s",
                msgs[0], msgs[1], msgs[2]);
  }
  else if(msgi == 2)
  {
    player_send(player, "HELP OK :%-12s %s",
                msgs[0], msgs[1]);
  }
  else if(msgi == 1)
  {
    player_send(player, "HELP OK :%s",
                msgs[0]);
  }
}

