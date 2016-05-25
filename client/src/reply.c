/* $Id: reply.c,v 1.104 2005/05/23 02:12:17 smoli Exp $
 * -------------------------------------------------------------------------- *
 *  .___.    .                                                                *
 *    |  * _.|_ . .        Portabler, SDL-basierender Client für das          *
 *    |  |(_.[ )(_|             Multiplayer-Kartenspiel Tichu.                *
 *  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . *
 *                                                                            *
 *               (c) 2004-2005 by Martin Zangger, Roman Senn                  *
 *                                                                            *
 *    Dieses Programm ist freie Software. Sie können es unter den Bedingungen *
 * der GNU General Public License, wie von der Free Software Foundation ver-  *
 * öffentlicht, weitergeben und/oder modifizieren, entweder gemäss Version 2  *
 * der Lizenz oder (nach Ihrer Option) jeder späteren Version.                *
 *                                                                            *
 *    Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, dass es  *
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die  *
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BE-    *
 * STIMMTEN ZWECK. Details finden Sie in der GNU General Public License.      *
 *                                                                            *
 *    Sie sollten eine Kopie der GNU General Public License zusammen mit      *
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free     *
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          *
 * 02111-1307, USA.                                                           *
 * -------------------------------------------------------------------------- */

#include <string.h>

/* -------------------------------------------------------------------------- *
 * Das REPLY Modul verarbeitet die Daten die vom Server kommen und            *
 * aktualisiert damit die Engine und das Benutzerinterface                    *
 * -------------------------------------------------------------------------- */
#include "reply.h"
#include "client.h"
#include "stack.h"
#include "game.h"
#include "fan.h"

#include "ui_chat.h"
#include "ui_game.h"
#include "ui_settings.h"

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static size_t reply_tokenize(char *s, char **v, size_t maxtok)
{
  size_t c = 0;

  for(;;)
  {
    /* Skip and zero whitespace */
    while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
      *s++ = '\0';

    /* We finished */
    if(*s == '\0')
      break;

    if(c == maxtok)
      break;

    /* Stop tokenizing when we spot a ':' at token start */
    if(*s == ':')
    {
      /* The remains are a single argument
         so it can include blanks also */

      v[c++] = &s[1];

      break;
    }
  
    /* Add to token list */
    v[c++] = s;
  
    if(c == maxtok)
      break;
   
    /* Scan for end or whitespace */
    while(*s && !(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n'))
      s++;
  }
    
  if(c == maxtok || *s == ':')
  {
    while(*s)
    {
      if(*s == '\r' || *s == '\n')
      {
        *s = '\0';
        break;
      }

      s++;
    }
    
    do

      s--;
    while(*s == ' ' || *s == '\t');

    *++s = '\0';
  }

  v[c] = NULL;

  return c;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
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
static void reply_abandon  (char *prefix, int argc, char *argv[]);
static void reply_points   (char *prefix, int argc, char *argv[]);
static void reply_end      (char *prefix, int argc, char *argv[]);
static void reply_round    (char *prefix, int argc, char *argv[]);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct reply reply_table[] = {
  { "LOGIN",    3, 3,  reply_login },
  { "JOIN",     2, 3,  reply_join },
  { "LOGOUT",   3, 3,  reply_logout },
  { "MSG",      3, 4,  reply_msg },
  { "HELP",     3, 3,  reply_help },
  { "PLAYERS",  1, 4,  reply_players },
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
  { "SCHUPFE",  2, 4,  reply_schupfe },
  { "ABANDON",  1, 2,  reply_abandon },
  { "POINTS",   3, 4,  reply_points },  
  { "END",      1, 2,  reply_end },  
  { "ROUND",    1, 2,  reply_round },  
};

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
  
  reply_debug(INFO, "%s", msg);

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
    reply_log(STATUS, "Unbekannte Serverantwort: %s %s", argv[0], args);
    return -1;
  }
  
  n = reply_tokenize(args, &argv[1], reply->maxargs - 1);  
  n++;
  
  if(n < reply->args)
  {
    reply_log(STATUS, "Serveranwort %s hat zu wenig Argumente.", reply->name);
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
    client_puts("Login fehlgeschlagen: %s", argv[2]);
    return;
  }
  if(!strcasecmp(argv[1], "OK"))
  {
    client_puts("Login erfolgreich: %s", argv[2]);
    return;
  }
  client_puts("Ungültige Server antwort");
  
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein LOGOUT command                                 *
 * -------------------------------------------------------------------------- */
static void reply_logout(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1], "OK"))
  {
    client_puts("Logout erfolgreich: %s", argv[2]);
    return;
  }

  client_puts("Ungültige Server antwort");
}

/* -------------------------------------------------------------------------- *
 * Eingehende Nachrichten für den Spieler                                     *
 * -------------------------------------------------------------------------- */
static void reply_msg(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    client_puts("msg fehlgeschlagen: %s", argv[2]);
    return;
  }
  
  if(prefix)
  {
    if(!strcmp(argv[1], client_config.user))
      client_puts("(private from->*%s*) %s", prefix, argv[2]);
    else if(!strcmp(prefix, client_config.user) && *argv[1] != '@')
      client_puts("(private to->*%s*) %s", argv[1], argv[2]);
    else      
      client_puts("<%s> %s", prefix, argv[2]);    
  }
  
  return;  
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein PLAYERS command                                *
 * -------------------------------------------------------------------------- */
static void reply_players(char *prefix, int argc, char *argv[])
{
  struct sgWidget     *listbox = NULL;
  struct sgListboxItem *item;
  
  if(argc != 4)
    return;
  
  if(!strcasecmp(argv[1], "FAIL"))
   {
     reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
     return;
   }
  
  if(client_status == CLIENT_CHAT)
    listbox = ui_chat_players;
  else if(client_status == CLIENT_SETTINGS)
    listbox = ui_settings_players;
  
  if(listbox && !sgGetListboxItemByCaption(listbox, argv[1]))
  {
    item = sgAddListboxItem(listbox, argv[1], NULL);
    
    /* Falls wir in einem Game sind -> Haben die Spieler bereits akzeptiert und team gewählt? */
    if(client_status == CLIENT_SETTINGS)
    {      
      if(atoi(argv[2]))
        sgSetListboxItemSymbol(listbox, item, 0, ui_symbol_team[atoi(argv[2])-1]);
      
      sgSetListboxItemSymbol(listbox, item, 1, ui_symbol_accept[atoi(argv[3])]);
    }
    
    return;
  }
   
}  

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein GAMES command                                  *
 * -------------------------------------------------------------------------- */
static void reply_games(char *prefix, int argc, char *argv[])
{
  struct sgListboxItem *item = NULL;

  if(!strcasecmp(argv[1], "FAIL"))
   {
     reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
     return;
   }
  
  if(argc == 2)
    return;
  
  /* games liste nur in ui_chat */
  if(client_status != CLIENT_CHAT)
    return;
  
  if(!(item = sgGetListboxItemByCaption(ui_chat_games, argv[1])))
  {
    char gamename[50];
    
    /* public channel wird nicht nach dem gleichen prinzip bennent */
    if(!strcmp(argv[1], "@\0"))
    {
      return;
    }
    else
    {
      strcpy(gamename, argv[1]);
      strcpy((gamename+strlen(argv[1])), "'s Spiel");            
    }
    
    item = sgAddListboxItem(ui_chat_games, gamename, argv[1]);
  }
  
  if(strcmp(argv[1], "@\0"))
  {    
    sgSetListboxItemSymbol(ui_chat_games, item,  0, ui_symbol_icon[atoi(argv[3])]);
    sgSetListboxItemSymbol(ui_chat_games, item,  1, ui_symbol_user[atoi(argv[2])-1]);
  }  
  
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein JOIN command                                   *
 * -------------------------------------------------------------------------- */
static void reply_join(char *prefix, int argc, char *argv[])
{
  struct sgWidget   *listbox = NULL;

  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
    
  if(prefix)
  {
    reply_log(STATUS, "*** %s", argv[2]);
    
    /* hat der user den channel gewechselt? */
    if(!strcmp(prefix,client_config.user))
    {      
      /* dann->
       * standart target = new chan
       */
      strcpy(client_target, argv[1]);
      
      /* public channel joinen */      
      if(!strcmp(argv[1], "@"))
      {              
        if(client_status == CLIENT_GAME)
        {  
          game_unset(GAME_RUN);
        }        
        else if(client_status == CLIENT_SETTINGS)
        {
          sgClearWidgetStatus(ui_settings_dialog, SG_RUNNING);
          client_status = CLIENT_CHAT;
        }        
          
        /* players list für den chan anfordern */
        net_send("PLAYERS %s", argv[1]);                        
      } 
      /* in das setting ui wechseln */
      else
      {         
        sgClearWidgetStatus(ui_chat_dialog, SG_RUNNING);
        client_status = CLIENT_SETTINGS;
      }
      
    }
    else
    {
      /* sont ->
       * den neu gejointen player in die players list adden 
       */
      if(client_status == CLIENT_CHAT)
        listbox = ui_chat_players;
      else if(client_status == CLIENT_SETTINGS)
        listbox = ui_settings_players;
      else
        listbox = NULL;
      
      if(listbox)
      {
        sgListboxItem *item;
        
        item = sgAddListboxItem(listbox, prefix, NULL);
        
        if(client_status == CLIENT_SETTINGS)
          sgSetListboxItemSymbol(listbox, item, 1, ui_symbol_accept[0]);
      }
      
      if(client_status == CLIENT_CHAT)
        /* neue game list anfordern (wegen userzahlen) */
        net_send("GAMES");
    }
  }
  
  return;  
  
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein LEAVE command                                  *
 * -------------------------------------------------------------------------- */
static void reply_leave(char *prefix, int argc, char *argv[])
{
  struct sgWidget *listbox;
  
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s", argv[2]);
    return;
  }  
  
  if(client_status == CLIENT_CHAT)
    listbox = ui_chat_players;
  else if(client_status == CLIENT_SETTINGS)
    listbox = ui_settings_players;
  else
    listbox = NULL;
  
  if(prefix && listbox)
  {    
    sgListboxItem *item = sgGetListboxItemByCaption(listbox, prefix);
    
    reply_log(STATUS, "*** %s", argv[2]);
    
    if(item)
      sgDeleteListboxItem(listbox, item);
    
    /* wenn der game founder das game verlassen hat, game löschen */
    item = sgGetListboxItemByCaption(ui_chat_games, argv[1]);
    if(item)
      sgDeleteListboxItem(ui_chat_games, item);
                                  
    /* neue game list anfordern (wegen userzahlen) */
    net_send("GAMES");

  }
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein CREATE command                                 *
 * -------------------------------------------------------------------------- */
static void reply_create(char *prefix, int argc, char *argv[])
{
  char gamename[50];
  char itemname[50];
  
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  
  if(prefix && client_status == CLIENT_CHAT)
  {
    /* text zum anzeigen */
    strcpy(gamename, "@");
    strcpy(gamename + 1, prefix);
    strcpy((gamename + strlen(prefix) + 1), "'s Spiel");   
    
    /* wert des eintrags */
    strcpy(itemname, "@");
    strcpy(itemname + 1, prefix);
           
    sgAddListboxItem(ui_chat_games, gamename, prefix);
  }
  
  return;    
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein ACCEPT command                                 *
 * -------------------------------------------------------------------------- */
static void reply_accept(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  if(prefix && client_status == CLIENT_SETTINGS)
  {
    reply_log(STATUS, "*** %s", argv[2]);

    sgListboxItem *item;
    
    item = sgGetListboxItemByCaption(ui_settings_players, prefix);    
    
    sgSetListboxItemSymbol(ui_settings_players, item, 1, ui_symbol_accept[1]);
  }
  
  return;    
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein TEAM command                                   *
 * -------------------------------------------------------------------------- */
static void reply_team(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  if(prefix && client_status == CLIENT_SETTINGS) 
  {
    reply_log(STATUS, "*** %s", argv[3]);
    
    sgListboxItem *item;
    
    item = sgGetListboxItemByCaption(ui_settings_players, prefix);    
    
    if(item)
      sgSetListboxItemSymbol(ui_settings_players, item, 0, ui_symbol_team[atoi(argv[2])-1]);
  }
  
  return;    
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein START command                                  *
 * -------------------------------------------------------------------------- */
static void reply_start(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  if(!strcasecmp(argv[1],"OK")) 
  {
    /* Netzwerk erst wieder im Gameloop auslesen */
    sgSetDialogTimer(ui_settings_dialog, NULL, 0, NULL);
    
    reply_log(INFO, "In Game-Modus wechseln...");
    
    client_status = CLIENT_GAME;
    sgClearWidgetStatus(ui_settings_dialog, SG_RUNNING);
  }
  
  return;    
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein KICK command                                   *
 * -------------------------------------------------------------------------- */
static void reply_kick(char *prefix, int argc, char *argv[])
{
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  if(!strcasecmp(argv[1],"OK")) 
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  return;    
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein CARDS command                                  *
 * -------------------------------------------------------------------------- */
static void reply_cards(char *prefix, int argc, char *argv[])
{
  struct card *card;
  
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  reply_log(STATUS, "*** %s",  argv[2]);
  
  /* nur wenn wir uns in game status befinden */
  if(client_status == CLIENT_GAME)
  {
    /* alle karten verteilt?*/
    if(!strcasecmp(argv[1],"OK")) 
    {
//      fan_calc();
      player_start();
      return;
    }
    
    /* karte dem fächer hinzufügen */
    if((card = card_get(argv[1])))
    {      
      if(fan_list.size < 14)
      {
        fan_add(card);
        fan_calc();
      }
    }
    
  }
  
  return;
 
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein ORDER command                                  *
 * -------------------------------------------------------------------------- */
static void reply_order(char *prefix, int argc, char *argv[])
{
  
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
    
  /* nur wenn wir uns im game status befinden */
  if(client_status == CLIENT_GAME)
  {
    SDL_Rect rect;
    
    struct color c = gfxutil_white;
    
    if(argv[3])
      gfxutil_parsecolor(&c, argv[3]);
    
    if(!player_find(argv[1]))
      player_new(argv[1], atoi(argv[2]), c);
   
    /* berechnung der positionen für die labels im game menu */
    rect = ui_game_status_player[atoi(argv[2])]->rect;
    rect.w = sgTextWidth(ui_game_status_player[atoi(argv[2])]->font[1], argv[1]);

    if(atoi(argv[2]) == 3)
    {      
      if(rect.w > sgGroup(ui_game_menu_group)->body.w / 2 - 10)
        rect.w = sgGroup(ui_game_menu_group)->body.w / 2 -10;
      rect.x = sgGroup(ui_game_menu_group)->body.w / 4 - (rect.w / 2);
      if(rect.x < 10)
        rect.x = 10;
      
    }    
    else if(atoi(argv[2]) == 1)
    {
      if(rect.w > sgGroup(ui_game_menu_group)->body.w / 2 - 10)
        rect.w = sgGroup(ui_game_menu_group)->body.w / 2 -10;
      rect.x = (sgGroup(ui_game_menu_group)->body.w / 4 * 3 - 10) - (rect.w / 2);
    }
    else
    {      
      rect.x = sgGroup(ui_game_menu_group)->body.w / 2 - rect.w / 2;
    }
    
    sgSetWidgetCaption(ui_game_status_player[atoi(argv[2])], argv[1]);
    sgSetWidgetRect(ui_game_status_player[atoi(argv[2])], rect);
  }     
  
  return;
 
}

/* -------------------------------------------------------------------------- *
 * Antowrt des Servers auf ein TICHU command                                  *
 * -------------------------------------------------------------------------- */
static void reply_tichu(char *prefix, int argc, char *argv[])
{
     
  if(!strcasecmp(argv[1],"FAIL"))
  {
    reply_log(STATUS, "*** %s %s: %s", argv[0], argv[1], argv[2]);
    return;
  }
  
  /* Jetzt gelegenheit um grosses Tichu anzukündigen */
  if(!strcasecmp(argv[1],"START"))
  {
    ui_game_tichu_start();
    return;
  }

  /* gelegenheit vorbei um grosses Tichu anzukündigen */
  if(!strcasecmp(argv[1],"OK"))
  {
    ui_game_tichu_end();
    return;
  }
  
  reply_log(STATUS, "*** %s %s: %s", argv[0], "la", "intressant");

  
  return; 
}

/* -------------------------------------------------------------------------- *
 * Server command ACTOR                                                       *
 * -------------------------------------------------------------------------- */
static void reply_actor(char *prefix, int argc, char *argv[])
{  
  struct player *player;
  
  /* nur wenn wir uns im game status befinden */
  if(client_status == CLIENT_GAME)
  {
    if((player = player_find(argv[1])))
    {
      sgSetWidgetCaption(ui_game_actor, argv[1]);
      
      stack_actor(player);
    }
  }
  
  return;
}

/* -------------------------------------------------------------------------- *
 * Server command PRICK                                                       *
 * -------------------------------------------------------------------------- */
static void reply_prick(char *prefix, int argc, char *argv[])
{  
  /* nur wenn wir uns im game status befinden */
  if(client_status == CLIENT_GAME)
  {    
    stack_clear();
    reply_log(STATUS, "*** %s", argv[1]);   
  }
  
  return;
 
}

/* -------------------------------------------------------------------------- *
 * Antwort des Servers auf ein PLAYCARDS command                              *
 * -------------------------------------------------------------------------- */
static void reply_playcards(char *prefix, int argc, char *argv[])
{
  int i;
  struct list cards;
  struct card *card;

  dlink_list_zero(&cards);
  
  if(!strcasecmp(argv[1], "FAIL"))
  {
    reply_log(STATUS, "*** Karten wurden abgelehnt: %s", argv[2]);
    stack_refuse();
  }
  
  if(prefix)
  {
    reply_log(STATUS, "*** %s", argv[argc-1]);
    
    if(!strcmp(prefix, client_config.user))
    {
      stack_drop();
    }
    else
    {
      struct player *player;
      
      /* Den Spieler der die Karten gespielt hat finden */
      if((player = player_find(prefix)))
      {
        /* Durch die Karten gehen und auf die Stapelliste adden */
        for(i = 1; i < argc-1; i++)
        {
          if((card = card_get(argv[i])))
            stack_add(card, player);
        }
        
        /* Karten erscheinen lassen */
        stack_appear(&cards, player, 250);
        
        /* Bis die Karten auf dem Stapel sind sollten
           keine weiteren Replys verarbeitet werden */
        net_set(NET_SHUTUP);           
      }
    }    
    
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_schupfe(char *prefix, int argc, char *argv[])
{
  struct card *card;
  struct player *from;

  if(!strcasecmp(argv[1], "FAIL"))
  {
    reply_log(STATUS, "*** Karte wurden abgelehnt: %s", argv[2]);
    return;
  }
  
  reply_debug(INFO, "SCHUPFILEIN", argv[2]);

  if(!strcasecmp(argv[1], "OK"))
  {
    if(!strcasecmp(argv[2], "END"))
    {
      /* Schupfungen beendet */      
      reply_log(STATUS, "*** %s", argv[3]);
    
      /* fächer sortieren */
      fan_sort();
      
      /* Stapel ist jetzt Drag&Drop-Ziel */
      fan_target = CARD_STACK;
    }
    else
    {
      reply_log(STATUS, "*** %s", argv[2]);
    }
    
    return;
  }  
  
  if(prefix)
  {
    if(!(from = player_find(prefix)))
      return;
    
    reply_log(STATUS, "*** %s", argv[argc-1]);
        
    if((card = card_get(argv[1])))
      player_return(from, card);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_error(char *prefix, int argc, char *argv[])
{
  client_puts("%s", argv[1]);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_help(char *prefix, int argc, char *argv[])
{
  client_puts("%s", argv[2]);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_abandon(char *prefix, int argc, char *argv[])
{
  reply_log(STATUS, "*** %s", argv[1]);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_points(char *prefix, int argc, char *argv[])
{  
  if(client_status == CLIENT_GAME)
  {    
    if(atoi(argv[1]) == 1)
      sgSetWidgetCaption(ui_game_status_points1, argv[2]);
    else if(atoi(argv[1]) == 2)
      sgSetWidgetCaption(ui_game_status_points2, argv[2]);
  }
  else if(client_status == CLIENT_GAME_END)
  {
    if(atoi(argv[1]) == 1)
      sgSetWidgetCaption(ui_game_end_points1, argv[2]);
    else if(atoi(argv[1]) == 2)
      sgSetWidgetCaption(ui_game_end_points2, argv[2]);
  }    
   
  reply_log(STATUS, "*** %s",  argv[3]);
    
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void reply_end(char *prefix, int argc, char *argv[])
{  
  client_status = CLIENT_GAME_END;
  
  game_unset(GAME_RUN);
}


static void reply_round(char *prefix, int argc, char *argv[])
{  
  /* Game ui Reseten */
//  ui_game_status = 0;  wtf?
 
}

