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
 * $Id: mfile.c,v 1.16 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/mfile.h>
#include <libchaos/timer.h>
#include <libchaos/dlink.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int           mfile_log;
struct sheap  mfile_heap;
struct dheap  mfile_dheap;
struct list   mfile_list;
uint32_t      mfile_id;

/* -------------------------------------------------------------------------- *
 * Initialize mfile heap.                                                     *
 * -------------------------------------------------------------------------- */
void mfile_init(void)
{
  mfile_log = log_source_register("mfile");
  
  dlink_list_zero(&mfile_list);

  mfile_id = 0;
  
  mem_static_create(&mfile_heap, sizeof(struct mfile), MFILE_BLOCK_SIZE);
  mem_static_note(&mfile_heap, "mfile block heap");
  mem_dynamic_create(&mfile_dheap, MFILE_LINELEN);
  mem_dynamic_note(&mfile_dheap, "mfile line heap");

  log(mfile_log, L_status, "Initialized [mfile] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy mfile heap.                                                        *
 * -------------------------------------------------------------------------- */
void mfile_shutdown(void)
{
  struct mfile *mfptr;
  struct mfile *next;
  
  log(mfile_log, L_status, "Shutting down [mfile] module...");
  
  dlink_foreach_safe(&mfile_list, mfptr, next)
    mfile_delete(mfptr);
  
  mem_dynamic_destroy(&mfile_dheap);
  mem_static_destroy(&mfile_heap);
  
  log_source_unregister(mfile_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void mfile_read(int fd, struct mfile *mfptr)
{
  char          buf[MFILE_LINELEN];
  char         *eol = NULL;
  char         *line;
/*  struct mline *mlptr;*/
  struct node  *nptr;
  
  if(io_list[fd].status.err)
  {
    mfptr->fd = -1;
    
    if(io_list[fd].error > 0)
    {
      log(mfile_log, L_warning, "Cannot read %s: %s",
          syscall_strerror(io_list[fd].error));
      return;
    }
    
    while(io_gets(fd, buf, MFILE_LINELEN - 1))
    {
      if((eol = strchr(buf, '\r')))
        *eol = '\0';
      if((eol = strchr(buf, '\n')))
        *eol = '\0';
      if(eol == NULL)
        eol = buf + strlen(buf);
      
      line = mem_dynamic_alloc(&mfile_dheap, eol - buf + 1);
      nptr = dlink_node_new();
      dlink_add_tail(&mfptr->lines, nptr, line);
      
      strcpy(line, buf);
    }
    
    log(mfile_log, L_status, "Read %u lines from %s.",
        mfptr->lines.size, mfptr->path);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_load(const char *path)
{
  struct stat   st;
  struct mfile *mfptr;
  int           fd;
  char         *p;
  
  if(stat(path, &st) == -1)
  {
    log(mfile_log, L_warning, "Cannot stat %s: %s", 
        path, syscall_strerror(syscall_errno));
    
    return NULL;
  }
  
  if((fd = io_open(path, IO_READ)) == -1)
  {
    log(mfile_log, L_warning, "Cannot open %s: %s", 
        path, syscall_strerror(syscall_errno));
    
    return NULL;
  }
  
  mfptr = mem_static_alloc(&mfile_heap);
  
  mfptr->fd = fd;
  
  strlcpy(mfptr->path, path, sizeof(mfptr->path));

  if((p = strrchr(path, '/')))
    strlcpy(mfptr->name, &p[1], sizeof(mfptr->name));
  else
    strlcpy(mfptr->name, path, sizeof(mfptr->name));
  
  mfptr->phash = strhash(mfptr->path);
  mfptr->nhash = strihash(mfptr->name);
  
  io_queue_control(mfptr->fd, ON, OFF, ON);
  io_register(mfptr->fd, IO_CB_READ, mfile_read, mfptr);

  log(mfile_log, L_status, "Opened mfile: %s", mfptr->path);
  
  return mfptr;
}

/* -------------------------------------------------------------------------- *
 * Add a mfile.                                                               *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_add(const char *path)
{
  struct mfile *mfptr;
  
  mfptr = mfile_load(path);
  
  if(mfptr == NULL)
    return NULL;

  mfptr->id = mfile_id++;
  mfptr->refcount = 1;
  
  dlink_list_zero(&mfptr->lines);
  
  dlink_add_head(&mfile_list, &mfptr->node, mfptr);

/*  debug(mfile_log, "Added mfile: %s", mfptr->path);*/
  
  return mfptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int mfile_update(struct mfile *mfptr)
{
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove a mfile.                                                            *
 * -------------------------------------------------------------------------- */
void mfile_delete(struct mfile *mfptr)
{
  log(mfile_log, L_status, "Unloading mfile: %s", mfptr->path);
  
/*  if(io_valid(mfptr->fd))
  {*/
    io_shutup(mfptr->fd);
    mfptr->fd = -1;
//  }

  dlink_delete(&mfile_list, &mfptr->node);
  
  mem_static_free(&mfile_heap, mfptr);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_find_path(const char *path)
{
  struct mfile *mfptr;
  uint32_t      hash;
    
  hash = strhash(path);
  
  dlink_foreach(&mfile_list, mfptr)
  {
    if(mfptr->phash == hash)
    {
      if(!strcmp(mfptr->path, path))
        return mfptr;
    }
  }
  
  return NULL;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_find_name(const char *name)
{
  struct mfile *mfptr;
  uint32_t       hash;
    
  hash = strhash(name);
  
  dlink_foreach(&mfile_list, mfptr)
  {
    if(mfptr->nhash == hash)
    {
      if(!strcmp(mfptr->name, name))
        return mfptr;
    }
  }
  
  return NULL;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_find_id(uint32_t id)
{
  struct mfile *mfptr;
  
  dlink_foreach(&mfile_list, mfptr)
  {
    if(mfptr->id == id)
      return mfptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_pop(struct mfile *mfile)
{
  if(mfile)
  {
    if(!mfile->refcount)
      log(mfile_log, L_warning, "Poping deprecated mfile: %s",
          mfile->name);

    mfile->refcount++;
  }

  return mfile;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile *mfile_push(struct mfile **mfileptr)
{
  if(*mfileptr)
  {
    if((*mfileptr)->refcount == 0)
    {
      log(mfile_log, L_warning, "Trying to push deprecated user: %s",
          (*mfileptr)->name);
    }
    else
    {
      if(--(*mfileptr)->refcount == 0)
        mfile_delete(*mfileptr);
      
      (*mfileptr) = NULL;
    }
  }
  
  return *mfileptr;
}

/* -------------------------------------------------------------------------- *
 * Dump mfile list and heap.                                                  *
 * -------------------------------------------------------------------------- */
void mfile_dump(struct mfile *mfptr)
{
  if(mfptr == NULL)
  {
    dump(mfile_log, "[================ mfile summary ================]");
    
    dlink_foreach(&mfile_list, mfptr)
    {
      dump(mfile_log, " #%u: [%u] %-20s (%i)",
           mfptr->id, mfptr->refcount, mfptr->name, mfptr->fd);
    }

    dump(mfile_log, "[============= end of mfile summary ============]");
  }
  else
  {
    struct node *nptr;
    
    dump(mfile_log, "[================= mfile dump =================]");
    
    dump(mfile_log, "         id: #%u", mfptr->id);
    dump(mfile_log, "   refcount: %u", mfptr->refcount);
    dump(mfile_log, "      nhash: %p", mfptr->nhash);
    dump(mfile_log, "      phash: %p", mfptr->phash);
    dump(mfile_log, "         fd: %i", mfptr->fd);
    dump(mfile_log, "       path: %s", mfptr->path);
    dump(mfile_log, "       name: %s", mfptr->name);
    dump(mfile_log, "      lines: %u", mfptr->lines.size);
    
    dump(mfile_log, "------------------ mfile data ------------------");

    dlink_foreach(&mfptr->lines, nptr)
      dump(mfile_log, "%s", nptr->data ? nptr->data : "");
    
    dump(mfile_log, "[============== end of mfile dump =============]");    
  }
}
