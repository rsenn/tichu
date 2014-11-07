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
 * $Id: str.h,v 1.28 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_STR_H
#define LIB_STR_H

#include <string.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/mem.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#undef isspace
#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

#undef isdigit
#define isdigit(c) (c <= '9' && c >= '0')
#undef isalpha
#define isalpha(c) ((c >= 'A' && c <= 'Z') || \
                    (c >= 'a' && c <= 'z'))
#undef isalnum
#define isalnum(c) (isdigit((c)) || isalpha((c)))

#undef tolower
#define tolower(c) (c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c)
#undef islower
#define islower(c) (c >= 'a' && c <= 'z')
#undef toupper
#define toupper(c) (c >= 'a' && c <= 'a' ? c - ('a' - 'A') : c)
#undef isupper
#define isupper(c) (c >= 'A' && c <= 'Z')

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
typedef void (str_format_cb)(char   **pptr, size_t  *bptr,
                             size_t   n,    int      padding,
                             int      left, void    *arg);
               
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char     str_hexchars[];
extern str_format_cb *str_table[64];

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void str_init       (void);
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void str_shutdown   (void);
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void str_register   (char           c, 
                            str_format_cb *cb);
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void str_unregister (char           c);

/* -------------------------------------------------------------------------- *
 * Get string length.                                                         *
 * -------------------------------------------------------------------------- */
#ifdef USE_IA32_LINUX_INLINE
extern size_t strlen(const char *s);

/*extern inline size_t strlen(const char *s)
{
  size_t len = 0;
  
  for(EVER)
  {
    if(!s[len]) return len; len++;
    if(!s[len]) return len; len++;
    if(!s[len]) return len; len++;
    if(!s[len]) return len; len++;
  }
}*/
#endif /* USE_IA32_LINUX_INLINE */

/* -------------------------------------------------------------------------- *
 * Get first occurance of char <c> in string <s>                              *
 * -------------------------------------------------------------------------- */
#if 0
extern char *strchr(const char *s, int c);

extern inline char *strchr(const char *s, int c)
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
 * -------------------------------------------------------------------------- */
extern char *strcat(char *d, const char *s);

extern inline char *strcat(char *d, const char *s)
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

/* -------------------------------------------------------------------------- *
 * Copy string from <s> to <d>.                                               *
 * -------------------------------------------------------------------------- */
#if 0
extern size_t strcpy(char *d, const char *s);

extern inline size_t strcpy(char *d, const char *s)
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
extern size_t strlcpy(char *d, const char *s, size_t n);

extern inline size_t strlcpy(char *d, const char *s, size_t n)
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
extern size_t strlcat(char *d, const char *s, size_t n);
  
extern inline size_t strlcat(char *d, const char *s, size_t n)
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
#undef strcmp
extern int strcmp(const char *s1, const char *s2);

extern inline int strcmp(const char *s1, const char *s2)
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

/* -------------------------------------------------------------------------- *
 * Compare string.                                                            *
 * -------------------------------------------------------------------------- */
extern int stricmp(const char *s1, const char *s2);

extern inline int stricmp(const char *s1, const char *s2)
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
/*#undef strncmp
extern int strncmp(const char *s1, const char *s2, size_t n);

extern inline int strncmp(const char *s1, const char *s2, size_t n)
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

/* -------------------------------------------------------------------------- *
 * Compare string, abort after <n> chars.                                     *
 * -------------------------------------------------------------------------- */
extern int strnicmp(const char *s1, const char *s2, size_t n);

extern inline int strincmp(const char *s1, const char *s2, size_t n)
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
 * Formatted vararg print to string                                           *
 * -------------------------------------------------------------------------- */
#undef vsnprintf
#define vsnprintf str_vsnprintf
extern int str_vsnprintf(char *str, size_t n, const char *format, va_list args);

/* -------------------------------------------------------------------------- *
 * Converts a string to a signed int.                                         *
 * -------------------------------------------------------------------------- */
#define atoi __api_atoi
extern int atoi(const char *s);

/* -------------------------------------------------------------------------- *
 * Formatted print to string                                                  *
 * -------------------------------------------------------------------------- */
#undef snprintf
#define snprintf str_snprintf
extern int str_snprintf(char *str, size_t n, const char *format, ...);

extern inline int str_snprintf(char *str, size_t n, const char *format, ...)
{
  int ret;
  
  va_list args;
  
  va_start(args, format);
  
  ret = vsnprintf(str, n, format, args);
  
  va_end(args);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Converts a string to a signed int.                                         *
 * -------------------------------------------------------------------------- */
extern int atoi(const char *s);

extern inline int atoi(const char *s)
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
extern size_t strtokenize(char *s, char **v, size_t maxtok);

#if 0
extern inline size_t strtokenize(char *s, char **v, size_t maxtok)
{
  size_t c = 0;
  
  if(!maxtok)
    return 0;
  
  for(EVER)
  {
    /* Skip and zero whitespace */
    while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
      *s++ = '\0';
    
    /* We finished */
    if(*s == '\0')
      break;
    
    /* Stop tokenizing when we spot a ':' at token start */
    if(*s == ':')
    {
      /* The remains are a single argument
         so it can include blanks also */
      v[c++] = &s[1];
      break;
    }
    /* Add to token list */
    v[c++] = s;
    
    /* We finished */
    if(c == maxtok)
      break;
    
    /* Scan for end or whitespace */
    while(*s != ' ' && *s != '\t' && *s != '\0' && *s != '\r' && *s != '\n')
      s++;
  }
  
  v[c] = NULL;
  
  return c;
}
#endif

/* -------------------------------------------------------------------------- *
 * Splits a string into tokens.                                               *
 *                                                                            *
 * Like the one above but allows to specify delimiters.                       *
 * -------------------------------------------------------------------------- */
extern size_t strtokenize_d(char       *s,
                            char      **v, 
                            size_t      maxtok,
                            const char *delim);

/* -------------------------------------------------------------------------- *
 * Splits a string into tokens.                                               *
 *                                                                            *
 * Like the one above but allows to specify one delimiter.                    *
 * -------------------------------------------------------------------------- */
extern size_t strtokenize_s(char       *s,
                            char      **v, 
                            size_t      maxtok,
                            char        delim);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#undef strdup
#define strdup str_strdup
extern char *str_strdup(const char *s);

extern inline char *str_strdup(const char *s)
{
  char *r;
  
  r = malloc(strlen(s) + 1);
  
  if(r != NULL)
    strcpy(r, s);
  
  return r;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint32_t strhash(const char *s);

#define ROR(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
#define ROL(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
extern inline uint32_t strhash(const char *s)
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
#undef ROL
#undef ROR

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint32_t strihash(const char *s);

#define ROR(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
#define ROL(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
extern inline uint32_t strihash(const char *s)
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
extern unsigned long int strtoul(const char *nptr, char **endptr, int base);

/* -------------------------------------------------------------------------- *
 * Convert a string to a long.                                                *
 * -------------------------------------------------------------------------- */
extern long int strtol(const char *nptr, char **endptr, int base);

/* -------------------------------------------------------------------------- *
 * Convert a string to a unsigned long long.                                  *
 * -------------------------------------------------------------------------- */
extern unsigned long long int strtoull(const char *nptr, char **endptr, int base);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int strmatch(const char *str, const char *mask);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int strimatch(const char *str, const char *mask);
  

extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void strtrim(char *s);
  

#endif /* LIB_STR_H */

