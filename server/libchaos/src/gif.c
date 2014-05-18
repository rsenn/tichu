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
 * $Id: gif.c,v 1.14 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE
 
/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/mem.h>
#include <libchaos/gif.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/io.h>
 
#ifdef HAVE_SDL
#include <SDL.h>
#endif


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int            gif_log;
struct sheap   gif_heap;
struct sheap   gif_image_heap;
struct dheap   gif_palette_heap;
struct dheap   gif_data_heap;
struct sheap   gif_ht_heap;
struct list    gif_list;
uint32_t       gif_id;


/*static int gif_read(struct gif *gif, void *buf, size_t len);
static int gif_write(struct gif *gif, const void *buf, size_t len);*/

/* -------------------------------------------------------------------------- *
 * Bit masks for LZW codes                                                    *
 * -------------------------------------------------------------------------- */
static uint8_t gif_compress_masks[] = {
  0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static uint32_t gif_decompress_masks[] = { 
  0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f,
  0x003f, 0x007f, 0x00ff, 0x01ff, 0x03ff, 0x07ff,
  0x0fff
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static inline int gif_bit_size(int n) 
{
  int i;
  
  for(i = 1; i <= 8; i++) 
  {
    if((1 << i) >= n)
      break;
  }
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Read GIF data from memory or directly from file                            *
 * -------------------------------------------------------------------------- */
static int gif_read(struct gif *gif, void *buf, size_t len) 
{  
  /* Read from a queue */
  if(gif->fd >= 0)
  {
    return io_read(gif->fd, buf, len);
  } 
  /* Read from memory */
  else if(gif->mem)
  {
    /* We're at the end, return EOF */
    if(gif->offset >= gif->size)
      return 0;
    
    /* Otherwise copy the stuff and return length */
    if(gif->offset + len > gif->size)
    {
      /* More bytes requested than available, truncate */
      len = gif->size - gif->offset;
      memcpy(buf, &gif->mem[gif->offset], len);
      
      /* We're now at EOF */
      gif->offset = gif->size;
      
      return len;
    }
    else
    {
      /* Enough bytes available */
      memcpy(buf, &gif->mem[gif->offset], len);
      gif->offset += len;
    
      return len;
    }
  }
  
  /* No file and no memory present */
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Write GIF data to memory or directly to a file                             *
 * -------------------------------------------------------------------------- */
static int gif_write(struct gif *gif, const void *buf, size_t len) 
{  
  /* Write to file -> Queue data via io.c */
  if(gif->fd >= 0)
  {
    return io_write(gif->fd, buf, len);
  } 
  /* Write to memory -> Realloc data space */
  else if(gif->mem) 
  {
    size_t newsize;
    
    /* Calculate new data size */
    newsize = gif->offset + len;
    
    /* Enlarge data space */
    if((gif->mem = 
        mem_dynamic_realloc(&gif_data_heap, gif->mem, newsize)) == NULL)
      return -1;
    
    /* Copy data and update pointer */
    memcpy(&gif->mem[gif->offset], buf, len);    
    gif->offset += len;
    gif->size = newsize;
    
    return len;
  }
  
  /* No file and no memory present */
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Read one byte from GIF buffer                                              *
 * -------------------------------------------------------------------------- */
static int gif_get_byte(struct gif *gif, uint8_t *byte) 
{
  uint8_t buf[1];
  
  if(gif_read(gif, buf, 1) != 1)
    return -1;
  
  *byte = buf[0];  
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Read a word from GIF buffer                                                *
 * -------------------------------------------------------------------------- */
static int gif_get_word(struct gif *gif, uint16_t *word) 
{
  uint8_t buf[2];
  
  if(gif_read(gif, buf, 2) != 2)
    return -1;
  
  *word = (((uint16_t)buf[1]) << 8) | buf[0];
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Put one byte to GIF buffer                                                 *
 * -------------------------------------------------------------------------- */
static int gif_put_byte(struct gif *gif, uint8_t byte) 
{
  uint8_t buf;
  
  buf = byte;
  
  if(gif_write(gif, &buf, 1) != 1)
    return -1;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Put a word to GIF buffer                                                   *
 * -------------------------------------------------------------------------- */
static int gif_put_word(struct gif *gif, uint16_t word) 
{
  uint8_t buf[2];
  
  buf[0] = word & 0xff;
  buf[1] = (word >> 8) & 0xff;
  
  if(gif_write(gif, buf, 2) != 2) 
    return -1;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Verify GIF Header                                                          *
 * -------------------------------------------------------------------------- */
static int gif_verify(struct gif *gif)
{
  if(gif->status == GIF_STATE_READ)
  {
    char buf[GIF_MAGIC_LEN + 1];
    
    /* Verify GIF magic */
    if(gif_read(gif, buf, GIF_MAGIC_LEN) != GIF_MAGIC_LEN) 
    {
      log(gif_log, L_warning, "No GIF signature on file '%s'.",
          gif->name);
      return -1;
    }
    
    buf[3] = '\0';

    if(strcmp(buf, "GIF")) 
    {
      log(gif_log, L_warning, "Invalid GIF signature on file '%s'.",
          gif->name);
      return -1;
    }
    
    /* Get screen descriptor */
    if(gif_screen_get(gif)) 
    {
      log(gif_log, L_warning, "Invalid screen descriptor on file '%s'.",
          gif->name);
      return -1;
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Hashtable functions for LZW                                                *
 * -------------------------------------------------------------------------- */
static inline int gif_ht_mask(unsigned long item) 
{
  return ((item >> 12) ^ item) & GIF_HT_KEY_MASK;
}

static inline void gif_ht_clear(struct gif_hashtable *table) 
{
  memset(table->htable, 0xff, GIF_HT_SIZE * sizeof(long));
}

static struct gif_hashtable *gif_ht_new() 
{
  
  struct gif_hashtable *table;
  
  if((table = mem_static_alloc(&gif_ht_heap)) == NULL)
    return NULL;
  
  gif_ht_clear(table);
  
  return table;
}

static void gif_ht_free(struct gif_hashtable *table)
{
  mem_static_free(&gif_ht_heap, table);
}

static void gif_ht_insert(struct gif_hashtable *table, unsigned long key, 
                          int                   code) 
{
  unsigned long *htable = table->htable;
  int hkey              = gif_ht_mask(key);
  
  while(GIF_HT_GET_KEY(htable[hkey]) != 0xfffffL)
    hkey = (hkey + 1) & GIF_HT_KEY_MASK;
  
  htable[hkey] = GIF_HT_PUT_KEY(key) | 
                 GIF_HT_PUT_CODE(code);
}

static int gif_ht_exists(struct gif_hashtable *table, unsigned long key) 
{  
  unsigned long *htable = table->htable;
  uint32_t       htkey;
  int            hkey   = gif_ht_mask(key);
  
  while((htkey = GIF_HT_GET_KEY(htable[hkey])) != 0xfffffL) 
  {
    if(key == htkey)
      return GIF_HT_GET_CODE(htable[hkey]);
    
    hkey = (hkey + 1) & GIF_HT_KEY_MASK;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * LZW Decompression code                                                     *
 * -------------------------------------------------------------------------- */
static int gif_decompress_init(struct gif *gif) 
{  
  uint32_t  i;
  uint8_t   code_size;
  
  if(gif_get_byte(gif, &code_size))
    return -1;
  
  gif->lzw.buf[0]       = 0;
  gif->lzw.code_size    = code_size;
  gif->lzw.clear_code   = (1 << code_size);
  gif->lzw.eof_code     = gif->lzw.clear_code + 1;
  gif->lzw.running_code = gif->lzw.eof_code + 1;
  gif->lzw.running_bits = code_size + 1;
  gif->lzw.max_code1    = 1 << gif->lzw.running_bits;
  gif->lzw.stack_ptr    = 0;
  gif->lzw.last_code    = GIF_NO_SUCH_CODE;
  gif->lzw.shift_state  = 0;
  gif->lzw.shift_dword  = 0;
  
  for(i = 0; i <= GIF_LZ_MAX_CODE; i++)
    gif->lzw.prefix[i] = GIF_NO_SUCH_CODE;

  return 0;
}

static int gif_decompress_buffer(struct gif *gif, uint8_t *buf, uint8_t *next) 
{
  if(buf[0] == 0) 
  {
    if(gif_get_byte(gif, buf))
      return -1;
    
    if(gif_read(gif, &buf[1], buf[0]) != buf[0])
      return -1;
      
    *next = buf[1];
    buf[1] = 2;
    buf[0]--;
  } 
  else 
  {
    *next = buf[buf[1]++];
    buf[0]--;
  }
  
  return 0;
}

static int gif_decompress_input(struct gif *gif, int *code) 
{
  uint8_t next;
  
  while(gif->lzw.shift_state < gif->lzw.running_bits) 
  {
    if(gif_decompress_buffer(gif, gif->lzw.buf, &next))
      return -1;
    
    gif->lzw.shift_dword |= ((uint32_t)next) << gif->lzw.shift_state;
    gif->lzw.shift_state += 8;
  }
  
  *code = gif->lzw.shift_dword & 
          gif_decompress_masks[gif->lzw.running_bits];
  
  gif->lzw.shift_dword >>= gif->lzw.running_bits;
  gif->lzw.shift_state  -= gif->lzw.running_bits;
  
  if(++gif->lzw.running_code > gif->lzw.max_code1 &&
     gif->lzw.running_bits < GIF_LZ_BITS) 
  {
    gif->lzw.max_code1 <<= 1;
    gif->lzw.running_bits++;
  }
  
  return 0;
}

static int gif_decompress_prefix(uint32_t *prefix, int code, int clear_code) 
{
  uint32_t i = 0;
  
  while(code > clear_code && i++ <= GIF_LZ_MAX_CODE)
    code = prefix[code];
  
  return code;
}

static int gif_decompress_code(struct gif *gif, uint8_t **block) 
{
  uint8_t buf;
  
  if(gif_get_byte(gif, &buf))
    return -1;
  
  if(buf > 0) 
  {
    *block = gif->lzw.buf;
    (*block)[0] = buf;
    
    if(gif_read(gif, &((*block)[1]), 1) != 1)
      return -1;
    
  }
  else 
  {
    *block = NULL;
    gif->lzw.buf[0] = 0;
    gif->lzw.pixel_count = 0;
  }
  
  return 0;
}    

static int gif_decompress_line(struct gif *gif, uint8_t *line, uint16_t len) 
{
  int  i = 0;
  int  j;
  int  crnt_code;
  int  eof_code;
  int  clear_code;
  int  crnt_prefix;
  int  last_code;
  int  stack_ptr;
  uint8_t  *stack;
  uint8_t  *suffix;
  uint32_t *prefix;
  
  stack_ptr  = gif->lzw.stack_ptr;
  prefix     = gif->lzw.prefix;
  suffix     = gif->lzw.suffix;
  stack      = gif->lzw.stack;
  eof_code   = gif->lzw.eof_code;
  clear_code = gif->lzw.clear_code;
  last_code  = gif->lzw.last_code;
  
  if(stack_ptr != 0) 
  {
    while(stack_ptr != 0 && i < len)
      line[i++] = stack[--stack_ptr];
  }
  
  while(i < len) 
  {
    if(gif_decompress_input(gif, &crnt_code))
      return -1;
    
    if(crnt_code == eof_code) {
      
      if(i != len - 1 || gif->lzw.pixel_count != 0)
        return -1;
      
      i++;
      
    } 
    else if(crnt_code == clear_code) 
    {
      for(j = 0; j <= GIF_LZ_MAX_CODE; j++)
        prefix[j] = GIF_NO_SUCH_CODE;
      
      gif->lzw.running_code = gif->lzw.eof_code + 1;
      gif->lzw.running_bits = gif->lzw.code_size + 1;
      gif->lzw.max_code1    = 1 << gif->lzw.running_bits;
      
      last_code = gif->lzw.last_code = GIF_NO_SUCH_CODE;
      
    } 
    else 
    {
      if(crnt_code < clear_code) 
      {
        line[i++] = crnt_code;
      }
      else 
      {
        if(prefix[crnt_code] == GIF_NO_SUCH_CODE) 
        {
          if(crnt_code == gif->lzw.running_code - 2) 
          {
            crnt_prefix = last_code;
            
            suffix[gif->lzw.running_code - 2] =
            stack[stack_ptr++] = 
              gif_decompress_prefix(prefix, last_code, clear_code);
          } 
          else 
          {
            return -1;
          }
        } 
        else 
        {
          crnt_prefix = crnt_code;
        }
        
        j = 0;
        
        while(j++ <= GIF_LZ_MAX_CODE &&
              crnt_prefix > clear_code &&
              crnt_prefix <= GIF_LZ_MAX_CODE) 
        {
          stack[stack_ptr++] = suffix[crnt_prefix];
          crnt_prefix = prefix[crnt_prefix];
        }
        
        if(j >= GIF_LZ_MAX_CODE || crnt_prefix > GIF_LZ_MAX_CODE)
          return -1;
          
        stack[stack_ptr++] = crnt_prefix;
        
        while(stack_ptr != 0 && i < len)
          line[i++] = stack[--stack_ptr];
      }
      
      if(last_code != GIF_NO_SUCH_CODE) 
      {
        prefix[gif->lzw.running_code - 2] = last_code;
        
        if(crnt_code == gif->lzw.running_code - 2)
          suffix[gif->lzw.running_code - 2] = 
            gif_decompress_prefix(prefix, last_code, clear_code);
        else
          suffix[gif->lzw.running_code - 2] =
            gif_decompress_prefix(prefix, crnt_code, clear_code);
      }
      
      last_code = crnt_code;
    }
  }
  
  gif->lzw.last_code = last_code;
  gif->lzw.stack_ptr = stack_ptr;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * LZW Compression code                                                       *
 * -------------------------------------------------------------------------- */
static int gif_compress_buffer(struct gif *gif, uint8_t *buf, int code) 
{  
  if(code == GIF_FLUSH_OUTPUT) 
  {
    
    if(buf[0] != 0 && gif_write(gif, buf, buf[0] + 1) != buf[0] + 1)
      return -1;
    
    buf[0] = 0;
    
    if(gif_put_byte(gif, buf[0]))
      return -1;
  }
  else 
  { 
    if(buf[0] == 255) 
    {
      if(gif_write(gif, buf, buf[0] + 1) != buf[0] + 1)
        return -1;
      
      buf[0] = 0;
    }
    
    buf[++buf[0]] = code;
  }
  
  return 0;
}

static int gif_compress_output(struct gif *gif, int code) 
{
  int ret = 0;
  
  if(code == GIF_FLUSH_OUTPUT) 
  {
    while(gif->lzw.shift_state > 0) 
    {
      if(gif_compress_buffer(gif, gif->lzw.buf, 
                             gif->lzw.shift_dword & 0xff))
        ret = -1;
      
      gif->lzw.shift_dword >>= 8;
      gif->lzw.shift_state -= 8;
    }
    
    gif->lzw.shift_state = 0;
    
    if(gif_compress_buffer(gif, gif->lzw.buf, GIF_FLUSH_OUTPUT))
      ret = -1;
  } 
  else 
  {
    gif->lzw.shift_dword |= ((long)code) << gif->lzw.shift_state;
    gif->lzw.shift_state += gif->lzw.running_bits;
    
    while(gif->lzw.shift_state >= 8) 
    {
      if(gif_compress_buffer(gif, gif->lzw.buf, 
                             gif->lzw.shift_dword & 0xff))
        ret = -1;
      
      gif->lzw.shift_dword >>= 8;
      gif->lzw.shift_state -= 8;
    }
  }
  
  if(gif->lzw.running_code >= gif->lzw.max_code1 && code <= 4096)
    gif->lzw.max_code1 = 1 << ++gif->lzw.running_bits;
  
  return ret;
}

static int gif_compress_init(struct gif *gif) 
{  
  int     bpp;
  uint8_t buf;
  
  if(gif->desc.palette != NULL)
    bpp = gif->desc.palette->bpp;
  else if(gif->palette != NULL)
    bpp = gif->palette->bpp;
  else
    return -1;
  
  buf = bpp = (bpp < 2 ? 2 : bpp);
  
  if(gif_put_byte(gif, buf))
    return -1;
  
  gif->lzw.buf[0]       = 0;
  gif->lzw.code_size    = bpp;
  gif->lzw.clear_code   = (1 << bpp);
  gif->lzw.eof_code     = gif->lzw.clear_code + 1;
  gif->lzw.running_code = gif->lzw.eof_code + 1;
  gif->lzw.running_bits = bpp + 1;
  gif->lzw.max_code1    = 1 << gif->lzw.running_bits;
  gif->lzw.crnt_code    = GIF_FIRST_CODE;
  gif->lzw.shift_state  = 0;
  gif->lzw.shift_dword  = 0;
  
  gif_ht_clear(gif->lzw.table);
  
  if(gif_compress_output(gif, gif->lzw.clear_code))
    return -1;
  
  return 0;
}


static int gif_compress_line(struct gif *gif, uint8_t *line, uint32_t len) 
{
  int                   i = 0;
  int                   crnt_code;
  int                   new_code;
  uint32_t              new_key;
  uint8_t               pixel;
  struct gif_hashtable *table;
  
  table = gif->lzw.table;
  
  if(gif->lzw.crnt_code == GIF_FIRST_CODE)
    crnt_code = line[i++];
  else
    crnt_code = gif->lzw.crnt_code;
  
  while(i < len) 
  {
    pixel = line[i++];
    
    new_key = (((uint32_t)crnt_code) << 8) + pixel;
    
    if((new_code = gif_ht_exists(table, new_key)) >= 0) 
    {
      crnt_code = new_code;
    } 
    else 
    { 
      if(gif_compress_output(gif, crnt_code))
        return -1;
    
      crnt_code = pixel;
      
      if(gif->lzw.running_code >= GIF_LZ_MAX_CODE) { 
        
        if(gif_compress_output(gif, gif->lzw.clear_code))
          return -1;
        
        gif->lzw.running_code = gif->lzw.eof_code + 1;
        gif->lzw.running_bits = gif->lzw.code_size + 1;
        gif->lzw.max_code1    = 1 << gif->lzw.running_bits;
        
        gif_ht_clear(table);
        
      } else { 
        gif_ht_insert(table, new_key, gif->lzw.running_code++);
      }
    }
  }

  gif->lzw.crnt_code = crnt_code;
  
  if(gif->lzw.pixel_count == 0) { 
    
    if(gif_compress_output(gif, crnt_code))
      return -1;
    
    if(gif_compress_output(gif, gif->lzw.eof_code))
      return -1;
    
    if(gif_compress_output(gif, GIF_FLUSH_OUTPUT))
      return -1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Initialize GIF code                                                        *
 * -------------------------------------------------------------------------- */
void gif_init(void)
{
  gif_log = log_source_register("gif");
  
  mem_static_create(&gif_heap, sizeof(struct gif), GIF_BLOCK_SIZE);
  mem_static_note(&gif_heap, "gif heap");
  mem_static_create(&gif_image_heap, sizeof(struct gif_image), GIF_BLOCK_SIZE);
  mem_static_note(&gif_image_heap, "gif image heap");
  mem_static_create(&gif_ht_heap, sizeof(struct gif_hashtable), GIF_BLOCK_SIZE);
  mem_static_note(&gif_ht_heap, "gif hashtable heap");
  mem_dynamic_create(&gif_data_heap, 128 * 1024);
  mem_dynamic_note(&gif_data_heap, "gif data heap");
  mem_dynamic_note(&gif_palette_heap, "gif data heap");
  mem_dynamic_create(&gif_palette_heap, (sizeof(struct palette) +
                                         sizeof(struct color) * 256) *
                                         GIF_BLOCK_SIZE * 2);
  mem_dynamic_note(&gif_palette_heap, "gif palette heap");
  
  dlink_list_zero(&gif_list);
  
  log(gif_log, L_status, "Initialized [gif] module.");
}

/* -------------------------------------------------------------------------- *
 * Shut down GIF code                                                         *
 * -------------------------------------------------------------------------- */
void gif_shutdown(void)
{
  log(gif_log, L_status, "Shutting down [gif] module.");
  
  mem_dynamic_destroy(&gif_data_heap);
  mem_dynamic_destroy(&gif_palette_heap);
  mem_static_destroy(&gif_image_heap);
  mem_static_destroy(&gif_ht_heap);
  mem_static_destroy(&gif_heap);
  
  log_source_unregister(gif_log);
}

/* -------------------------------------------------------------------------- *
 * Create a GIF instance                                                      *
 * -------------------------------------------------------------------------- */
struct gif *gif_new(const char *name, int state)
{
  struct gif           *gif;
  struct gif_hashtable *ht;
  
  if((ht = gif_ht_new()) == NULL)
    return NULL;
  
  gif = mem_static_alloc(&gif_heap);
  
  memset(gif, 0, sizeof(struct gif));
  
  gif->id = gif_id++;
  gif->lzw.table = ht;
  
  dlink_add_tail(&gif_list, &gif->node, gif);
  
  strlcpy(gif->name, name, sizeof(gif->name));
  gif->nhash = strhash(gif->name);
  
  gif->status = state;
  gif->mem = mem_dynamic_alloc(&gif_data_heap, 0);
  gif->size = 0;
  gif->fd = -1;
  
  return gif;
}

/* -------------------------------------------------------------------------- *
 * Delete a GIF instance                                                      *
 * -------------------------------------------------------------------------- */
void gif_delete(struct gif *gif)
{
  if(gif->lzw.table)
    gif_ht_free(gif->lzw.table);
  
  dlink_delete(&gif_list, &gif->node);
  
  mem_static_free(&gif_heap, gif);
}

/* -------------------------------------------------------------------------- *
 * Open GIF file                                                              *
 * -------------------------------------------------------------------------- */
struct gif *gif_open_file(const char *filename, uint32_t state) 
{
  int fd;
  
  fd = io_open(filename, 
               (GIF_STATE_WRITE ? (O_WRONLY | O_TRUNC | O_CREAT) : O_RDONLY),
               0644);
  
  if(fd == -1)
    return NULL;
  
  return gif_open_fd(fd, state);
}

/* -------------------------------------------------------------------------- *
 * Open GIF filedescriptor                                                    *
 * -------------------------------------------------------------------------- */
struct gif *gif_open_fd(int fd, int state)
{ 
  struct gif *gif;
  
  if(fd == -1)
    return NULL;

  if((gif = gif_new(io_list[fd].note, state)) == NULL)
    return NULL;
  
  gif->fd = fd;
  
  if(gif_verify(gif) == -1)
  {
    io_shutup(fd);
    gif_delete(gif);
    return NULL;
  }
  
  return gif;
}

/* -------------------------------------------------------------------------- *
 * Open GIF file                                                              *
 * -------------------------------------------------------------------------- */
struct gif *gif_open_mem(void *mem, size_t n, int state)
{
  
  struct gif *gif;
  
  if((gif = gif_new("memory", state)) == NULL)
    return NULL;
  
  if(state & GIF_READ) 
  {
    gif->mem = mem;
    gif->offset = 0;
    gif->size = n;
  } 
  else if(state & GIF_WRITE) 
  {
    gif->mem = NULL;
    gif->offset = 0;
    gif->size = 0;
  }
  
  return gif;
}

/* -------------------------------------------------------------------------- *
 * Clear structural data                                                      *
 * -------------------------------------------------------------------------- */
void gif_clear_struct(struct gif *gif)
{
  if(gif->desc.palette)
    gif_palette_free(gif->desc.palette);
  
  if(gif->palette)
    gif_palette_free(gif->palette);
  
  if(gif->lzw.table != NULL)
    gif_ht_free(gif->lzw.table);
}

/* -------------------------------------------------------------------------- *
 * Clear raw data                                                             *
 * -------------------------------------------------------------------------- */
void gif_clear_raw(struct gif *gif)
{
  if(gif->mem)
    mem_dynamic_free(&gif_data_heap, gif->mem);
  
  gif->mem = NULL;
  gif->size = 0;
  gif->offset = 0;
}

/* -------------------------------------------------------------------------- *
 * Clear all                                                                  *
 * -------------------------------------------------------------------------- */
void gif_clear(struct gif *gif)
{
  gif_clear_struct(gif);
  gif_clear_raw(gif);
} 

/* -------------------------------------------------------------------------- *
 * Terminate and close file                                                   *
 * -------------------------------------------------------------------------- */
void gif_close(struct gif *gif) 
{
  
  uint8_t buf;
  
  if(gif->status & GIF_WRITE)
  {
    buf = ';';
    gif_put_byte(gif, buf);
  }

//  if(io_valid(gif->fd))
  io_shutup(gif->fd);
}

/* -------------------------------------------------------------------------- *
 * Get screen descriptor                                                      *
 * -------------------------------------------------------------------------- */
int gif_screen_get(struct gif *gif) 
{  
  uint32_t i;
  uint32_t bpp;
  uint8_t  buf[3];
  
  if(!(gif->status & GIF_STATE_READ))
    return -1;
  
  if(gif_get_word(gif, &gif->width) ||
     gif_get_word(gif, &gif->height))
  {
    log(gif_log, L_warning, "cannot read screen dimensions");
    return -1;
  }
  
  if(gif_read(gif, buf, 3) != 3)
  {
    log(gif_log, L_warning, "cannot read screen resolution");
    return -1;
  }

  gif->resolution = (((buf[0] & 0x70) + 1) >> 4) + 1;
  
  bpp = (buf[0] & 0x07) + 1;
  
  gif->background = buf[1];
  
  if(buf[0] & 0x80) {
    
    if((gif->palette = gif_palette_new(1 << bpp)) == NULL)
    {      
      log(gif_log, L_warning, "cannot allocate palette");
      return -1;
    }
    
    for(i = 0; i < gif->palette->count; i++) {
      if(gif_read(gif, buf, 3) != 3)
      {
        log(gif_log, L_warning, "cannot read palette entry");
        return -1;
      }
      
      gif->palette->colors[i].r = buf[0];
      gif->palette->colors[i].g = buf[1];
      gif->palette->colors[i].b = buf[2];
      gif->palette->colors[i].a = 0xff;
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Put screen descriptor                                                      *
 * -------------------------------------------------------------------------- */
int gif_screen_put(struct gif *gif,        uint16_t        width,
                   uint16_t    height,     int             resolution, 
                   uint8_t     background, struct palette *pal) 
{
  uint8_t  buf[3];
  uint32_t i;
  
  if(gif->status & GIF_STATE_SCREEN)
    return -1;
  
  if(!(gif->status & GIF_STATE_WRITE)) 
    return -1;
  
  if(gif_write(gif, GIF_MAGIC, GIF_MAGIC_LEN) != GIF_MAGIC_LEN) 
    return -1;
  
  gif->width      = width;
  gif->height     = height;
  gif->resolution = resolution;
  gif->background = background;
  
  if(pal == NULL)
    gif->palette = NULL;
  else
    gif->palette = gif_palette_copy(pal);
  
  if(gif_put_word(gif, width) ||
     gif_put_word(gif, height) ||
     gif_put_byte(gif, (pal == NULL ? 0x00 : 0x80) |
                       ((resolution - 1) << 4) |
                       (pal == NULL ? 0 : pal->bpp - 1)) ||
     gif_put_byte(gif, background) ||
     gif_put_byte(gif, 0))
    return -1;
    
  if(pal != NULL) 
  {
    for(i = 0; i < pal->count; i++) 
    {
      buf[0] = pal->colors[i].r;
      buf[1] = pal->colors[i].g;
      buf[2] = pal->colors[i].b;
      
      if(gif_write(gif, buf, 3) != 3)
        return -1;
    }
  }
  
  gif->status |= GIF_STATE_SCREEN;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Get an image                                                               *
 * -------------------------------------------------------------------------- */
int gif_image_get(struct gif *gif) 
{  
  uint32_t          i;
  uint32_t          bpp;
  uint8_t           buf[3];
  struct gif_image *image;
  size_t            n;
  
  if(!(gif->status & GIF_STATE_READ))
    return -1;
  
  if(gif_get_word(gif, &gif->desc.left) ||
     gif_get_word(gif, &gif->desc.top) ||
     gif_get_word(gif, &gif->desc.width) ||
     gif_get_word(gif, &gif->desc.height))
    return -1;

  n = gif->desc.width * gif->desc.height;
  
  if(gif_get_byte(gif, buf))
    return -1;
  
  bpp = (buf[0] & 0x07) + 1;
  gif->desc.interlace = (buf[0] & 0x40);
  
  if(buf[0] & 0x80) 
  {
    if(gif->desc.palette != NULL && gif->images.size == 0)
      gif_palette_free(gif->desc.palette);
    
    gif->desc.palette = gif_palette_new(1 << bpp);
    
    for(i = 0; i < gif->desc.palette->count; i++) 
    {
      if(gif_read(gif, buf, 3) != 3)
        return -1;
      
      gif->desc.palette->colors[i].r = buf[0];
      gif->desc.palette->colors[i].g = buf[1];
      gif->desc.palette->colors[i].b = buf[2];
      gif->desc.palette->colors[i].a = 0xff;
    }
  }
  
  image = mem_dynamic_alloc(&gif_data_heap, sizeof(struct gif_image) + n);
  
  dlink_add_tail(&gif->images, &image->node, image);
  
  memcpy(&image->desc, &gif->desc, sizeof(struct gif_descriptor));
  
  if(gif->desc.palette != NULL) 
  {
    image->desc.palette = malloc(sizeof(struct palette));
    memcpy(image->desc.palette, gif->desc.palette, sizeof(struct palette));
    
    image->desc.palette->colors = malloc(sizeof(struct color) * 
                                         gif->desc.palette->count);
    memcpy(image->desc.palette->colors, gif->desc.palette->colors,
           sizeof(struct color) * gif->desc.palette->count);
  }
  
  image->bits = NULL;
  
  dlink_list_zero(&image->exts);
  
  gif->lzw.pixel_count = (uint32_t)gif->desc.width * 
                           (uint32_t)gif->desc.height;
  
  return gif_decompress_init(gif);
}

/* -------------------------------------------------------------------------- *
 * Put an image                                                               *
 * -------------------------------------------------------------------------- */
int gif_image_put(struct gif     *gif,   int16_t  left,   int16_t top, 
                  uint16_t        width, uint16_t height, int     interlace, 
                  struct palette *pal) 
{
  uint32_t i;
  uint8_t  buf[3];
  
  if(gif->status & GIF_STATE_IMAGE)
    return -1;
  
  if(!(gif->status & GIF_STATE_WRITE))
    return -1;
  
  gif->desc.left      = left;
  gif->desc.top       = top;
  gif->desc.width     = width;
  gif->desc.height    = height;
  gif->desc.interlace = interlace;
  
  if(pal == NULL)
    gif->desc.palette = NULL;
  else
    gif->desc.palette = gif_palette_copy(pal);

  if(gif_put_byte(gif, ',') ||
     gif_put_word(gif, left) ||
     gif_put_word(gif, top) ||
     gif_put_word(gif, width) ||
     gif_put_word(gif, height) ||
     gif_put_byte(gif, (pal == NULL ? 0x00 : 0x80) |
                       (interlace ? 0x40 : 0x00) |
                       (pal == NULL ? 0 : pal->bpp - 1)))
    return -1;
  
  if(pal != NULL) 
  {
    for(i = 0; i < pal->count; i++) 
    { 
      buf[0] = pal->colors[i].r;
      buf[1] = pal->colors[i].g;
      buf[2] = pal->colors[i].b;
      
      if(gif_write(gif, buf, 3) != 3)
        return -1;
    }
  }
  
  if(gif->palette == NULL && gif->desc.palette == NULL)
    return -1;
  
  gif->status |= GIF_STATE_IMAGE;
  gif->lzw.pixel_count = width * height;
  
  return gif_compress_init(gif);
}

/* -------------------------------------------------------------------------- *
 * Get data from current image                                                *
 * -------------------------------------------------------------------------- */
int gif_data_get(struct gif *gif, uint8_t *data, uint32_t len)
{
  uint8_t *dummy;

  if(!(gif->status & GIF_STATE_READ))
    return -1;
  
  if(len == 0)
    len = gif->desc.width;
  
  if((gif->lzw.pixel_count -= len) > 0xffff0000)
    return -1;
  
  if(!gif_decompress_line(gif, data, len)) 
  {
    if(gif->lzw.pixel_count == 0) 
    {
      do 
      {
        if(gif_decompress_code(gif, &dummy))
          return -1;
        
      } 
      while(dummy != NULL);
    }
    
    return 0;
  }
  
  return -1;
}    

/* -------------------------------------------------------------------------- *
 * Put data into current image                                                *
 * -------------------------------------------------------------------------- */
int gif_data_put(struct gif *gif, uint8_t *data, uint32_t len)
{  
  uint32_t i;
  uint8_t  mask;
  
  if(!(gif->status & GIF_STATE_WRITE))
    return -1;
  
  if(len == 0)
    len = gif->desc.width;
  
  if(gif->lzw.pixel_count < len) { 
    return -1;
  }
  
  gif->lzw.pixel_count -= len;
  
  mask = gif_compress_masks[gif->lzw.code_size];
  
  for(i = 0; i < len; i++)
    data[i] &= mask;
  
  return gif_compress_line(gif, data, len);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_extension_next(struct gif *gif, uint8_t **ext) 
{ 
  uint8_t buf;
  
  if(gif_get_byte(gif, &buf))
    return -1;
  
  if(buf > 0) 
  {
    *ext      = gif->lzw.buf;
    (*ext)[0] = buf;
    
    if(gif_read(gif, &((*ext)[1]), buf) != buf)
      return -1;
  } 
  else 
  {
    *ext = NULL;
  }
  
  return 0;
}  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_extension_get(struct gif *gif, uint32_t *type, uint8_t **ext) 
{
  uint8_t buf;
  
  if(!(gif->status & GIF_STATE_READ))
    return -1;
  
  if(gif_get_byte(gif, &buf))
    return -1;
  
  *type = buf;
  
  return gif_extension_next(gif, ext);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_extension_put(struct gif *gif, uint8_t  extcode,
                      void       *ext, uint32_t extlen) 
{
  uint8_t buf[3];
  
  if(!(gif->status & GIF_STATE_WRITE))
    return -1;
  
  if(extcode == 0) 
  {
    if(gif_put_byte(gif, extlen))
      return -1;
  }
  else 
  {
    buf[0] = '!';
    buf[1] = extcode;
    buf[2] = extlen;
    
    if(gif_write(gif, buf, 3) != 3)
      return -1;
  }
  
  if(gif_write(gif, ext, extlen) != extlen)
    return -1;
  
  if(gif_put_byte(gif, 0))
    return -1;

  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_comment_put(struct gif *gif, char *comment)
{
  return gif_extension_put(gif, GIF_EXT_COMMENT, comment, strlen(comment));
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_get_gfx_control(struct gif_image *image,      int *disposal,
                        int              *user_input, int *trans,
                        uint16_t         *delay)
{
  struct gif_ext *ext;
  
  dlink_foreach(&image->exts, ext)
  {
    if(ext->type == GIF_EXT_GRAPHICS) 
      break;
  }
  
  if(ext == NULL)
    return -1;
  
  if(disposal != NULL)
    *disposal = (ext->buf[0] >> 1) & 0x07;
  
  if(user_input != NULL)
    *user_input = (ext->buf[0] >> 1) & 0x01;
  
  if(trans != NULL) 
  {
    *trans = ext->buf[0] & 0x01;
  
    if(*trans)
      *trans = ext->buf[3];
    else
      *trans = -1;
  }
  
  if(delay != NULL)
    *delay = (ext->buf[1] << 8) | ext->buf[2];
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_put_gfx_control(struct gif *gif,        int disposal,
                        int         user_input, int trans,
                        uint16_t    delay) 
{
  uint8_t ext[4];
  
  ext[0] = ((disposal & 0x07) << 1) |
           ((user_input & 0x01) << 1) |
           ((trans >= 0 && trans <= 255) ? 1 : 0);
  ext[1] = delay >> 8;
  ext[2] = delay & 0xff;
  ext[3] = ((trans >= 0 && trans <= 255) ? trans & 0xff : 0);
  
  return gif_extension_put(gif, GIF_EXT_GRAPHICS, ext, 4);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_get_record_type(struct gif *gif, enum gif_record *record) 
{  
  uint8_t type;
  
  if(!(gif->status & GIF_STATE_READ))
    return -1;
  
  if(gif_get_byte(gif, &type))
    return -1;
  
  switch(type) {
    case ',': *record = GIF_RECORD_IMAGE_DESC; break;
    case '!': *record = GIF_RECORD_EXTENSION;  break;
    case ';': *record = GIF_RECORD_TERMINATE;  break;
    default:  *record = GIF_RECORD_UNDEFINED;  return -1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_slurp(struct gif *gif) 
{  
  uint32_t          size;
  struct gif_image *image;
  enum gif_record   type;
  uint8_t          *ext;
  struct gif_image  temp;
  int               y;
  
  dlink_list_zero(&temp.exts);
  
  do { 
    
    if(gif_get_record_type(gif, &type))
      return -1;
    
    switch(type) {
      
      case GIF_RECORD_IMAGE_DESC:
      {
        if(gif_image_get(gif))
          return -1;
      
        image = (struct gif_image *)
          (gif->images.head ? gif->images.head->data : NULL);
        
        size = image->desc.width * image->desc.height;
        image->bits = malloc(size);
      
        if(image->bits == NULL)
          return -1;
      
        for(y = 0; y < image->desc.height; y++)
          if(gif_data_get(gif, &image->bits[y * image->desc.width], 
                          image->desc.width))
            return -1;
      
        image->exts = temp.exts;

        break;
      }
      case GIF_RECORD_EXTENSION:
      {
        if(gif_extension_get(gif, &temp.type, &ext))
          return -1;
      
        while(ext != NULL) 
        {
          if(gif_extension_add(&temp, ext[0], &ext[1]) == NULL)
            return -1;
          
          if(gif_extension_next(gif, &ext))
            return -1;
          
          temp.type = 0;
        }
      
        break;
      }
      case GIF_RECORD_TERMINATE:
        break;
      
      default:
        break;
    }
  } while(type != GIF_RECORD_TERMINATE);
  
  if(temp.exts.size)
    gif_extension_clear(&temp);
  
  return 0;
}



#ifdef HAVE_SDL
SDL_Surface *struct gif_to_surface(struct gif *gif) { 
     
  struct palette *pal = NULL;
  SDL_Surface   *ret;
  int            trans = -1;
  
  if(gif->images == NULL) return NULL;
  
  if((pal = gif->palette) == NULL &&
     (pal = gif->desc.palette) == NULL &&
     (pal = gif->images->desc.palette) == NULL)
    return NULL;
  
  ret = SDL_CreateRGBSurfaceFrom(gif->images->bits, gif->desc.width, 
                                 gif->desc.height, 8,
                                 gif->desc.width, 0, 0, 0, 0);
  if(ret == NULL)
    return NULL;
  
  SDL_SetColors(ret, (SDL_Color *)pal->colors, 0, pal->count);
  
  gif_get_gfx_control(gif->images, NULL, NULL, &trans, NULL);
  
  if(trans > 0)
    SDL_SetColorKey(ret, SDL_SRCCOLORKEY, trans);

  return ret;
}
#endif

/* -------------------------------------------------------------------------- *
 * Create a new palette                                                       *
 * -------------------------------------------------------------------------- */
struct palette *gif_palette_new(uint32_t ncolors)
{
  struct palette *pal;
  size_t          n;
  int             bpp;
  
  /* Round to either 2, 4, 8, 16, 32, 64, 128, 256 colors */
  bpp = gif_bit_size(ncolors);
  ncolors = 1 << bpp;
  
  /* Allocate memory for the palette */
  n = sizeof(struct palette) + (ncolors * sizeof(struct color));
  
  pal = mem_dynamic_alloc(&gif_palette_heap, n);
  
  /* Initialize palette header */
  pal->count = ncolors;
  pal->bpp = bpp;
  pal->colors = (struct color *)&pal[1];
  
  /* Initialize colors */
  memset(pal->colors, 0, pal->count * sizeof(struct color));
  
  return pal;
}

/* -------------------------------------------------------------------------- *
 * Create a new palette and initialise it                                     *
 * -------------------------------------------------------------------------- */
struct palette *gif_palette_make(uint32_t      ncolors,
                                 struct color *colors)
{
  struct palette *pal;

  /* Create new palette */
  pal = gif_palette_new(ncolors);
  
  /* Copy the colors */
  memcpy(pal->colors, colors, ncolors * sizeof(struct color));
  
  return pal;
}

/* -------------------------------------------------------------------------- *
 * Copy a palette                                                             *
 * -------------------------------------------------------------------------- */
struct palette *gif_palette_copy(struct palette *pal)
{
  return gif_palette_make(pal->count, pal->colors);
}

/* -------------------------------------------------------------------------- *
 * Free a palette                                                             *
 * -------------------------------------------------------------------------- */
void gif_palette_free(struct palette *pal)
{
  mem_dynamic_free(&gif_palette_heap, pal);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct gif_image *gif_image_add(struct gif     *gif,    int16_t  left,
                                int16_t         top,    uint16_t width, 
                                uint16_t        height, int      interlace,
                                struct palette *pal)
{
  struct gif_image *image;
  
  image = mem_static_alloc(&gif_image_heap);
  
  image->desc.left = left;
  image->desc.top = top;
  image->desc.width = width;
  image->desc.height = height;
  image->desc.interlace = interlace;
  image->desc.palette = pal;
  
  dlink_add_tail(&gif->images, &image->node, image);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void gif_image_delete(struct gif *gif, struct gif_image *image)
{
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void gif_image_clear(struct gif *gif)
{
}

/* -------------------------------------------------------------------------- *
 * Add an extension to an image header                                        *
 * -------------------------------------------------------------------------- */
struct gif_ext *gif_extension_add(struct gif_image *image, uint32_t len, 
                                  uint8_t          *data)
{
  struct gif_ext *ext;
  
  ext = mem_dynamic_alloc(&gif_data_heap, sizeof(struct gif_ext) + len);
  
  dlink_add_tail(&image->exts, &ext->node, ext);
  
  ext->size = len;
  ext->buf = (uint8_t *)&ext[1];
  ext->type = image->type;
  
  if(data && len)
    memcpy(ext->buf, data, len);
  else
    memset(ext->buf, 0, len);
  
  return ext;
}

/* -------------------------------------------------------------------------- *
 * Remove an extension from an image header                                   *
 * -------------------------------------------------------------------------- */
void gif_extension_delete(struct gif_image *image, struct gif_ext *ext)
{  
  dlink_delete(&image->exts, &ext->node);
  mem_dynamic_free(&gif_data_heap, ext);
}

/* -------------------------------------------------------------------------- *
 * Remove all extensions from an image header                                 *
 * -------------------------------------------------------------------------- */
void gif_extension_clear(struct gif_image *image)
{
  struct gif_ext *ext;
  struct node    *next;
  
  dlink_foreach_safe(&image->exts, ext, next)
    gif_extension_delete(image, ext);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int gif_save(struct gif *gif)
{
  gif->fd = io_open(gif->name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  if(gif->fd == -1)
    return -1;
  
  io_queue_control(gif->fd, OFF, OFF, OFF);
  
  io_write(gif->fd, gif->mem, gif->size);
  
  io_close(gif->fd);
  
  gif->fd = -1;
  
  return 0;
}
