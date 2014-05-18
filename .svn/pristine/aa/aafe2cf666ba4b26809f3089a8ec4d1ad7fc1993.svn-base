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
 * $Id: mem.h,v 1.20 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_MEM_H
#define LIB_MEM_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#ifndef MEM_PAGE_SIZE
#define MEM_PAGE_SIZE    (1 << 12)
#endif /* PAGE_SIZE */
#define MEM_PAD_BLOCKS   MEM_PAGE_SIZE

#define MEM_DYNAMIC_SIZE 512 * 1024
#define MEM_DEV_ZERO     "/dev/zero"

/* -------------------------------------------------------------------------- *
 * Forward declarations                                                       *
 * -------------------------------------------------------------------------- */
struct schunk;
struct dchunk;
struct sblock;
struct dblock;
struct sheap;
struct dheap;

/* -------------------------------------------------------------------------- *
 * Chunk with static size                                                     *
 * -------------------------------------------------------------------------- */
struct schunk {
  struct node    node;         /* The node which links this chunk to a block */
  struct sblock *block;        /* Points back to the block this chunk is on */
};

/* -------------------------------------------------------------------------- *
 * Chunk with dynamic size                                                    *
 * -------------------------------------------------------------------------- */
struct dchunk {
  struct node    node;         /* The node which links this chunk to a block */
  struct dblock *block;        /* Points back to the block this chunk is on */
  size_t         size;         /* Actual size of the chunk */
};

/* -------------------------------------------------------------------------- *
 * Block containing static chunks                                             *
 * -------------------------------------------------------------------------- */
struct sblock {
  struct sblock *next;        /* reference to the next block if present */
  struct sheap  *heap;        /* points back to the heap this block lives on */
  size_t         size;        /* size of this block in bytes */
  struct schunk *chunks;      /* element list */
  size_t         free_chunks; /* number of elements available */
  struct list    free_ones;   /* list of unused elements */
  struct list    used_ones;   /* list of used elements */
};
 
/* -------------------------------------------------------------------------- *
 * Block allowing allocation of dynamic length chunks.                        *
 * -------------------------------------------------------------------------- */
struct dblock {
  struct dblock *next;       /* reference to the next block if present */
  size_t         size;       /* size of this block in bytes */
  struct list    chunks;
/*  size_t         chunk_count;*/
  size_t         max_bytes;  /* length of the biggest free chunk w/o node */
  size_t         free_bytes;
  size_t         used_bytes;
  struct dheap  *heap;
};

/* -------------------------------------------------------------------------- *
 * A heap containing static blocks.                                           *
 * -------------------------------------------------------------------------- */
struct sheap {
  struct node    node;
  uint32_t       id;
  struct sblock *base;
  size_t         block_count;
  size_t         free_chunks;
  size_t         block_size;
  size_t         chunk_size;
  size_t         chunks_per_block;
  char           note[128];
};

/* -------------------------------------------------------------------------- *
 * A heap containing dynamic blocks.                                          *
 * -------------------------------------------------------------------------- */
struct dheap {
  struct node    node;
  uint32_t       id;
  struct dblock *base;
  size_t         block_count;
  size_t         block_size;
  size_t         free_bytes;
  size_t         used_bytes;
  size_t         max_bytes;
  char           note[128];
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int          mem_log;       /* Log source */
extern struct dheap mem_dheap;     /* Dynamic heap for malloc() and consorts */
extern struct list  mem_slist;
extern struct list  mem_dlist;
extern uint32_t     mem_id;
#ifndef MAP_ANON
extern int          mem_zero;      /* /dev/zero if we havent MAP_ANON */
#endif /* MAP_ANON */

/* -------------------------------------------------------------------------- *
 * Initialize allocator                                                       *
 * -------------------------------------------------------------------------- */
extern void  mem_init                (void);

/* -------------------------------------------------------------------------- *
 * Close fd for allocator                                                     *
 * -------------------------------------------------------------------------- */
extern void  mem_shutdown            (void);

/* -------------------------------------------------------------------------- *
 * Whine and exit if we got no memory                                         *
 * -------------------------------------------------------------------------- */
extern void  mem_fatal               (void);

/* -------------------------------------------------------------------------- *
 * Create a static heap                                                       *
 *                                                                            *
 * <msptr>          Pointer to a static heap structure                        *
 * <size>           How big any element is (usually the size of the structure *
 *                  you want to store)                                        *
 * <count>          How many elements a block can contain                     *
 * -------------------------------------------------------------------------- */
extern void  mem_static_create       (struct sheap *shptr,
                                      size_t        size,
                                      size_t        count);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void  mem_static_note         (struct sheap *shptr,
                                      const char   *format,
                                      ...);

/* -------------------------------------------------------------------------- *
 * Return pointer to a free element on a static heap                          *
 *                                                                            *
 * <msptr>          Pointer to a static heap structure                        *
 *                                                                            *
 * Returns pointer to free element or exits the program if failed.            *
 * -------------------------------------------------------------------------- */
extern void *mem_static_alloc        (struct sheap *shptr);

/* -------------------------------------------------------------------------- *
 * Free an element on a static heap                                           *
 *                                                                            *
 * <msptr>          Pointer to a static heap structure                        *
 * 
 * 
 * -------------------------------------------------------------------------- */
extern void  mem_static_free         (struct sheap *shptr,
                                      void         *scptr);

/* -------------------------------------------------------------------------- *
 * Free all blocks which have no used elements.                               *
 * -------------------------------------------------------------------------- */
extern int   mem_static_collect      (struct sheap *shptr);

/* -------------------------------------------------------------------------- *
 * Destroy the whole heap.                                                    *
 * -------------------------------------------------------------------------- */
extern void  mem_static_destroy      (struct sheap *shptr);

/* -------------------------------------------------------------------------- *
 * DEBUG FUNCTION: see if <element> is valid.                                 *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
extern int   mem_static_valid        (struct sheap *shptr,
                                      void         *scptr);
#endif /* DEBUG */ 

/* -------------------------------------------------------------------------- *
 * Create a new dynamic heap                                                  *
 *                                                                            *
 * <dhptr>   Pointer to a dynamic heap structure                              *
 * <size>    How big the a chunk can be at the maximum                        *
 * -------------------------------------------------------------------------- */
extern void  mem_dynamic_create      (struct dheap *dhptr, 
                                      size_t        size);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void  mem_dynamic_note        (struct dheap *dhptr,
                                      const char   *format,
                                      ...);

/* -------------------------------------------------------------------------- *
 * Return pointer to a newly allocated chunk on the heap.                     *
 * -------------------------------------------------------------------------- */
extern void *mem_dynamic_alloc       (struct dheap *dhptr,
                                      size_t        size);

/* -------------------------------------------------------------------------- *
 * Try to resize block, otherwise free and allocate new one.                  *
 * -------------------------------------------------------------------------- */
extern void *mem_dynamic_realloc     (struct dheap *dhptr,
                                      void         *ptr,
                                      size_t        size);
  
/* -------------------------------------------------------------------------- *
 * Free a chunk. haha                                                         *
 * -------------------------------------------------------------------------- */
extern void  mem_dynamic_free        (struct dheap *dhptr,
                                      void         *ptr);

/* -------------------------------------------------------------------------- *
 * Free all blocks which have no chunks.                                      *
 * -------------------------------------------------------------------------- */
extern int   mem_dynamic_collect     (struct dheap *dhptr);

/* -------------------------------------------------------------------------- *
 * Destroy a dynamic heap, munmap() the blocks.                               *
 * -------------------------------------------------------------------------- */
extern void  mem_dynamic_destroy     (struct dheap *dhptr);

/* -------------------------------------------------------------------------- *
 * DEBUG FUNCTION: see if <chnk> is valid.                                    *
 * this is time-consuming and will not be built without -DDEBUG               *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
extern int   mem_dynamic_valid       (struct dheap *dhptr,
                                      void         *dcptr);
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * DEBUG FUNCTION: dump a heap                                                *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
extern void  mem_dynamic_dump        (struct dheap *dhptr);
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Fuck libc! :P                                                              *
 * -------------------------------------------------------------------------- */
#ifdef USE_IA32_LINUX_INLINE
#undef memset
extern void *memset               (void       *s,
                                   int         c,
                                   size_t      n);

#undef memcpy
extern void *memcpy               (void       *d,
                                   const void *s,
                                   size_t      n);

#undef memcmp
extern int   memcmp               (const void *d,
                                   const void *s,
                                   size_t      n);

#ifdef __GCC__
extern inline void *memset(void *s, int c, size_t n)
{
  size_t i;

  /* n is a multiple of 8, so do 64bit copying */
  if(!(n & 0x07) && (n >= 8))
  {
    int64_t q = ((int64_t)(c & 0xff) <<  0) |
                ((int64_t)(c & 0xff) <<  8) |
                ((int64_t)(c & 0xff) << 16) |
                ((int64_t)(c & 0xff) << 24) |
                ((int64_t)(c & 0xff) << 32) |
                ((int64_t)(c & 0xff) << 40) |
                ((int64_t)(c & 0xff) << 48) |
                ((int64_t)(c & 0xff) << 56);
    n >>= 3;

    for(i = 0; i < n; i++)
      ((int64_t *)s)[i] = q;
  }
  /* n is a multiple of 4, so do 32bit copying */
  else if(!(n & 0x03) && (n >= 4))
  {
    int32_t q = ((int32_t)(c & 0xff) <<  0) |
                ((int32_t)(c & 0xff) <<  8) |
                ((int32_t)(c & 0xff) << 16) |
                ((int32_t)(c & 0xff) << 24);
    n >>= 2;
 
    for(i = 0; i < n; i++)
      ((int32_t *)s)[i] = q;
  }
  /* n is a multiple of 2, so do 16bit copying */
  else if(!(n & 0x01) && (n >= 2))
  {
    int16_t q = ((int16_t)(c & 0xff) <<  0) |
                ((int16_t)(c & 0xff) <<  8);
    n >>= 1;
 
    for(i = 0; i < n; i++)
      ((int16_t *)s)[i] = q;
  }
  /* otherwise do 8bit copying */
  else
  {
    int8_t q = (int8_t)(c & 0xff);

    for(i = 0; i < n; i++)
      ((int8_t *)s)[i] = q;
  }

  return s;
}

extern inline void *memcpy(void *d, const void *s, size_t n)
{
  size_t i;

  /* n is a multiple of 8, so do 64bit copying */
  if(!(n & 0x07) && (n >= 8))
  {
    n >>= 3;

    for(i = 0; i < n; i++)
      ((int64_t *)d)[i] = ((int64_t *)s)[i];
  }
  /* n is a multiple of 4, so do 32bit copying */
  else if(!(n & 0x03) && (n >= 4))
  {
    n >>= 2;

    for(i = 0; i < n; i++)
      ((int32_t *)d)[i] = ((int32_t *)s)[i];
  }
  /* n is a multiple of 2, so do 16bit copying */
  else if(!(n & 0x01) && (n >= 2))
  { 
    n >>= 1;
                
    for(i = 0; i < n; i++)
      ((int16_t *)d)[i] = ((int16_t *)s)[i];
  }
  /* otherwise do 8bit copying */
  else
  {
    for(i = 0; i < n; i++)
      ((int8_t *)d)[i] = ((int8_t *)s)[i];
  }
  
  return d;
}

extern inline void *memmove(void *d, const void *s, size_t n)
{
  size_t i;
  ssize_t dist;
  
  dist = (size_t)d - (size_t)s;
  
  if(dist <= 0)
  {
    /* n is a multiple of 8, so do 64bit copying */
    if(!(n & 0x07) && (n >= 8) && (dist >= 8 || dist == 0))
    {
      n >>= 3;
      
      for(i = 0; i < n; i++)
        ((int64_t *)d)[i] = ((int64_t *)s)[i];
    }
    /* n is a multiple of 4, so do 32bit copying */
    else if(!(n & 0x03) && (n >= 4) && (dist >= 4))
    {
      n >>= 2;
      
      for(i = 0; i < n; i++)
        ((int32_t *)d)[i] = ((int32_t *)s)[i];
    }
    /* n is a multiple of 2, so do 16bit copying */
    else if(!(n & 0x01) && (n >= 2) && (dist >= 2))
    {
      n >>= 1;
      
      for(i = 0; i < n; i++)
        ((int16_t *)d)[i] = ((int16_t *)s)[i];
    }
    /* otherwise do 8bit copying */
    else
    {
      for(i = 0; i < n; i++)
        ((int8_t *)d)[i] = ((int8_t *)s)[i];
    }
  }
  else
  {
    /* n is a multiple of 8, so do 64bit copying */
    if(!(n & 0x07) && (n >= 8) && (dist <= -8))
    {
      n >>= 3;

      for(i = n - 1;; i--)
      {
        ((int64_t *)d)[i] = ((int64_t *)s)[i];

        if(i == 0)
          break;
      }
    }
    /* n is a multiple of 4, so do 32bit copying */
    else if(!(n & 0x03) && (n >= 4) && (dist >= 4))
    {
      n >>= 2;
      
      for(i = n - 1;; i--)
      {
        ((int32_t *)d)[i] = ((int32_t *)s)[i];
        
        if(i == 0)
          break;
      }
    }
    /* n is a multiple of 2, so do 16bit copying */
    else if(!(n & 0x01) && (n >= 2) && (dist >= 2))
    {
      n >>= 1;
      
      for(i = n - 1;; i--)
      {
        ((int16_t *)d)[i] = ((int16_t *)s)[i];
        
        if(i == 0)
          break;
      }
    }
    /* otherwise do 8bit copying */
    else
    {
      for(i = n - 1;; i--)
      {
        ((int8_t *)d)[i] = ((int8_t *)s)[i];
        
        if(i == 0)
          break;
      }
    }
  }

  return d;
}

extern inline int memcmp(const void *d, const void *s, size_t n)
{
  size_t i;
  
  /* n is a multiple of 8, so do 64bit comparing */
  if(!(n & 0x07) && (n >= 8))
  {
    n >>= 3;
    
    for(i = 0; i < n; i++)
      if(((int64_t *)d)[i] != ((int64_t *)s)[i])
        return 1;
  }
  /* n is a multiple of 4, so do 32bit comparing */
  else if(!(n & 0x03) && (n >= 4))
  {
    n >>= 2;
    
    for(i = 0; i < n; i++)
      if(((int32_t *)d)[i] != ((int32_t *)s)[i])
        return 1;
  }
  /* n is a multiple of 2, so do 16bit comparing */
  else if(!(n & 0x01) && (n >= 2))
  {
    n >>= 1;
    
    for(i = 0; i < n; i++)
      if(((int16_t *)d)[i] != ((int16_t *)s)[i])
        return 1;
  }
  /* otherwise do 8bit comparing */
  else
  {
    for(i = 0; i < n; i++)
      if(((int8_t *)d)[i] != ((int8_t *)s)[i])
        return 1;
  }
  
  return 0;
}

#endif /* __GCC__ */

#endif /* USE_IA32_LINUX_INLINE */

#undef malloc
#undef realloc
#undef free

#define malloc(size)        mem_dynamic_alloc(&mem_dheap, (size))
#define realloc(ptr, size)  mem_dynamic_realloc(&mem_dheap, (ptr), (size))
#define free(ptr)           do { \
                              if((ptr)) { \
                                mem_dynamic_free(&mem_dheap, (ptr)); \
                                (ptr) = NULL; \
                              } \
                            } while(0)
/*
  * calloc is kludgy and we can't alloc memory 
  * of 64bit sizes anyway (n * size), use heaps
  * for fixed size stuff anyway!!!
  */

#undef calloc
#define calloc              __don_t_use_this_its_a_kludge__

/* -------------------------------------------------------------------------- *
 * Dump static heap information                                               *
 * -------------------------------------------------------------------------- */
extern struct sheap *mem_static_find         (uint32_t      id);
extern void          mem_static_dump         (struct sheap *shptr);

/* -------------------------------------------------------------------------- *
 * Dump dynamic heap information                                              *
 * -------------------------------------------------------------------------- */
extern struct dheap *mem_dynamic_find        (uint32_t      id);
extern void          mem_dynamic_dump        (struct dheap *dhptr);

#endif /* __MEM_H */
