/* $Id: list.h,v 1.3 2005/05/16 05:19:55 smoli Exp $
 * ------------------------------------------------------------------------- *
 *                 /                                                         *
 *  ___  ___                                                                 *
 * |___ |   )|   )|        Simple and smooth GUI library :)                  *
 *  __/ |__/ |__/ |        Copyright (C) 2003-2005  Roman Senn               *
 *      __/                                                                  *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU Library General Public              *
 *  License as published by the Free Software Foundation; either             *
 *  version 2 of the License, or (at your option) any later version.         *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Library General Public License for more details.                         *
 *                                                                           *
 *  You should have received a copy of the GNU Library General Public        *
 *  License along with this library; if not, write to the Free               *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA *
 * ------------------------------------------------------------------------- */

#ifndef SGUI_LIST_H
#define SGUI_LIST_H

/** @defgroup sgList sgList: Handles doubly-linked lists
 *  @{
 */

#include <stdlib.h>
#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** An element of a list */
struct sgNode
{
  void   *data;    /**< pointer to the actual data */
  sgNode *prev;    /**< pointer to the previous node */
  sgNode *next;    /**< pointer to the next node */
};

/** A dlink list contains a pointer to the head item and to the tail node
 *  and the count of the nodes.
 */
struct sgList
{
  unsigned long  size; /**< number of nodes on the list */
  sgNode        *head; /**< pointer to the first node */
  sgNode        *tail; /**< pointer to the last node */
};

/** Walks through a linked list from head to tail, «n» is set to the current node */
#define sgForeachDown(list, n) \
  for((n) = (void *)(list)->head; \
      (n) != NULL; \
      (n) = (void *)(((sgNode *)n)->next))
 
/** Walks through a linked list from head to tail, and «d» is set to n->data */
#define sgForeachDownData(list, n, d) \
  for((n) = (void *)(list)->head; \
      ((n) != NULL) && (((d) = (void *)((sgNode *)n)->data) != NULL); \
      (n) = (void *)((sgNode *)n)->next)
 
/** Safely walks through a linked list from head to tail (that means you can remove
 *  the current node while looping) «n» is set to the current node and n->next is 
 *  backupped into «m» before loop body for safe walk-throught when nodes get deleted 
 */
#define sgForeachDownSafe(list, n, m) \
  for((n) = (void *)(list)->head, \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->next : NULL; \
      (n) != NULL; \
      (n) = (void *)((sgNode *)m), \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->next : NULL)

/** Same as sgForeachDownSafe but sets «d» = n->data */
#define sgForeachDownSafeData(list, n, m, d) \
  for((n) = (void *)(list)->head, \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->next : NULL; \
      ((n) != NULL) && (((d) = (void *)((sgNode *)n)->data) != NULL); \
      (n) = (void *)((sgNode *)m), \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->next : NULL)

/** Walks through a linked list from tail to head, «n» is set to the current node */
#define sgForeachUp(list, n) \
  for((n) = (void *)(list)->tail; \
      (n) != NULL; \
      (n) = (void *)(((sgNode *)n)->prev))
 
/** Walks through a linked list from tail to head, and «d» is set to n->data */
#define sgForeachUpData(list, n, d) \
  for((n) = (void *)(list)->tail; \
      ((n) != NULL) && (((d) = (void *)((sgNode *)n)->data) != NULL); \
      (d) = (void *)((sgNode *)n)->data, \
      (n) = (void *)((sgNode *)n)->prev)

/** Safely walks through a linked list from tail to head (that means you can remove
 *  the current node while looping) «n» is set to the current node and n->next is 
 *  backupped into «m» before loop body for safe walk-throught when nodes get deleted 
 */
#define sgForeachUpSafe(list, n, m) \
  for((n) = (void *)(list)->tail, \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->prev : NULL; \
      (n) != NULL; \
      (n) = (void *)((sgNode *)m), \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->prev : NULL)

/** Same as sgForeachUpSafe but sets «d» = n->data */
#define sgForeachUpSafeData(list, n, m, d) \
  for((n) = (void *)(list)->tail, \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->prev : NULL; \
      ((n) != NULL) && (((d) = (void *)((sgNode *)n)->data) != NULL); \
      (d) = (void *)((sgNode *)n)->data, \
      (n) = (void *)((sgNode *)m), \
      (m) = (void *)((sgNode *)n) != NULL ? (void *)((sgNode *)n)->prev : NULL)

/** Alias for convenience, lists are normally walked through from head to tail */
#define sgForeach         sgForeachDown
/** Alias for convenience, lists are normally walked through from head to tail */
#define sgForeachData     sgForeachDownData
/** Alias for convenience, lists are normally walked through from head to tail */
#define sgForeachSafe     sgForeachDownSafe
/** Alias for convenience, lists are normally walked through from head to tail */
#define sgForeachSafeData sgForeachDownSafeData

/** Alias for convenience, new nodes are normally added to the tail of the list */
#define sgAddList sgAddListTail

/** Zero a node */
#define sgZeroNode(x) do { \
  ((struct sgNode *)x)->data = NULL; \
  ((struct sgNode *)x)->next = NULL; \
  ((struct sgNode *)x)->prev = NULL; \
} while(0);

/** Zero a dlink list */
#define sgZeroList(x) do { \
  ((struct sgList *)x)->head = NULL; \
  ((struct sgList *)x)->tail = NULL; \
  ((struct sgList *)x)->size = 0; \
} while(0);

/** Add node to the head of the list */
void    sgAddListHead    (sgList *list, sgNode *node, void *ptr);

/** Add node to the tail of the list. */
void    sgAddListTail    (sgList *list, sgNode *node, void *ptr);
  
/** Add node to the list before the specified node. */
void    sgAddListBefore  (sgList *list, sgNode *node, 
                          sgNode *before, void *ptr);
  
/** Remove the node from the list. */
void    sgDeleteList     (sgList *list, sgNode *node);

/** Find a node in list. */
sgNode *sgFindList       (sgList *list, void *ptr);

/** Find a node in a list and delete it. 
 * 
 *  @param list  List on which we'll search for the node
 *  @param ptr   Value of the «data» member of the wanted node
 * 
 *  @return      a node when found and deleted, NULL otherwise.
 */
sgNode *sgFindListDelete (sgList *list, void *ptr);

/** Index a node in a list. */
sgNode *sgIndexList      (sgList *list, size_t index);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
  
#endif /* SGUI_LIST_H */
