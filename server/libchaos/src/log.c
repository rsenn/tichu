/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003-2005  Roman Senn <smoli@paranoya.ch>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 * 
 * $Id: log.c,v 1.39 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/config.h>
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define LOG_SOURCE_COUNT (sizeof(uint64_t) << 3)

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int          log_log;
struct sheap log_heap;
struct list  log_list;
int          log_dirty;
struct slog  log_sources[LOG_SOURCE_COUNT];
uint32_t     log_id;
struct dlog  log_drain;

const char  *log_levels[5][2] = {
  { "!!!", "fatal"   },
  { " ! ", "warning" },
  { " * ", "status"  },
  { " - ", "verbose" },
  { " x ", "debug"   }
};

const char  *log_months[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

/* -------------------------------------------------------------------------- *
 * Initialize logging engine.                                                 *
 * -------------------------------------------------------------------------- */
void log_init(int fd, uint64_t sources, int level)
{
  size_t i;
  
  dlink_list_zero(&log_list);
  
  for(i = 0; i < LOG_SOURCE_COUNT; i++)
  {
    log_sources[i].flag = 1LLU << i;
    log_sources[i].name[0] = '\0';
  }

  log_id = 0;
  log_dirty = 0;
  
  if(fd > 0)
  {
    strcpy(log_drain.path, "stderr");
    log_drain.sources = sources;
    log_drain.id = log_id++;
    log_drain.level = level;
    log_drain.fd = fd;
    log_drain.prefix = 0;
  }
  
  log_log = log_source_register("log");
  
  dlink_list_zero(&mem_slist);
  dlink_list_zero(&mem_dlist);

  mem_static_create(&log_heap, sizeof(struct dlog), LOG_BLOCK_SIZE);
  mem_static_note(&log_heap, "log drain heap");
  
  log(log_log, L_status, "Initialized [log] module.");
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void log_level(uint64_t sources, int level)
{
    log_drain.sources = sources;
    log_drain.level = level;
}

/* -------------------------------------------------------------------------- *
 * Shutdown logging engine.                                                   *
 * -------------------------------------------------------------------------- */
void log_shutdown(void)
{
  struct dlog *drain;
  struct dlog *next;
  
  log(log_log, L_status, "Shutting down [log] module...");
  
  dlink_foreach_safe(&log_list, drain, next)
    log_drain_delete(drain);
  
  log_source_unregister(log_log);
  
  mem_static_destroy(&log_heap);
}

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
void log_collect(void)
{
  if(log_dirty)
  {
    mem_static_collect(&log_heap);
    
    log_dirty = 0;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int log_source_register(const char *name)
{
  size_t             i;
  struct slog *source = NULL;
  
  for(i = 0; i < LOG_SOURCE_COUNT; i++)
  {
    if(log_sources[i].name[0] == '\0')
    {
      source = &log_sources[i];
      break;
    }
  }
  
  if(source)
  {
    strlcpy(source->name, name, sizeof(source->name));
    source->hash = strihash(source->name);

    log(log_log, L_verbose, "Registered log source: %s", source->name);
  
    return i;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int log_source_unregister(int id)
{
  if(id < 0 || id >= LOG_SOURCE_COUNT)
    return -1;
  
  log(log_log, L_verbose, "Unregistered log source: %s", 
      log_sources[id].name);
  
  log_sources[id].name[0] = '\0';
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int log_source_find(const char *name)
{
  uint32_t i;
  
  for(i = 0; i < LOG_SOURCE_COUNT; i++)
  {
    if(log_sources[i].name[0])
    {
      if(!stricmp(log_sources[i].name, name))
        return i;
    }
  }

  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *log_source_assemble(uint64_t flags)
{
  static char ret[512];
  size_t      n = 0;
  uint32_t    i;
  
  if(flags == LOG_ALL)
  {
    strcpy(ret, "all");
    return ret;
  }
  
  for(i = 0; i < LOG_SOURCE_COUNT; i++)
  {
    if(log_sources[i].name[0] && (flags & log_sources[i].flag))
    {
      if(n)
        ret[n++] = ' ';
      
      n += strlcpy(&ret[n], log_sources[i].name, 512 - n - 1);
    }
  }

  ret[n] = '\0';

  if(n == 0)
    strcpy(ret, "none");
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int log_level_parse(const char *levstr)
{
  size_t i;
  
  for(i = 0; i <= L_debug; i++)
    if(!stricmp(log_levels[i][1], levstr))
      return i;
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint64_t log_source_parse(const char *sources)
{
  struct slog *source;
  uint64_t     ret = 0;
  uint32_t     hash;
  size_t       srcc;
  size_t       i;
  size_t       j;
  char        *srcv[LOG_SOURCE_COUNT + 1];
  char         buf[1024];
  
  strlcpy(buf, sources, sizeof(buf));
  
  srcc = strtokenize(buf, srcv, LOG_SOURCE_COUNT);
  
  for(i = 0; i < srcc; i++)
  {
    if(!stricmp(srcv[i], "all"))
      return LOG_ALL;
    
    hash = strihash(srcv[i]);
  
    for(j = 0; j < LOG_SOURCE_COUNT; j++)
    {
      source = &log_sources[j];
      
      if(source->name[0] && source->hash == hash)
      {
        if(!stricmp(source->name, srcv[i]))
          ret |= source->flag;
      }      
    }
  }

  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void log_drain_default(struct dlog *drain)
{
  memset(drain, 0, sizeof(struct dlog));
  
  drain->level = L_status;
  drain->sources = -1LL;
  drain->fd = -1;
  drain->prefix = 0;
  drain->truncate = 0;
  
  strcpy(drain->path, "/dev/tty");
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_open(const char *path,  uint64_t sources, 
                            int         level, int      prefix,
                            int         truncate)
{
  struct dlog *drain;
  int          fd;
  
  fd = io_open(path, O_WRONLY | O_CREAT | O_APPEND | 
               (truncate ? O_TRUNC : 0), 0644);
  
  if(fd == -1)
    return NULL;
  
  drain = mem_static_alloc(&log_heap);
  
  drain->fd = fd;
  drain->sources = sources;
  drain->level = level;
  drain->prefix = prefix;
  drain->truncate = truncate;
  drain->callback = NULL;
  strlcpy(drain->path, path, sizeof(drain->path));
  
  drain->hash = strhash(drain->path);
  drain->refcount = 1;
  drain->id = log_id++;
  
  dlink_add_tail(&log_list, &drain->node, drain);
  
/*#ifdef DEBUG
  io_queue_control(drain->fd, OFF, OFF, ON);
#else*/
  io_queue_control(drain->fd, OFF, ON, ON);
/*#endif*/ /* DEBUG */
  
  return drain;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_setfd(int fd, uint64_t sources, int level, int prefix)
{
  struct dlog *drain;
  
  drain = mem_static_alloc(&log_heap);
  
  drain->fd = fd;
  drain->sources = sources;
  drain->level = level;
  drain->prefix = prefix;
  drain->callback = NULL;
  
  if(fd == 1)
    strcpy(drain->path, "stdout");
  else if(fd == 2)
    strcpy(drain->path, "stderr");
  else
    snprintf(drain->path, sizeof(drain->path), "fd: %i", fd);
  
  drain->hash = strhash(drain->path);
  drain->refcount = 1;
  drain->id = log_id++;
  
  dlink_add_tail(&log_list, &drain->node, drain);

  return drain;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_callback(void *callback, uint64_t sources, 
                                int   level,    ...)
{
  struct dlog *drain;
  va_list      args;
  
  drain = mem_static_alloc(&log_heap);
  
  drain->fd = -1;
  drain->sources = sources;
  drain->level = level;
  drain->prefix = 1;
  drain->callback = callback;
  
  strcpy(drain->path, "callback");
  
  drain->hash = (size_t)callback;
  drain->refcount = 1;
  drain->id = log_id++;
  
  /* There can be up to 4 user-supplied arguments
     which are passed to the callback */
  va_start(args, level);
  
  drain->args[0] = va_arg(args, void *);
  drain->args[1] = va_arg(args, void *);
  drain->args[2] = va_arg(args, void *);
  drain->args[3] = va_arg(args, void *);
  
  va_end(args);
  
  dlink_add_tail(&log_list, &drain->node, drain);
  
  return drain;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_find_path(const char *path)
{
  struct dlog *dlptr;
  uint32_t     hash = 0;
  
  hash = strhash(path);
  
  dlink_foreach(&log_list, dlptr)
  {
    if(path && dlptr->hash == hash)
    {
      if(!strcmp(dlptr->path, path))
        return dlptr;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_find_cb(void *cb)
{
  struct dlog *dlptr;
  
  dlink_foreach(&log_list, dlptr)
  {
    if(dlptr->callback == cb)
      return dlptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_find_id(uint32_t id)
{
  struct dlog *dlptr;
  
  dlink_foreach(&log_list, dlptr)
  {
    if(dlptr->id == id)
      return dlptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int log_drain_update(struct dlog *drain, uint64_t sources, 
                     int          level, int      prefix)
{
  drain->sources = sources;
  drain->level = level;
  drain->prefix = prefix;

  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void log_drain_delete(struct dlog *drain)
{
  dlink_delete(&log_list, &drain->node);

  mem_static_free(&log_heap, drain);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_pop(struct dlog *ldptr)
{
  if(ldptr)
  {
    ldptr->refcount++;
  }
  
  return ldptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog *log_drain_push(struct dlog **ldptrptr)
{
  if(*ldptrptr)
  {
    if(!(*ldptrptr)->refcount)
      log(log_log, L_warning, "Trying to push deprecated log drain: %s",
          (*ldptrptr)->path);
    else
      (*ldptrptr)->refcount--;
    
    if(!(*ldptrptr)->refcount)
      log_dirty = 1;
    
    (*ldptrptr) = NULL;
  }
        
  return *ldptrptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void log_drain_level(struct dlog *drain, int level)
{
  drain->level = level;
}

/* -------------------------------------------------------------------------- *
 * Log a line.                                                                *
 * -------------------------------------------------------------------------- */
void log_voutput(int src, int level, const char *format, va_list ap)
{
  struct slog *source;
  struct dlog *drain;
  char         date[64];
  char         logmsg[2048];
  
  source = &log_sources[src];
  
  snprintf(date, 64, "(%s) [%s %2u %02u:%02u:%02u] <%s> ",
           log_levels[level][0], log_months[timer_dtime.tm_mon], timer_dtime.tm_mday,
           timer_dtime.tm_hour, timer_dtime.tm_min, timer_dtime.tm_sec, source->name);
           
  vsnprintf(logmsg, 2048, format, ap);
  
  dlink_foreach(&log_list, drain)
  {
    if(level > drain->level)
      continue;
    
    if(!(source->flag & drain->sources))
      continue;
      
    if(drain->fd > -1)
    {
      if(drain->prefix)
        io_puts(drain->fd, "%s%s", date, logmsg);
      else
        io_puts(drain->fd, "%s", logmsg);
    }
    if(drain->callback)
    {
      drain->callback(source->flag, level, log_levels[level][1],
                      source->name, date, logmsg,
                      drain->args[0], drain->args[1],
                      drain->args[2], drain->args[3]);
    }
  }
  
  if(log_list.head == NULL)
  {
    if(log_drain.fd > -1 &&
       (source->flag & log_drain.sources) && level <= log_drain.level)
    {
      if(log_drain.prefix)
        io_puts(log_drain.fd, "%s%s", date, logmsg);
      else
        io_puts(log_drain.fd, "%s", logmsg);    
    }
    else
    {
      if(level == L_startup)
      {
        syscall_write(1, logmsg, strlen(logmsg));
        syscall_write(1, "\n", 1);
      }
    }
  }
}


void log_output(int src, int level, const char *format, ...) 
{
  va_list ap;
  
  va_start(ap, format);
  
  log_voutput(src, level, format, ap);
  
  va_end(ap);  
} 

/* -------------------------------------------------------------------------- *
 * Log a line.                                                                *
 * -------------------------------------------------------------------------- */
#ifndef HAVE_VARARG_MACROS
void log_output_debug(int src, const char *format, ...)
{
  va_list ap;
  
  va_start(ap, format);
  
  log_voutput(src, L_debug, format, ap);
  
  va_end(ap);
}
#endif /* HAVE_VARARG_MACROS */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifndef HAVE_VARARG_MACROS
void log_output_dummy(int src, const char *format, ...)
{
}
#endif /* HAVE_VARARG_MACROS */

/* -------------------------------------------------------------------------- *
 * Log a line in debugging mode.                                              *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void log_debug(const char *file,  int line,
               int         src,   int level,
               const char *format, ...) 
{
  struct slog *source;
  struct dlog *drain;
  char         date[64];
  char         logmsg[2048];
  va_list      ap;
  
  source = &log_sources[src];
  
  va_start(ap, format);
  
  snprintf(date, 64, "[%s] (%s) %s:%u -- ",
           log_levels[level][0], source->name, file, line);
           
  vsnprintf(logmsg, 2048, format, ap);
  
  va_end(ap);

  dlink_foreach(&log_list, drain)
  {
    if(level > drain->level)
      continue;
    
    if(!(source->flag & drain->sources))
      continue;
      
    if(drain->fd > -1)
    {
      if(drain->prefix)
        io_puts(drain->fd, "%s%s", date, logmsg);
      else
        io_puts(drain->fd, "%s", logmsg);
    }
    if(drain->callback)
    {
      drain->callback(source->flag, level, log_levels[level][0],
                      source->name, date, logmsg,
                      drain->args[0], drain->args[1],
                      drain->args[2], drain->args[3]);
    }
  }
  
  if(log_list.head == NULL)
  {
    if(log_drain.fd > -1 &&
       (source->flag & log_drain.sources) && level <= log_drain.level)
    {
      if(log_drain.prefix)
        io_puts(log_drain.fd, "%s%s", date, logmsg);
      else
        io_puts(log_drain.fd, "%s", logmsg);    
    }
    else
    {
      if(level == L_startup)
      {
        syscall_write(1, logmsg, strlen(logmsg));
        syscall_write(1, "\n", 1);
      }
    }
  }
} 
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Dump log loglist and heap.                                                 *
 * -------------------------------------------------------------------------- */
void log_drain_dump(struct dlog *dlptr)
{
  if(dlptr == NULL)
  {
    dump(log_log, "[============= log drain summary ==============]");
    
    dlink_foreach(&log_list, dlptr)
    {
      if(dlptr->callback)
      {
        dump(log_log, " #%03u: [%u] (%s) %p ",
             dlptr->id, dlptr->refcount, 
             log_levels[dlptr->level][1], dlptr->callback);
      }
      else
      {
        dump(log_log, " #%03u: [%u] (%s) %s ",
             dlptr->id, dlptr->refcount, 
             log_levels[dlptr->level][1], dlptr->path);
      }
    }
      
    dump(log_log, "[========== end of log drain summary ==========]");
  }
  else
  {
    dump(log_log, "[============== log drain dump ===============]");
  
    dump(log_log, "         id: #%u", dlptr->id);
    dump(log_log, "   refcount: %u", dlptr->refcount);
    dump(log_log, "       hash: %p", dlptr->hash);
    dump(log_log, "    sources: %s", log_source_assemble(dlptr->sources));
    dump(log_log, "      level: %s", log_levels[dlptr->sources][1]);
    dump(log_log, "         fd: %i", dlptr->fd);
    dump(log_log, "     prefix: %s", (dlptr->prefix ? "yes" : "no"));
    dump(log_log, "   truncate: %s", (dlptr->truncate ? "yes" : "no"));
    dump(log_log, "   callback: %p", dlptr->callback);
    dump(log_log, "       args: %p, %p, %p, %p",
         dlptr->args[0], dlptr->args[1], dlptr->args[2], dlptr->args[3]);
    dump(log_log, "       path: %s", dlptr->path);
    
    dump(log_log, "[=========== end of log drain dump ===========]");
  }
}

void log_source_dump(int id)
{
  if(id < 0 || id > LOG_SOURCE_COUNT)
  {
    dump(log_log, "[============= log source summary ==============]");
    
    for(id = 0; id < LOG_SOURCE_COUNT; id++)
    {
      if(log_sources[id].name[0] == '\0')
        continue;
      
      dump(log_log, " #%03u: %-20s (%p)",
           id, log_sources[id].name, log_sources[id].hash);
    }
    
    dump(log_log, "[========== end of log source summary ==========]");
  }
  else
  {
    dump(log_log, "[============== log source dump ===============]");
  
    dump(log_log, "         id: #%u", id);
    dump(log_log, "       flag: %llu", log_sources[id].flag);
    dump(log_log, "       hash: %p", log_sources[id].hash);
    dump(log_log, "       name: %s", log_sources[id].name);
    
    dump(log_log, "[=========== end of log source dump ===========]");
  }
}
