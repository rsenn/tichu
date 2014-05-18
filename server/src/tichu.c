/* chaosircd - pi-networks irc server
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
 * $Id: tichu.c,v 1.52 2005/04/15 07:29:00 slurp Exp $
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
#include <libchaos/listen.h>
#include <libchaos/module.h>
#include <libchaos/dlink.h>
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
#include <tichu/tichu.h>
#include <tichu/card.h>
#include <tichu/msg.h>
#include <tichu/class.h>
#include <tichu/chars.h>
#include <tichu/cnode.h>
#include <tichu/combo.h>
#include <tichu/structs.h>
#include <tichu/player.h>
#include <tichu/game.h>

/* -------------------------------------------------------------------------- *
 * Global variables for the daemon code                                       *
 * -------------------------------------------------------------------------- */
const char  *tichu_package = PACKAGE_NAME;
const char  *tichu_version = PACKAGE_VERSION;
const char  *tichu_release = PACKAGE_RELEASE;
uint64_t     tichu_start;
struct dlog *tichu_drain;
struct sheap tichu_heap;
int          tichu_log;
int          tichu_log_in;
int          tichu_log_out;
int          tichu_argc = 0;
char       **tichu_argv = NULL;
char       **tichu_envp = NULL;
char         tichu_path[PATH_MAX];
struct game *tichu_public;
struct db   *tichu_db;


/* -------------------------------------------------------------------------- *
 * Die if we cannot bind to a port during coldstart.                          *
 * -------------------------------------------------------------------------- */
static void tichu_listen(const char *address, uint16_t port, const char *error)
{
  log(tichu_log, L_fatal, "Could not bind to %s:%u: %s",
      address, (uint32_t)port, error);
  syscall_exit(1);
}

/* -------------------------------------------------------------------------- *
 * Print the uptime of tichu server in a human readable format.               *
 * -------------------------------------------------------------------------- */
const char *tichu_uptime(void)
{
  static char upstr[TICHU_LINELEN - 1];
  uint32_t    msecs;
  uint32_t    secs;
  uint32_t    mins;
  uint32_t    hrs;
  uint32_t    days;
  uint64_t    uptime;
  
  uptime = timer_mtime - tichu_start;
  
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
/*static int tichu_writepid(struct config *config, pid_t pid)
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
/*static void tichu_detach(struct config *config)
{
  pid_t pid;
  
  pid = syscall_fork();
  
  if(pid == -1)
    return;
  
  if(pid == 0)
  {
    log_drain_level(tichu_drain, L_fatal);    
    log_drain_delete(tichu_drain);
    syscall_setsid();
  }
  else
  {
    if(tichu_writepid(config, pid) == -1)
    {
      log(tichu_log, L_status, "*** Could not write PID file!!! ***");
      syscall_kill(pid, SIGTERM);
      syscall_exit(1);
    }
    
    log(tichu_log, L_status, "*** Detached [%u] ***", pid);
    
    syscall_exit(0);
  }
}*/

/* -------------------------------------------------------------------------- *
 * Read the file with the process ID and check if there is already a running  *
 * tichu server...                                                            *
 * -------------------------------------------------------------------------- */
/*static pid_t tichu_check(struct config *config)
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
/*static int tichu_coldstart(struct config *config)
{
  pid_t pid;
  
  log(tichu_log, L_status, "*** Config file coldstart done ***");
  
  if(config->global.name[0] == '\0')
  {
    log(tichu_log, L_fatal, "server has no name!!!");
    syscall_exit(1);
  }
  
  if(config->global.pidfile[0] == '\0')
  {
    log(tichu_log, L_fatal, "server has no PID file!!!");
    syscall_exit(1);
  }
  
  if((pid = tichu_check(config)))
  {
    log(tichu_log, L_fatal, "server already running [%u]", pid);
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
    tichu_detach(config);
  }
  else
  {
    if(tichu_writepid(config, syscall_getpid()))
    {
      log(tichu_log, L_fatal, "*** Could not write PID file!!! ***");
    }
  }
  
  tichu_start = timer_mtime;

  hook_unregister(conf_done, HOOK_DEFAULT, tichu_coldstart);
  
  return 0;
}*/

/* -------------------------------------------------------------------------- *
 * Initialize things.                                                         *
 * -------------------------------------------------------------------------- */
void tichu_init(int argc, char **argv, char **envp)
{
  log(tichu_log, L_startup, "*** Firing up %s v%s - %s ***",
      PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_RELEASE);

  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  io_init_except(STDOUT_FILENO, STDERR_FILENO, STDOUT_FILENO);  
  mem_init();
  str_init();
  timer_init();
  listen_init();
  queue_init();
  dlink_init();
  module_init();
  net_init();
  ini_init();
  cfg_init();
  hook_init();
  db_init();
  
  tichu_log = log_source_register("tichu");
  tichu_log_in = log_source_register("in");
  tichu_log_out = log_source_register("out");
  
  mem_static_create(&tichu_heap, sizeof(struct support), SUPPORT_BLOCK_SIZE);
  mem_static_note(&tichu_heap, "support heap");
  
  log(tichu_log, L_status, "*** Done initialising %s library ***", PACKAGE_NAME);

  msg_init();
  class_init();
  player_init();
  game_init();
  card_init();
  cnode_init();
  structs_init();
  combo_init();
  
  log(tichu_log, L_status, "*** Done initialising %s core ***", PACKAGE_NAME);
  
#if 1
  tichu_drain = log_drain_setfd(1, LOG_ALL, L_debug, 0);
#else
  tichu_drain = log_drain_setfd(1, LOG_ALL, L_status, 0);
#endif /* DEBUG */
  
/*  hook_register(conf_done, HOOK_DEFAULT, tichu_coldstart);*/
  hook_register(listen_add, HOOK_DEFAULT, tichu_listen);
  
/*  conf_init(argc, argv, envp);*/
  
}

/* -------------------------------------------------------------------------- *
 * Loop around some timer stuff and the i/o multiplexer.                      *
 * -------------------------------------------------------------------------- */
void tichu_loop(void)
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
/*    tichu_collect();*/
  }
}

/* -------------------------------------------------------------------------- *
 * Dump status information.                                                   *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void tichu_dump(void)
{
  debug(tichu_log, "--- tichu complete dump ---");
  
/*  conf_dump(&conf_current);*/

/*  connect_dump(NULL);*/
  listen_dump(NULL);
  log_source_dump(-1);
  log_drain_dump(NULL);
  timer_dump(NULL);
/*  queue_dump(NULL);*/
  dlink_dump();
  module_dump(NULL);
  net_dump(NULL);
/*  ini_dump(NULL);*/

  debug(tichu_log, "--- end of tichu complete dump ---");
}
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Garbage collect.                                                           *
 * -------------------------------------------------------------------------- */
void tichu_collect(void)
{
/*  child_collect();*/
/*  connect_collect();*/
  listen_collect();
  log_collect();
  dlink_collect();
  timer_collect();
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int tichu_restart(void)
{
  pid_t pid;
  int status;

#ifdef HAVE_SOCKET_FILTER  
/*  filter_shutdown();*/
#endif /* HAVE_SOCKET_FILTER */
  listen_shutdown();
/*  child_shutdown();*/
  
/*  syscall_unlink(conf_current.global.pidfile);*/
  
  pid = fork();
  
  if(pid)
  {
    log(tichu_log, L_status, "new child status: %i", waitpid(pid, &status, WNOHANG));
  }
  else
  {
    syscall_execve(tichu_path, tichu_argv, tichu_envp);
    
    log(tichu_log, L_status, "Failed executing myself (%s)!", tichu_path);
    tichu_shutdown();
  }
  
  log(tichu_log, L_status, "Restart succeeded.");
  
  tichu_shutdown();
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Clean things up.                                                           *
 * -------------------------------------------------------------------------- */
void tichu_shutdown(void)
{
  log(tichu_log, L_status, "*** Shutting down %s ***", PACKAGE_NAME);

/*  syscall_unlink(conf_current.global.pidfile);*/
  
/*  if(!conf_new.global.nodetach)
    log_drain_delete(tichu_drain);*/
  
  module_shutdown();
  
  cnode_shutdown();
  card_shutdown();
  player_shutdown();
  class_shutdown();
  msg_shutdown();
  structs_shutdown();
  combo_shutdown();
  
  mem_static_destroy(&tichu_heap);

  db_shutdown();
  listen_shutdown();
  hook_shutdown();
  cfg_shutdown();
  ini_shutdown();
  net_shutdown();
  io_shutdown();
  dlink_shutdown();
  queue_shutdown();
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
  
  tichu_argc = argc;
  tichu_argv = argv;
  tichu_envp = envp;
  
  /* Change to working directory */
  syscall_chdir(PREFIX);
  
  /* Get argv0 */
  snprintf(link, sizeof(link), "/proc/%u/exe", syscall_getpid());
  
  if((n = syscall_readlink(link, tichu_path, sizeof(tichu_path) - 1)) > -1)
  {
    tichu_path[n] = '\0';
  }
  else
  {
    tichu_path[0] = '\0';
  }
  
  /* Catch some signals */
  syscall_signal(SIGINT, (void *)tichu_shutdown);
  syscall_signal(SIGHUP, (void *)tichu_shutdown);
  syscall_signal(SIGTERM, (void *)tichu_shutdown);
  syscall_signal(SIGPIPE, (void *)1);
  
  /* Always dump core! */
  syscall_setrlimit(RLIMIT_CORE, &core);
  
  /* Initialise all modules */
  tichu_init(argc, argv, envp);
  
  /* Temporary hardcoded config */
  listen_add("0.0.0.0", 2222, 20, 0, NULL, "tichu");
  
  module_setpath(PLUGINDIR);
  
  module_add("m_login");
  module_add("m_logout");
  module_add("m_msg");
  module_add("m_create");
  module_add("m_dump");
  module_add("m_join");
  module_add("m_leave");
  module_add("m_accept");
  module_add("m_start");
  module_add("m_players");
  module_add("m_games");
  module_add("m_help");
  module_add("m_schupfe");
  module_add("m_cards");
  module_add("m_order");
  module_add("m_tichu");
  module_add("m_playcards");
  module_add("m_schupfe");
  module_add("m_abandon");
  module_add("m_team");
  module_add("m_kick");
  module_add("m_ban");
  module_add("m_color");
  module_dump(NULL);
  
  /* create public game */
  tichu_public = game_new("@");

  /* create db connection */
  if((tichu_db = db_new(DB_TYPE_MYSQL)))
  {
    if(db_connect(tichu_db, "www.tichu.ch", "tichu", "moekel", "raffelti_tichu"))
      log(tichu_log, L_fatal, "*** Could not connect to de database!!! ***");
  }
  
  /* Handle events */
  tichu_loop();
  
  /* Shutdown all modules */
  tichu_shutdown();  
  
  return 0;
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
