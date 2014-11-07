$/* chaosircd - pi-networks irc server
 *
 * Copyright (C) 2003  Roman Senn <smoli@paranoya.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: conf.h,v 1.2 2004/12/14 19:40:05 slurp Exp $
 */

#ifndef __CONF_H
#define __CONF_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif /* HAVE_STDINT_H */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*#include "connect.h"
#include "listen.h"
#include "module.h"
#include "dlink.h"
#include "mfile.h"
#include "ini.h"
#include "log.h"*/

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*#include "class.h"
#include "ircd.h"
#include "oper.h"*/

/* -------------------------------------------------------------------------- *
 * command line option structure                                              *
 * -------------------------------------------------------------------------- */
struct option {
  const char *name;
  int         has_arg;
  int        *flag;
  int         val;
};

/* -------------------------------------------------------------------------- *
 * section structures                                                         *
 * -------------------------------------------------------------------------- */
struct conf_global {
  uint32_t hash;
  char     name[IRCD_HOSTLEN + 1];
  char     info[IRCD_INFOLEN + 1];
  char     configfile[IRCD_PATHLEN + 1];
  char     pidfile[IRCD_PATHLEN + 1];
  int      nodetach;
  int      hub;
};

struct conf_listen {
  char     password[IRCD_PASSWDLEN + 1];
  char     class[IRCD_CLASSLEN + 1];
};

struct conf_connect {
  char     passwd[IRCD_PASSWDLEN + 1];
  char     class[IRCD_CLASSLEN + 1];
  char     cipher[IRCD_CIPHERLEN + 1];
  char     key[IRCD_PATHLEN + 1];
  int      cryptlink;
  int      ziplink;  
};

/* -------------------------------------------------------------------------- *
 * main config structure                                                      *
 * -------------------------------------------------------------------------- */
struct config {
  struct conf_global global;
  struct list        logs;
  struct list        inis;
  struct list        modules;
  struct list        mfiles;
  struct list        classes;
  struct list        listeners;
  struct list        connects;
  struct list        children;
  struct list        opers;
  struct list        ssl;
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct config conf_current;
extern struct config conf_new;
extern int           conf_fd;
extern int           conf_log;

/* -------------------------------------------------------------------------- *
 * some declarations for flex/bison                                           *
 * -------------------------------------------------------------------------- */
//#define yyerror conf_yy_error
#define yyerror conf_yy_error
#define yyparse conf_yy_parse

/* importing from lexer */
extern int  yylex(void);
extern int  lineno;
extern char linebuf[IRCD_BUFSIZE];

/* exporting to parser */
extern int           yydebug;
extern struct global globalopts;
extern int           conf_fd;
extern void          yyerror(char *);
extern int           yyparse(void);
extern void          yy_fatal_error(const char *);

#undef stdin
#undef stdout
#undef stderr
#define stdin  (void *)conf_fd
#define stdout (void *)0
#define stderr (void *)0

#define isatty(x) 0


/* -------------------------------------------------------------------------- *
 * Getopt and configfile coldstart.                                           *
 * -------------------------------------------------------------------------- */
extern void               conf_init         (int            argc,
                                             char         **argv,
                                             char         **envp);

/* -------------------------------------------------------------------------- *
 * Shutdown config code.                                                      *
 * -------------------------------------------------------------------------- */
extern void               conf_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * Read the config file(s).                                                   *
 * -------------------------------------------------------------------------- */
extern void               conf_read         (void);

/* -------------------------------------------------------------------------- *
 * This is called when a config file is read successfully.                    *
 * -------------------------------------------------------------------------- */
extern void               conf_done         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void               conf_rehash       (void);

/* -------------------------------------------------------------------------- *
 * Dump a config structure.                                                   *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
extern void               conf_dump         (struct config *cfgptr);
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Bison parser error functions.                                              *
 * -------------------------------------------------------------------------- */
extern void conf_yy_error  (char     *msg);
extern int  conf_yy_fatal  (char     *msg);

#endif /* __CONF_H */
