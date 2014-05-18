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
 * $Id: queue.c,v 1.28 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>
#include <libchaos/queue.h>
#include <libchaos/mem.h>
#include <libchaos/log.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */

int           queue_log;
struct sheap  queue_heap;
struct timer *queue_timer;
uint32_t      queue_serial;
struct list   queue_blocks;

/* -------------------------------------------------------------------------- *
 * Initialize heap for queue blocks.                                          *
 * -------------------------------------------------------------------------- */
void queue_init(void)
{
  queue_log = log_source_register("queue");
 
  queue_serial = 0;
  
  mem_static_create(&queue_heap, sizeof(struct fblock), QUEUE_BLOCK_SIZE);
  mem_static_note(&queue_heap, "queue heap");

  log(queue_log, L_status, "Initialized [queue] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy heap for queue blocks.                                             *
 * -------------------------------------------------------------------------- */
void queue_shutdown(void)
{
  log(queue_log, L_status, "Shutting down [queue] module...");
  
  mem_static_destroy(&queue_heap);
  
  log_source_unregister(queue_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void queue_zero(struct fqueue *fifoptr)
{
  fifoptr->lines = 0;
  fifoptr->size = 0;
  dlink_list_zero(&fifoptr->blocks);
  fifoptr->head.head = 0;
  fifoptr->head.tail = 0;
  fifoptr->head.size = 0;
  fifoptr->head.lines = 0;
  fifoptr->tail.head = 0;
  fifoptr->tail.tail = 0;
  fifoptr->tail.size = 0;
  fifoptr->tail.lines = 0;
}

/* -------------------------------------------------------------------------- *
 * Write data to the queue tail.                                              *
 * -------------------------------------------------------------------------- */
uint32_t queue_write(struct fqueue *fifoptr, const void *buf, uint32_t n)
{
  register struct finfo  *infoptr;
  register struct fblock *tailptr;
  struct fblock          *headptr;
  struct node            *nptr;
  uint32_t                i;
  
  tailptr = fifoptr->blocks.tail ? fifoptr->blocks.tail->data : NULL;
  headptr = fifoptr->blocks.head ? fifoptr->blocks.head->data : NULL;
  infoptr = &fifoptr->tail;  
  
  /* Loop through buffer while queueing data */
  for(i = 0; i < n; i++)
  {
    /* There's no tail, tail block is full or last fifo block
       was a multicast one, we need to start a new block */
    if(tailptr == NULL || infoptr->tail == QUEUE_CHUNK_SIZE ||
       (tailptr && tailptr->refcount > 1))
    {
      /* Update queue info when there is a tail 
         block, but DO NOT touch multicast blocks */
      if(tailptr && tailptr->refcount == 1)
        tailptr->info = *infoptr;
         
      /* It was the first block, so we need to update head information */
      if(headptr == tailptr)
        fifoptr->head = *infoptr;
         
      /* Allocate and link new queue block */
      tailptr = mem_static_alloc(&queue_heap);
      nptr = dlink_node_new();
      dlink_add_tail(&fifoptr->blocks, nptr, tailptr);      
      tailptr->refcount = 1;
      
      /* We added first block, update headptr */
      if(nptr->prev == NULL)
        headptr = tailptr;
      
      /* Clear queue info */
      infoptr->head = 0;
      infoptr->tail = 0;
      infoptr->size = 0;
      infoptr->lines = 0;
    }
    
    /* Copy a byte and update line count if a newline is spotted */
    if((tailptr->data[infoptr->tail++] = ((char *)buf)[i]) == '\n')
    {
      infoptr->lines++;
      fifoptr->lines++;
    }
    
    /* Update byte count */
    infoptr->size++;
    fifoptr->size++;
  }
  
  /* It was the first block, so we need to update head information */
  if(headptr == tailptr)
    fifoptr->head = *infoptr;

  if(tailptr)
    tailptr->info = *infoptr;
  
  /* Return bytes written */
  return i;
}

/* -------------------------------------------------------------------------- *
 * Write a string to queue tail.                                              *
 * -------------------------------------------------------------------------- */
uint32_t queue_puts(struct fqueue *fifoptr, const char *s)
{
  register struct finfo  *infoptr;
  register struct fblock *tailptr;
  struct fblock          *headptr;
  struct node            *nptr;
  uint32_t                i;
  
  tailptr = fifoptr->blocks.tail ? fifoptr->blocks.tail->data : NULL;
  headptr = fifoptr->blocks.head ? fifoptr->blocks.head->data : NULL;
  infoptr = &fifoptr->tail;  
  
  /* Loop through buffer while queueing data */
  for(i = 0; s[i]; i++)
  {
    /* There's no tail, tail block is full or last fifo block
       was a multicast one, we need to start a new block */
    if(tailptr == NULL || infoptr->tail == QUEUE_CHUNK_SIZE ||
       (tailptr && tailptr->refcount > 1))
    {
      /* Update queue info when there is a tail 
         block, but DO NOT touch multicast blocks */
      if(tailptr && tailptr->refcount == 1)
        tailptr->info = *infoptr;
         
      /* It was the first block, so we need to update head information */
      if(headptr == tailptr)
        fifoptr->head = *infoptr;

      /* Allocate and link new queue block */
      tailptr = mem_static_alloc(&queue_heap);
      nptr = dlink_node_new();
      dlink_add_tail(&fifoptr->blocks, nptr, tailptr);
      tailptr->refcount = 1;

      /* We added first block, update headptr */
      if(nptr->prev == NULL)
        headptr = tailptr;
      
      /* Clear queue info */
      infoptr->head = 0;
      infoptr->tail = 0;
      infoptr->size = 0;
      infoptr->lines = 0;
    }
    
    /* Queue a byte and update line count if a newline is spotted */
    if((tailptr->data[infoptr->tail++] = s[i]) == '\n')
    {
      infoptr->lines++;
      fifoptr->lines++;
    }
    
    /* Update byte count */
    infoptr->size++;
    fifoptr->size++;
  }
  
  /* We enqueued to head block, so we need to update head information */
  if(headptr == tailptr)
    fifoptr->head = *infoptr;

  if(tailptr)
    tailptr->info = *infoptr;
  
  /* Return bytes written */
  return i;
}

/* -------------------------------------------------------------------------- *
 * Read stuff from the queue head and dequeue it.                             *
 * -------------------------------------------------------------------------- */
uint32_t queue_read(struct fqueue *fifoptr, void *buf, uint32_t n)
{
  register struct finfo  *infoptr;
  register struct fblock *headptr;
  struct node            *nptr;
  struct node            *next;
  uint32_t                i;
  
  nptr = fifoptr->blocks.head;
  infoptr = &fifoptr->head;
  
  if(nptr == NULL)
    return 0;
  
  headptr = nptr->data;
  
  /* Loop through buffer while dequeueing data */
  for(i = 0; i < n; i++)
  {
    /* Head reached tail of the head block */
    if(infoptr->head == infoptr->tail)
    {
      /* Backup reference to next block */
      next = nptr->next;
      
      /* Unlink the head block */
      dlink_delete(&fifoptr->blocks, nptr);
      dlink_node_free(nptr);

      /* If its not a multicast block we have to free it */
      if(--headptr->refcount == 0)
        mem_static_free(&queue_heap, headptr);

      /* Advance to next block */
      if((nptr = next) == NULL)
      {
        infoptr->head = 0;
        infoptr->tail = 0;
        infoptr->size = 0;
        infoptr->lines = 0;
        break;
      }
      
      /* Get new head reference and update head info */
      headptr = nptr->data;      
      *infoptr = headptr->info;
    }
   
    /* Dequeue a byte and update line count if a newline is spotted */
    if((((char *)buf)[i] = headptr->data[infoptr->head++]) == '\n')
    {
      infoptr->lines--;
      fifoptr->lines--;
    }
    
    infoptr->size--;
    fifoptr->size--;
  }

  /* We dequeued from tail block, so we need to update tail information */
  if(nptr == fifoptr->blocks.tail)
    fifoptr->tail = *infoptr;

  mem_static_collect(&queue_heap);
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Get a line from queue head and dequeue it.                                 *
 * -------------------------------------------------------------------------- */
uint32_t queue_gets(struct fqueue *fifoptr, void *buf, uint32_t n)
{
  register struct finfo  *infoptr;
  register struct fblock *headptr;
  struct node            *nptr;
  struct node            *next;
  uint32_t                i;
  
  nptr = fifoptr->blocks.head;
  infoptr = &fifoptr->head;
  
  if(fifoptr->lines == 0)
    return 0;
  
  headptr = nptr->data;
  
  /* Loop through buffer while dequeueing data */
  for(i = 0;; i++)
  {
    /* Head reached tail of the head block */
    if(infoptr->head == infoptr->tail)
    {
      /* Backup reference to next block */
      next = nptr->next;
      
      /* Unlink the head block */
      dlink_delete(&fifoptr->blocks, nptr);
      dlink_node_free(nptr);

      /* If its not a multicast block we have to free it */
      if(--headptr->refcount == 0)
        mem_static_free(&queue_heap, headptr);

      /* Advance to next block */
      if((nptr = next) == NULL)
      {
        infoptr->head = 0;
        infoptr->tail = 0;
        infoptr->size = 0;
        infoptr->lines = 0;
        break;
      }
      
      /* Get new head reference and update head info */
      headptr = nptr->data;      
      *infoptr = headptr->info;
    }
   
    /* Dequeue a byte and update line count if a newline is spotted */
    infoptr->size--;
    fifoptr->size--;

    if(i < n)
    {
      if((((char *)buf)[i] = headptr->data[infoptr->head++]) == '\n')
      {
        infoptr->lines--;
        fifoptr->lines--;
        
        break;
      }
    }
    else
    {
      if(headptr->data[infoptr->head++] == '\n')
      {
        infoptr->lines--;
        fifoptr->lines--;
        
        break;
      }      
    }
  }

  /* We dequeued from tail block, so we need to update tail information */
  if(nptr == fifoptr->blocks.tail)
    fifoptr->tail = *infoptr;
  
  if(++i < n)
    ((char *)buf)[i] = '\0';
  else
    ((char *)buf)[n - 1] = '\0';
  
  mem_static_collect(&queue_heap);
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Read stuff from the queue tail but do NOT dequeue it.                      *
 * -------------------------------------------------------------------------- */
uint32_t queue_map(struct fqueue *fifoptr, void *buf, uint32_t n)
{
  register struct finfo  *infoptr;
  register struct fblock *headptr;
  struct finfo            headinfo;
  struct node            *nptr;
  struct node            *next;
  uint32_t                i;
  
  nptr = fifoptr->blocks.head;
  headinfo = fifoptr->head;  
  infoptr = &headinfo;
  
  if(nptr == NULL)
    return 0;
  
  headptr = nptr->data;
  
  /* Loop through buffer while mapping data */
  for(i = 0; i < n; i++)
  {
    /* Head reached tail of the head block */
    if(infoptr->head == infoptr->tail)
    {
      /* Backup reference to next block */
      next = nptr->next;
      
      /* Advance to next block */
      if((nptr = next) == NULL)
        break;
      
      /* Get new head reference and update head info */
      headptr = nptr->data;
      *infoptr = headptr->info;
    }
   
    /* Dequeue a byte */
    ((char *)buf)[i] = headptr->data[infoptr->head++];
  }
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Dequeue n bytes from queue head.                                           *
 * -------------------------------------------------------------------------- */
uint32_t queue_cut(struct fqueue *fifoptr, uint32_t n)
{
  register struct finfo  *infoptr;
  register struct fblock *headptr;
  struct node            *nptr;
  struct node            *next;
  uint32_t                i;
  
  nptr = fifoptr->blocks.head;
  infoptr = &fifoptr->head;
  
  if(nptr == NULL)
    return 0;
  
  headptr = nptr->data;
  
  /* Loop through buffer while dequeueing data */
  for(i = n; infoptr->size <= i;)
  {
    i -= infoptr->size;
    fifoptr->size -= infoptr->size;
    fifoptr->lines -= infoptr->lines;
    
    /* Backup reference to next block */
    next = nptr->next;
    
    /* Unlink the head block */
    dlink_delete(&fifoptr->blocks, nptr);
    dlink_node_free(nptr);
    
    /* If its not a multicast block we have to free it */
    if(--headptr->refcount == 0)
      mem_static_free(&queue_heap, headptr);

    /* Advance to next block */
    if((nptr = next) == NULL)
    {
      infoptr->head = 0;
      infoptr->tail = 0;
      infoptr->size = 0;
      infoptr->lines = 0;
      break;
    }
      
    /* Get new head reference and update head info */
    headptr = nptr->data;      
    *infoptr = headptr->info;
  }
  
  if(nptr)
  {
    infoptr->size -= i;
    fifoptr->size -= i;
    
    while(i--)
    {
      if(headptr->data[infoptr->head++] == '\n')
      {
        infoptr->lines--;
        fifoptr->lines--;
      }
      
    }
  }
  
  /* We dequeued from tail block, so we need to update tail information */
  if(nptr == fifoptr->blocks.tail)
    fifoptr->tail = *infoptr;
  
  mem_static_collect(&queue_heap);
  
  return i;
}
  
/* -------------------------------------------------------------------------- *
 * Link a queue to the tail of another.                                       *
 * -------------------------------------------------------------------------- */
void queue_link(struct fqueue *from, struct fqueue *to)
{
  struct fblock *fbcptr = NULL;
  struct list    newlist;
  struct node   *nptr;

  if(from->size)
  {
    if(to->blocks.head == NULL)
      to->head = from->head;
    
    to->tail = from->tail;
    
    to->size += from->size;
    to->lines += from->lines;
    
    dlink_foreach_data(&from->blocks, nptr, fbcptr)
      fbcptr->refcount++;
    
    dlink_copy(&from->blocks, &newlist);
    
    dlink_move_tail(&newlist, &to->blocks);
  }
}

/* -------------------------------------------------------------------------- *
 * Release a multicast list.                                                  *
 * -------------------------------------------------------------------------- */
void queue_destroy(struct fqueue *fifoptr)
{
  struct fblock *fbptr = NULL;
  struct node   *next;
  struct node   *node;
  
  dlink_foreach_safe_data(&fifoptr->blocks, node, next, fbptr)
  {
    if(--fbptr->refcount == 0)
      mem_static_free(&queue_heap, fbptr);
      
    dlink_node_free(node);
  }

  mem_static_collect(&queue_heap);
    
  queue_zero(fifoptr);
}

/* -------------------------------------------------------------------------- *
 * Dump a queue.                                                              *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void queue_dump(struct fqueue *fifoptr)
{
}
#endif /* DEBUG */
