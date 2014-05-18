/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <tichu/player.h>
#include <tichu/tichu.h>
#include <tichu/msg.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int         msg_log;
uint32_t    msg_id;
struct list msg_table[MSG_HASH_SIZE];

/* -------------------------------------------------------------------------- *
 * Initialize message heap.                                                   *
 * -------------------------------------------------------------------------- */
void msg_init(void)
{
  msg_log = log_source_register("msg");
  msg_id = 0;
  
  memset(msg_table, 0, MSG_HASH_SIZE * sizeof(struct list));
  
  log(msg_log, L_status, "Initialised [msg] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy message heap.                                                      *
 * -------------------------------------------------------------------------- */
void msg_shutdown(void)
{
  struct node *next;
  struct msg  *mptr;
  size_t       i;
  
  log(msg_log, L_status, "Shutting down [msg] module...");
  
  for(i = 0; i < MSG_HASH_SIZE; i++)
  {
    dlink_foreach_safe(&msg_table[i], mptr, next)
      dlink_delete(&msg_table[i], &mptr->node);
  }
  
  log_source_unregister(msg_log);
}

/* -------------------------------------------------------------------------- *
 * Find a message.                                                            *
 * -------------------------------------------------------------------------- */
struct msg *msg_find(const char *name)
{
  struct node *node;
  struct msg  *m;
  uint32_t     hash;
  
  hash = strihash(name);
  
  dlink_foreach(&msg_table[hash % MSG_HASH_SIZE], node)
  {
    m = node->data;
    
    if(m->hash == hash && !stricmp(m->cmd, name))
      return m;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct msg *msg_find_id(uint32_t id)
{
  struct node *nptr;
  struct msg  *m;
  uint32_t     i;
  
  for(i = 0; i < MSG_HASH_SIZE; i++)
  {
    dlink_foreach_data(&msg_table[i], nptr, m)
      if(m->id == id)
        return m;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Register a message.                                                        *
 * -------------------------------------------------------------------------- */
struct msg *msg_register(struct msg *msg)
{
  if(msg_find(msg->cmd))
  {
    log(msg_log, L_warning, "Message %s was already registered.", msg->cmd);
    return NULL;
  }
  
  msg->hash = strihash(msg->cmd);
  msg->id = msg_id++;
  
  dlink_add_tail(&msg_table[msg->hash % MSG_HASH_SIZE], &msg->node, msg);
  
  return msg;
}
  
/* -------------------------------------------------------------------------- *
 * Unregister a message.                                                      *
 * -------------------------------------------------------------------------- */
void msg_unregister(struct msg *msg)
{
  dlink_delete(&msg_table[msg->hash % MSG_HASH_SIZE], &msg->node);
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void m_unregistered(struct player *player, int argc, char **argv)
{
/*  player_send(player, numeric_format(ERR_NOTREGISTERED),
               client_me->name, player->name);*/
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void m_registered(struct player *player, int argc, char **argv)
{
/*  player_send(player, numeric_format(ERR_ALREADYREGISTRED),
               client_me->name, cptr->name);*/
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void m_ignore(struct player *player, int argc, char **argv)
{
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void m_not_oper(struct player *player, int argc, char **argv)
{
/*  player_send(player, numeric_format(ERR_NOPRIVILEGES),
               client_me->name, argv[0] ? argv[0] : "*");*/
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void msg_dump(struct msg *mptr)
{
  struct node *nptr;
  uint32_t     i;
  
  if(mptr == NULL)
  {
    dump(msg_log, "[================ msg summary ================]");

    for(i = 0; i < MSG_HASH_SIZE; i++)
    {
      dlink_foreach_data(&msg_table[i], nptr, mptr)
        dump(msg_log, " #%03u: %-12s (%3u/%3u/%3u/%3u)",
             mptr->id, mptr->cmd, 
             mptr->counts[MSG_UNREGISTERED],
             mptr->counts[MSG_PLAYER],
             mptr->counts[MSG_OPER]);
    }

    dump(msg_log, "[============= end of msg summary ============]");
  }
  else
  {
    dump(msg_log, "[================= msg dump =================]");
    
    dump(msg_log, "        cmd: #%u", mptr->cmd);
    dump(msg_log, "       args: #%u", mptr->args);
    dump(msg_log, "    maxargs: #%u", mptr->maxargs);
    dump(msg_log, "      flags:%s%s%s",
         (mptr->flags & MFLG_UNREG ? " MFLG_UNREG" : ""),
         (mptr->flags & MFLG_PLAYER ? " MFLG_PLAYER" : ""),
         (mptr->flags & MFLG_OPER ? " MFLG_OPER" : ""));         
    dump(msg_log, "   handlers: %p, %p, %p",
         mptr->handlers[MSG_UNREGISTERED],
         mptr->handlers[MSG_PLAYER],
         mptr->handlers[MSG_OPER]);
    dump(msg_log, "       help: %p", mptr->help);
    dump(msg_log, "       hash: %p", mptr->hash);
    dump(msg_log, "     counts: %u, %u, %u",
         mptr->counts[MSG_UNREGISTERED],
         mptr->counts[MSG_PLAYER],
         mptr->counts[MSG_OPER]);
    dump(msg_log, "      bytes: %u", mptr->bytes);
    dump(msg_log, "         id: %u", mptr->id);
    
    dump(msg_log, "[============== end of msg dump =============]");
  }
}
