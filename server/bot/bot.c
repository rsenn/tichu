/*
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
 * $Id: bot.c,v 1.12 2005/04/25 04:25:17 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/syscall.h>
#include <libchaos/filter.h>
#include <libchaos/connect.h>
#include <libchaos/module.h>
#include <libchaos/dlink.h>
#include <libchaos/child.h>
#include <libchaos/sauth.h>
#include <libchaos/queue.h>
#include <libchaos/timer.h>
#include <libchaos/hook.h>
#include <libchaos/ini.h>
#include <libchaos/cfg.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>
#include <libchaos/io.h>
#include <libchaos/db.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <tichu/config.h>

#include "bot.h"
#include "game.h"
#include "reply.h"
#include "player.h"
#include "card.h"

/* -------------------------------------------------------------------------- *
 * Global variables for the daemon code                                       *
 * -------------------------------------------------------------------------- */
const char      *bot_package = PACKAGE_NAME;
const char      *bot_version = PACKAGE_VERSION;
const char      *bot_release = PACKAGE_RELEASE;
uint64_t         bot_start;
struct dlog     *bot_drain;
struct sheap     bot_heap;
struct ini      *bot_ini;
struct child    *bot_child;
struct timer    *bot_game_timer;
int              bot_checkinterval = 20000;
int              bot_log;
int              bot_log_in;
int              bot_log_out;
int              bot_argc = 0;
char           **bot_argv = NULL;
char           **bot_envp = NULL;
char             bot_path[PATH_MAX];
char             bot_user[33];
char             bot_password[33];
char             bot_info[512];
char             bot_host[64];
char            *bot_names[64];
char             bot_color[7];
struct game     *bot_game;
struct player   *bot_player;
int              bot_port;
struct protocol *bot_protocol;
int              bot_connected = 0;
struct connect  *bot_connection;
int              bot_fd = -1;
const char      *bot_paths[] =
{
  ".",
  "..",
  "./data",
  "../data",
  "./server/data",
  "./tichu/server/data",
#ifdef SYSCONFDIR
  SYSCONFDIR,
  SYSCONFDIR"/"PFX,
#endif
#ifdef DATADIR
  DATADIR,
  DATADIR"/"PFX,
#endif
  NULL
};

/* -------------------------------------------------------------------------- *
 * Die if we cannot bind to a port during coldstart.                          *
 * -------------------------------------------------------------------------- */
/*static void bot_listen(const char *address, uint16_t port, const char *error)
{
  log(bot_log, L_fatal, "Could not bind to %s:%u: %s",
      address, (uint32_t)port, error);
  syscall_exit(1);
}*/

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *bot_getname(void)
{
  int n;
  
  for(n = 0; bot_names[n]; n++);
  
  return bot_names[rand() % n];
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int bot_disconnect(void)
{
  timer_cancel(&bot_game_timer);
  
  player_shutdown();
  game_shutdown();
  game_init();
  player_init();

  connect_cancel(bot_connection);
  connect_start(bot_connection);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_mkcolor(char *color, const char *name)
{
  uint32_t hash;
  static const char hexalpha[] = "0123456789abcdef";
  
  hash = strhash(name);
  
  color[0] = hexalpha[(hash & 0xf)];
  color[1] = hexalpha[(hash & 0xf0) >> 4];
  color[2] = hexalpha[(hash & 0xf00) >> 8];
  color[3] = hexalpha[(hash & 0xf000) >> 12];
  color[4] = hexalpha[(hash & 0xf0000) >> 16];
  color[5] = hexalpha[(hash & 0xf00000) >> 20];
  color[6] = '\0';
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int bot_checkgames(void)
{
  log(bot_log, L_status, "requesting game list");
  
  bot_send("GAMES");
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_connect(int fd, struct connect *cptr)
{
  if(cptr->status == CONNECT_DONE)
  {
    /* We must have a valid fd */
    if(!io_valid(fd))
    {
      log(bot_log, L_warning,
          "Invalid filedescriptor while connecting...");
      
      return;
    }
    
    bot_connected = 1;
    
    bot_fd = fd;
    
    /* Register it for the readable callback */
    io_register(fd, IO_CB_WRITE, NULL);
    io_register(fd, IO_CB_READ, bot_recv);
//    io_register(fd, IO_CB_ERROR, bot_recv);
    
    io_queue_control(fd, ON, ON, ON);
    
    bot_login();
  }
  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int bot_open(const char *file, int mode)
{
  int   i;
  int ret;
  char  path[PATH_MAX];
  
  for(i = 0; bot_paths[i]; i++)
  {
    size_t n;
     
    strcpy(path, bot_paths[i]);
    
    n = strlen(path);
    
    if(n && path[n - 1] != '/')
      strcat(path, "/");
    
    strcat(path, file);
    
    if((ret = io_open(path, mode)) > -1)
      return ret;
  }
  
  log(bot_log, L_fatal, "cannot open file %s", file);
  exit(1);
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_confcb(struct ini *ini)
{
  struct ini_section *isptr;
  struct ini_key *ikptr;
  struct node *node;
  int i;
  
//  ini_load(bot_ini);
  
  ini_dump(bot_ini);
  
  if(!(isptr = ini_section_find(bot_ini, "server")))
  {
    log(bot_log, L_fatal, "%s needs section [server]", bot_ini->name);
    exit(1);
  }
  
  ini_get_str(isptr, "host", bot_host, sizeof(bot_host));
  ini_read_int(isptr, "port", &bot_port);

  bot_connection = connect_add(bot_host, bot_port, bot_protocol, 
                               30000L, 10000L, 0, 0, NULL);
  
  if(!(isptr = ini_section_find(bot_ini, "names")))
  {
    log(bot_log, L_fatal, "%s needs section [names]", bot_ini->name);
    exit(1);
  }
  
  i = 0;
  
  dlink_foreach_data(&isptr->keys, node, ikptr)
  {
    if(!ikptr->value)
      continue;
    
    if(i < 63)
    {
      bot_names[i++] = strdup(ikptr->value);
    }
    else
    {
      bot_names[i] = NULL;
      break;
    }
    
  }

  ini_clear(bot_ini);
  ini_close(bot_ini);
  
  connect_start(bot_connection);  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_loadconf(void)
{
  int fd;
  bot_ini = ini_add("bot.ini");
  
  fd = bot_open(bot_ini->name, O_RDONLY);
  
  ini_open_fd(bot_ini, fd);
  
  ini_callback(bot_ini, bot_confcb);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_login(void)
{
  char *botname;
  
  botname = bot_getname();
  strlcpy(bot_user, botname, sizeof(bot_user));
  
  bot_mkcolor(bot_color, bot_user);
  
  log(bot_log, L_status, "logging in as %s...", bot_user);
  
  bot_send("LOGIN %s %s :%s", bot_user, bot_password, bot_info);
  bot_send("COLOR #%s", bot_color);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_vsend(const char *format, va_list args)
{
  char buf[4096];
  size_t n;
  
  n = vsnprintf(buf, sizeof(buf) - 3, format, args);
  
  log(bot_log, L_status, "--> SERVER: %s", buf);
  
  buf[n++] = '\r';
  buf[n++] = '\n';
  
  io_write(bot_fd, buf, n);
}  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_send(const char *format, ...)
{
  va_list args;
  
  va_start(args, format);
  
  bot_vsend(format, args);
  
  va_end(args);
}  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void bot_recv(int fd)
{
  int n;
  char buf[4096];

  if(io_list[fd].recvq.lines)
  {
    while((n = io_gets(fd, buf, sizeof(buf))) > 0)
      reply_parse(buf);
  }
  
  log(bot_log, L_status, "bot recv event: %i", io_list[fd].status.err);
  
  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    if(io_list[fd].error <= 0)
    {
      log(bot_log, L_warning, "server closed connection");
    }
    else 
    {
      log(bot_log, L_warning, "i/o error: %s", 
          syscall_strerror(io_list[fd].error));
    }
    
    bot_disconnect();
  }
}  

/* -------------------------------------------------------------------------- *
 * Print the uptime of tichu bot in a human readable format.                  *
 * -------------------------------------------------------------------------- */
const char *bot_uptime(void)
{
  static char upstr[BOT_LINELEN - 1];
  uint32_t    msecs;
  uint32_t    secs;
  uint32_t    mins;
  uint32_t    hrs;
  uint32_t    days;
  uint64_t    uptime;
  
  uptime = timer_mtime - bot_start;
  
  msecs = (uint32_t)(uptime  % 1000L);
  secs = ((uint32_t)(uptime /= 1000L) % 60);
  mins = ((uint32_t)(uptime /= 60L)   % 60);
  hrs  = ((uint32_t)(uptime /= 60L)   % 24);
  days =  (uint32_t)(uptime / 24L);
  
  if(days == 0)
  {
    if(hrs == 0)
    {
      if(mins == 0)
        snprintf(upstr, sizeof(upstr), "%u seconds, %u msecs", secs, msecs);
      else
        snprintf(upstr, sizeof(upstr), "%u minutes, %u seconds", mins, secs);
    }
    else
    {
      snprintf(upstr, sizeof(upstr), "%u hours, %u minutes", hrs, mins);
    }
  }
  else
  {
    snprintf(upstr, sizeof(upstr), "%u days, %u hours", days, hrs);
  }
  
  return upstr;
}

/* -------------------------------------------------------------------------- *
 * Write the process ID to a file.                                            *
 * -------------------------------------------------------------------------- */
/*static int bot_writepid(struct config *config, pid_t pid)
{
  int fd;
  
  fd = io_open(config->global.pidfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  
  if(fd == -1)
    return -1;
  
  io_queue_control(fd, OFF, OFF, OFF);
  io_puts(fd, "%u", pid);
  io_close(fd);
  
  return 0;
}*/

/* -------------------------------------------------------------------------- *
 * Create a detached child and exit the parent (going to background)          *
 * -------------------------------------------------------------------------- */
/*static void bot_detach(struct config *config)
{
  pid_t pid;
  
  pid = syscall_fork();
  
  if(pid == -1)
    return;
  
  if(pid == 0)
  {
    log_drain_level(bot_drain, L_fatal);    
    log_drain_delete(bot_drain);
    syscall_setsid();
  }
  else
  {
    if(bot_writepid(config, pid) == -1)
    {
      log(bot_log, L_status, "*** Could not write PID file!!! ***");
      syscall_kill(pid, SIGTERM);
      syscall_exit(1);
    }
    
    log(bot_log, L_status, "*** Detached [%u] ***", pid);
    
    syscall_exit(0);
  }
}*/

/* -------------------------------------------------------------------------- *
 * Read the file with the process ID and check if there is already a running  *
 * tichu bot...                                                               *
 * -------------------------------------------------------------------------- */
/*static pid_t bot_check(struct config *config)
{
  struct stat st;
  pid_t       pid;
  char        proc[32];
  char        buf[16];
  int         fd;
  
  fd = io_open(config->global.pidfile, O_RDONLY);
  
  if(fd == -1)
    return 0;
  
  io_queue_control(fd, OFF, OFF, OFF);
  
  if(io_read(fd, buf, sizeof(buf)) > 0)
  {
    pid = strtoul(buf, NULL, 10);
    
    snprintf(proc, sizeof(proc), "/proc/%u", pid);
    
    if(syscall_stat(proc, &st) == 0)
      return pid;
  }
  
  return 0;
}*/

/* -------------------------------------------------------------------------- *
 * After coldstart we check config file sanity and then we initialize the     *
 * client instance referring to ourselves...                                  *
 * -------------------------------------------------------------------------- */
/*static int bot_coldstart(struct config *config)
{
  pid_t pid;
  
  log(bot_log, L_status, "*** Config file coldstart done ***");
  
  if(config->global.name[0] == '\0')
  {
    log(bot_log, L_fatal, "bot has no name!!!");
    syscall_exit(1);
  }
  
  if(config->global.pidfile[0] == '\0')
  {
    log(bot_log, L_fatal, "bot has no PID file!!!");
    syscall_exit(1);
  }
  
  if((pid = bot_check(config)))
  {
    log(bot_log, L_fatal, "bot already running [%u]", pid);
    syscall_exit(1);
  }
  
  strlcpy(server_me->name, config->global.name, sizeof(server_me->name));
          
  client_set_name(client_me, config->global.name);
  lclient_set_name(lclient_me, config->global.name);
  server_set_name(server_me, config->global.name);
  
  strlcpy(client_me->info, config->global.info, sizeof(client_me->info));
  strlcpy(lclient_me->info, config->global.info, sizeof(lclient_me->info));

  if(!config->global.nodetach)
  {
    bot_detach(config);
  }
  else
  {
    if(bot_writepid(config, syscall_getpid()))
    {
      log(bot_log, L_fatal, "*** Could not write PID file!!! ***");
    }
  }
  
  bot_start = timer_mtime;

  hook_unregister(conf_done, HOOK_DEFAULT, bot_coldstart);
  
  return 0;
}*/

/* -------------------------------------------------------------------------- *
 * Initialize things.                                                         *
 * -------------------------------------------------------------------------- */
void bot_init(int argc, char **argv, char **envp)
{
  log(bot_log, L_startup, "*** Firing up tichu-bot v%s - %s ***",
      PACKAGE_VERSION, PACKAGE_RELEASE);

  log_init(STDOUT_FILENO, LOG_ALL, L_debug);
  io_init_except(STDIN_FILENO, STDERR_FILENO, STDOUT_FILENO);  
  mem_init();
  str_init();
  timer_init();
  connect_init();
  queue_init();
  child_init();
  sauth_init();
  dlink_init();
  module_init();
  net_init();
  ini_init();
  cfg_init();
  hook_init();
  db_init();
  
  bot_child = child_new(LIBEXECDIR"/servauth", 1, "%r0 %w0", 20000L, 0);
  child_set_name(bot_child, "-sauth");

  sauth_launch();
  
  bot_protocol = net_register(NET_CLIENT, "tichu", bot_connect);
  
  bot_log = log_source_register("tichu");
  bot_log_in = log_source_register("in");
  bot_log_out = log_source_register("out");

  bot_game = NULL;
  bot_user[0] = '\0';
  
  reply_init();
  game_init();
  player_init();
  card_init();
  
  child_dump(bot_child);
  
  log(bot_log, L_status, "*** Done initialising tichu-bot ***", PACKAGE_NAME);

#if 1
  bot_drain = log_drain_setfd(1, LOG_ALL, L_debug, 0);
#else
  bot_drain = log_drain_setfd(1, LOG_ALL, L_status, 0);
#endif /* DEBUG */
}

/* -------------------------------------------------------------------------- *
 * Loop around some timer stuff and the i/o multiplexer.                      *
 * -------------------------------------------------------------------------- */
void bot_loop(void)
{
  int ret = 0;
  int64_t *timeout;
  int64_t remain = 0LL;

  while(ret >= 0)
  {
    /* Calculate timeout value */
    timeout = timer_timeout();

    /* Do I/O multiplexing and event handling */
#ifdef USE_POLL
    ret = io_poll(&remain, timeout);
#else
    ret = io_select(&remain, timeout);
#endif /* USE_SELECT | USE_POLL */
    
    /* Remaining time is 0msecs, we need to run a timer */
    if(remain == 0LL)
      timer_run();
    
    if(timeout)
      timer_drift(*timeout - remain);
    
    io_handle();
    
    timer_collect();
/*    bot_collect();*/
  }
}

/* -------------------------------------------------------------------------- *
 * Dump status information.                                                   *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void bot_dump(void)
{
  debug(bot_log, "--- tichu-bot complete dump ---");
  
/*  conf_dump(&conf_current);*/

  connect_dump(NULL);
  /*listen_dump(NULL);*/
  log_source_dump(-1);
  log_drain_dump(NULL);
  timer_dump(NULL);
/*  queue_dump(NULL);*/
  dlink_dump();
  module_dump(NULL);
  net_dump(NULL);
/*  ini_dump(NULL);*/

  debug(bot_log, "--- end of tichu-bot complete dump ---");
}
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Garbage collect.                                                           *
 * -------------------------------------------------------------------------- */
void bot_collect(void)
{
/*  child_collect();*/
  connect_collect();
/*  listen_collect();*/
  log_collect();
  dlink_collect();
  timer_collect();
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int bot_restart(void)
{
  pid_t pid;
  int status;

#ifdef HAVE_SOCKET_FILTER  
/*  filter_shutdown();*/
#endif /* HAVE_SOCKET_FILTER */
  connect_shutdown();
/*  child_shutdown();*/
  
/*  syscall_unlink(conf_current.global.pidfile);*/
  
  pid = fork();
  
  if(pid)
  {
    log(bot_log, L_status, "new child status: %i", waitpid(pid, &status, WNOHANG));
  }
  else
  {
    syscall_execve(bot_path, bot_argv, bot_envp);
    
    log(bot_log, L_status, "Failed executing myself (%s)!", bot_path);
    bot_shutdown();
  }
  
  log(bot_log, L_status, "Restart succeeded.");
  
  bot_shutdown();
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Clean things up.                                                           *
 * -------------------------------------------------------------------------- */
void bot_shutdown(void)
{
  log(bot_log, L_status, "*** Shutting down %s ***", PACKAGE_NAME);

/*  syscall_unlink(conf_current.global.pidfile);*/
  
/*  if(!conf_new.global.nodetach)
    log_drain_delete(bot_drain);*/
  
  module_shutdown();

  card_shutdown();
  player_shutdown();
  game_shutdown();
  reply_shutdown();
  
  db_shutdown();
  connect_shutdown();
  hook_shutdown();
  cfg_shutdown();
  ini_shutdown();
  net_shutdown();
  io_shutdown();
  dlink_shutdown();
  queue_shutdown();
  sauth_shutdown();
  child_shutdown();
  log_shutdown();
  timer_shutdown();
  str_shutdown();
  mem_shutdown();
  
  syscall_exit(0);
}

/* -------------------------------------------------------------------------- *
 * Program entry.                                                             *
 * -------------------------------------------------------------------------- */
int main(int argc, char **argv, char **envp) 
{
  struct rlimit core = { RLIM_INFINITY, RLIM_INFINITY };
  char          link[64];
  int           n;
  
  bot_argc = argc;
  bot_argv = argv;
  bot_envp = envp;
  
  /* Change to working directory */
  syscall_chdir(PREFIX);
  
  /* Get argv0 */
  snprintf(link, sizeof(link), "/proc/%u/exe", syscall_getpid());
  
  if((n = syscall_readlink(link, bot_path, sizeof(bot_path) - 1)) > -1)
  {
    bot_path[n] = '\0';
  }
  else
  {
    bot_path[0] = '\0';
  }
  
  /* Catch some signals */
  syscall_signal(SIGINT, (void *)bot_shutdown);
  syscall_signal(SIGHUP, (void *)bot_shutdown);
  syscall_signal(SIGTERM, (void *)bot_shutdown);
  syscall_signal(SIGPIPE, (void *)1);
  
  /* Always dump core! */
  syscall_setrlimit(RLIMIT_CORE, &core);
  
  /* Initialise all modules */
  bot_init(argc, argv, envp);

  srand(timer_mtime);
  
  bot_loadconf();
  
  /* Handle events */
  bot_loop();
  
  /* Shutdown all modules */
  bot_shutdown();  
  
  return 0;
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
