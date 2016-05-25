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
 * $Id: timer.c,v 1.55 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/syscall.h>
#include <libchaos/timer.h>
#include <libchaos/dlink.h>
#include <libchaos/mem.h>
#include <libchaos/log.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int            timer_log;
struct sheap   timer_heap;
struct timer  *timer_timer;
struct list    timer_list;
int            timer_dirty;
uint32_t       timer_id;
uint32_t       timer_systime = 0;        /* real unixtime */
uint32_t       timer_loctime = 0;        /* system time after timezone conversion */
uint64_t       timer_otime   = 0LLU;     /* old unixtime in miliseconds */
uint64_t       timer_mtime   = 0LLU;     /* unixtime in miliseconds */
int64_t        timer_offset;
struct timeval timer_utime   = { 0, 0 }; /* unixtime in microseconds */
struct tm      timer_dtime;              /* daytime */
struct list    timer_shifts;

/* isleap(year) evaluates to true if its a year that has 366 days */
#undef isleap
#define isleap(year)  (!(year % 4) && ((year % 100) || !(year % 400)))

/* Seconds per day */
#define SPD (24 * 60 * 60)

/* The year at which unix time countation starts */
#define EPOCH 1970

/* Days per month -- nonleap! */
static const short dpm[12] = {
  0,
  (31),
  (31 + 28),
  (31 + 28 + 31),
  (31 + 28 + 31 + 30),
  (31 + 28 + 31 + 30 + 31),
  (31 + 28 + 31 + 30 + 31 + 30),
  (31 + 28 + 31 + 30 + 31 + 30 + 31),
  (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
  (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
  (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
  (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
};

static const char *timer_weekdays[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static const char *timer_months[] = {
  "January", "February", "March", "April", "May", "June", "July", "August", "September", "November", "October", "December"
};

/* -------------------------------------------------------------------------- *
 * Convert unixtime in miliseconds to a struct tm                             *
 * -------------------------------------------------------------------------- */
struct tm *timer_gmtime(uint64_t mtime)
{
  uint32_t i;
  register uint32_t work = (mtime / 1000LLU) % SPD;
  static struct tm ret;
  
  /* Calculate HH:MM:SS */
  ret.tm_sec = work % 60;
  work /= 60;
  ret.tm_min = work % 60;
  ret.tm_hour = work / 60;
  
  /* Now do the day stuff */
  work = (mtime / 1000LLU) / SPD;
  
  /* 
   * 01.01.1970 was a thursday, so the day 
   * value in <work> is relative to thursday 
   */
  ret.tm_wday = (4 + work) % 7;
  
  /* Now count up the years since epoch */
  for(i = EPOCH;; i++)
  {
    register uint32_t k = isleap(i) ? 366 : 365;
    
    if(work >= k)
      work -= k;
    else
      break;
  }
  
  ret.tm_year = i - 1900;
  ret.tm_yday = work;
  ret.tm_mday = 1;
  
  /* 
   * If we're in a leap and we're beyond 28. february
   * we need to adjust everything by a offset of -1 
   */
  if(isleap(i) && (work > 58))
  {
    /* 29. february is a special case */
    if(work == 59) 
      ret.tm_mday = 2;
    
    work -= 1;
  }

  /* Find the month */
  for(i = 11; i && (dpm[i] > work); i--);

  ret.tm_mon = i;
  
  /* Day relative to month start */
  ret.tm_mday += work - dpm[i];
  
  return &ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
size_t timer_strftime(char *s, size_t max, const char *format, 
                      const struct tm *tm)
{
  char   c;
  size_t i = 0;
  
  while((c = *format++) && i < max)
  {
    if(c != '%')
    {
      s[i++] = c;
      continue;
    }
    
    c = *format++;
    
    switch(c)
    {
      case '%':
      {
        s[i++] = '%';
        break;
      }
      /* Day Sun - Mon */
      case 'a':
      {
        if(i + 3 < max)
          i += strlcpy(&s[i], timer_weekdays[tm->tm_wday], 4);
        
        break;
      }
      /* Day Sunday - Monday */
      case 'A':
      {
        size_t len = strlen(timer_weekdays[tm->tm_wday]);
        
        if(i + len < max)
          i += strlcpy(&s[i], timer_weekdays[tm->tm_wday], len + 1);
        
        break;
      }
      /* Month Jan - Dec */
      case 'b':
      {
        if(i + 3 < max)
        {
          s[i++] = timer_months[tm->tm_mon % 12][0];
          s[i++] = timer_months[tm->tm_mon % 12][1];
          s[i++] = timer_months[tm->tm_mon % 12][2];
          s[i++] = 0;
        }
        
        break;
      }
      /* Month January - December */
      case 'B':
      {
        size_t len = strlen(timer_months[tm->tm_mon]);
        
        if(i + len < max)
          i += strlcpy(&s[i], timer_months[tm->tm_mon], len + 1);
        
        break;
      }
      /* Month 01 - 12 */
      case 'm':
      {
        if(i + 2 < max)
        {
          s[i++] = ((tm->tm_mon + 1) / 10) + '0';
          s[i++] = ((tm->tm_mon + 1) % 10) + '0';
        }

        break;
      }
      /* Month 01 - 12 */
      case 'H':
      {
        if(i + 2 < max)
        {
          s[i++] = (tm->tm_hour / 10) + '0';
          s[i++] = (tm->tm_hour % 10) + '0';
        }
        
        break;
      }
      /* Day 01 - 31 */
      case 'd':
      {
        if(i + 2 < max)
        {
          s[i++] = (tm->tm_mday / 10) + '0';
          s[i++] = (tm->tm_mday % 10) + '0';
        }
        
        break;
      }
      /* Day 1 - 31 */
      case 'D':
      {
        if(i + 2 < max)
        {
          if(tm->tm_mday / 10)
            s[i++] = (tm->tm_mday / 10) + '0';
          
          s[i++] = (tm->tm_mday % 10) + '0';
        }
        
        break;
      }
      /* Minute 00 - 59 */
      case 'M':
      {
        if(i + 2 < max)
        {
          s[i++] = (tm->tm_min / 10) + '0';
          s[i++] = (tm->tm_min % 10) + '0';
        }
        
        break;
      }
      /* Second 00 - 59 */
      case 'S':
      {
        if(i + 2 < max)
        {
          s[i++] = (tm->tm_sec / 10) + '0';
          s[i++] = (tm->tm_sec % 10) + '0';
        }
        
        break;
      }
      /* Year 00 - 99 */
      case 'y':
      {
        uint32_t year = tm->tm_year % 100;
        
        if(i + 2 < max)
        {
          s[i++] = (year / 10) + '0';
          s[i++] = (year % 10) + '0';
        }
        
        break;
      }
      /* Year 1900 - ... */
      case 'Y':
      {
        uint32_t year = tm->tm_year + 1900;
        
        if(i + 4 < max)
        {
          s[i++] = (year / 1000) + '0';
          s[i++] = (year % 1000 / 100) + '0';
          s[i++] = (year % 100 / 10) + '0';
          s[i++] = (year % 10) + '0';
        }
        
        break;
      }
    }
  }

  if(i >= max)
    i = max - 1;
    
  s[i++] = '\0';
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Convert a struct tm to unixtime in miliseconds                             *
 * -------------------------------------------------------------------------- */
uint64_t timer_mktime(struct tm *tm)
{
  register time_t  day;
  register time_t  i;
  
  if(tm->tm_year < 70)
    return (uint64_t)-1LL;
  
  day = tm->tm_yday = dpm[tm->tm_mon] + tm->tm_mday - 1 +
        (isleap(tm->tm_year + 1900) & (tm->tm_mon > 1));
  
  for(i = 70; i < tm->tm_year; i++)
    day += 365 + isleap(i + 1900);
  
  tm->tm_wday = (day + 4) % 7;
  
  day *= 24;
  
/*  return (uint64_t)(((day + tm->tm_hour) * 3600) + (tm->tm_min * 60) + tm->tm_sec) * 1000LLU;*/
  return (uint64_t)mktime(tm) * 1000LLU;
}

/* -------------------------------------------------------------------------- *
 * Parse a time in HH:MM([.:]SS) format                                       *
 * -------------------------------------------------------------------------- */
uint64_t timer_parse_time(const char *t)
{
  struct tm     ret;
  unsigned long hour;
  unsigned long minute;
  unsigned long second;
  char         *p;
  
  if((hour = strtoul(t, &p, 10)) == ULONG_MAX)
    return (uint64_t)-1LL;
  
  if(*p++ != ':')
    return (uint64_t)-1LL;
  
  if((minute = strtoul(p, &p, 10)) == ULONG_MAX)
    return (uint64_t)-1LL;
  
  ret.tm_hour = hour % 24;
  ret.tm_min = minute % 60;
  ret.tm_sec = 0;
  
  if(*p == ':' || *p == '.')
  {
    p++;
  
    if((second = strtoul(p, NULL, 10)) != ULONG_MAX)
      ret.tm_sec = second % 60;
  }
    
  return ((ret.tm_hour * 3600) + (ret.tm_min * 60) + ret.tm_sec) * 1000LLU;
}
  
/* -------------------------------------------------------------------------- *
 * Parse a date in DD.MM(.YY(YY)) format                                       *
 * -------------------------------------------------------------------------- */
uint64_t timer_parse_date(const char *d)
{
  char         *p;
  struct tm     ret;
  unsigned long day;
  unsigned long month;
  unsigned long year;
  
  if((day = strtoul(d, &p, 10)) == ULONG_MAX)
    return (uint64_t)-1LL;
  
  if(*p++ != '.' || day == 0)
    return (uint64_t)-1LL;
  
  if((month = strtoul(p, &p, 10)) == ULONG_MAX)
    return (uint64_t)-1LL;
  
  ret.tm_mday = ((day) % 32);
  ret.tm_mon = (month - 1) % 12;
  ret.tm_year = timer_thisyear;
  
  if(*p == '.')
  {
    p++;
  
    if((year = strtoul(p, NULL, 10)) != ULONG_MAX)
    {
      if(year < 1000)
        ret.tm_year = year + 100;
      else
        ret.tm_year = year - 1900;
    }
  }
    
  ret.tm_wday = 0;
  ret.tm_yday = 0;
  ret.tm_isdst = 0;
  ret.tm_hour = 0;
  ret.tm_min = 0;
  ret.tm_sec = 0;
#ifndef WIN32
  ret.tm_gmtoff = 0;
  ret.tm_zone = 0;
#endif 

  return timer_mktime(&ret);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void timer_format_time(char **pptr, size_t *bptr, size_t n, 
                              int padding, int left, void *arg)
{ 
  char      *p;
  char       buf[9];
  struct tm *tm;
  
  tm = timer_gmtime(*(uint64_t *)arg);
  
  buf[0] = ((tm->tm_hour / 10) % 10) + '0';
  buf[1] = (tm->tm_hour % 10) + '0';
  buf[2] = ':';
  buf[3] = ((tm->tm_min / 10) % 10) + '0';
  buf[4] = (tm->tm_min % 10) + '0';
  buf[5] = ':';
  buf[6] = ((tm->tm_sec / 10) % 10) + '0';
  buf[7] = (tm->tm_sec % 10) + '0';
  buf[8] = '\0';
  
  p = buf;
  
  while(*p && *bptr < n)
  {
    *(*pptr)++ = *p++;
    (*bptr)++;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void timer_format_time_short(char **pptr, size_t *bptr, size_t n, 
                                    int padding, int left, void *arg)
{ 
  char      *p;
  char       buf[6];
  struct tm *tm;
  
  if(*(int64_t *)arg == -1LL)
  {
    buf[0] = ' ';
    buf[1] = ' ';
    buf[2] = ' ';
    buf[3] = ' ';
    buf[4] = ' ';
  }
  else
  {
    tm = timer_gmtime(*(uint64_t *)arg);
    
    buf[0] = ((tm->tm_hour / 10) % 10) + '0';
    buf[1] = (tm->tm_hour % 10) + '0';
    buf[2] = ':';
    buf[3] = ((tm->tm_min / 10) % 10) + '0';
    buf[4] = (tm->tm_min % 10) + '0';
  }
  
  buf[5] = '\0';
  
  p = buf;
  
  while(*p && *bptr < n)
  {
    *(*pptr)++ = *p++;
    (*bptr)++;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void timer_format_date(char **pptr, size_t *bptr, size_t n, 
                              int padding, int left, void *arg)
{ 
  char      *p;
  char       buf[11];
  struct tm *tm;
  uint32_t   i = 0;
  
  if(*(int64_t *)arg != -1LL)
  {
    tm = timer_gmtime(*(uint64_t *)arg);
    
    if((tm->tm_mday + 1) / 10)
      buf[i++] = (((tm->tm_mday + 1) / 10) % 10) + '0';
    
    buf[i++] = ((tm->tm_mday + 1) % 10) + '0';
    buf[i++] = '.';
    
    if((tm->tm_mon + 1) / 10)
      buf[i++] = (((tm->tm_mon + 1) / 10) % 10) + '0';
    
    buf[i++] = ((tm->tm_mon + 1) % 10) + '0';
    buf[i++] = '.';
    buf[i++] = ((((tm->tm_year + 1900)) / 1000) % 10) + '0';
    buf[i++] = ((((tm->tm_year + 1900)) / 100) % 10) + '0';
    buf[i++] = ((((tm->tm_year + 1900)) / 10) % 10) + '0';
    buf[i++] = (((tm->tm_year + 1900)) % 10) + '0';
  }
  
  padding -= i;
  
  buf[i++] = '\0';
  
  p = buf;
  
  if(left && padding)
  {
    while(*bptr < n && padding > 0)
    {
      *(*pptr)++ = ' ';
      (*bptr)++;
      padding--;
    }
  }
  
  while(*p && *bptr < n)
  {
    *(*pptr)++ = *p++;
    (*bptr)++;
  }

  if(padding > 0)
  {
    while(*bptr < n && padding)
    {
      *(*pptr)++ = ' ';
      (*bptr)++;
      padding--;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Initialize the timer code.                                                 *
 * -------------------------------------------------------------------------- */
void timer_init(void)
{
  timer_log = log_source_register("timer");
  
  /* Update system time */
  timer_update();

  /* Zero timer list */
  dlink_list_zero(&timer_list);
  dlink_list_zero(&timer_shifts);
  
  /* Zero id */
  timer_id = 0;
  timer_dirty = 0;
  
  /* Allocate timer heap and setup garbage collect timer */
  mem_static_create(&timer_heap, sizeof(struct timer), TIMER_BLOCK_SIZE);
  mem_static_note(&timer_heap, "timer heap");

  str_register('T', timer_format_time);
  str_register('t', timer_format_time_short);
  str_register('D', timer_format_date);
  
  log(timer_log, L_status, "Initialized [timer] module.");
}

/* -------------------------------------------------------------------------- *
 * Shutdown the timer code.                                                   *
 * -------------------------------------------------------------------------- */
void timer_shutdown(void)
{
  struct timer *tptr;
  struct node  *next;
  struct node  *nptr;
  
  timer_dirty = 1;
  
  timer_collect();

  
  log(timer_log, L_status, "Shutting down [timer] module...");
  
  str_unregister('T');
  str_unregister('t');
  str_unregister('D');
  
  /* Cancel all timers */
  dlink_foreach_safe(&timer_list, tptr, next)
    timer_remove(tptr);
  
  dlink_foreach_safe(&timer_shifts, nptr, next)
    dlink_node_free(nptr);

  mem_static_destroy(&timer_heap);  
  
  log_source_unregister(timer_log);
}

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
int timer_collect(void)
{
  struct timer *tptr;
  struct timer *next;
  
  /* Free all timer blocks with a zero refcount */
  dlink_foreach_safe(&timer_list, tptr, next)
    if(tptr->refcount == 0)
      timer_remove(tptr);
  
  /* Collect garbage on timer_heap */
  mem_static_collect(&timer_heap);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Convert from timeval to miliseconds.                                       *
 *                                                                            *
 * <src>            - pointer to timeval to convert                           *
 * <dst>            - pointer to 64bit integer to store result                *
 * -------------------------------------------------------------------------- */
void timer_to_msec(uint64_t *dst, struct timeval *src)
{
  *dst = ((uint64_t)src->tv_sec * 1000LLU) +    
         ((uint32_t)src->tv_usec / 1000LLU);  
}

/* -------------------------------------------------------------------------- *
 * Convert from miliseconds to timeval.                                       *
 *                                                                            *
 * <src>            - pointer to 64bit integer to convert                     *
 * <dst>            - pointer to timeval to store result                      *
 * -------------------------------------------------------------------------- */
void timer_to_timeval(struct timeval *dst, uint64_t *src)
{
  dst->tv_sec = (long)((*src) / 1000);
  dst->tv_usec = (long)(((*src) % 1000) * 1000);
}

/* -------------------------------------------------------------------------- *
 * Update the system time.                                                    *
 * Returns -1 if the underlying systemcall fails and 0 on success.            *
 * -------------------------------------------------------------------------- */
int timer_update(void)
{
  struct timeval  tv;
  struct timezone tz;
  
  /* This system call gives us usecs since epoch */
  if(syscall_gettimeofday(&tv, &tz) == -1)
    return -1;
  
  /* Set old time */
  timer_otime = timer_mtime;
  
  /* Time since epoch in msecs */
  timer_to_msec(&timer_mtime, &tv);
  
  timer_mtime += timer_offset;
  
  /* Time since epoch in secs */
  timer_systime = timer_mtime / 1000LLU;
  timer_loctime = timer_systime + (tz.tz_minuteswest * 60);
  
  /* Update calendar time */
  timer_dtime = *timer_gmtime(timer_mtime);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Add a timer shifting callback                                              *
 * -------------------------------------------------------------------------- */
void timer_shift_register(timer_shift_cb *shift_cb)
{
  struct node *nptr;
  
  nptr = dlink_node_new();
  
  dlink_add_tail(&timer_shifts, nptr, shift_cb);
}

/* -------------------------------------------------------------------------- *
 * Remove a timer shifting callback                                           *
 * -------------------------------------------------------------------------- */
void timer_shift_unregister(timer_shift_cb *shift_cb)
{
  struct node *nptr;
  
  nptr = dlink_find_delete(&timer_shifts, shift_cb);
  
  dlink_node_free(nptr);
}

/* -------------------------------------------------------------------------- *
 * Add the specified offset to all timer deadlines.                           *
 *                                                                            *
 * <delta>          - offset to be added to deadlines in milliseconds         *
 * -------------------------------------------------------------------------- */
void timer_shift(int64_t delta)
{
  struct timer *timer;
  struct node  *node;
  
  /* Nothing left to say :P */
  dlink_foreach(&timer_list, node)
  {
    timer = (struct timer *)node;
    
    timer->deadline += delta;
  }
  
  dlink_foreach(&timer_shifts, node)
  {
    if(node->data)
      ((timer_shift_cb *)node->data)(delta);
  }
  
  log(timer_log, L_verbose, "Shifting timers by %lli milliseconds...",
      delta);
}

/* -------------------------------------------------------------------------- *
 * See if we have clock drift, modify deadlines if necessary.                 *
 *                                                                            *
 * <waited>         - how long the select()/poll() lasted.                    *
 * -------------------------------------------------------------------------- */
void timer_drift(int64_t waited)
{
  uint64_t expected;
  int64_t drift;
  
  if(timer_otime)
  {
    /* 
     * We expect the mtime to be 
     * timer_otime + time spent at poll/select
     */
    expected = timer_otime + waited;
    
    /*
     * Now calculate how much the systime drifted 
     */
    drift = timer_mtime - expected;
    
    /*
     * If the drift exceeded the limit we warn about it
     * and we shift all the timers by the drift delta.
     */
    if(drift < -TIMER_MAX_DRIFT || drift > TIMER_MAX_DRIFT)
    {
      log(timer_log, L_warning, 
          "Timer drifted %llimsecs, recalcing deadlines...", drift);
      
      timer_shift(drift);
    }
  }
}

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
struct timer *timer_start(void *callback, uint64_t interval, ...)
{
  va_list args;
  struct timer *timer;
  
  /* Allocate timer block and add it to the list */
  timer = mem_static_alloc(&timer_heap);
  
  if(timer == NULL)
    return NULL;
  
  dlink_add_tail(&timer_list, &timer->node, timer);  

  /* Externally initialised stuff */
  timer->interval = interval;
  timer->deadline = timer_mtime + interval;
  timer->callback = callback;
  
  /* There can be up to 4 user-supplied arguments
     which are passed to the callback */
  va_start(args, interval);
  
  timer->args[0] = va_arg(args, void *);
  timer->args[1] = va_arg(args, void *);
  timer->args[2] = va_arg(args, void *);
  timer->args[3] = va_arg(args, void *);

  va_end(args);
  
  /* Internally initialised stuff */
  timer->id = timer_id++;
  timer->refcount = 1;
  
  /* Be verbose */
  debug(timer_log, "New timer #%u: interval = %llu",
        timer->id, timer->interval);
  
  return timer;
}

/* -------------------------------------------------------------------------- *
 * Stop and remove a timer.                                                   *
 *                                                                            *
 * <timer>           - pointer to a timer structure returned by timer_start() *
 *                     or timer_find()                                        *
 * -------------------------------------------------------------------------- */
void timer_remove(struct timer *timer)
{
  /* Be verbose */
/*  debug(timer_log, "Cancelled timer #%u: interval = %llu",
        timer->id, timer->interval);*/
  
  /* Remove from timer list */
  dlink_delete(&timer_list, &timer->node);
  
  /* Free block */
  mem_static_free(&timer_heap, timer);
}

/* -------------------------------------------------------------------------- *
 * Find a timer by callback and 1st userarg.                                  *
 *                                                                            *
 * <callback>        - the callback supplied on timer_start()                 *
 * <userarg>         - the userarg supplied on timer_start()                  *
 *                                                                            *
 * Returns NULL if not found.                                                 *
 * -------------------------------------------------------------------------- */
struct timer *timer_find(void *callback, ...)
{
  va_list       args;
  void         *arg;
  struct timer *timer;

  /* Get userarg */
  va_start(args, callback);
  arg = va_arg(args, void *);
  va_end(args);

  /* Loop through timer list */
  dlink_foreach(&timer_list, timer)
  {
    /* Compare callback and userarg pointer */
    if(timer->callback == callback && timer->args[0] == arg)
    {
      /* Be verbose */
      debug(timer_log, "Found timer: id = #%u, callback = %p, "
                       "interval = %llu, deadline = %llu, "
                       "args = [ %p, %p, %p, %p ]",
            timer->id, timer->callback, timer->interval, timer->deadline,
            timer->args[0], timer->args[1], timer->args[2], timer->args[3]);

      return timer;
    }
  }

  debug(timer_log, "Did not find timer: callback = %p, arg = %p",
        callback, arg);
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a timer by id                                                         *
 * -------------------------------------------------------------------------- */
struct timer *timer_find_id(uint32_t id)
{
  struct timer *timer;

  /* Loop through timer list */
  dlink_foreach(&timer_list, timer)
  {
    if(timer->id == id)
      return timer;    
  }

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Write a description to the timer structure for timer(s)_dump()             *
 *                                                                            *
 * <timer>           - pointer to a timer structure returned by timer_start() *
 *                     or timer_find()                                        *
 * <format>          - format string                                          *
 *                   - your args                                              *
 * -------------------------------------------------------------------------- */
void timer_vnote(struct timer *timer, const char *format, va_list args)
{
  /* Write the note */
  if(timer)
  {
    vsnprintf(timer->note, sizeof(timer->note), format, args);

    debug(timer_log, "Denoting timer #%u: %s",
          timer->id, timer->note);
  }
}

void timer_note(struct timer *timer, const char *format, ...)
{
  va_list args;
  
  /* Write the note */
  if(timer)
  {
    va_start(args, format);
    timer_vnote(timer, format, args);
    va_end(args);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct timer *timer_pop(struct timer *tptr)
{  
  if(tptr)
  {
    if(!tptr->refcount)
      debug(timer_log, "Poping deprecated timer #%u",
          tptr->id);

    tptr->refcount++;
  }
  
  return tptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void timer_cancel(struct timer **tptrptr)
{
  if(*tptrptr)
  {
    (*tptrptr)->refcount = 0;
    
    timer_remove(*tptrptr);
    
    (*tptrptr) = NULL;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct timer *timer_push(struct timer **tptrptr)
{
  if(*tptrptr)
  {
    if(!(*tptrptr)->refcount)
    {
      debug(timer_log, "Trying to push deprecated timer #%u",
          (*tptrptr)->id);
    }
    else
    {
      --(*tptrptr)->refcount;
    }

    (*tptrptr) = NULL;
  }
  
  return *tptrptr;
}

/* -------------------------------------------------------------------------- *
 * Run pending timers.                                                        *
 *                                                                            *
 * Will return number of timers runned.                                       *
 * -------------------------------------------------------------------------- */
int timer_run(void)
{
  struct timer *timer;
  struct timer *next;
  int64_t       delta;
  int           ret = 0;

  /* Safely walk through timer list as we may remove some timer */
  dlink_foreach_safe(&timer_list, timer, next)
  {
    if(!timer->refcount)
    {
      timer_remove(timer);
      continue;
    }

    /* Has this timer expired? */
    if(timer->deadline <= timer_mtime)
    {
      delta = timer_mtime - timer->deadline;
      
      /* Warn about timer deltas */
      if(delta >= TIMER_WARN_DELTA && delta <= -TIMER_WARN_DELTA)
      {
        debug(timer_log,
              "Timer delta for timer #%u exceeded by %llimsecs",
              timer->id, delta);
      }
      
      if(timer->callback(timer->args[0], timer->args[1],
                         timer->args[2], timer->args[3]))
      {
        timer->refcount = 0;
      }
      else
      {
        /* Else schedule it again */
        timer->deadline += timer->interval;
      }
      
      /* Count the callbacks called */
      ret++;
    }
  }

  return ret;
}

/* -------------------------------------------------------------------------- *
 * Get the time at which the next timer will expire.                          *
 * Return 0LLU when there is no timer.                                        *
 * -------------------------------------------------------------------------- */
uint64_t timer_deadline(void)
{
  struct timer *timer;
  struct node  *next;
  uint64_t      deadline = 0LLU;

  dlink_foreach_safe(&timer_list, timer, next)
  {
    if(!timer->refcount)
    {
      timer_remove(timer);
      continue;
    }
    
    /* First or lower deadline, update final deadline */
    if(deadline == 0LLU || timer->deadline < deadline)
      deadline = timer->deadline;
  }
  
  return deadline;
}

/* -------------------------------------------------------------------------- *
 * Get the optimal timeout for select()/poll()                                *
 * (The time until the lowest deadline)                                       *
 * -------------------------------------------------------------------------- */
int64_t *timer_timeout(void)
{
  uint64_t       deadline;
  static int64_t timeout;
  
  deadline = timer_deadline();
  
  if(deadline == 0)
    return NULL;
  
  timeout = deadline - timer_mtime;

  if(timeout < 0LL)
  {
    debug(timer_log, "Negative timeout value, setting to 0msecs...");
    timeout = 0LL;
  }
  
  return &timeout;
}

/* -------------------------------------------------------------------------- *
 * Dump timers.                                                               *
 * -------------------------------------------------------------------------- */
void timer_dump(struct timer *tptr)
{
  if(tptr == NULL)
  {
    dump(timer_log, "[============== timer summary ===============]");
    
    dlink_foreach(&timer_list, tptr)
      dump(timer_log, " #%03u: [%u] %10llu %s",
           tptr->id, tptr->refcount,
           tptr->interval, tptr->note);
    
    dump(timer_log, "[=========== end of timer summary ===========]");
  }
  else
  {
    dump(timer_log, "[============== timer dump ===============]");
    
    dump(timer_log, "         id: #%u", tptr->id);
    dump(timer_log, "   refcount: %u", tptr->refcount);
    dump(timer_log, "       note: %s", tptr->note);
    dump(timer_log, "   callback: %p", tptr->callback);
    dump(timer_log, "       args: %p, %p, %p, %p",
         tptr->args[0], tptr->args[1], tptr->args[2], tptr->args[3]);
    dump(timer_log, "   interval: %llu", tptr->interval);
    dump(timer_log, "   deadline: %llu (%lli remaining)", tptr->deadline, tptr->deadline - timer_mtime);
    
    dump(timer_log, "[=========== end of timer dump ===========]");
  }
}
