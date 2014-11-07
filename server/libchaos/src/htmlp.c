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
 * $Id: htmlp.c,v 1.6 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/connect.h>
#include <libchaos/htmlp.h>
#include <libchaos/timer.h>
#include <libchaos/dlink.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int              htmlp_log;
struct sheap     htmlp_heap;
struct sheap     htmlp_tag_heap;
struct sheap     htmlp_var_heap;
struct dheap     htmlp_dheap;
struct list      htmlp_list;
uint32_t         htmlp_id;
struct {
  const char *key;
  char        c;
} htmlp_table[] = {
  { "nbsp", ' ' },
  { NULL, '\0'  },
};

/* -------------------------------------------------------------------------- *
 * Initialize htmlp heap.                                                     *
 * -------------------------------------------------------------------------- */
void htmlp_init(void)
{
  htmlp_log = log_source_register("htmlp");
  
  dlink_list_zero(&htmlp_list);

  htmlp_id = 0;
  
  mem_static_create(&htmlp_heap, sizeof(struct htmlp), HTMLP_BLOCK_SIZE);
  mem_static_note(&htmlp_heap, "htmlp block heap");
  mem_static_create(&htmlp_tag_heap, sizeof(struct htmlp_tag), HTMLP_BLOCK_SIZE * 4);
  mem_static_note(&htmlp_tag_heap, "htmlp tag heap");
  mem_static_create(&htmlp_var_heap, sizeof(struct htmlp_var), HTMLP_BLOCK_SIZE * 4);
  mem_static_note(&htmlp_var_heap, "htmlp var heap");
  mem_dynamic_create(&htmlp_dheap, HTMLP_MAX_BUF);
  mem_dynamic_note(&htmlp_dheap, "htmlp buffer heap");

  log(htmlp_log, L_status, "Initialized [htmlp] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy htmlp heap.                                                        *
 * -------------------------------------------------------------------------- */
void htmlp_shutdown(void)
{
  struct htmlp *hpptr;
  struct htmlp *next;
  
  log(htmlp_log, L_status, "Shutting down [htmlp] module...");
  
  dlink_foreach_safe(&htmlp_list, hpptr, next)
    htmlp_delete(hpptr);
  
  mem_dynamic_destroy(&htmlp_dheap);
  mem_static_destroy(&htmlp_tag_heap);
  mem_static_destroy(&htmlp_heap);
  
  log_source_unregister(htmlp_log);
}

/* -------------------------------------------------------------------------- *
 * Add a htmlp.                                                               *
 * -------------------------------------------------------------------------- */
struct htmlp *htmlp_new(const char *name)
{
  struct htmlp *hpptr;
  
  hpptr = mem_static_alloc(&htmlp_heap);
  
  memset(hpptr, 0, sizeof(struct htmlp));
  
  hpptr->id = htmlp_id++;
  strlcpy(hpptr->name, name, sizeof(hpptr->name));
  
  hpptr->nhash = strhash(hpptr->name);
  
  dlink_add_tail(&htmlp_list, &hpptr->node, hpptr);
  
  return hpptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int htmlp_parse(struct htmlp *hpptr, const char *data, size_t n)
{
  char             *buf;
  char             *p1;
  char             *p2;
  char             *p3;
  char             *p4;
  size_t            i;
  size_t            quotecount;
  int               closing = 0;
  struct htmlp_tag *htptr = NULL;
  struct htmlp_var *hvptr = NULL;
  struct list       vars;
    
  dlink_list_zero(&vars);
  
  hpptr->buf = buf = mem_dynamic_alloc(&htmlp_dheap, n + 1);
  
  for(i = 0; i < n; i++)
  {
    if(data[i] == '\n' || data[i] == '\0') 
      buf[i] = ' ';
    else
      buf[i] = data[i];
  }
  
  buf[i] = '\0';
  p1 = buf;
  
  for(;;)
  {
    if((p1 = strchr(p1, '<')) == NULL)
      break;

    *p1++ = '\0';
    
    if(*p1 == '!')
    {
      if((p1 = strchr(p1, '>')) == NULL)
        break;
      
      continue;
    }
    
    if(*p1 == '/')
    {
      closing = 1;
      p1++;
    }
    else
    {
      closing = 0;
    }

    p2 = strchr(p1, ' ');
    
    p4 = p1;
    
    do
    {
      p3 = strchr(p4, '>');
      
      /* incomplete tag -> parse error */
      if(p3 == NULL)
        return -1;
      
      /* space is after closing '<' */
      if(p2 > p3)
      {
        p2 = NULL;
        break;
      }
      
      quotecount = 0;
      
      for(p4 = p2; p4 < p3; p4++)
      {
        if(*p4 == '"')
          quotecount++;
      }
      
      p4++;
    }
    while((quotecount % 2));
      
    
    /* space, maybe we have attributes */
    if(p2)
    {
      char *name;
      char *p4;
/*      char *p5;*/
      char *value;

      *p2++ = '\0';

      for(name = p2; name < p3 && p2 < p3;)
      {
        /* skip all whitespace until beginning of attribute name */
        for(name = p2; name < p3 && isspace(*name); name++);

        if(name + 1 >= p3)
          break;

        for(value = name; value < p3 && (isalpha(*value) || *value == '-'); value++);

        if(*value == '=')
        {
          *value++ = '\0';

          if(*value == '"')
          {
            *value++ = '\0';

            p4 = strchr(value, '"');

            if(p4 == NULL)
              return -1;

            *p4++ = '\0';            
          }
          else
          {
            p4 = strchr(value, ' ');

            if(p4 == NULL || p4 > p3)
              p4 = p3;

            *p4++ = '\0';
          }
        }
        else
        {
          p4 = value;
          value = NULL;
          *p4++ = '\0';
        }

        p2 = p4;
        
        if(*name == '\0')
          continue;
        
/*        log(htmlp_log, L_status, "tag: %s attr: %s value: %s",
            p1, name, value);*/
        
        if(value)
        {
          hvptr = mem_static_alloc(&htmlp_var_heap);
          
          dlink_add_tail(&vars, &hvptr->node, hvptr);
          
          strlcpy(hvptr->name, name, sizeof(hvptr->name));
          
          hvptr->hash = strihash(hvptr->name);
          
          strlcpy(hvptr->value, value, sizeof(hvptr->value));
        }
      }
    }
    
    p2 = p1;
    p1 = p3;
    
    *p1++ = '\0';
    
    htptr = mem_static_alloc(&htmlp_tag_heap);
    
    strlcpy(htptr->name, p2, sizeof(htptr->name));
    
    htptr->hash = strihash(htptr->name);
    htptr->closing = closing;
    htptr->text = p1;
    
    dlink_add_tail(&hpptr->tags, &htptr->node, htptr);
    
    hpptr->current = htptr;
    
    htptr->vars = vars;
    dlink_list_zero(&vars);
  }

/*  debug(htmlp_log, "got %u tags", hpptr->tags.size);*/
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int htmlp_update(struct htmlp *hpptr)
{
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void htmlp_clear(struct htmlp *hpptr)
{
  struct htmlp_tag *htptr;
  struct htmlp_var *hvptr;
  struct node      *next1;
  struct node      *next2;
  
  hpptr->status = 0;
  
  dlink_foreach_safe(&hpptr->tags, htptr, next1)
  {
    dlink_foreach_safe(&htptr->vars, hvptr, next2)
    {
      dlink_delete(&htptr->vars, &hvptr->node);
      mem_static_free(&htmlp_var_heap, hvptr);
    }
    
    dlink_delete(&hpptr->tags, &htptr->node);
    mem_static_free(&htmlp_tag_heap, htptr);
  }
}

/* -------------------------------------------------------------------------- *
 * Remove a htmlp.                                                            *
 * -------------------------------------------------------------------------- */
void htmlp_delete(struct htmlp *hpptr)
{
  htmlp_clear(hpptr);
  
  dlink_delete(&htmlp_list, &hpptr->node);
  
  mem_static_free(&htmlp_heap, hpptr);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp *htmlp_find_name(const char *name)
{
  struct htmlp *hpptr;
  uint32_t       hash;
    
  hash = strhash(name);
  
  dlink_foreach(&htmlp_list, hpptr)
  {
    if(hpptr->nhash == hash)
    {
      if(!strcmp(hpptr->name, name))
        return hpptr;
    }
  }
  
  return NULL;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp *htmlp_find_id(uint32_t id)
{
  struct htmlp *hpptr;
  
  dlink_foreach(&htmlp_list, hpptr)
  {
    if(hpptr->id == id)
      return hpptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void htmlp_vset_args(struct htmlp *htmlp, va_list args)
{
  htmlp->args[0] = va_arg(args, void *);
  htmlp->args[1] = va_arg(args, void *);
  htmlp->args[2] = va_arg(args, void *);
  htmlp->args[3] = va_arg(args, void *);
}

void htmlp_set_args(struct htmlp *htmlp, ...)
{
  va_list args;
  
  va_start(args, htmlp);
  htmlp_vset_args(htmlp, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp *htmlp_pop(struct htmlp *htmlp)
{
  if(htmlp)
  {
    if(!htmlp->refcount)
      log(htmlp_log, L_warning, "Poping deprecated htmlp: %s",
          htmlp->name);

    htmlp->refcount++;
  }

  return htmlp;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp *htmlp_push(struct htmlp **htmlpptr)
{
  if(*htmlpptr)
  {
    if((*htmlpptr)->refcount == 0)
    {
      log(htmlp_log, L_warning, "Trying to push deprecated user: %s",
          (*htmlpptr)->name);
    }
    else
    {
      if(--(*htmlpptr)->refcount == 0)
        htmlp_delete(*htmlpptr);
      
      (*htmlpptr) = NULL;
    }
  }
  
  return *htmlpptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp_tag *htmlp_tag_first(struct htmlp *htptr)
{
  htptr->current = htptr->tags.head ? htptr->tags.head->data : NULL;
  
  return htptr->current;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp_tag *htmlp_tag_next(struct htmlp *htptr)
{
  if(htptr->current == NULL)
    return NULL;
  
  htptr->current = htptr->current->node.next ? 
    htptr->current->node.next->data : NULL;
  
  return htptr->current;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp_tag *htmlp_tag_find(struct htmlp *htptr, const char *name)
{
  struct htmlp_tag *tag;
  uint32_t          hash = strihash(name);

  if(htptr->current)
  {
    for(tag = htptr->current; tag; tag = (struct htmlp_tag *)tag->node.next)
    {
      if(tag->hash == hash)
      {
        if(!stricmp(tag->name, name))
        {
          htptr->current = tag;
          return tag;
        }
      }
    }
    
    htptr->current = NULL;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint32_t htmlp_tag_count(struct htmlp *htptr)
{
  return htptr->tags.size;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp_tag *htmlp_tag_index(struct htmlp *htptr, uint32_t i)
{
  struct node *node;
  
  node = dlink_index(&htptr->tags, i);
  
  if(node)
  {
    htptr->current = node->data;
    return htptr->current;
  }
  else
  {
    htptr->current = NULL;
    return NULL;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp_var *htmlp_var_set(struct htmlp *htptr, const char *name, const char *value)
{
  struct htmlp_var *hvptr;
  uint32_t          hash;
  
  if(htptr->current == NULL)
    return NULL;
  
  if((hvptr = htmlp_var_find(htptr, name)) == NULL)
  {
    hvptr = mem_static_alloc(&htmlp_var_heap);
    
    dlink_add_tail(&htptr->current->vars, &hvptr->node, hvptr);
    
    strlcpy(hvptr->name, name, sizeof(hvptr->name));
    hvptr->hash = strihash(hvptr->name);
    
    hash = strihash(name);
  }
  
  strlcpy(hvptr->value, value, sizeof(hvptr->value));
  
  return hvptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp_var *htmlp_var_find(struct htmlp *htptr, const char *name)
{
  struct htmlp_var *hvptr;
  uint32_t          hash = strihash(name);
  
  if(htptr->current == NULL)
    return NULL;
  
  dlink_foreach(&htptr->current->vars, hvptr)
  {
    if(hvptr->hash == hash)
    {
      if(!stricmp(hvptr->name, name))
        return hvptr;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *htmlp_decode(const char *s)
{
  static char text[1024];
  uint32_t    i;
  uint32_t    di = 0;
  int         left = 1;
  size_t      len;
  
  for(i = 0; s[i] && di < 1023; i++)
  {
    if(left && !isspace(s[i]))
      left = 0;
    
    if(left)
      continue;
    
    if(s[i] == '&')
    {
      uint32_t ti;
      
      i++;
      
      if(s[i] == '#')
      {
        char *p;
        
        text[di++] = strtoul(&s[++i], &p, 10);
        
        p = strchr(&s[i], ';');
        p -= (size_t)&s[i];
        
        i += (size_t)p;
        
        continue;
      }
      
      for(ti = 0; htmlp_table[ti].key; ti++)
      {
        len = strlen(htmlp_table[ti].key);
        
        if(!strnicmp(htmlp_table[ti].key, &s[i], len))
        {
          text[di++] = htmlp_table[ti].c;
          i += len;
          break;
        }
      }
      
      if(s[i] != ';')
      {
        while(s[i] && s[i] != ';') i++;
        i--;
      }
      
      continue;
    }
    
    text[di++] = s[i];
  }

  text[di] = '\0';
  
  return text;
} 

/* -------------------------------------------------------------------------- *
 * Dump htmlp list and heap.                                                  *
 * -------------------------------------------------------------------------- */
void htmlp_dump(struct htmlp *hpptr)
{
  if(hpptr == NULL)
  {
    dump(htmlp_log, "[================ htmlp summary ================]");
    
    dlink_foreach(&htmlp_list, hpptr)
    {
      dump(htmlp_log, " #%u: [%u] %-20s (%i)",
           hpptr->id, hpptr->refcount, hpptr->name, hpptr->fd);
    }

    dump(htmlp_log, "[============= end of htmlp summary ============]");
  }
  else
  {
/*    struct node *nptr;*/
    
    dump(htmlp_log, "[================= htmlp dump =================]");
    
    dump(htmlp_log, "         id: #%u", hpptr->id);
    dump(htmlp_log, "   refcount: %u", hpptr->refcount);
    dump(htmlp_log, "      nhash: %p", hpptr->nhash);
    dump(htmlp_log, "         fd: %i", hpptr->fd);
    dump(htmlp_log, "       name: %s", hpptr->name);
    
/*    dump(htmlp_log, "------------------ htmlp data ------------------");

    dlink_foreach(&hpptr->lines, nptr)
      dump(htmlp_log, "%s", nptr->data ? nptr->data : "");*/
    
    dump(htmlp_log, "[============== end of htmlp dump =============]");    
  }
}
