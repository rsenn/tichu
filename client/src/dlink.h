/* $Id: dlink.h,v 1.12 2005/05/21 08:27:20 smoli Exp $
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

#ifndef DLINK_H
#define DLINK_H

#include <stdlib.h>
#include <stdint.h>

#define DLINK_BLOCK_SIZE 64

/* -------------------------------------------------------------------------- *
 * This is an element of a dlink_list                                         *
 * Contains pointer to previous/next element and a pointer to user data       *
 * -------------------------------------------------------------------------- */
#ifndef DLINK_NODE
#define DLINK_NODE
struct node 
{
  void        *data;
  struct node *prev;
  struct node *next;
};
#endif /* DLINK_NODE */

/* -------------------------------------------------------------------------- *
 * A dlink list contains a pointer to the head item and to the tail node      *
 * and the count of the nodes.                                                *
 * -------------------------------------------------------------------------- */
struct list 
{
  struct node *head;
  struct node *tail;
  size_t       size;
};

#define dlink_add dlink_add_head

/* -------------------------------------------------------------------------- *
 * Forward declaration of some mem.c                                          *
 * -------------------------------------------------------------------------- */
struct sheap;

extern void *mem_static_alloc   (struct sheap *shptr);

extern void  mem_static_free    (struct sheap *shptr,
                                 void         *scptr);

extern int   mem_static_collect (struct sheap *shptr);

/* -------------------------------------------------------------------------- *
 * Macros to walk through a dlink list from head to tail.                     *
 * -------------------------------------------------------------------------- */
/* n is set to the current node */
#define dlink_foreach_down(list, n) \
  for((n) = (void *)(list)->head; \
      (n) != NULL; \
      (n) = (void *)(((struct node *)n)->next))
 
/* n is set to the current node and d is set to n->data */
#define dlink_foreach_down_data(list, n, d) \
  for((n) = (void *)(list)->head; \
      ((n) != NULL) && (((d) = (void *)((struct node *)n)->data) != NULL); \
      (n) = (void *)((struct node *)n)->next)
 
/* n is set to the current node and n->next is backupped
   into m before loop body for safe walk-throught when 
   nodes get deleted */
#define dlink_foreach_down_safe(list, n, m) \
  for((n) = (void *)(list)->head, \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->next : NULL; \
      (n) != NULL; \
      (n) = (void *)((struct node *)m), \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->next : NULL)

/* same as above but sets d = n->data */
#define dlink_foreach_down_safe_data(list, n, m, d) \
  for((n) = (void *)(list)->head, \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->next : NULL; \
      ((n) != NULL) && (((d) = (void *)((struct node *)n)->data) != NULL); \
      (n) = (void *)((struct node *)m), \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->next : NULL)

/* -------------------------------------------------------------------------- *
 * Macros to walk through a dlink list from tail to head.                     *
 * -------------------------------------------------------------------------- */
/* n is set to the current node */
#define dlink_foreach_up(list, n) \
  for((n) = (void *)(list)->tail; \
      (n) != NULL; \
      (n) = (void *)(((struct node *)n)->prev))
 
/* n is set to the current node and d is set to n->data */
#define dlink_foreach_up_data(list, n, d) \
  for((n) = (void *)(list)->tail; \
      ((n) != NULL) && (((d) = (void *)((struct node *)n)->data) != NULL); \
      (d) = (void *)((struct node *)n)->data, \
      (n) = (void *)((struct node *)n)->prev)

/* n is set to the current node and n->prev is backupped
   into m before loop body for safe walk-throught when 
   nodes get deleted */
#define dlink_foreach_up_safe(list, n, m) \
  for((n) = (void *)(list)->tail, \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->prev : NULL; \
      (n) != NULL; \
      (n) = (void *)((struct node *)m), \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->prev : NULL)

/* same as above but sets d = n->data */
#define dlink_foreach_up_safe_data(list, n, m, d) \
  for((n) = (void *)(list)->tail, \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->prev : NULL; \
      ((n) != NULL) && (((d) = (void *)((struct node *)n)->data) != NULL); \
      (d) = (void *)((struct node *)n)->data, \
      (n) = (void *)((struct node *)m), \
      (m) = (void *)((struct node *)n) != NULL ? (void *)((struct node *)n)->prev : NULL)

/* aliases for backwards compatibility */
#define dlink_foreach           dlink_foreach_down
#define dlink_foreach_data      dlink_foreach_down_data
#define dlink_foreach_safe      dlink_foreach_down_safe
#define dlink_foreach_safe_data dlink_foreach_down_safe_data

/* -------------------------------------------------------------------------- *
 * Zero a dlink node                                                          *
 * -------------------------------------------------------------------------- */
#define dlink_node_zero(x) do { \
  ((struct node *)x)->data = NULL; \
  ((struct node *)x)->next = NULL; \
  ((struct node *)x)->prev = NULL; \
} while(0);

/* -------------------------------------------------------------------------- *
 * Zero a dlink list                                                          *
 * -------------------------------------------------------------------------- */
#define dlink_list_zero(x) do { \
  ((struct list *)x)->head = NULL; \
  ((struct list *)x)->tail = NULL; \
  ((struct list *)x)->size = 0; \
} while(0);

/* -------------------------------------------------------------------------- *
 * Allocate a new dlink node                                                  *
 * -------------------------------------------------------------------------- */
struct node               *dlink_node_new    (void);
extern inline struct node *dlink_node_new    (void)
{
  struct node *nptr;
  
  /* Allocate node block */
  nptr = malloc(sizeof(struct node));
  
  /* Zero */
  dlink_node_zero(nptr);
  
  return nptr;
}

/* -------------------------------------------------------------------------- *
 * Frees a dlink node                                                         *
 * -------------------------------------------------------------------------- */
void                dlink_node_free   (struct node *nptr);

extern inline void  dlink_node_free   (struct node *nptr)
{
  /* Free node block */
  free(nptr);
}

/* -------------------------------------------------------------------------- *
 * Add dlink node to the head of the dlink list.                              *
 *                                                                            *
 * <list>                   - list to add node to                             *
 * <node>                   - the node to add                                 *
 * <ptr>                    - a user-defined pointer                          *
 * -------------------------------------------------------------------------- */
void                dlink_add_head    (struct list *lptr,
                                       struct node *nptr,
                                       void        *ptr);

extern inline void  dlink_add_head    (struct list *lptr,
                                       struct node *nptr, 
                                       void        *ptr)
{
  /* Set the data pointer */
  nptr->data = ptr;
  
  /* We add to the list head, so there's no previous node */
  nptr->prev = NULL;
  
  /* Next node is the old head */
  nptr->next = lptr->head;
  
  /* If there already is a node at the head update 
     its prev-reference, else update the tail */
  if(lptr->head)
    lptr->head->prev = nptr;
  else
    lptr->tail = nptr;
  
  /* Now put the node to list head */
  lptr->head = nptr;
  
  /* Update list size */
  lptr->size++;
}

/* -------------------------------------------------------------------------- *
 * Add dlink node to the tail of the dlink list.                              *
 *                                                                            *
 * <list>                   - list to add node to                             *
 * <node>                   - the node to add                                 *
 * <ptr>                    - a user-defined pointer                          *
 * -------------------------------------------------------------------------- */
void                dlink_add_tail    (struct list *lptr,
                                       struct node *nptr, 
                                       void        *ptr);

extern inline void  dlink_add_tail    (struct list *lptr,
                                       struct node *nptr, 
                                       void        *ptr)
{
  /* Set the data pointer */
  nptr->data = ptr;
  
  /* We add to the list tail, so there's no next node */
  nptr->next = NULL;
  
  /* Previous node is the old tail */
  nptr->prev = lptr->tail;
  
  /* If there already is a node at the tail update 
     its prev-reference, else update the head */
  if(lptr->tail)
    lptr->tail->next = nptr;
  else
    lptr->head = nptr;
  
  /* Now put the node to list tail */
  lptr->tail = nptr;
  
  /* Update list size */
  lptr->size++;
}

/* -------------------------------------------------------------------------- *
 * Add dlink node to the dlink list before the specified node.                *
 *                                                                            *
 * <list>                   - list to add node to                             *
 * <node>                   - the node to add                                 *
 * <before>                 - add the new node before this node               *
 * <ptr>                    - a user-defined pointer                          *
 * -------------------------------------------------------------------------- */
void                dlink_add_before  (struct list *lptr,
                                       struct node *nptr,
                                       struct node *before,
                                       void        *ptr);

extern inline void  dlink_add_before  (struct list *lptr,
                                       struct node *nptr,
                                       struct node *before, 
                                       void        *ptr)
{
  /* If <before> is the list head, then a dlink_add_head() does the job */
  if(before == lptr->head)
  {
    dlink_add_head(lptr, nptr, ptr);
    return;
  }

  /* Set the data pointer */
  nptr->data = ptr;
  
  /* Make references on the new node */
  nptr->next = before;
  nptr->prev = before->prev;
  
  /* Update next-reference of the node before the <before> */
  before->prev->next = nptr;
  
  /* Update prev-reference of the <before> node */
  before->prev = nptr;
  
  /* Update list size */
  lptr->size++;
}

/* -------------------------------------------------------------------------- *
 * Add dlink node to the dlink list after the specified node.                 *
 *                                                                            *
 * <list>                   - list to add node to                             *
 * <node>                   - the node to add                                 *
 * <after>                  - add the new node after this node                *
 * <ptr>                    - a user-defined pointer                          *
 * -------------------------------------------------------------------------- */
void                dlink_add_after   (struct list *lptr,
                                       struct node *nptr,
                                       struct node *after, 
                                       void        *ptr);

extern inline void  dlink_add_after   (struct list *lptr,
                                       struct node *nptr,
                                       struct node *after, 
                                       void        *ptr)
{
  /* If <after> is the list tail, then a dlink_add_tail() does the job */
  if(after == lptr->tail)
  {
    dlink_add_tail(lptr, nptr, ptr);
    return;
  }

  /* Set the data pointer */
  nptr->data = ptr;
  
  /* Make references on the new node */
  nptr->next = after->next;
  nptr->prev = after;
  
  /* Update prev-reference of the node after the <after> node */
  after->next->prev = nptr;
  
  /* Update next-reference of the <after> node */
  after->next = nptr;
  
  /* Update list size */
  lptr->size++;
}

/* -------------------------------------------------------------------------- *
 * Remove the dlink node from the dlink list.                                 *
 *                                                                            *
 * <list>                   - list to delete node from                        *
 * <node>                   - the node to delete                              *
 * -------------------------------------------------------------------------- */
void                dlink_delete      (struct list *lptr, 
                                       struct node *nptr);

extern inline void  dlink_delete      (struct list *lptr,
                                       struct node *nptr)
{
  /* If there is a prev node, update its next-
     reference, otherwise update the head */
  if(nptr == lptr->head)
    lptr->head = nptr->next;
  else
    nptr->prev->next = nptr->next;
  
  /* If there is a next node, update its prev-
     reference otherwise update the tail */
  if(lptr->tail == nptr)
    lptr->tail = nptr->prev;
  else
    nptr->next->prev = nptr->prev;
  
  /* Zero references on this node */
  nptr->next = NULL;
  nptr->prev = NULL;
  
  /* Update list size */
  lptr->size--;
}

/* -------------------------------------------------------------------------- *
 * Find a node in a dlink list.                                               *
 *                                                                            *
 * <list>                   - list to find node on                            *
 * <ptr>                    - the wanted ptr-member of the node               *
 *                                                                            *
 * Returns a node when found, NULL otherwise.                                 *
 * -------------------------------------------------------------------------- */
struct node        *dlink_find        (struct list *lptr, 
                                       void        *ptr);

extern inline struct node *dlink_find (struct list *lptr, 
                                       void        *ptr)
{
  struct node *nptr;
  
  /* Loop through all nodes until we find the pointer */
  dlink_foreach(lptr, nptr)
  {
    if(nptr->data == ptr)
      return nptr;
  }

  /* Not found :( */
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a node in a dlink list and delete it.                                 *
 *                                                                            *
 * <list>                   - list to find node on                            *
 * <ptr>                    - the wanted ptr-member of the node               *
 *                                                                            *
 * Returns a node when found and deleted, NULL otherwise.                     *
 * -------------------------------------------------------------------------- */
struct node        *dlink_find_delete (struct list *lptr, void *ptr);

extern inline struct node *dlink_find_delete (struct list *lptr, void *ptr)
{
  struct node *nptr;

  /* Loop through all nodes until we find the pointer */
  dlink_foreach(lptr, nptr)
  {
    if(nptr->data == ptr)
    {
      /* If there is a prev node, update its next-
         reference, otherwise update the head */
      if(nptr->prev)
        nptr->prev->next = nptr->next;
      else
        lptr->head = nptr->next;
      
      /* If there is a next node, update its prev-
         reference otherwise update the tail */
      if(nptr->next)
        nptr->next->prev = nptr->prev;
      else
        lptr->tail = nptr->prev;
      
      /* Zero references on this node */
      nptr->next = NULL;
      nptr->prev = NULL;
      
      /* Update list size */
      lptr->size--;
      
      return nptr;
    }
  }

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Index a node in a dlink list.                                              *
 *                                                                            *
 * <list>                   - list to get node from                           *
 * <index>                  - an integer between 0 and lptr->size - 1         *
 *                                                                            *
 * Returns a node when the index was valid, NULL otherwise.                   *
 * -------------------------------------------------------------------------- */
struct node        *dlink_index       (struct list *lptr,
                                       size_t       index);

extern inline struct node *dlink_index(struct list *lptr,
                                       size_t       index)
{
  struct node *nptr;
  size_t       i = 0;
  
  /* Damn, index is invalid */
  if(index >= lptr->size)
    return NULL;
  
  /* Loop through list until index */
  dlink_foreach(lptr, nptr)
  {
    if(i == index)
      return nptr;
    
    i++;
  }

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Destroy a linked list (free every node and zero the list)                  *
 * NOTE: All nodes must be previously allocated by dlink_node_new()           *
 *                                                                            *
 * <list>                   - list to destroy                                 *
 * -------------------------------------------------------------------------- */
void                dlink_destroy     (struct list *lptr);

extern inline void  dlink_destroy     (struct list *lptr)
{
  struct node *nptr;
  struct node *next;
  
  dlink_foreach_safe(lptr, nptr, next)
    dlink_node_free(nptr);
  
  dlink_list_zero(lptr);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void                dlink_destroy_free(struct list *lptr);
  

/* -------------------------------------------------------------------------- *
 * Swap two links.                                                            *
 *                                                                            *
 * <list>                   - list to swap nodes on                           *
 * <node1>                  - first node to swap                              *
 * <node2>                  - second node to swap                             *
 * -------------------------------------------------------------------------- */
void                dlink_swap        (struct list *lptr, 
                                       struct node *nptr1,
                                       struct node *nptr2);
#if 0
inline void         dlink_swap        (struct list *lptr, 
                                       struct node *nptr1,
                                       struct node *nptr2)
{
  struct node *n1temp;
  struct node *p1temp;
  struct node *n2temp;
  struct node *p2temp;
  
  /* Return if its twice the same node */
  if(nptr1 == nptr2)
    return;
  
  /* Get next- and prev-references of the both nodes.
     these could reference to the other node, if so we
     use the node itself instead of its references */
  if((n1temp = nptr1->next) == nptr2)
    n1temp = nptr1;
  
  if((p1temp = nptr1->prev) == nptr2)
    p1temp = nptr1;
  
  if((n2temp = nptr2->next) == nptr1)
    n2temp = nptr2;
  
  if((p2temp = nptr2->prev) == nptr1)
    p2temp = nptr2;
  
  /* Now make new references while updating 
     head/tail when prev/next are NULL */
  if((nptr1->next = n2temp) == NULL)
    lptr->tail = nptr1;
      
  if((nptr1->prev = p2temp) == NULL)
    lptr->head = nptr1;
  
  if((nptr2->next = n1temp) == NULL)
    lptr->tail = nptr2;
  
  if((nptr2->prev = p1temp) == NULL)
    lptr->head = nptr2;
}
#endif

/* -------------------------------------------------------------------------- *
 * Move all nodes from a list to the head of another.                         *
 *                                                                            *
 * <from>                   - list to move nodes from                         *
 * <to>                     - list to move nodes to                           *
 * -------------------------------------------------------------------------- */
void              dlink_move_head     (struct list *from,
                                       struct list *to);

extern inline void dlink_move_head     (struct list *from, 
                                        struct list *to)
{
  /* Nothing in to-list */
  if(to->head == NULL)
  {
    /* Copy to to-list */
    to->head = from->head;
    to->tail = from->tail;
    to->size = from->size;
  }
  /* Add lists */
  else if(from->tail != NULL)
  {
    /* Prepend from-list */
    from->tail->next = to->head;
    to->head->prev = from->tail;
    to->head = from->head;
    to->size += from->size;

    /* Delete from-list */
    from->head = NULL;
    from->tail = NULL;
    from->size = 0;
  }

  /* Delete from-list */
  from->head = NULL;
  from->tail = NULL;
  from->size = 0;
}

/* -------------------------------------------------------------------------- *
 * Move all items from a list to the tail of another.                         *
 *                                                                            *
 * <from>                   - list to move nodes from                         *
 * <to>                     - list to move nodes to                           *
 * -------------------------------------------------------------------------- */
void                dlink_move_tail   (struct list *from,
                                       struct list *to);

extern inline void  dlink_move_tail   (struct list *from, 
                                       struct list *to)
{
  /* Nothing in to-list */
  if(to->tail == NULL)
  {
    /* Copy to to-list */
    to->head = from->head;
    to->tail = from->tail;
    to->size = from->size;
  }
  /* Add lists */
  else if(from->head != NULL)
  {
    /* Append from-list */
    from->head->prev = to->tail;
    to->tail->next = from->head;
    to->tail = from->tail;
    to->size += from->size;
  }

  /* Delete from-list */
  from->head = NULL;
  from->tail = NULL;
  from->size = 0;
}

/* -------------------------------------------------------------------------- *
 * Copy a list while overwriting destination and allocating new nodes         *
 * -------------------------------------------------------------------------- */
void                dlink_copy        (struct list *from, 
                                       struct list *to);

extern inline void  dlink_copy        (struct list *from, 
                                       struct list *to)
{
  struct node *fnptr;
  struct node *tnptr;
  struct node *prev = NULL;
  
  /* Clear destination */
  dlink_list_zero(to);
  
  /* Loop through source */
  dlink_foreach(from, fnptr)
  {
    /* Make node for destination */
    tnptr = dlink_node_new();
    
    /* Copy data */
    tnptr->data = fnptr->data;
    
    /* Its the head, update destination head */
    if(fnptr == from->head)
      to->head = tnptr;
    
    /* Its the tail, update destination tail */
    if(fnptr == from->tail)
      to->tail = tnptr;
    
    /* Make references */
    tnptr->prev = prev;
    tnptr->next = NULL;
    
    if(prev)
      prev->next = tnptr;
    
    prev = tnptr;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint32_t dlink_count_nodes(struct list *lptr);
  
/* -------------------------------------------------------------------------- *
 * Dump dlink statistics.                                                     *
 * -------------------------------------------------------------------------- */
void     dlink_dump        (void);

#endif /* DLINK_H */
