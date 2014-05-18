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
 * $Id: str.c,v 1.44 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE
#undef USE_IA32_LINUX_INLINE

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/defs.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char     str_hexchars[] = "0123456789abcdef";
str_format_cb *str_table[64];

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void str_init(void)
{
  size_t i;
  
  for(i = 0; i < 64; i++)
    str_table[i] = NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void str_shutdown(void)
{
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void str_register(char c, str_format_cb *cb)
{
  if(isalpha(c))
  {
    str_table[((uint32_t)c) - 0x40] = cb;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void str_unregister(char c)
{
  if(isalpha(c))
  {
    str_table[((uint32_t)c) - 0x40] = NULL;
  }
}

/* -------------------------------------------------------------------------- *
 * Convert long long (signed 64bit) to string                                 *
 * -------------------------------------------------------------------------- */
static inline size_t lltoa(char *s, int64_t ll)
{
  char bbuf[32];
  size_t bi = 0;
  char *p = s;
  
  if(ll < 0)
  {
    *p++ = '-';
    ll = - ll;
  }
  
  do
  {
    bbuf[bi++] = '0' + (ll % 10);
    ll /= 10;
  }
  while(ll);
  
  while(bi) 
    *p++ = bbuf[--bi];
  
  *p = '\0';
                
  return p - s;
}

/* -------------------------------------------------------------------------- *
 * Convert unsigned long long (unsigned 64bit) to string                      *
 * -------------------------------------------------------------------------- */
static inline size_t llutoa(char *s, uint64_t llu)
{
  char bbuf[32];
  size_t bi = 0;
  char *p = s;
  
  do
  {
    bbuf[bi++] = '0' + (llu % 10);
    llu /= 10;
  }
  while(llu);
  
  while(bi) 
    *p++ = bbuf[--bi];
  
  *p = '\0';
                
  return p - s;
}

/* -------------------------------------------------------------------------- *
 * Convert long (signed 32bit) to string                                      *
 * -------------------------------------------------------------------------- */
static inline size_t ltoa(char *s, long l)
{
  char bbuf[32];
  size_t bi = 0;
  char *p = s;
  
  if(l < 0)
  {
    *p++ = '-';
    l = - l;
  }
  
  do
  {
    bbuf[bi++] = '0' + (l % 10);
    l /= 10;
  }
  while(l);
  
  while(bi) 
    *p++ = bbuf[--bi];
  
  *p = '\0';
                
  return p - s;
}

/* -------------------------------------------------------------------------- *
 * Convert unsigned long (unsigned 32bit) to string                           *
 * -------------------------------------------------------------------------- */
static inline size_t lutoa(char *s, unsigned long l)
{
  char bbuf[32];
  size_t bi = 0;
  char *p = s;

  do
  {
    bbuf[bi++] = '0' + (l % 10);
    l /= 10;
  }
  while(l);
  
  while(bi) 
    *p++ = bbuf[--bi];
  
  *p = '\0';
                
  return p - s;
}

/* -------------------------------------------------------------------------- *
 * Converts a pointer to a hex string. returns length of the string           *
 * -------------------------------------------------------------------------- */
static inline unsigned int ptoa(char *buf, void *i) {

  register char *p = buf;
  register int n = 0; /* buffer index */
  register size_t v = (size_t)i;

  if(i == NULL) {
    strcpy(buf, "(nil)");
    n = 5;
  } else {
    p[n++] = '0';
    p[n++] = 'x';
    p[n++] = str_hexchars[(v & 0xf0000000) >> 0x1c];
    p[n++] = str_hexchars[(v & 0x0f000000) >> 0x18];
    p[n++] = str_hexchars[(v & 0x00f00000) >> 0x14];
    p[n++] = str_hexchars[(v & 0x000f0000) >> 0x10];
    p[n++] = str_hexchars[(v & 0x0000f000) >> 0x0c];
    p[n++] = str_hexchars[(v & 0x00000f00) >> 0x08];
    p[n++] = str_hexchars[(v & 0x000000f0) >> 0x04];
    p[n++] = str_hexchars[(v & 0x0000000f)];
    p[n] = '\0';
  }

  return n;
}

/* -------------------------------------------------------------------------- *
 * Formatted print to string                                                  *
 *                                                                            *
 * Supported formats are currently string, integer, unsigned integer          *
 * with left- and right-aligned padding and pointer (without align)           *
 *                                                                            *
 *   %s   - just a string                                                     *
 *   %5s  - string padded to 5 chars (right aligned)                          *
 *   %-5s - string padded to 5 chars (left aligned)                           *
 *   %u   - unsigned integer                                                  *
 *   %5u  - unsigned integer, padded to 5 chars (right aligned)               *
 *   %-5u - unsigned integer, padded to 5 chars (left aligned)                *
 *   %i   - signed integer                                                    *
 *   %5i  - signed integer, padded to 5 chars (right aligned)                 *
 *   %-5i - signed integer, padded to 5 chars (left aligned)                  *
 *   %p   - pointer in hexadecimal notation                                   *
 *                                                                            *
 * -------------------------------------------------------------------------- */
int str_vsnprintf(char *str, size_t n, const char *format, va_list args) 
{  
  register const char *f     = format;
  int                  bytes = 0;
  char                *p     = str;
  register char        c;
  register int         longlev = 0;
  
  if(n-- == 0) return 0;
  
  while((c = *f++) && bytes < n) 
  {
    if(c == '%') 
    {
      register int  left    = 0;
      register int  padding = 0;
      register char pad     = ' ';
      
      c = *f++;
      
      /* a '-' and padding means left align */
      if(c == '-') 
      {
        left = 1;
        c = *f++;
      }
          
      /* a '0' means pad with zeroes */
      if(c == '0') 
      {
        pad = '0';
        c = *f++;
      }
      
      /* get the padding value if present */
      while(c >= '0' && c <= '9') 
      {
        padding *= 10;
        padding += c - '0';
        c = *f++;
      }
      
      if(c == 'l')
      {
        longlev++;
        c = *f++;
      }

      if(c == 'l')
      {
        longlev++;
        c = *f++;
      }

      /* a string, probably with padding */
      if(c == 's') 
      {
        register const char *p1  = va_arg(args, const char *);
        register size_t      len = (p1 ? strlen(p1) : 6);
        
        /* if left aligned, do padding now */
        if(!left && len < padding) 
        {
          padding -= len;
          
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        /* copy the string */
        if(p1 == NULL) p1 = "(null)";
        
        while(*p1 && bytes < n) 
        {
          *p++ = *p1++;
          bytes++;
          padding--;
        }
        
        /* if right aligned, do padding now */
        if(left && padding > 0) 
        {
          while(padding-- && bytes < n) 
          {
             *p++ = ' ';
            bytes++;
          }
        }
        
        continue;
      }

      /* an unsigned long long, probably with padding */
      if(longlev == 2 && c == 'u') 
      {
        char         is[22];
        register int len = llutoa(is, va_arg(args, uint64_t));
        register int idx = 0;
              
        /* if right aligned, do padding now */
        if(!left && len < padding) 
        {
          padding -= len;
          
          while(padding-- && bytes < n) 
          {
            *p++ = pad;
            bytes++;
          }
        }
        
        /* copy the string */
        while(len-- && bytes < n) 
        {
          *p++ = is[idx++];
          bytes++;
          padding--;
        }
        
        /* if left aligned, do padding now */
        if(left && padding > 0) 
        {
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }

        longlev = 0;
        
        continue;
      }
      
      /* an unsigned long, probably with padding */
      if(c == 'u') 
      {
        char         is[11];
        register int len = lutoa(is, va_arg(args, uint32_t));
        register int idx = 0;
              
        /* if right aligned, do padding now */
        if(!left && len < padding) 
        {
          padding -= len;
          
          while(padding-- && bytes < n) 
          {
            *p++ = pad;
            bytes++;
          }
        }
        
        /* copy the string */
        while(len-- && bytes < n) 
        {
          *p++ = is[idx++];
          bytes++;
          padding--;
        }
        
        /* if left aligned, do padding now */
        if(left && padding > 0) 
        {
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        longlev = 0;
        
        continue;
      }
      
      /* a signed int, probably with padding */
      if(longlev == 2 && (c == 'i' || c == 'd'))
      {
        char         is[24];
        register int len = lltoa(is, va_arg(args, int64_t));
        register int idx = 0;
        
        /* if right aligned, do padding now */
        if(!left && len < padding) 
        {
          padding -= len;
          
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        /* copy the string */
        while(len-- && bytes < n) 
        {
          *p++ = is[idx++];
          bytes++;
          padding--;
        }
        
        /* if left aligned, do padding now */
        if(left && padding > 0) 
        {
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        longlev = 0;
        
        continue;
      }

      /* a signed int, probably with padding */
      if(c == 'i' || c == 'd') 
      {
        char         is[12];
        register int len = ltoa(is, va_arg(args, int32_t));
        register int idx = 0;
        
        /* if right aligned, do padding now */
        if(!left && len < padding) 
        {
          padding -= len;
          
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        /* copy the string */
        while(len-- && bytes < n) 
        {
          *p++ = is[idx++];
          bytes++;
          padding--;
        }
        
        /* if left aligned, do padding now */
        if(left && padding > 0) 
        {
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        longlev = 0;
        
        continue;
      }
      
      /* hex int */
      if(c == 'x')
      {
        char         is[12];
        register int len = ptoa(is, va_arg(args, void *));
        register int idx = 0;
        
        /* if right aligned, do padding now */
        if(!left && len < padding) 
        {
          padding -= len;
          
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        /* copy the string */
        while(len-- && bytes < n) 
        {
          *p++ = is[idx++];
          bytes++;
          padding--;
        }
        
        /* if left aligned, do padding now */
        if(left && padding > 0) 
        {
          while(padding-- && bytes < n) 
          {
            *p++ = ' ';
            bytes++;
          }
        }
        
        longlev = 0;
        
        continue;
      }
      
      /* a pointer, no padding - useful for debugging*/
      if(c == 'p') 
      {
        char              ps[11];
        register size_t   len = ptoa(ps, va_arg(args, void *));
        register uint32_t idx = 0;
              
        while(len-- && bytes < n) 
        {
          *p++ = ps[idx++];
          bytes++;
        }
        
        continue;
      }
      
      /* a char */
      if(c == 'c') 
      {
        char cs = va_arg(args, int);
        
        if(bytes < n) 
        {
          *p++ = cs;
          bytes++;
        }
        
        continue;
      }
      
      if(isalpha(c) && str_table[((uint32_t)c) - 0x40])
      {
        str_table[((uint32_t)c) - 0x40](&p, (size_t *)&bytes, n, padding, left, va_arg(args, void *));
        continue;
      }
    }
    
    *p++ = c;
    bytes++;
  }
  
  *p = '\0';
  
  return bytes;
}

int str_vsprintf(char *str, const char *format, va_list args)
{
  return str_vsnprintf(str, (uint32_t)(int32_t)-1, format, args);
}

/* -------------------------------------------------------------------------- *
 * Get string length.                                                         *
 * -------------------------------------------------------------------------- */
#ifdef __i386__
size_t strlen(const char *s)
{
  size_t len = 0;

  for(EVER)
  {
    /* 32-bit unroll for faster prefetch */
    if(!s[len]) return len; len++;
    if(!s[len]) return len; len++;
    if(!s[len]) return len; len++;
    if(!s[len]) return len; len++;
  }
}
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#if 0
char *strchr(const char *s, int c)
{
  size_t i = 0;

  for(EVER)
  {
    /* 32-bit unroll for faster prefetch */
    if(!s[i]) break; if(s[i] == c) return (char *)&s[i]; i++;
    if(!s[i]) break; if(s[i] == c) return (char *)&s[i]; i++;
    if(!s[i]) break; if(s[i] == c) return (char *)&s[i]; i++;
    if(!s[i]) break; if(s[i] == c) return (char *)&s[i]; i++;
  }
  
  return NULL;
}
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * Get first occurance of char <c> in string <s>                              *
 * -------------------------------------------------------------------------- */
#ifdef __i386__

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *strcat(char *d, const char *s)
{
  size_t i;
  
  i = 0;
  
  for(EVER)
  {
    if(d[i] == '\0') break; i++;
    if(d[i] == '\0') break; i++;
    if(d[i] == '\0') break; i++;
    if(d[i] == '\0') break; i++;
  }
  
  for(EVER)
  {
    if(!(d[i] = *s++)) break; i++;
    if(!(d[i] = *s++)) break; i++;
    if(!(d[i] = *s++)) break; i++;
    if(!(d[i] = *s++)) break; i++;
  }

  return d;
}
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * Copy string from <s> to <d>.                                               *
 * -------------------------------------------------------------------------- */
#if 0
size_t strcpy(char *d, const char *s)
{
  size_t i = 0;
  
  for(EVER)
  {
    if(!(d[i] = s[i])) break; i++;
    if(!(d[i] = s[i])) break; i++;
    if(!(d[i] = s[i])) break; i++;
    if(!(d[i] = s[i])) break; i++;
  }
  
  d[i] = '\0';
  
  return i;
}
#endif

/* -------------------------------------------------------------------------- *
 * Copy string from <s> to <d>. Write max <n> bytes to <d> and always         *
 * null-terminate it. Returns new string length of <d>.                       *
 * -------------------------------------------------------------------------- */
#ifndef HAVE_STRLCPY
size_t strlcpy(char *d, const char *s, size_t n)
{
  size_t i = 0;
  
  if(n == 0)
    return 0;
  
  if(--n > 0) 
  {
    for(EVER)
    {
      if(!(d[i] = s[i])) break; if(++i == n) break;
      if(!(d[i] = s[i])) break; if(++i == n) break;
      if(!(d[i] = s[i])) break; if(++i == n) break;
      if(!(d[i] = s[i])) break; if(++i == n) break;
    }
  }
  
  d[i] = '\0';
  
  return i;
}
#endif /* HAVE_STRLCPY */

/* -------------------------------------------------------------------------- *
 * Append string <src> to <dst>. Don't let <dst> be bigger than <siz> bytes   *
 * and always null-terminate. Returns new string length of <dst>              *
 * -------------------------------------------------------------------------- */
#ifndef HAVE_STRLCAT
size_t strlcat(char *d, const char *s, size_t n)
{
  size_t i = 0;
  
  if(n == 0)
    return 0;
  
  if(--n > 0)
  {
    for(EVER)
    {
      if(!d[i]) break; if(++i == n) break;
      if(!d[i]) break; if(++i == n) break;
      if(!d[i]) break; if(++i == n) break;
      if(!d[i]) break; if(++i == n) break;
    }    
  }
  
  d[i] = '\0';
  
  if(n > i)
  {
    for(EVER)
    {
      if(!(d[i] = *s++)) break; if(++i == n) break;
      if(!(d[i] = *s++)) break; if(++i == n) break;
      if(!(d[i] = *s++)) break; if(++i == n) break;
      if(!(d[i] = *s++)) break; if(++i == n) break;
    }
    
    d[i] = '\0';
  }
  
  return i;
}
#endif /* HAVE_STRLCAT */
/* -------------------------------------------------------------------------- *
 * Compare string.                                                            *
 * -------------------------------------------------------------------------- */
#ifdef __i386__
int strcmp(const char *s1, const char *s2)
{
  size_t i = 0;
  
  for(EVER)
  {
    if(s1[i] != s2[i]) break; if(!s1[i]) break; i++;
    if(s1[i] != s2[i]) break; if(!s1[i]) break; i++;
    if(s1[i] != s2[i]) break; if(!s1[i]) break; i++;
    if(s1[i] != s2[i]) break; if(!s1[i]) break; i++;
  }
  
  return ((int)(unsigned int)(unsigned char)s1[i]) -
         ((int)(unsigned int)(unsigned char)s2[i]);
}
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * Compare string.                                                            *
 * -------------------------------------------------------------------------- */
int stricmp(const char *s1, const char *s2)
{
  size_t i = 0;
  
  for(EVER)
  {
    if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
    if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
    if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
    if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
  }
  
  return ((int)(unsigned int)(unsigned char)tolower(s1[i])) -
         ((int)(unsigned int)(unsigned char)tolower(s2[i]));
}

/* -------------------------------------------------------------------------- *
 * Compare string, abort after <n> chars.                                     *
 * -------------------------------------------------------------------------- */
#ifdef __i386__
/*int strncmp(const char *s1, const char *s2, size_t n)
{
  size_t i = 0;
  
  if(n == 0)
    return 0;
  
  for(EVER)
  {
    if(s1[i] != s2[i]) break; if(i == n) return 0; if(!s1[i]) break; i++;
    if(s1[i] != s2[i]) break; if(i == n) return 0; if(!s1[i]) break; i++;
    if(s1[i] != s2[i]) break; if(i == n) return 0; if(!s1[i]) break; i++;
    if(s1[i] != s2[i]) break; if(i == n) return 0; if(!s1[i]) break; i++;
  }
  
  return ((int)(unsigned int)(unsigned char)s1[i]) -
         ((int)(unsigned int)(unsigned char)s2[i]);
}*/
#endif /* __i386__ */
/* -------------------------------------------------------------------------- *
 * Compare string, abort after <n> chars.                                     *
 * -------------------------------------------------------------------------- */
int strnicmp(const char *s1, const char *s2, size_t n)
{
  size_t i = 0;
  
  if(n == 0)
    return 0;
  
  for(EVER)
  {
    if(i == n) return 0; if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
    if(i == n) return 0; if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
    if(i == n) return 0; if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
    if(i == n) return 0; if(tolower(s1[i]) != tolower(s2[i])) break; if(!s1[i]) break; i++;
  }
  
  return ((int)(unsigned int)(unsigned char)tolower(s1[i])) -
         ((int)(unsigned int)(unsigned char)tolower(s2[i]));
}

/* -------------------------------------------------------------------------- *
 * Formatted print to string                                                  *
 * -------------------------------------------------------------------------- */
int str_snprintf(char *str, size_t n, const char *format, ...)
{
  int ret;
  
  va_list args;
  
  va_start(args, format);
  
  ret = vsnprintf(str, n, format, args);
  
  va_end(args);
  
  return ret;
}

int str_sprintf(char *str, const char *format, ...)
{
  int ret;
  
  va_list args;
  
  va_start(args, format);
  
  ret = vsnprintf(str, (uint32_t)(int32_t)-1, format, args);
  
  va_end(args);
  
  return ret;
}

/*int sprintf(char *str, const char *format, ...)
{
  int ret;
  
  va_list args;
  
  va_start(args, format);
  
  ret = vsnprintf(str, (uint32_t)(int32_t)-1, format, args);
  
  va_end(args);
  
  return ret;
}*/

/* -------------------------------------------------------------------------- *
 * Converts a string to a signed int.                                         *
 * -------------------------------------------------------------------------- */
int atoi(const char *s)
{
#define ISNUM(c) ((c) >= '0' && (c) <= '9')
  register uint32_t i = 0;
  register uint32_t sign = 0;
  register const uint8_t *p = s;
  
  /* Conversion starts at the first numeric character or sign. */
  while(*p && !ISNUM(*p) && *p != '-') p++;
  
  /* 
   * If we got a sign, set a flag.
   * This will negate the value before return.
   */
  if(*p == '-')
  {
    sign++;
    p++;
  }
  
  /* Don't care when 'u' overflows (Bug?) */
  while(ISNUM(*p))
  {
    i *= 10;
    i += *p++ - '0';
  }
  
  /* Return according to sign */
  if(sign)
    return - i;
  else
    return i;
  
#undef ISNUM
}

/* -------------------------------------------------------------------------- *
 * Splits a string into tokens.                                               *
 *                                                                            *
 * delimiters are hardcoded to [ \t]                                          *
 * irc style: a ':' at token start will stop tokenizing                       *
 *                                                                            *
 *                                                                            *
 *                 - string to be tokenized.                                  *
 *                   '\0' will be written to whitespace                       *
 *                                                                            *
 * v[]             - will contain pointers to tokens, must be at least        *
 *                   (maxtok + 1) * sizeof(char *) in size.                   *
 *                                                                            *
 * v[return value] - will be NULL                                             *
 *                                                                            *
 * return value will not be bigger than maxtok                                *
 * -------------------------------------------------------------------------- */
size_t strtokenize(char *s, char **v, size_t maxtok)
{
  size_t c = 0;
  
  for(EVER)
  {
    /* Skip and zero whitespace */
    while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
      *s++ = '\0';
    
    /* We finished */
    if(*s == '\0')
      break;
    
    if(c == maxtok)
      break;
    
    /* Stop tokenizing when we spot a ':' at token start */
#if 1
    if(*s == ':')
    {
      /* The remains are a single argument
         so it can include blanks also */
  
      v[c++] = &s[1];
      
      break;
    }
#endif
    /* Add to token list */
    v[c++] = s;

    if(c == maxtok)
      break;
    
    /* Scan for end or whitespace */
    while(*s && !(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n'))
      s++;    
  }
  
  if(c == maxtok || *s == ':')
  {
    while(*s)
    {
      if(*s == '\r' || *s == '\n')
      {
        *s = '\0';
        break;
      }
      
      s++;
    }
    
    do
      s--;
    while(*s == ' ' || *s == '\t');
    
    *++s = '\0';
  }
  
  v[c] = NULL;
  
  return c;
}

/* -------------------------------------------------------------------------- *
 * Splits a string into tokens.                                               *
 *                                                                            *
 * Like the one above but allows to specify delimiters.                       *
 * -------------------------------------------------------------------------- */
size_t strtokenize_d(char *s, char **v, size_t maxtok, const char *delim)
{
  size_t c = 0;
  
  for(EVER)
  {
    /* Skip and zero whitespace */
    while(strchr(delim, *s))
      *s++ = '\0';
    
    /* We finished */
    if(*s == '\0')
      break;
    
    if(c == maxtok)
      break;
    
    /* Add to token list */
    v[c++] = s;

    if(c == maxtok)
      break;
    
    /* Scan for end or whitespace */
    while(*s && strchr(delim, *s) == NULL)
      s++;    
  }
  
  if(c == maxtok)
  {
    while(*s)
    {
      if(strchr(delim, *s))
      {
        *s = '\0';
        break;
      }
      
      s++;
    }
    
    do
      s--;
    while(strchr(delim, *s));
    
    *++s = '\0';
  }
  
  v[c] = NULL;
  
  return c;
}

/* -------------------------------------------------------------------------- *
 * Splits a string into tokens.                                               *
 *                                                                            *
 * Like the one above but allows to specify one delimiter.                    *
 * -------------------------------------------------------------------------- */
size_t strtokenize_s(char *s, char **v, size_t maxtok, char delim)
{
  size_t c = 0;
  
  for(EVER)
  {
    /* Skip and zero whitespace */
    while(*s == delim)
      *s++ = '\0';
    
    /* We finished */
    if(*s == '\0')
      break;
    
    if(c == maxtok)
      break;
    
    /* Add to token list */
    v[c++] = s;

    if(c == maxtok)
      break;
    
    /* Scan for end or whitespace */
    while(*s && delim != *s)
      s++;    
  }
  
  if(c == maxtok)
  {
    while(*s)
    {
      if(*s == delim)
      {
        *s = '\0';
        break;
      }
      
      s++;
    }
    
    do
      s--;
    while(*s == delim);
    
    *++s = '\0';
  }
  
  v[c] = NULL;
  
  return c;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *str_strdup(const char *s)
{
  char *r;
  
  r = malloc(strlen(s) + 1);
  
  if(r != NULL)
    strcpy(r, s);
  
  return r;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

#define ROR(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
#define ROL(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
uint32_t strhash(const char *s)
{  
  uint32_t ret = 0xcafebabe;
  uint32_t temp;
  uint32_t i;
  
  if(s == NULL)
    return ret;
  
  for(i = 0; s[i]; i++)
  {
    temp = ret;
    ret = ROR(ret, s[i]);
    temp ^= s[i];
    temp = ROL(temp, ret);
    ret -= temp;
  }

  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

uint32_t strihash(const char *s)
{  
  uint32_t ret = 0xcafebabe;
  uint32_t temp;
  uint32_t i;
  
  if(s == NULL)
    return ret;
  
  for(i = 0; s[i]; i++)
  {
    temp = ret;
    ret = ROR(ret, tolower(s[i]));
    temp ^= tolower(s[i]);
    temp = ROL(temp, ret);
    ret -= temp;
  }

  return ret;
}
#undef ROL
#undef ROR

/* -------------------------------------------------------------------------- *
 * Convert a string to an unsigned long.                                      *
 * -------------------------------------------------------------------------- */
#ifdef __i386__
unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
  int neg = 0;
  unsigned long int v = 0;
          
  while(isspace(*nptr))
    ++nptr;

  if(*nptr == '-')
  {
    neg = 1;
    nptr++;
  }
  if(*nptr == '+')
    ++nptr;
  if(base == 16 && nptr[0] == '0')
    goto skip0x;
  if(!base)
  {
    if(*nptr == '0')
    {
      base=8;
skip0x:
      if(nptr[1] == 'x' || nptr[1] == 'X')
      {
        nptr += 2;
        base = 16;
      }
    }
    else
    {
      base = 10;
    }
  }
  while(*nptr)
  {
    register unsigned char c = *nptr;

    c = (c >= 'a' ? 
         c - 'a' + 10 : 
         c >= 'A' ? 
         c - 'A' + 10 : 
         c <= '9' ? 
         c - '0' : 0xff);

    if(c >= base)
      break;
    {
      register unsigned long int w = v * base;

      if(w<v)
      {
        syscall_errno = ERANGE;
        return ULONG_MAX;
      }
      
      v = w + c;
    }
    ++nptr;
  }
  if(endptr) *endptr = (char *)nptr;
  syscall_errno = 0; /* in case v == ULONG_MAX, ugh! */
  return (neg ? - v : v);
}
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * Convert a string to a long.                                                *
 * -------------------------------------------------------------------------- */
#define ABS_LONG_MIN 2147483648UL

#ifdef __i386__
long int strtol(const char *nptr, char **endptr, int base)
{
  int neg = 0;
  unsigned long int v;

  while(isspace(*nptr))
    nptr++;

  if(*nptr == '-')
  {
    neg = -1;
    nptr++;
  }

  v = strtoul(nptr, endptr, base);
  
  if(v >= ABS_LONG_MIN)
  {
    if(v == ABS_LONG_MIN && neg)
    {
      syscall_errno = 0;
      return v;
    }
  
    syscall_errno = ERANGE;
    return (neg ? LONG_MIN : LONG_MAX);
  }
      
  return (neg ? -v : v);
}
#endif /* __i386__ */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define expect(foo,bar) (foo)
#undef __likely
#undef __unlikely
#define __likely(foo) expect((foo),1)
#define __unlikely(foo) expect((foo),0)

unsigned long long int strtoull(const char *nptr, char **endptr, int base)
{
  long long int v = 0;
  
  while(isspace(*nptr)) 
    ++nptr;
  
  if(*nptr == '+') 
    ++nptr;
  
  if(!base)
  {
    if(*nptr == '0')
    {
      base = 8;
      
      if((*(nptr + 1) == 'x') || (*(nptr + 1) == 'X'))
      {
        nptr += 2;
        base = 16;
      }
    }
    else
    {
      base = 10;
    }
  }
  
  while(__likely(*nptr))
  {
    register unsigned char c = *nptr;
    
    c = (c >= 'a' ? c- 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0');
    
    if(__unlikely(c >= base)) 
      break;
    
    v = v * base + c;
    ++nptr;
  }
  
  if(endptr) 
    *endptr = (char *)nptr;
  
  return v;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define MATCH_MAX_CALLS 512
int strmatch(const char *str, const char *mask)
{
  const uint8_t *m = (const uint8_t *)mask;
  const uint8_t *n = (const uint8_t *)str;
  const uint8_t *ma = (const uint8_t *)mask;
  const uint8_t *na = (const uint8_t *)str;
  int wild = 0;
  int calls = 0;
  
  if(mask == NULL || str == NULL)
    return 0;
  
  while(calls++ < MATCH_MAX_CALLS)
  {
    if(*m == '*')
    {
      while(*m == '*')
        m++;
      
      wild = 1;
      ma = m;
      na = n;
    }
    
    if(*m == '\0')
    {
      if(*n == '\0')
        return 1;
      
      for(m--; m > (const uint8_t *)mask && *m == '?'; m--);
      
      if(*m == '*' && m > (const uint8_t *)mask)
        return 1;
      
      if(!wild)
        return 0;
      
      m = ma;
      n = ++na;
    }
    else if(*n == '\0')
    {
      while(*m == '*')
        m++;
      
      return (*m == '\0');
    }
    if(*m != *n && *m != '?')
    {
      if(!wild)
        return 0;
      
      m = ma;
      n = ++na;
    }
    else
    {
      if(*m)
        m++;
      
      if(*n)
        n++;
    }
  }
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int strimatch(const char *str, const char *mask)
{
  const uint8_t *m = (const uint8_t *)mask;
  const uint8_t *n = (const uint8_t *)str;
  const uint8_t *ma = (const uint8_t *)mask;
  const uint8_t *na = (const uint8_t *)str;
  int wild = 0;
  int calls = 0;
  
  if(mask == NULL || str == NULL)
    return 0;
  
  while(calls++ < MATCH_MAX_CALLS)
  {
    if(*m == '*')
    {
      while(*m == '*')
        m++;
      
      wild = 1;
      ma = m;
      na = n;
    }
    
    if(*m == '\0')
    {
      if(*n == '\0')
        return 1;
      
      for(m--; m > (const uint8_t *)mask && *m == '?'; m--);
      
      if(*m == '*' && m > (const uint8_t *)mask)
        return 1;
      
      if(!wild)
        return 0;
      
      m = ma;
      n = ++na;
    }
    else if(*n == '\0')
    {
      while(*m == '*')
        m++;
      
      return (*m == '\0');
    }
    if(tolower(*m) != tolower(*n) && *m != '?')
    {
      if(!wild)
        return 0;
      
      m = ma;
      n = ++na;
    }
    else
    {
      if(*m)
        m++;
      
      if(*n)
        n++;
    }
  }
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void strtrim(char *s)
{
  int  i;
  char buf[1024];
  
  for(i = 0; s[i] && i < sizeof(buf) - 1 && isspace(s[i]); i++);
  
  i = strlcpy(buf, &s[i], sizeof(buf)) - 2;
  
  while(i >= 0 && isspace(buf[i]))
    buf[i--] = '\0';
  
  strcpy(s, buf);
}

