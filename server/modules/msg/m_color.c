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
static void m_color(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_color_help[] = {
  "COLOR #<color>",
  "",
  "Sets color for the player",
  NULL
};

static struct msg m_color_msg = {
  "COLOR", 1, 1, MFLG_PLAYER,
  { m_registered, m_color,  m_color },
  m_color_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_color_load(void)
{
  if(msg_register(&m_color_msg) == NULL)
    return -1;

  return 0;
}

void m_color_unload(void)
{
  msg_unregister(&m_color_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "COLOR"                                                          *
 * argv[1] - color                                                            *
 * -------------------------------------------------------------------------- */
static void m_color(struct player *player, int argc, char **argv)
{  
  if(argv[1])
  {
    struct color color;
    
    image_color_parse(&color, argv[1]);
    
    player->color = color;
  }
  
  return;
}
