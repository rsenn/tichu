/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003,2004  Roman Senn <smoli@paranoya.ch>
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
 * $Id: wav.c,v 1.1 2005/01/24 13:44:51 smoli Exp $
 */

#define _GNU_SOURCE

#include <math.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/config.h>
#include <libchaos/mem.h>
#include <libchaos/wav.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/gif.h>

struct wav_header
{
  unsigned char riff[4];
  unsigned char size[4];
  unsigned char wave[4];
  unsigned char fmt[4];
  unsigned char chnksize[4];
  unsigned char type[2];
  unsigned char channels[2];
  unsigned char rate[4];
  unsigned char bps[4];
  unsigned char align[2];
  unsigned char bpsample[2];
  unsigned char data[4];
  unsigned char datasize[4];
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int                wav_log; 
struct sheap       wav_heap;       /* heap containing wav blocks */
struct dheap       wav_data_heap;  
struct list        wav_list;       /* list linking wav blocks */
uint32_t           wav_id;
int                wav_dirty;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void wav_fourcc(unsigned char *buf, const char *str)
{
  buf[0] = str[0];
  buf[1] = str[1];
  buf[2] = str[2];
  buf[3] = str[3];
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void wav_newchunk(struct wav *wav)
{
  struct wav_chunk *chunk;
  
  chunk = mem_dynamic_alloc(&wav_data_heap, sizeof(struct wav_header) + 
                            WAV_CHUNK_SIZE);
  
  chunk->size = 0;
  
  dlink_add_tail(&wav->chunks, &chunk->node, chunk);
  
  wav->wpos = 0;
  
  wav->wchunk = chunk;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*static void wav_delchunk(struct wav *wav)
{
  struct wav_chunk *chunk;
  
  if(wav->chunks.head == NULL)
    return;
  
  chunk = wav->chunks.head->data;
  
  dlink_delete(&wav->chunks, &chunk->node);
  
  mem_dynamic_free(&wav_data_heap, chunk);
  
  wav->rpos = 0;
  
  wav->rchunk = (struct wav_chunk *)wav->chunks.head;
}*/
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void wav_inithdr(struct wav_header *header)
{
  memset(header, 0, sizeof(struct wav_header));
  
  wav_fourcc(header->riff, "RIFF");
  wav_fourcc(header->wave, "WAVE");
  wav_fourcc(header->fmt, "fmt ");
  wav_fourcc(header->data, "data");
  
  header->chnksize[0] = (header->data - header->type) & 0xff;
  header->chnksize[1] = (header->data - header->type) >> 8;
  
  header->type[0] = 0x01;
  header->type[1] = 0x00;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- *
 * Initialize wav heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void wav_init(void)
{
  wav_log = log_source_register("wav");
  
  dlink_list_zero(&wav_list);
  
  wav_id = 0;
  wav_dirty = 0;
  
  mem_static_create(&wav_heap, sizeof(struct wav), WAV_BLOCK_SIZE);
  mem_static_note(&wav_heap, "wav block heap");

  mem_dynamic_create(&wav_data_heap, sizeof(struct wav_header) + WAV_CHUNK_SIZE);
  mem_dynamic_note(&wav_data_heap, "wav data heap");

  log(wav_log, L_status, "Initialized [wav] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy wav heap                                                           *
 * -------------------------------------------------------------------------- */
void wav_shutdown(void)
{
  struct wav *iptr;
  struct wav *next;
  
  /* Report status */
  log(wav_log, L_status, "Shutting down [wav] module...");
  
  /* Remove all wav blocks */
  dlink_foreach_safe(&wav_list, iptr, next)
  {
    if(iptr->refcount)
      iptr->refcount--;

    wav_delete(iptr);
  }

  mem_static_destroy(&wav_heap);
  mem_dynamic_destroy(&wav_data_heap);
    
  /* Unregister log source */
  log_source_unregister(wav_log);
}

/* -------------------------------------------------------------------------- *
 * Collect wav block garbage.                                                 *
 * -------------------------------------------------------------------------- */
int wav_collect(void)
{
  struct wav *cnptr;
  struct wav *next;
  size_t         n = 0;
  
  if(wav_dirty)
  {
    /* Report verbose */
    log(wav_log, L_verbose, 
        "Doing garbage collect for [wav] module.");
    
    /* Free all wav blocks with a zero refcount */
    dlink_foreach_safe(&wav_list, cnptr, next)
    {
      if(!cnptr->refcount)
      {
        wav_delete(cnptr);
        
        n++;
      }
    }
  
    /* Collect garbage on wav_heap */
    mem_static_collect(&wav_heap);
    
    wav_dirty = 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void wav_default(struct wav *wav)
{
  dlink_node_zero(&wav->node);
  
  strcpy(wav->name, "default");
  wav->id = 0;
  wav->refcount = 0;
  wav->hash = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct wav *wav_new(const char *name, int bits, int channels, int rate)
{
  struct wav *wav;
  
  wav = mem_static_alloc(&wav_heap);
  
  wav->id = wav_id++;
  wav->refcount = 1;
  
  strlcpy(wav->name, name, sizeof(wav->name));
  strlcpy(wav->path, name, sizeof(wav->path));
  strlcat(wav->path, ".wav", sizeof(wav->path));
  
  wav->hash = strhash(wav->name);
  
  wav->bits = bits;
  wav->channels = channels;
  wav->rate = rate;
  
  wav->rpos = 0;
  wav->wpos = 0;
  wav->size = 0;
  wav->chunksize = WAV_CHUNK_SIZE;
  
  dlink_list_zero(&wav->chunks);
  
  dlink_add_tail(&wav_list, &wav->node, wav);

  log(wav_log, L_status, "Added wav block: %s (%u bits, %u channels, %u Hz",
      wav->name, bits, channels, rate);
  
  return wav;
}
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int wav_save(struct wav *wav)
{
  struct wav_header header;
  struct wav_chunk *chunk;
  size_t size;
  size_t rate;
  
  if((wav->fd = io_open(wav->path, O_WRONLY|O_CREAT|O_TRUNC, 0644)))
    return -1;
  
  wav_inithdr(&header);
  
  size = wav->size + sizeof(struct wav_header) - 8;
  
  header.size[0] = (size & 0x000000ff);
  header.size[1] = (size & 0x0000ff00) >> 8;
  header.size[2] = (size & 0x00ff0000) >> 16;
  header.size[3] = (size & 0xff000000) >> 24;
  
  header.channels[0] = wav->channels & 0xff;
  header.channels[1] = wav->channels >> 8;
    
  header.rate[0] = (wav->rate & 0x000000ff);
  header.rate[1] = (wav->rate & 0x0000ff00) >> 8;
  header.rate[2] = (wav->rate & 0x00ff0000) >> 16;
  header.rate[3] = (wav->rate & 0xff000000) >> 24;
  
  rate = wav->rate * wav->channels;
    
  if(wav->bits == WAV_16BIT)
    rate *= 2;
  
  header.bps[0] = (rate & 0x000000ff);
  header.bps[1] = (rate & 0x0000ff00) >> 8;
  header.bps[2] = (rate & 0x00ff0000) >> 16;
  header.bps[3] = (rate & 0xff000000) >> 24;
  
  rate = wav->channels * wav->bits;
  
  header.bpsample[0] = rate;
  
  io_write(wav->fd, &header, sizeof(struct wav_header));

  dlink_foreach(&wav->chunks, chunk)
  {
    io_write(wav->fd, chunk->data, chunk->size);
  }
  
  io_close(wav->fd);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void wav_put(struct wav *wav, short data)
{
  if(!wav->wchunk || wav->wpos >= wav->chunksize)
    wav_newchunk(wav);
    
  if(wav->bits == WAV_8BIT)
  {
    char *dataptr = (char *)wav->wchunk + wav->wpos;
    
    *dataptr = data;
    
    wav->size++;
    wav->wpos++;
    wav->wchunk->size++;
  }
  else
  {
    short *dataptr = (short *)((char *)wav->wchunk + wav->wpos);
    
    *dataptr = data;
    
    wav->size += 2;
    wav->wpos += 2;
    wav->wchunk->size += 2;
  }
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void wav_sample_put(struct wav *wav, short left, short right)
{
  wav_put(wav, left);
  wav_put(wav, right);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void wav_sample_putmono(struct wav *wav, short data)
{
  wav_put(wav, data);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void wav_delete(struct wav *wav)
{
  log(wav_log, L_status, "Deleting wav block: %s", wav->name);
 
  dlink_delete(&wav_list, &wav->node);
  
  mem_static_free(&wav_heap, wav);
}

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
void wav_release(struct wav *iptr)
{
  wav_dirty = 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void wav_set_name(struct wav *wav, const char *name)
{
  strlcpy(wav->name, name, sizeof(wav->name));
  
  wav->hash = strihash(wav->name);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *wav_get_name(struct wav *wav)
{
  return wav->name;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct wav *wav_find_name(const char *name)
{
  struct node   *node;
  struct wav *wav;
  uint32_t       hash;
  
  hash = strihash(name);
  
  dlink_foreach(&wav_list, node)
  {
    wav = node->data;
    
    if(wav->hash == hash)
    {
      if(!stricmp(wav->name, name))
        return wav;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct wav *wav_find_id(uint32_t id)
{
  struct wav *iptr;
  
  dlink_foreach(&wav_list, iptr)
  {
    if(iptr->id == id)
      return iptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Dump wavs and wav heap.                                                *
 * -------------------------------------------------------------------------- */
void wav_dump(struct wav *iptr)
{
  if(iptr == NULL)
  {
    dump(wav_log, "[============== wav summary ===============]");
    
    dlink_foreach(&wav_list, iptr)
      dump(wav_log, " #%03u: [%u] %-20s",
           iptr->id, 
           iptr->refcount,
           iptr->name);
    
    dump(wav_log, "[========== end of wav summary ============]");
  }
  else
  {
    dump(wav_log, "[============== wav dump ===============]");
    dump(wav_log, "         id: #%u", iptr->id);
    dump(wav_log, "   refcount: %u", iptr->refcount);
    dump(wav_log, "       hash: %p", iptr->hash);
    dump(wav_log, "       name: %s", iptr->name);

    dump(wav_log, "[========== end of wav dump ============]");    
  }
}
