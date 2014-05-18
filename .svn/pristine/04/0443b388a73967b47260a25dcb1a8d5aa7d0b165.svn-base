#include <libchaos/str.h>
#include <libchaos/log.h>
#include <libchaos/timer.h>

#include "reply.h"
#include "game.h"
#include "player.h"
#include "bot.h"
#include "card.h"

int reply_log;

static void reply_login    (char *prefix, int argc, char *argv[]);
static void reply_logout   (char *prefix, int argc, char *argv[]);
static void reply_join     (char *prefix, int argc, char *argv[]);
static void reply_msg      (char *prefix, int argc, char *argv[]);
static void reply_help     (char *prefix, int argc, char *argv[]);
static void reply_players  (char *prefix, int argc, char *argv[]);
static void reply_games    (char *prefix, int argc, char *argv[]);
static void reply_error    (char *prefix, int argc, char *argv[]);
static void reply_leave    (char *prefix, int argc, char *argv[]);
static void reply_create   (char *prefix, int argc, char *argv[]);
static void reply_accept   (char *prefix, int argc, char *argv[]);
static void reply_team     (char *prefix, int argc, char *argv[]);
static void reply_start    (char *prefix, int argc, char *argv[]);
static void reply_kick     (char *prefix, int argc, char *argv[]);
static void reply_cards    (char *prefix, int argc, char *argv[]);
static void reply_order    (char *prefix, int argc, char *argv[]);
static void reply_prick    (char *prefix, int argc, char *argv[]);
static void reply_actor    (char *prefix, int argc, char *argv[]);
static void reply_tichu    (char *prefix, int argc, char *argv[]);
static void reply_playcards(char *prefix, int argc, char *argv[]);
static void reply_schupfe  (char *prefix, int argc, char *argv[]);
static void reply_round    (char *prefix, int argc, char *argv[]);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct reply reply_table[] = {
  { "LOGIN",    3, 3,  reply_login },
  { "JOIN",     2, 3,  reply_join },
  { "LOGOUT",   3, 3,  reply_logout },
  { "MSG",      3, 4,  reply_msg },
  { "HELP",     3, 3,  reply_help },
  { "PLAYERS",  2, 4,  reply_players },
  { "GAMES",    2, 4,  reply_games },
  { "ERROR",    2, 2,  reply_error },
  { "DUMP",     2, 2,  reply_error },
  { "LEAVE",    3, 3,  reply_leave },
  { "CREATE",   2, 3,  reply_create },
  { "ACCEPT",   2, 3,  reply_accept },
  { "TEAM",     3, 4,  reply_team },
  { "START",    2, 3,  reply_start },
  { "KICK",     2, 3,  reply_kick },
  { "CARDS",    2, 3,  reply_cards },
  { "ORDER",    4, 4,  reply_order },
  { "PRICK",    2, 2,  reply_prick },
  { "ACTOR",    2, 2,  reply_actor },
  { "TICHU",    2, 3,  reply_tichu },
  { "PLAYCARDS",3, 16, reply_playcards },
  { "SCHUPFE",  2, 3,  reply_schupfe },
  { "ROUND",    2, 3,  reply_round },
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void reply_init(void)
{
  reply_log = log_source_register("reply");
  
  log(reply_log, L_status, "Initialised [reply] module.");
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void reply_shutdown(void)
{
  log(reply_log, L_status, "Shutting down [reply] module...");
  
  log_source_unregister(reply_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct reply *reply_find(char *name)
{
  size_t i;
  
  for(i = 0; i < sizeof(reply_table) / sizeof(reply_table[0]); i++)
    if(!strcasecmp(name, reply_table[i].name))
      return &reply_table[i];
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int reply_parse(char *msg)
{
  char  *argv[32];
  size_t n;
  char *prefix;
  char *args;
  struct reply *reply;
  
  if((args = strchr(msg, '\n')))
    *args = '\0';  
  
  log(bot_log, L_status, "<-- SERVER: %s", msg);
  
  prefix = NULL;
  
  /* Antwort hat ein prefix */
  if(*msg == ':')
  {
    prefix = &msg[1];
    
    if(!(msg = strchr(msg, ' ')))
      return -1;
    
    *msg++ = '\0';
  }
  
  /* Kommando rausfinden */
  argv[0] = msg;
  
  if(!(args = strchr(msg, ' ')))
    args = msg + strlen(msg);
  else
    *args++ = '\0';
  
  if(!(reply = reply_find(argv[0])))
  {
    log(reply_log, L_warning, "Unbekannte Serverantwort: %s %s", argv[0], args);
    return -1;
  }
  
  n = strtokenize(args, &argv[1], reply->maxargs - 1);  
  n++;
  
  if(n < reply->args)
  {
    log(reply_log, L_warning, "Serveranwort %s hat zu wenig Argumente.", reply->name);
    return -1;
  }

  reply->callback(prefix, n, argv);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein LOGIN command                                  *
 * -------------------------------------------------------------------------- */
static void reply_login(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    char *anothername;
    
    anothername = bot_getname();
    
    strlcpy(bot_user, anothername, sizeof(bot_user));
    
    bot_login();
    
    return;
  }
  if(!strcasecmp(argv[1], "OK"))
  {
    log(reply_log, L_status, "Login erfolgreich: %s", argv[2]);
    return;
  }
  log(reply_log, L_status, "Ungültige Server antwort");
  
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein LOGOUT command                                 *
 * -------------------------------------------------------------------------- */
static void reply_logout(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1], "OK"))
  {
    log(reply_log, L_status, "Logout erfolgreich: %s", argv[2]);
    return;
  }

  log(reply_log, L_status, "Ungültige Server antwort");
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein MSG command                                    *
 * -------------------------------------------------------------------------- */
static void reply_msg(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    log(reply_log, L_status, "msg fehlgeschlagen: %s", argv[2]);
    return;
  }
  
  if(prefix)
  {
    if(!strcmp(argv[1], bot_user))
      log(reply_log, L_status, "(private from->*%s*) %s", prefix, argv[2]);
    else if(!strcmp(prefix, bot_user) && *argv[1] != '@')
      log(reply_log, L_status, "(private to->*%s*) %s", argv[1], argv[2]);
    else      
      log(reply_log, L_status, "<%s> %s", prefix, argv[2]);    
  }
  
  return;  
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein PLAYERS command                                *
 * -------------------------------------------------------------------------- */
static void reply_players(char *prefix, int argc, char *argv[])
{
  /* player-listen eintrag */
  if(argc == 4)
  {
    struct player *player;
    
    if((player = player_find_name(argv[1])))
    {
      player_update(player, atoi(argv[2]), atoi(argv[3]));
    }
    else
    {
      player = player_new(argv[1], atoi(argv[2]), atoi(argv[3]));
      
      if(!strcmp(argv[1], bot_user))
        bot_player = player;
    }
  }
  /* ende der player liste */
  else
  {
    struct player *player;
    struct node *node;
    
    /* games die nicht erstellt oder updated wurden löschen */
    dlink_foreach_safe(&player_list, player, node)
    {
      if(player->serial != player_serial)
        player_delete(player);
    }
    
    player_serial++;
    
    player_dump(NULL);
    
    game_check_team(bot_game);
    game_check_accept(bot_game);
  }
  
}  

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein GAMES command                                  *
 * -------------------------------------------------------------------------- */
static void reply_games(char *prefix, int argc, char *argv[])
{
  log(reply_log, L_status, "game reply argc: %u", argc);
  
  /* ein game */
  if(argc == 4)
  {
    struct game *game;
    
    if((game = game_find_name(argv[1])))
    {
      game_update(game, atoi(argv[2]), atoi(argv[3]));
    }
    else
    {
      game_new(argv[1], atoi(argv[2]), atoi(argv[3]));
    }
  }
  /* ende der game liste */
  else
  {
    struct game *game;
    struct node *node;
    
    /* games die nicht erstellt oder updated wurden löschen */
    dlink_foreach_safe(&game_list, game, node)
    {
      if(game->serial != game_serial)
        game_delete(game);
    }
    
    /* games anzeigen */
    game_dump(NULL);
    
    game_serial++;
    
    if(game_list.size > 1)
    {
      struct list list;

      dlink_list_zero(&list);

      dlink_foreach(&game_list, game)
      {
        if(game->player_count < 4 && strcmp(game->name, "@"))
          dlink_add_tail(&list, dlink_node_new(), game);
      }
      
      node = dlink_index(&list, rand() % list.size);
      
      game_join(node->data);
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein JOIN command                                   *
 * -------------------------------------------------------------------------- */
static void reply_join(char *prefix, int argc, char *argv[])
{
  struct player *player;
  
  if(prefix && strcmp(prefix, bot_user))
  {
    if(bot_game)
    {
      if(!strcmp(argv[1], bot_game->name))
      {
        player = player_new(prefix, 0, 0);
        
        if(!strcmp(prefix, bot_user))
          bot_player = player;
        
        game_check_team(bot_game);
        game_check_accept(bot_game);
      }
    }
    
    return;
  }
  
  if(!strcmp(argv[1], "@"))
  {
    bot_game_timer = timer_start(bot_checkgames, bot_checkinterval);
    
    game_clear();
    
    bot_game = NULL;
    bot_player = NULL;
  }
  else
  {
    if(bot_game && bot_game_timer) 
    {
      player_clear();
      
      bot_send("PLAYERS %s", bot_game->name);
    } 
    
    timer_cancel(&bot_game_timer);
    
  }
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein LEAVE command                                  *
 * -------------------------------------------------------------------------- */
static void reply_leave(char *prefix, int argc, char *argv[])
{
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein CREATE command                                 *
 * -------------------------------------------------------------------------- */
static void reply_create(char *prefix, int argc, char *argv[])
{
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein ACCEPT command                                 *
 * -------------------------------------------------------------------------- */
static void reply_accept(char *prefix, int argc, char *argv[])
{
  struct player *player;
  
  if(prefix && (player = player_find_name(prefix)))
  {
    player_update(player, player->team, 1);
  }
  
  game_check_team(bot_game);
  game_check_accept(bot_game);
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein TEAM command                                 *
 * -------------------------------------------------------------------------- */
static void reply_team(char *prefix, int argc, char *argv[])
{
  struct player *player;

  if(!bot_game)
    return;
  
  if(!strcmp(argv[1], "FAIL"))
  {
    bot_game->team = 0;
    game_check_team(bot_game);    
    
    return;
  } 
  
  if(prefix && (player = player_find_name(prefix)))
  {
    player_update(player, atoi(argv[2]), player->accepted);
  }
  
  game_check_team(bot_game);
  game_check_accept(bot_game);
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein START command                                 *
 * -------------------------------------------------------------------------- */
static void reply_start(char *prefix, int argc, char *argv[])
{
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein KICK command                                   *
 * -------------------------------------------------------------------------- */
static void reply_kick(char *prefix, int argc, char *argv[])
{
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein CARDS command                                  *
 * -------------------------------------------------------------------------- */
static void reply_cards(char *prefix, int argc, char *argv[])
{
  if(bot_game && bot_player)
  {
    struct card *card;
    
    if(strcmp(argv[1], "OK"))
    {
      if(!(card = card_find_name(argv[1])))
      {
        log(reply_log, L_warning, "invalid card: %s", argv[1]);
        return;
      }
      
      dlink_add_tail(&bot_player->cards, &card->pnode, card);
    }
    else
    {
      game_check_schupfe(bot_game);
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein ORDER command                                  *
 * -------------------------------------------------------------------------- */
static void reply_order(char *prefix, int argc, char *argv[])
{
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein TICHU command                                  *
 * -------------------------------------------------------------------------- */
static void reply_tichu(char *prefix, int argc, char *argv[])
{
}

/* -------------------------------------------------------------------------- *
 * Server command ACTOR                                                       *
 * -------------------------------------------------------------------------- */
static void reply_actor(char *prefix, int argc, char *argv[])
{  
  if(bot_game && bot_player)
  {
    if(!strcmp(argv[1], bot_player->name)) 
    {
      timer_start(game_check_play, 1000, bot_game);
    } 
  }
}

/* -------------------------------------------------------------------------- *
 * Server command PRICK                                                       *
 * -------------------------------------------------------------------------- */
static void reply_prick(char *prefix, int argc, char *argv[])
{  
  if(bot_game) 
  {
    dlink_list_zero(&bot_game->cards);
  } 
}

/* -------------------------------------------------------------------------- *
 * Antwort des Servers auf ein PLAYCARDS command                              *
 * -------------------------------------------------------------------------- */
static void reply_playcards(char *prefix, int argc, char *argv[])
{
  if(bot_game && bot_player)
  {
    if(prefix && strcmp(argv[1], "OK"))
    {
      struct player *player = player_find_name(prefix);
      struct list list;
      struct card *card;
      
      dlink_list_zero(&list);
        
      if(player)
      {
        int i;
        
        for(i = 1; i < argc; i++)
        {
          if((card = card_find_name(argv[i])))
          {            
            dlink_add_tail(&list, dlink_node_new(), card);
            if(!strcmp(card->name, "xp"))
            {
              if(bot_game->cards.head && bot_game->cards.head->data)
                card->value = ((struct card *)bot_game->cards.head->data)->value + 1;
              else
                card->value = 1;
            }
            
          }
          
        }
        
        player_playcards(player, &list);
      }
      
      dlink_destroy(&list);
    }
  }
  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_schupfe(char *prefix, int argc, char *argv[])
{    
  if(prefix)
  {    
    struct card *card;
    
    if(!(card = card_find_name(argv[1])))
    {      
      log(reply_log, L_warning, "invalid card: %s", argv[1]);
      return;
    }    
    dlink_add_tail(&bot_player->cards, &card->pnode, card);
    log(reply_log, L_status, "recieved card: %s", argv[1]);
    
  }    
  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_error(char *prefix, int argc, char *argv[])
{
  log(reply_log, L_status, "%s", argv[1]);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_help(char *prefix, int argc, char *argv[])
{
  log(reply_log, L_status, "%s", argv[2]);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_round(char *prefix, int argc, char *argv[])
{
  
//  if(strcmp(argv[1], "OK"))
  /* zurücksetzten */
  bot_game->gschupft = 0;
    
  card_shutdown();
  card_init();
  dlink_list_zero(&bot_player->cards);
  dlink_list_zero(&bot_game->cards);
}
