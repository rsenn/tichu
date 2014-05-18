/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/db.h>

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
static void m_login(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */

/* Hilfetext zum Kommando - jede Zeile ein separater String, 
   NULL-terminiertes Array */
static char *m_login_help[] = {
  "LOGIN <username> :<realname>",
  "",
  "Used by clients to give additional information about",
  "the connecting client.",
  NULL
};

static struct msg m_login_msg = {
  /* Name des Kommandos */
  "LOGIN", 
    
  /* Minimale Argumentzahl, sind weniger Argumente vorhanden wird ein
     Fehler gemeldet und der Message Handler wird nicht aufgerufen */
  2, 
    
  /* Maximale Argumentzahl, sind mehr Argumente vorhanden werden die
     überschüssigen Argumente an das Letzte angehängt
   
     (aus "MSG lala hallo, ich möchte gern ein spiel beginnen" werden 
      3 argumente "MSG", "lala" und "hallo, ich möchte gern ein spiel beginnen"
      wenn die maximale Argumentzahl auf 2 ist)
   */
  3,
    
    MFLG_PLAYER | MFLG_UNREG,
  { m_login,                  /* Wird aufgerufen wenn player->type == PLAYER_UNKNOWN */
    m_registered,             /* Wird aufgerufen wenn player->type == PLAYER_CLIENT */
    m_registered,             /* Wird aufgerufen wenn player->type == PLAYER_MEMBER */
    m_registered },           /* Wird aufgerufen wenn player->type == PLAYER_OPER */
  
  /* Hilfetext */
  m_login_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_login_load(void)
{
  if(msg_register(&m_login_msg) == NULL)
    return -1;

  return 0;
}

void m_login_unload(void)
{
  msg_unregister(&m_login_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "LOGIN"                                                          *
 * argv[1] - username                                                         *
 * <argv[2] - password>                                                       *
 * argv[3] - realname                                                         *
 * -------------------------------------------------------------------------- */
static void m_login(struct player *player, int argc, char **argv)
{  
  /* Checke ob der Name gültig ist */
  if(!chars_valid_nick(argv[1]))
  {
    char *error = "Benutzername enthält ungültige Zeichen";
    
    player_send(player, "%s FAIL :%s", argv[0], error);
    player_exit(player, error);
    
    return;
  }
  
  /* Checke ob jemand den Namen bereits benutzt */
  if(player_find_name(argv[1]))
  {
    char *error = "Benutzername bereits in Benutzung";

    player_send(player, "%s FAIL :%s", argv[0], error);
    player_exit(player, error);
    return;
  }  
  
  /* Checke ob der Name in der Datenbank existriert */
  if(!tichu_db && player_find_sql(player, argv[1]))    
  {
    struct db_result *db_result;    
    
    if(argc != 3)
    {      
      char *error = "Sie haben einen Registrierten Benutzernamen gewählt! Geben sie das Passwort an.";
      
      player_send(player, "%s FAIL :%s", argv[0], error);
      player_exit(player, error);
      return;
    }
    
    db_result = db_query(tichu_db, "SELECT * FROM Benutzer WHERE BeID = '%i' AND BePasswort = MD5('%s')", 
                         player->db_id, argv[2]);
    
    if(!db_num_rows(db_result))
    {      
      char *error = "Falsches Passwort angegeben";
    
      player_send(player, "%s FAIL :%s", argv[0], error);
      player_exit(player, error);
      return;
    }
    
    db_free_result(db_result);
      
  }
  
  /* Alle checks erfolgreich, Benutzer wird eingeloggt... */
  
  /* Namen setzen und hashen */
  player_set_name(player, argv[1]);
  
  /* Benutzerinfo kopieren */
  strlcpy(player->info, argv[2], sizeof(player->info));
  
  /* Typ von Benutzer setzen */
  if(player->db_id)
    player_set_type(player, PLAYER_MEMBER);
  else
    player_set_type(player, PLAYER_USER);

  /* Spieler Willkommen heissen :) */
  player_send(player, "%s OK :Willkommen! Es sind %u Spieler online.",
              argv[0], player_list.size);
  
  /* Public-Channel joinen */
  game_join(tichu_public, player, "");
  
}
