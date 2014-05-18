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
 * $Id: timer.h,v 1.20 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_TIMER_H
#define LIB_TIMER_H

/* -------------------------------------------------------------------------- *
 * System headers                                                             *
 * -------------------------------------------------------------------------- */

#include <stdarg.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */

#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

/* Seconds per day */
#define TIMER_SPD (24 * 60 * 60)

#define TIMER_MAX_INTERVAL (24LL * 3600LL * 1000LL) /* max. interval = 1 day */

#define timer_today    ((timer_mtime / TIMER_SPD) * (TIMER_SPD))
#define timer_thisyear ((timer_dtime.tm_year))

/* -------------------------------------------------------------------------- *
 * Timer callback receives userarg from timer struct as argument.             *
 * If the callback returns with a value of 1 then the timer is removed.       *
 * If the callback returns 0 then the timer will be scheduled again in the    *
 * same interval.                                                             *
 * -------------------------------------------------------------------------- */
typedef int (timer_cb_t)(void *, void *, void *, void *);
typedef void (timer_shift_cb)(int64_t);

/* -------------------------------------------------------------------------- *
 * Timer block structure                                                      *
 * -------------------------------------------------------------------------- */

struct timer {
  struct node node;           /* linking node for timer block list */
  
  /* externally initialised */
  timer_cb_t *callback;       /* the function called on timer deadline */
  uint64_t    interval;       /* timer interval (deadline = mtime + interval) */
  void       *args[4];        /* 4 user-defineable arguments for the callback */
  char        note[104];      /* timer description */
  int         refcount;
  
  /* internally initialised */
  uint64_t    deadline;       /* time at which the callback will be called */
  uint32_t    id;
};
/* Sorry for this leetness, but timer_t is already defined elsewhere */

/* -------------------------------------------------------------------------- *
 * Exported variables                                                         *
 * -------------------------------------------------------------------------- */
extern int            timer_log;
extern struct sheap   timer_heap;
extern struct timer  *timer_timer;
extern struct list    timer_list;
extern int            timer_dirty;
extern uint32_t       timer_id;
extern uint32_t       timer_systime; /* real unixtime */
extern uint32_t       timer_loctime; /* system time after timezone conversion */
extern int64_t        timer_offset;  /* offset in miliseconds */
extern uint64_t       timer_mtime;   /* unixtime in miliseconds */
extern uint64_t       timer_otime;   /* unixtime in miliseconds */
extern struct timeval timer_utime;   /* unixtime in microseconds */
extern struct tm      timer_dtime;   /* daytime */

/* -------------------------------------------------------------------------- *
 * Convert unixtime in miliseconds to a struct tm                             *
 * -------------------------------------------------------------------------- */
extern struct tm    *timer_gmtime     (uint64_t            mtime);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern size_t        timer_strftime   (char               *s,
                                       size_t              max, 
                                       const char         *format,
                                       const struct tm    *tm);
  
/* -------------------------------------------------------------------------- *
 * Convert a struct tm to unixtime in miliseconds                             *
 * -------------------------------------------------------------------------- */
extern uint64_t      timer_mktime     (struct tm          *tm);

/* -------------------------------------------------------------------------- *
 * Parse a time in HH:MM([.:]SS) format                                       *
 * -------------------------------------------------------------------------- */
extern uint64_t      timer_parse_time (const char         *t);

/* -------------------------------------------------------------------------- *
 * Parse a date in DD.MM(.YY(YY)) format                                       *
 * -------------------------------------------------------------------------- */
extern uint64_t      timer_parse_date (const char         *d);
    
/* -------------------------------------------------------------------------- *
 * Initialize the timer code.                                                 *
 * -------------------------------------------------------------------------- */
extern void          timer_init       (void);

/* -------------------------------------------------------------------------- *
 * Shutdown the timer code.                                                   *
 * -------------------------------------------------------------------------- */
extern void          timer_shutdown   (void);  

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int           timer_collect    (void);

/* -------------------------------------------------------------------------- *
 * Convert from timeval to miliseconds.                                       *
 *                                                                            *
 * <src>            - pointer to timeval to convert                           *
 * <dst>            - pointer to 64bit integer to store result                *
 * -------------------------------------------------------------------------- */
extern void          timer_to_msec    (uint64_t           *dst, 
                                       struct timeval     *src);
  
/* -------------------------------------------------------------------------- *
 * Convert from miliseconds to timeval.                                       *
 *                                                                            *
 * <src>            - pointer to 64bit integer to convert                     *
 * <dst>            - pointer to timeval to store result                      *
 * -------------------------------------------------------------------------- */
extern void          timer_to_timeval (struct timeval *dst, 
                                       uint64_t       *src);    

/* -------------------------------------------------------------------------- *
 * Update the system time.                                                    *
 * Returns -1 if the underlying systemcall fails and 0 on success.            *
 * -------------------------------------------------------------------------- */
extern int           timer_update     (void);

/* -------------------------------------------------------------------------- *
 * Add a timer shifting callback                                              *
 * -------------------------------------------------------------------------- */
extern void          timer_shift_register (timer_shift_cb *shift_cb);
  
/* -------------------------------------------------------------------------- *
 * Remove a timer shifting callback                                           *
 * -------------------------------------------------------------------------- */
extern void          timer_shift_unregister (timer_shift_cb *shift_cb);

/* -------------------------------------------------------------------------- *
 * Add the specified offset to all timer deadlines.                           *
 *                                                                            *
 * <delta>          - offset to be added to deadlines in milliseconds         *
 * -------------------------------------------------------------------------- */
extern void          timer_shift      (int64_t         delta);

/* -------------------------------------------------------------------------- *
 * See if we have clock drift, modify deadlines if necessary.                 *
 *                                                                            *
 * <waited>         - how long the select()/poll() lasted.                    *
 * -------------------------------------------------------------------------- */
extern void          timer_drift      (int64_t         waited);

/* -------------------------------------------------------------------------- *
 * Add and start a timer.                                                     *
 *                                                                            *
 * <callback>              - function to call when the timer expired          *
 * <interval>              - call the callback in this many miliseconds       *
 * <...>                   - up to four user-supplied arguments which are     *
 *                           passed to the callback.                          *
 *                                                                            *
 * Returns NULL on failure and a pointer to the timer on success.             *
 * -------------------------------------------------------------------------- */
extern struct timer *timer_start      (void           *callback,
                                       uint64_t        interval,
                                       ...);

/* -------------------------------------------------------------------------- *
 * Stop and remove a timer.                                                   *
 *                                                                            *
 * <timer>           - pointer to a timer structure returned by timer_start() *
 *                     or timer_find()                                        *
 * -------------------------------------------------------------------------- */
extern void          timer_remove     (struct timer   *timer);

/* -------------------------------------------------------------------------- *
 * Find a timer by callback and 1st userarg.                                  *
 *                                                                            *
 * <callback>        - the callback supplied on timer_start()                 *
 * <userarg>         - the userarg supplied on timer_start()                  *
 *                                                                            *
 * Returns NULL if not found.                                                 *
 * -------------------------------------------------------------------------- */
extern struct timer *timer_find       (void           *callback, 
                                       ...);

/* -------------------------------------------------------------------------- *
 * Find a timer by id.                                                        *
 * -------------------------------------------------------------------------- */
extern struct timer *timer_find_id    (uint32_t        id);

/* -------------------------------------------------------------------------- *
 * Stop and remove a timer by callback and userarg.                           *
 *                                                                            *
 * <callback>        - the callback supplied on timer_start()                 *
 * <userarg>         - the userarg supplied on timer_start()                  *
 *                                                                            *
 * Returns -1 if the timer wasn't found.                                      *
 * -------------------------------------------------------------------------- */
extern int           timer_find_cancel(void           *callback, 
                                       void           *userarg);

/* -------------------------------------------------------------------------- *
 * Write a description to the timer structure for timer(s)_dump()             *
 *                                                                            *
 * <timer>           - pointer to a timer structure returned by timer_start() *
 *                     or timer_find()                                        *
 * <format>          - format string                                          *
 *                   - your args                                              *
 * -------------------------------------------------------------------------- */
extern void          timer_vnote      (struct timer   *timer, 
                                       const char     *format,
                                       va_list         args);

extern void          timer_note       (struct timer   *timer, 
                                       const char     *format,
                                       ...);

/* -------------------------------------------------------------------------- *
 * Write a description to the timer structure for timer(s)_dump()             *
 *                                                                            *
 * <timer>           - pointer to a timer structure returned by timer_start() *
 *                     or timer_find()                                        *
 * <format>          - format string                                          *
 *                   - your args                                              *
 * -------------------------------------------------------------------------- */
extern void          timer_find_note  (struct timer   *timer,
                                       const char     *format,
                                       ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct timer *timer_pop        (struct timer   *timer);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          timer_cancel     (struct timer  **tptrptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct timer *timer_push       (struct timer  **timer);

/* -------------------------------------------------------------------------- *
 * Run pending timers.                                                        *
 *                                                                            *
 * Will return number of timers runned.                                       *
 * -------------------------------------------------------------------------- */
extern int           timer_run        (void);

/* -------------------------------------------------------------------------- *
 * Get the time at which the next timer will expire.                          *
 * Return 0LLU when there is no timer.                                        *
 * -------------------------------------------------------------------------- */
extern uint64_t      timer_deadline   (void);

/* -------------------------------------------------------------------------- *
 * Get the optimal timeout for select()/poll()                                *
 * (The time until the lowest deadline)                                       *
 * -------------------------------------------------------------------------- */
extern int64_t      *timer_timeout    (void);

/* -------------------------------------------------------------------------- *
 * Dump timers.                                                               *
 * -------------------------------------------------------------------------- */
extern void          timer_dump       (struct timer *timer);

#endif /* LIB_TIMER_H */
