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
 * $Id: log.h,v 1.27 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_LOG_H
#define LIB_LOG_H

#include <stdio.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Log levels                                                                 *
 * 
 * The actual type of a log message                                           *
 * -------------------------------------------------------------------------- */
#define L_fatal      0x00000000   /* leads to service termination :( */
#define L_warning    0x00000001   /* warns about something going wrong :/ */
#define L_status     0x00000002   /* status of the service :) */
#define L_verbose    0x00000003   /* things nice to know :D */
#define L_debug      0x00000004   /* brain overflow */
#define L_startup    0x00000005

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

#define LOG_ALL 0xffffffffffffffffLLU
#define LOG_SOURCE_COUNT (sizeof(uint64_t) << 3)
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
typedef void (log_drain_cb_t)(uint64_t    src,   int         lvl,
                              const char *level, const char *source,
                              const char *date,  const char *msg,
                              void       *arg0,  void       *arg1, 
                              void       *arg2,  void       *arg3);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct slog {
  uint64_t flag;                  /* source bitflag for masking purposes */
  uint32_t hash;
  char     name[16];              /* log source name */
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct dlog {
  struct node     node;
  uint32_t        hash;
  uint32_t        refcount;
  uint32_t        id;
  uint64_t        sources;
  int             level;
  int             fd;
  int             prefix;
  int             truncate;
  log_drain_cb_t *callback;
  void           *args[4];
  char            path[256];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          log_log;
extern struct list  log_list;
extern struct slog  log_sources[LOG_SOURCE_COUNT];

/* -------------------------------------------------------------------------- *
 * Initialize logging engine.                                                 *
 * -------------------------------------------------------------------------- */
extern void         log_init              (int           fd,
                                           uint64_t      sources,
                                           int           level);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         log_level             (uint64_t      sources,
                                           int           level);

/* -------------------------------------------------------------------------- *
 * Shutdown logging engine.                                                   *
 * -------------------------------------------------------------------------- */
extern void         log_shutdown          (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern void         log_collect           (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          log_source_register   (const char   *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          log_source_unregister (int           id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          log_source_find       (const char   *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern char        *log_source_assemble   (uint64_t      flags);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          log_level_parse       (const char   *levstr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint64_t     log_source_parse      (const char   *sources);


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         log_drain_default     (struct dlog  *drain);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_open        (const char   *path,
                                           uint64_t      sources,
                                           int           level,
                                           int           prefix,
                                           int           truncate);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_setfd       (int           fd,
                                           uint64_t      sources,
                                           int           level,
                                           int           prefix);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_callback    (void         *callback,
                                           uint64_t      sources,
                                           int           level,
                                           ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_find_path   (const char   *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_find_cb     (void         *cb);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_find_id     (uint32_t      id);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_add         (uint64_t      sources,
                                           int           level);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          log_drain_update      (struct dlog  *dlptr,
                                           uint64_t      sources,
                                           int           level,
                                           int           prefix);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         log_drain_delete      (struct dlog  *dlptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_pop         (struct dlog  *dlptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct dlog *log_drain_push        (struct dlog **dlptrptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         log_drain_level       (struct dlog  *dlptr,
                                           int           level);
  
/* -------------------------------------------------------------------------- *
 * Write a log line.                                                          *
 * -------------------------------------------------------------------------- */
extern void         log_output            (int           src,
                                           int           level,
                                           const char   *format,
                                           ...);

/* -------------------------------------------------------------------------- *
 * Dump log entries and heap.                                                 *
 * -------------------------------------------------------------------------- */
extern void         log_source_dump       (int id);
extern void         log_drain_dump        (struct dlog *dlptr);

#define fprintf(ios, format...) \
  log_output(conf_log, L_warning, format)

#if 1
# if 0

/* 
 * when DEBUG is defined the normal log 
 * messages get file/line instead of 
 * date/time.
 */
#  define log(src, level, format...) \
            log_debug(__FILE__, __LINE__, \
            (src), (level), format)

#  define debug(src, format...) \
            log_debug(__FILE__, __LINE__, src, L_debug, format)

#  define dump(src, format...) \
            log_debug(__FILE__, __LINE__, (src), L_debug, format)

extern void log_debug(const char *file, int line,
                      int src, int level, const char *format, ...);
# else

/* 
 * when DEBUG is NOT defined, then 
 * the debug() directions don't even
 * get compiled.
 */

#  define debug(format...) ;

#  define dump(src, format...) \
            log_output((src), L_debug, format)


/*#  define log(src, level, format...) \
            log_output((src), (level), format)*/
#define log log_output

# endif

# define puts(s...) log(log_log, L_verbose, s)
#else

#define log log_output
#define dump log_output_debug

#ifdef DEBUG
# define debug log_output_debug
#else
# define debug log_output_dummy
#endif

#define puts(x) log_output(log_log, L_verbose, x)

extern void log_output_debug(int src, const char *format, ...);  
extern void log_output_dummy(int src, const char *format, ...);  

#endif /* HAVE_VARARG_MACROS */

#endif /* LIB_LOG_H */
