#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/connect.h>
#include <libchaos/listen.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>
#include <libchaos/sauth.h>
#include <libchaos/hook.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/io.h>
#include <libchaos/db.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/tichu.h>
#include <tichu/player.h>
#include <tichu/combo.h>
#include <tichu/structs.h>
#include <tichu/msg.h>
#include <tichu/game.h>
#include <tichu/chars.h>
#include <tichu/class.h>
#include <tichu/cnode.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int             player_log;                /* player log source */
struct sheap    player_heap;               /* heap for struct player */
struct timer   *player_timer;              /* timer for heap gc */
uint32_t        player_serial;
uint32_t        player_max;
struct list     player_list;               /* list with all of them */
struct list     player_lists[4];           /* unreg, clients, servers, opers */
uint64_t        player_seed;
unsigned long   player_recvb;
unsigned long   player_sendb;
unsigned long   player_recvm;
unsigned long   player_sendm;
char           *player_types[] = {
  "unregistered",
  "client",
  "server",
  "oper"
};  
char            player_recvbuf[TICHU_BUFSIZE];

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void player_shift(int64_t delta)
{
  struct player *player;
  struct node    *nptr;
  
  dlink_foreach_data(&player_list, nptr, player)
  {
    if(player->ping)
      player->ping += delta;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*static uint32_t player_count(struct in_addr *ip)
{
  struct player *player;
  struct node    *nptr;
  uint32_t        ret = 0;
  
  dlink_foreach_data(&player_lists[PLAYER_UNKNOWN], nptr, player)
  {
    if(ip->s_addr == player->addr_remote.s_addr)
      ret++;
  }
  
  dlink_foreach_data(&player_lists[PLAYER_USER], nptr, player)
  {
    if(ip->s_addr == player->addr_remote.s_addr)
      ret++;
  }
  
  return ret;
}*/

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_clear(struct player *player) 
{
  struct node *node;
  struct node *next;
  struct cnode *cnode;
  
  dlink_foreach_safe_data(&player->cards, node, next, cnode) 
    cnode_delete(cnode);
  
  dlink_list_zero(&player->cards);
} 

/* -------------------------------------------------------------------------- *
 * Initialize player module                                                  *
 * -------------------------------------------------------------------------- */
void player_init(void)
{  
  player_log = log_source_register("player");
  
  /* Zero all player lists */
  dlink_list_zero(&player_list);
  dlink_list_zero(&player_lists[PLAYER_UNKNOWN]);
  dlink_list_zero(&player_lists[PLAYER_USER]);
  dlink_list_zero(&player_lists[PLAYER_MEMBER]);
  dlink_list_zero(&player_lists[PLAYER_OPER]);
  
  /* Setup player heap & timer */
  mem_static_create(&player_heap, sizeof(struct player), PLAYER_BLOCK_SIZE);
  mem_static_note(&player_heap, "player heap");
  
  /* Register tichu protocol handlers */
  net_register(NET_SERVER, "tichu", player_accept);
/*  net_register(NET_CLIENT, "irc", player_connect);*/
  
  player_serial   = 0;

  /* Add myself as local client */
  player_recvb = 0;
  player_sendb = 0;
  player_recvm = 0;
  player_sendm = 0;
  
  timer_shift_register(player_shift);
  
  log(player_log, L_status, "Initialised [player] module.");
}

/* -------------------------------------------------------------------------- *
 * Shutdown player module                                                    *
 * -------------------------------------------------------------------------- */
void player_shutdown(void)
{
  struct player *player;
  struct player *next;
  
  log(player_log, L_status, "Shutting down [player] module.");
  
  timer_shift_unregister(player_shift);
  
  /* Push all players */
  dlink_foreach_safe(&player_list, player, next)
  {
    if(player->refcount)
      player->refcount--;
    
    player_delete(player);
  }
  
  /* Destroy static heap */
  mem_static_destroy(&player_heap);
  
  /* Unregister log source */
  log_source_unregister(player_log);
}

/* -------------------------------------------------------------------------- *
 * Garbage collect player blocks                                             *
 * -------------------------------------------------------------------------- */
void player_collect(void)
{
  mem_static_collect(&player_heap);
}

/* -------------------------------------------------------------------------- *
 * Create a new player block                                                 *
 * -------------------------------------------------------------------------- */
struct player *player_new(int fd)
{
  struct player *player = NULL;
  
  /* Allocate and zero player block */
  player = mem_static_alloc(&player_heap);

  memset(player, 0, sizeof(struct player));
  
  /* Link it to the main list and to the appropriate typed list */
  dlink_add_tail(&player_list, &player->node, player);
  dlink_add_tail(&player_lists[player->type], &player->tnode, player);
  
  /* Initialise the block */
  player->refcount = 1;
  player->type = PLAYER_UNKNOWN;
  player->fds[0] = fd;
  player->fds[1] = fd;
  player->serial = player_serial++;
  player->shut = 0;
  strcpy(player->name, "<new client>");
  player->password = NULL;
  player->accepted = 0;
  player->finished = 0;
  player->ping = 0LLU;
  player->plugdata[PLAYER_PLUGDATA_COOKIE] = NULL;
    
  player->addr_local = io_list[fd].a_local.sin_addr;
  player->addr_remote = io_list[fd].a_remote.sin_addr;
  player->port_local = net_ntohs(io_list[fd].a_local.sin_port);
  player->port_remote = net_ntohs(io_list[fd].a_remote.sin_port);
  
  net_ntoa_r(player->addr_remote, player->hostip);
  strcpy(player->host, player->hostip);
  
  dlink_list_zero(&player->cards);
  
  /* Inform about the new player */
  log(player_log, L_verbose, "New local client %s:%u on %s:%u",
      player->host, player->port_remote,
      net_ntoa(player->addr_local), player->port_local);
  
  if(player_list.size > player_max)
    player_max = player_list.size;
  
  return player;
}
  
/* -------------------------------------------------------------------------- *
 * Delete a player block                                                     *
 * -------------------------------------------------------------------------- */
void player_delete(struct player *player)
{
  player_release(player);
  
  /* Unlink from main list and typed list */
  dlink_delete(&player_list, &player->node);
  dlink_delete(&player_lists[player->type], &player->tnode);
  
  debug(player_log, "Deleted player block: %s:%u",
        net_ntoa(player->addr_remote), player->port_remote);
  
  /* Free the block */
  mem_static_free(&player_heap, player);
}  
  
/* -------------------------------------------------------------------------- *
 * Loose all references of an player block                                   *
 * -------------------------------------------------------------------------- */
void player_release(struct player *player)
{
  hooks_call(player_release, HOOK_DEFAULT, player);
  
  if(player->fds[1] >= 0 && player->fds[1] < MAX_FDS)
  {
    io_unregister(player->fds[1], IO_CB_READ);
    io_unregister(player->fds[1], IO_CB_WRITE);
    io_close(player->fds[1]);
  }
  
  if(player->fds[0] >= 0 && player->fds[0] < MAX_FDS)
  {
    io_unregister(player->fds[0], IO_CB_READ);
    io_unregister(player->fds[0], IO_CB_WRITE);
    io_close(player->fds[0]);
  }

  player->fds[0] = -1;
  player->fds[1] = -1;

  class_push(&player->class);
  
  player->listen = NULL;
  timer_cancel(&player->ptimer);
  
  player->refcount = 0;
  player->shut = 1;
}

/* -------------------------------------------------------------------------- *
 * Get a reference to an player block                                        *
 * -------------------------------------------------------------------------- */
struct player *player_pop(struct player *player)
{
  if(player)
  {
    if(!player->refcount)
      debug(player_log, "Poping deprecated player %s:%u",
            net_ntoa(player->addr_remote), player->port_remote);

    player->refcount++;
  }
  
  return player;
}

/* -------------------------------------------------------------------------- *
 * Push back a reference to an player block                                  *
 * -------------------------------------------------------------------------- */
struct player *player_push(struct player **playerptr)
{
  if(*playerptr)
  {
    if((*playerptr)->refcount)
    {
      if(--(*playerptr)->refcount == 0)
        player_release(*playerptr);
    }
      
    (*playerptr) = NULL;
  }
  
  return *playerptr;
}
  
/* -------------------------------------------------------------------------- *
 * Set the type of an player and move it to the appropriate list             *
 * -------------------------------------------------------------------------- */
void player_set_type(struct player *player, uint32_t type)
{
  uint32_t newtype = (type & 0x03);
  
  /* If the type changes, then move to another list */
  if(newtype != player->type)
  {
    /* Delete from old list */
    dlink_delete(&player_lists[player->type & 0x03], &player->tnode);
    
    /* Move to new list */
    dlink_add_tail(&player_lists[newtype], &player->tnode, player);
    
    /* Set type */
    player->type = newtype;
  }  
}  

/* -------------------------------------------------------------------------- *
 * Set the name of an player block                                           *
 * -------------------------------------------------------------------------- */
void player_set_name(struct player *player, const char *name)
{
  /* clear old name completely */
  memset(player->name, 0, sizeof(player->name));
  
  /* Update player->name/hash */ 
  strlcpy(player->name, name, sizeof(player->name));
  
  player->hash = strihash(player->name);
  
  debug(player_log, "Set name for %s:%u to %s",
        net_ntoa(player->addr_remote), player->port_remote, player->name);
  
  io_note(player->fds[0], "local client: %s", player->name);
}

/* -------------------------------------------------------------------------- *
 * Accept a local client                                                      *
 *                                                                            *
 * <fd>                     - filedescriptor of the new connection            *
 *                            (may be invalid?)                               *
 * <listen>                 - the corresponding listen{} block                *
 * -------------------------------------------------------------------------- */
void player_accept(int fd, struct listen *listen)
{
  struct player     *player;
/*  struct conf_listen *lconf;*/
/*  struct class       *clptr;*/
  
  /* We're accepting a connection */
  if(listen->status == LISTEN_CONNECTION)
  {
    /* We must have a valid fd */
    if(fd < 0 || fd >= MAX_FDS)
    {
      log(player_log, L_warning, 
          "Invalid filedescriptor while accepting local client (%i:%i)", fd, io_list[fd].type);
      
      return;
    }
      
    /* Add a local client to player_UNKNOWN list */
    player = player_new(fd);
  
    /* Get references */
    player->listen = listen_pop(listen);
/*    player->class = class_pop(clptr);*/
    player->addr_remote = listen->addr_remote;
    player->port_remote = listen->port_remote;
    
    /* Setup ping timeout callback */
/*    player->ptimer = timer_start(player_exit, clptr->ping_freq, player,
                                "timeout: %llumsecs", clptr->ping_freq);*/

    /* Set queue behaviour */
    io_queue_control(fd, ON, ON, OFF);
    
    /* Register it for the readable callback */
    io_register(fd, IO_CB_READ, player_recv, player);
    
    io_note(player->fds[0], "unknown client from %s:%u", 
            net_ntoa(player->addr_remote),
            (uint32_t)player->port_remote);
  }
  /* We failed accepting the connection */
  else
  {
    /* Warn about the pity */
    log(player_log, L_warning, "Error accepting local client: %s",
        syscall_strerror(io_list[fd].error));
    
    /* We had a valid fd, shut it! */
    io_push(&fd);
  }
}  
  
/* -------------------------------------------------------------------------- *
 * Read data from a local connection and process it                           *
 * -------------------------------------------------------------------------- */
void player_recv(int fd, struct player *player)
{
  if(io_list[fd].status.dead)
    return;
  
  /* Check if the socket was closed */
  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    if(io_list[fd].error <= 0)
    {
      player_exit(player, "(%s) connection closed", player->name);
    }
    else if(io_list[fd].error > 0 && io_list[fd].error < 125)
    {
      player_exit(player, "%s", syscall_strerror(io_list[fd].error));
    }
#ifdef HAVE_SSL    
    else if(io_list[fd].error == 666)
    {
      player_exit(player, "%s", ssl_strerror(fd));
    }
#endif    
    else
    {
      player_exit(player, "unknown error: %i", io_list[fd].error);
    }
    
    return;
  }
  
  /* Check if we exceeded receive queue size */
/*  if(io_list[fd].recvq.size > player->class->recvq)
  {
    player_exit(player, "recvq exceeded (%u bytes)",
                 io_list[fd].recvq.size);
    return;
  }*/

  /* Flood hook */
  hooks_call(player_recv, HOOK_DEFAULT, fd, player);
  
  /* Get lines from the queue as long as there are */
  if(/*io_list[fd].recvq.lines && */!player->shut)
    player_process(fd, player);
}

/* -------------------------------------------------------------------------- *
 * Read a line from queue and process it                                      *
 * -------------------------------------------------------------------------- */
void player_process(int fd, struct player *player)
{
  int n = 0;  
  char *p;
  
  while((n = io_gets(fd, player_recvbuf, TICHU_BUFSIZE)) > 0)
  {
    player_update_recvb(player, n);
    
    if(n < TICHU_BUFSIZE)
      player_recvbuf[n] = '\0';
    else
      player_recvbuf[TICHU_BUFSIZE - 1] = '\0';
    
    if((p = strchr(player_recvbuf, '\r')))
      *p = '\0';
    
    if((p = strchr(player_recvbuf, '\n')))
      *p = '\0';
    
    player_parse(player, player_recvbuf, n);
  } 
}
  
/* -------------------------------------------------------------------------- *
 * Parse the prefix and the command                                           *
 * -------------------------------------------------------------------------- */
void player_parse(struct player *player, char *s, size_t n)
{
  char *argv[256];
 
  log(player_log, L_verbose, "From %s: %s", player->name, s);
  
  if(hooks_call(player_parse, HOOK_DEFAULT, player, s))
    return;
  
  /* got the command */
  argv[0] = s;
  
  /* ugh, we're at string end :( */
  if(*s == '\0')
    return;
  
  /* skip the command */
  while(*s && !isspace(*s)) s++;
  
  /* null-terminate the command */
  *s++ = '\0';
  
  /* skip whitespace */
  while(*s && isspace(*s)) s++;
  
  /* process the command */
  player_message(player, argv, s, n);
}

/* -------------------------------------------------------------------------- *
 * Decide whether a message is numeric or not and call the appropriate        *
 * message handler.                                                           *
 * -------------------------------------------------------------------------- */
void player_message(struct player *client, char **argv, char *arg, size_t n)
{
  size_t i;
  int isnum = 1;
  
  /* loop through argv[1] char by char */
  for(i = 0; argv[0][i]; i++)
  {
    /* if we find a non-digit char, abort and set isnum = 0 */
    if(!isdigit(argv[0][i]))
    {
      isnum = 0;
      break;
    }
  }
    
  /* call numeric/command handler */
/*  if(isnum)
   player_numeric(client, argv, arg);
  else*/
    player_command(client, argv, arg, n);
}
  
/* -------------------------------------------------------------------------- *
 * Process a command                                                          *
 * -------------------------------------------------------------------------- */
void player_command(struct player *player, char **argv, char *arg, size_t n)
{
  struct msg    *msg;
  size_t         ac;
  
  /* find message structure for the command */
  msg = msg_find(argv[0]);
  
/*  log(player_log, L_status, "player_command: %s (%s)", msg->cmd, arg);*/
  
  /* did not find the command */
  if(msg == NULL)
  {
    /* send error message & return */
    if(player_is_unknown(player))
      player_exit(player, "protocol mismatch");
    else
      player_send(player, "ERROR :Unbekanntes Kommando: %s (Probiere /HELP)",
                  argv[0]);

    return;
  }

  if(msg->handlers[player->type] == NULL)
  {
    /* send error message & return */
    if(player_is_unknown(player))
      player_exit(player, "protocol mismatch");
    else 
      player_send(player, "%s ERROR :unknown command", argv[0]);
    
    return;
  }
  
  argv[0] = msg->cmd;
  
  /* tokenize remaining line appropriate to message struct */
  ac = strtokenize(arg, &argv[1], msg->maxargs ? msg->maxargs : 253);
  
  /* check required args */
  if(msg->args && ac < msg->args)
  {
    if(player_is_unknown(player))
      player_exit(player, "protocol mismatch: need %u args, got %u", 
                   msg->args, ac);
   else
      player_send(player, "ERROR :Benötige %u Argumente, gegeben sind %u.",
                  msg->args, ac);
    return;
  }

  /* call the message handler */  
  msg->counts[player->type]++;
  msg->bytes += n;
  msg->handlers[player->type](player, ac, argv);
}

/* -------------------------------------------------------------------------- *
 * Parse the prefix and find the appropriate client                           *
 * -------------------------------------------------------------------------- */
struct player *player_prefix(struct player *player, const char *pfx)
{
/*  struct server *sptr;*/
    
/*  if(player->caps & CAP_UID)
  {
    struct user *uptr;
    
    if((uptr = user_find_uid(pfx)))
      return uptr->player;
  }*/
  
/*  if((sptr = server_find_name(pfx)))
    return sptr->player;
    
  return player_find_name(pfx);*/
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Exit a local client and leave him an error message if he has registered.   *
 * -------------------------------------------------------------------------- */
void player_vexit(struct player *player, char *format, va_list args)
{
  /* Format the exit message */
  char buf[TICHU_TOPICLEN + 1];
  
  vsnprintf(buf, sizeof(buf), format, args);
  
  hooks_call(player_exit, HOOK_DEFAULT, player, format, args);
  
  /* Leave a log notice */
  log(player_log, L_verbose, "Exiting %s:%u: %s",
      net_ntoa(player->addr_remote), player->port_remote, buf);

  /* If he has registered send him an error */
  if(!player_is_unknown(player)/* && io_valid(player->fds[1])*/)
    player_send(player, "ERROR :%s", buf);

//  if(io_valid(player->fds[1]))
    io_close(player->fds[1]);
  
//  if(io_valid(player->fds[0]) && player->fds[1] != player->fds[0])
    io_close(player->fds[0]);
  
  player->fds[0] = -1;
  player->fds[1] = -1;
  player->shut = 1;
  player->refcount = 0;
  
  if(player->game)
    game_leave(player->game, player);
  
  player_delete(player);
  
  if(player->game)
  {
    game_send(player->game, ":%s LEAVE %s :%s",
              player->name, player->game->name, buf);
    
    dlink_delete(&player->game->players, &player->gnode);
  }
  
/*  player_collect();*/
}

int player_exit(struct player *player, char *format, ...)
{
  va_list args;
  
  va_start(args, format);
  player_vexit(player, format, args);
  va_end(args);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Update client message/byte counters                                        *
 * -------------------------------------------------------------------------- */
void player_update_recvb(struct player *player, size_t n)
{
  player_recvb += n;
  player_recvm += 1;
  
  /* message count */
  player->recvm++;
  
  /* increment bytes */
  player->recvb += n;
  
  /* bytes are beyond 1023, so update kbytes */
  if(player->recvb >= 0x0400)
  {
    player->recvk += (player->recvb >> 10);
    player->recvb &= 0x03ff; /* 2^10 = 1024, 3ff = 1023 */
  }  
}

void player_update_sendb(struct player *player, size_t n)
{
  player_sendb += n;
  player_sendm += 1;
  
  /* message count */
  player->sendm++;
  
  /* increment bytes */
  player->sendb += n;
  
  /* bytes are beyond 1023, so update kbytes */
  if(player->sendb >= 0x0400)
  {
    player->sendk += (player->sendb >> 10);
    player->sendb &= 0x03ff; /* 2^10 = 1024, 3ff = 1023 */
  }
}

/* -------------------------------------------------------------------------- *
 * Send a line to a local client                                              *
 * -------------------------------------------------------------------------- */
void player_vsend(struct player *player, const char *format, va_list args)
{
  char   buf[TICHU_LINELEN + 1];
  size_t n;
  
  if(player == NULL)
    return;
  
/*  player_source = player;*/
  
  /* Formatted print */
  n = vsnprintf(buf, sizeof(buf) - 2, format, args);
      
  log(tichu_log_out, L_status, "To %s: %s", player->name, buf);
  
  /* Add line separator */
  buf[n++] = '\r';
  buf[n++] = '\n';
  
  /* Queue the data */
  io_write(player->fds[1], buf, n);

  /* Update sendbytes */
  player_update_sendb(player, n);
}

void player_send(struct player *player, const char *format, ...)
{
  va_list args;

  va_start(args, format);

  player_vsend(player, format, args);

  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * Send a line to a client list but one                                       *
 * -------------------------------------------------------------------------- */
void player_vsend_list(struct player *one,    struct list *list,
                        const char     *format, va_list      args)
{
  struct player *player;
  struct node    *node;
  struct fqueue   multi;
  size_t          n;
  char            buf[TICHU_LINELEN + 1];
  
  /* Formatted print */
  n = vsnprintf(buf, sizeof(buf) - 2, format, args);
      
  /* Add line separator */
  buf[n++] = '\r';
  buf[n++] = '\n';
  
  io_multi_start(&multi);
  
  io_multi_write(&multi, buf, n);
  
  dlink_foreach(list, node)
  {
    player = node->data;
    
    /* Update sendbytes */
    player_update_sendb(player, n);
    
    io_multi_link(&multi, player->fds[1]);
  }    
  
  io_multi_end(&multi);
}

void player_send_list(struct player *one,    struct list  *list,
                       const char     *format, ...)
{
  va_list args;
  
  va_start(args, format);
  
  player_vsend_list(one, list, format, args);
  
  va_end(args);  
}

/* -------------------------------------------------------------------------- *
 * Start client handshake after valid USER/NICK has been sent.                *
 * -------------------------------------------------------------------------- */
void player_handshake(struct player *player)
{
  /* When nickname is invalid then let the user try again */
/*  if(!chars_valid_nick(player->name))
  {
    numeric_lsend(player, ERR_ERRONEUSNICKNAME, player->name);
    return;
  }*/
  
  /* When the username is invalid then exit the user */
/*  if(!chars_valid_user(player->user->name))
  {
    player_set_type(player, player_USER);
    player_exit(player, "invalid username [%s]", player->user->name);
    return;
  }*/
  
  /* If no hooks were called then register the client */
  if(hooks_call(player_handshake, HOOK_DEFAULT, player) == 0)
    player_register(player);
}

/* -------------------------------------------------------------------------- *
 * Check for valid USER/NICK and start handshake                              *
 * -------------------------------------------------------------------------- */
int player_register(struct player *player)
{
/*  struct msg *motd;*/
  
  if(player->refcount == 0)
    return 1;
  
  if(player->type == PLAYER_UNKNOWN)
  {
/*    player_welcome(player);*/
/*    player_lusers(player->player);*/
    
    /* Send the MOTD if available */
/*    if((motd = msg_find("MOTD")) &&& motd->handlers[player->type])
      numeric_send(player->player, ERR_NOMOTD);
    else*
      motd->handlers[player->type](player, player->player, 2, NULL);*/
    
    /* Cancel timeout timer */
    timer_cancel(&player->ptimer);
    
    /* Start the ping timeout timer */
    player->ptimer = timer_start(player_ping, player->class->ping_freq, player);
    
    /* Hooks for flood stuff */
    hooks_call(player_register, HOOK_2ND, player);
    
    io_note(player->fds[0], "registered client %s from %s:%u",
            player->name,
            net_ntoa(player->addr_remote),
            (uint32_t)player->port_remote);
  }

  player->shut = 0;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Check if we got a PONG, if not exit the client otherwise send another PING *
 * -------------------------------------------------------------------------- */
int player_ping(struct player *player)
{
  int saved;
  
  if(player->lag > -1LL)
  {
    /* We got a PONG, send another PING */
    player_send(player, "PING");
    
    /* Set the time we sent the ping and reset lag */
    player->ping = timer_mtime;
    player->lag = -1LL;
  }
  else
  {
    /* We didn't get a PONG, exit the client */
    saved = player->fds[0];
    player_exit(player, "ping timeout: %u seconds",
                (uint32_t)((player->class->ping_freq + 500LL) / 1000LL));
    io_close(saved);
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * USER/NICK has been sent but not yet validated                              *
 * -------------------------------------------------------------------------- */
void player_login(struct player *player)
{
/*  struct client *cptr;*/
  
/*  cptr = player_find_nick(player->name);*/
  
/*  if(cptr)
  {
    numeric_lsend(player, ERR_NICKNAMEINUSE, player->name);
    player->name[0] = '\0';
    return;
  }*/
  
  if(hooks_call(player_login, HOOK_DEFAULT, player) == 0)
    player_handshake(player);
}

/* -------------------------------------------------------------------------- *
 * Send welcome messages to the client                                        *
 * -------------------------------------------------------------------------- */
/*void player_welcome(struct player *player)
{
  char usermodes[sizeof(uint64_t) * 8 + 1];
  char chanmodes[sizeof(uint64_t) * 8 + 1];
  
  usermodes[0] = '\0';
  chanmodes[0] = '\0';
  
  chanmode_flags_build(chanmodes, CHANMODE_TYPE_ALL, CHFLG(ALL));
  usermode_assemble(-1LL, usermodes);
  
  if(usermodes[0] == '\0')
    strcpy(usermodes, "-");
  
  if(chanmodes[0] == '\0')
    strcpy(chanmodes, "-");
  
  numeric_lsend(player, RPL_WELCOME, player->name);
  
  numeric_lsend(player, RPL_YOURHOST, player_me->name, 
                tichu_package, tichu_release);
  
  numeric_lsend(player, RPL_CREATED, ircd_uptime());

  numeric_lsend(player, RPL_MYINFO, player_me->name, ircd_version, 
                usermodes, chanmodes);

  ircd_support_show(player->player);
}*/

/* -------------------------------------------------------------------------- *
 * Find a player by its id                                                   *
 * -------------------------------------------------------------------------- */
struct player *player_find_id(int id)
{
  struct player *player;
  
  dlink_foreach(&player_list, player)
    if(player->serial == id)
      return player;
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a player by its name                                                 *
 * -------------------------------------------------------------------------- */
struct player *player_find_name(const char *name)
{
  struct player *player;
  uint32_t        hash;
  
  hash = strihash(name);
  
  dlink_foreach(&player_list, player)
  {
    if(player->hash == hash)
    {
      if(!stricmp(player->name, name))
        return player;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a player by its name in the db                                        *
 * -------------------------------------------------------------------------- */
struct player *player_find_sql(struct player *player, const char *name)
{  
  /* db files testing */
  struct db_result *db_result;
  char            **row;
      
  if(!tichu_db)
    return NULL;

  db_result = db_query(tichu_db, "SELECT * FROM Benutzer WHERE BeNickname = '%s'", name);
  
  if(db_num_rows(db_result))
  {
    row = db_fetch_row(db_result);    

    /* set player->db_id */
    player->db_id = atoi(row[0]);
    
    /* set player->passwd */
    player->password = strdup(row[2]);
    
    db_free_result(db_result);
    
    return player;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_dump(struct player *player)
{
  if(player == NULL)
  {
    struct node *node;

    dump(player_log, "[============== player summary ===============]");

    if(player_lists[PLAYER_UNKNOWN].size)
    {
      dump(player_log, " --------------- unknown clients ---------------");

      dlink_foreach_data(&player_lists[PLAYER_UNKNOWN], node, player)
        dump(player_log, " #%03u: [%u] %-20s (%s)",
              player->serial, player->refcount,
              player->name[0] ? player->name : "<unregistered>",
              player->host);
    }
    if(player_lists[PLAYER_USER].size)
    {
      dump(player_log, " -------------- anonymous clients -------------");

      dlink_foreach_data(&player_lists[PLAYER_USER], node, player)
        dump(player_log, " #%03u: [%u] %-20s (%s)",
              player->serial, player->refcount, player->name,
              player->host);
    }
    if(player_lists[PLAYER_MEMBER].size)
    {
      dump(player_log, " ------------------- members ------------------");

      dlink_foreach_data(&player_lists[PLAYER_MEMBER], node, player)
        dump(player_log, " #%03u: [%u] %-20s (%s)",
              player->serial, player->refcount, player->name,
              player->host);
    }
    if(player_lists[PLAYER_OPER].size)
    {
      dump(player_log, " ------------------ operators -----------------");

      dlink_foreach_data(&player_lists[PLAYER_OPER], node, player)
        dump(player_log, " #%03u: [%u] %-20s (%s)",
              player->serial, player->refcount, player->name,
              player->host);
    }

    dump(player_log, "[=========== end of player summary ===========]");
  }
  else
  {
    dump(player_log, "[============== player dump ===============]");
    dump(player_log, "     serial: #%u", player->serial);
    dump(player_log, "   refcount: %u", player->refcount);
    dump(player_log, "       type: %s", player_types[player->type]);
    dump(player_log, "       hash: %p", player->hash);
    dump(player_log, "     listen: %-27s [%u]",
          player->listen ? player->listen->name : "(nil)",
          player->listen ? player->listen->refcount : 0);
    dump(player_log, "      fds[]: { %i, %i }",
          player->fds[0], player->fds[1]);
    dump(player_log, "     remote: %s:%u",
          net_ntoa(player->addr_remote), (uint32_t)player->port_remote);
    dump(player_log, "      local: %s:%u",
          net_ntoa(player->addr_local), (uint32_t)player->port_local);
    dump(player_log, "     ptimer: #%i [%u]",
          player->ptimer ? player->ptimer->id : -1,
          player->ptimer ? player->ptimer->refcount : 0);
    dump(player_log, "       recv: %ub %uk %um",
          player->recvb, player->recvk, player->recvm);
    dump(player_log, "       send: %ub %uk %um",
          player->sendb, player->sendk, player->sendm);
    dump(player_log, "       caps: %llu", player->caps);
    dump(player_log, "         ts: %lu", player->ts);
    dump(player_log, "        lag: %llu", player->lag);
    dump(player_log, "       ping: %llu", player->ping);
    dump(player_log, "       name: %s", player->name);
    dump(player_log, "       host: %s", player->host);
    dump(player_log, "     hostip: %s", player->hostip);
    dump(player_log, "       pass: %s", player->pass);
    dump(player_log, "       info: %s", player->info);

    dump(player_log, "[=========== end of player dump ===========]");
  }
}
