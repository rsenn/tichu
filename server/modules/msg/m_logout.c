/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/log.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/msg.h>
#include <tichu/chars.h>
#include <tichu/player.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_logout(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_logout_help[] = {
  "LOGOUT",
  "",
   "User quits.",
  NULL
};

static struct msg m_logout_msg = {
  "LOGOUT", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_logout, m_logout },
  m_logout_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_logout_load(void)
{
  if(msg_register(&m_logout_msg) == NULL)
    return -1;

  return 0;
}

void m_logout_unload(void)
{
  msg_unregister(&m_logout_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "LOGOUT"                                                          *
 * -------------------------------------------------------------------------- */
static void m_logout(struct player *player, int argc, char **argv)
{  
    /* Spieler verabschieden */
  player_send(player, "%s %s :Gute nacht %s.....",
              argv[0], argv[1], player->name);
  
  player_delete(player);
}
