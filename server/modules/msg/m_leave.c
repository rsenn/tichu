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
#include <tichu/game.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_leave(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_leave_help[] = {
  "LEAVE",
  "",
  "Leavs Game.",
  NULL
};

static struct msg m_leave_msg = {
  "LEAVE", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_leave,  m_leave },
  m_leave_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_leave_load(void)
{
  if(msg_register(&m_leave_msg) == NULL)
    return -1;

  return 0;
}

void m_leave_unload(void)
{
  msg_unregister(&m_leave_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "LEAVE"                                                          *
 * -------------------------------------------------------------------------- */
static void m_leave(struct player *player, int argc, char **argv)
{  
  
  if(player->game == tichu_public)
  {
   player_send(player, "%s FAIL :%s", argv[0], "Public Channel kann nicht verlassen werden");
    return;
  }

  /* public channel joinen 
   * aktueller chan wird automatisch verlassen (ausgelöst in game_join)
   */
  game_join(tichu_public, player, "");
  
}
