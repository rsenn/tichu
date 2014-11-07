/* $Id: queue.h,v 1.4 2005/05/21 08:27:20 smoli Exp $
 * -------------------------------------------------------------------------- *
 *  .___.    .                                                                *
 *    |  * _.|_ . .        Portabler, SDL-basierender Client für das          *
 *    |  |(_.[ )(_|             Multiplayer-Kartenspiel Tichu.                *
 *  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . *
 *                                                                            *
 *               (c) 2004-2005 by Martin Zangger, Roman Senn                  *
 *                                                                            *
 *    Dieses Programm ist freie Software. Sie können es unter den Bedingungen *
 * der GNU General Public License, wie von der Free Software Foundation ver-  *
 * öffentlicht, weitergeben und/oder modifizieren, entweder gemäss Version 2  *
 * der Lizenz oder (nach Ihrer Option) jeder späteren Version.                *
 *                                                                            *
 *    Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, dass es  *
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die  *
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BE-    *
 * STIMMTEN ZWECK. Details finden Sie in der GNU General Public License.      *
 *                                                                            *
 *    Sie sollten eine Kopie der GNU General Public License zusammen mit      *
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free     *
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          *
 * 02111-1307, USA.                                                           *
 * -------------------------------------------------------------------------- */

#ifndef QUEUE_H
#define QUEUE_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include "dlink.h"

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define QUEUE_BLOCK_SIZE 32
#define QUEUE_CHUNK_SIZE 128

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
struct finfo {
  uint32_t head;                   /* Read ptr inside block */
  uint32_t tail;                   /* Write ptr inside block */
  uint32_t size;                   /* Byte count of the block */
  uint32_t lines;                  /* Line count of the block */
};

struct fblock 
{
  uint32_t     refcount;
  struct finfo info;
  uint8_t      data[QUEUE_CHUNK_SIZE]; /* Actual data */
};

struct fqueue 
{
  uint32_t     size;
  uint32_t     lines;
  struct list  blocks;
  struct finfo head;
  struct finfo tail;
};

#define queue_zero_info(x) do { \
  (x)->head = 0; \
  (x)->tail = 0; \
  (x)->size = 0; \
  (x)->lines = 0; \
} while(0);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void     queue_zero        (struct fqueue *fifoptr);

/* -------------------------------------------------------------------------- *
 * Garbage collect queue data                                                 *
 * -------------------------------------------------------------------------- */
extern void     queue_collect     (void);

/* -------------------------------------------------------------------------- *
 * Write data to the queue tail.                                              *
 * -------------------------------------------------------------------------- */
extern uint32_t queue_write       (struct fqueue *fifoptr,
                                   const void    *buf,
                                   uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Write a string to queue tail.                                              *
 * -------------------------------------------------------------------------- */
extern uint32_t queue_puts        (struct fqueue *fifoptr,
                                   const char    *s);

/* -------------------------------------------------------------------------- *
 * Read data from the queue head.                                             *
 * -------------------------------------------------------------------------- */
extern uint32_t queue_read        (struct fqueue *fifoptr,
                                   void          *buf,
                                   uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Get a line from queue tail.                                                *
 * -------------------------------------------------------------------------- */
extern uint32_t  queue_gets       (struct fqueue *fifoptr,
                                   void          *buf,
                                   uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Read stuff from the queue tail but do NOT dequeue it.                      *
 * -------------------------------------------------------------------------- */
extern uint32_t  queue_map         (struct fqueue *fifoptr,
                                    void          *buf, 
                                    uint32_t       n);  

/* -------------------------------------------------------------------------- *
 * Remove n bytes from queue tail.                                            *
 * -------------------------------------------------------------------------- */
extern uint32_t  queue_cut         (struct fqueue *fifoptr,
                                    uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Frees a queue.                                                             *
 * -------------------------------------------------------------------------- */
extern void      queue_free        (struct fqueue *fifoptr);

/* -------------------------------------------------------------------------- *
 * Link a queue to the tail of another.                                       *
 * -------------------------------------------------------------------------- */
extern void      queue_link        (struct fqueue *from, 
                                    struct fqueue *to);

/* -------------------------------------------------------------------------- *
 * Release a multicast list.                                                  *
 * -------------------------------------------------------------------------- */
extern void      queue_destroy     (struct fqueue *fifoptr);
    
/* -------------------------------------------------------------------------- *
 * Dump a queue.                                                              *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
extern void      queue_dump        (struct fqueue *fifoptr);
#endif /* DEBUG */  

#endif /* QUEUE_H */
