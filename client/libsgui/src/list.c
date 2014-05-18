/* $Id: list.c,v 1.4 2005/05/16 05:19:55 smoli Exp $
 * ------------------------------------------------------------------------- *
 *                     /                                                     *
 *      ___  ___                                                             *
 *     |___ |   )|   )|        Simple and smooth GUI library :)              *
 *      __/ |__/ |__/ |        Copyright (C) 2003-2005  Roman Senn           *
 *          __/                                                              *
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

#include <libsgui/list.h>

/* ------------------------------------------------------------------------- *
 * Add node to the head of the list                                          *
 * ------------------------------------------------------------------------- */
void sgAddListHead(sgList *list, sgNode *node, void *ptr)
{
  /* Set the data pointer */
  node->data = ptr;
  
  /* We add to the list head, so there's no previous node */
  node->prev = NULL;
  
  /* Next node is the old head */
  node->next = list->head;
  
  /* If there already is a node at the head update 
     its prev-reference, else update the tail */
  if(list->head)
    list->head->prev = node;
  else
    list->tail = node;
  
  /* Now put the node to list head */
  list->head = node;
  
  /* Update list size */
  list->size++;
}

/* ------------------------------------------------------------------------- *
 * Add node to the tail of the list.                                         *
 * ------------------------------------------------------------------------- */
void sgAddListTail(sgList *list, sgNode *node, void *ptr)
{
  /* Set the data pointer */
  node->data = ptr;
  
  /* We add to the list tail, so there's no next node */
  node->next = NULL;
  
  /* Previous node is the old tail */
  node->prev = list->tail;
  
  /* If there already is a node at the tail update 
     its prev-reference, else update the head */
  if(list->tail)
    list->tail->next = node;
  else
    list->head = node;
  
  /* Now put the node to list tail */
  list->tail = node;
  
  /* Update list size */
  list->size++;
}

/* -------------------------------------------------------------------------- *
 * Remove the node from the list.                                             *
 * -------------------------------------------------------------------------- */
void sgDeleteList(sgList *list, sgNode *node)
{
  /* If there is a prev node, update its next-
     reference, otherwise update the head */
  if(node == list->head)
    list->head = node->next;
  else
    node->prev->next = node->next;
  
  /* If there is a next node, update its prev-
     reference otherwise update the tail */
  if(list->tail == node)
    list->tail = node->prev;
  else
    node->next->prev = node->prev;
  
  /* Zero references on this node */
  node->next = NULL;
  node->prev = NULL;
  
  /* Update list size */
  list->size--;
}

/* -------------------------------------------------------------------------- *
 * Find a node in list.                                                       *
 * -------------------------------------------------------------------------- */
sgNode *sgFindList(sgList *list, void *ptr)
{
  sgNode *node;
  
  /* Loop through all nodes until we find the pointer */
  sgForeach(list, node)
  {
    if(node->data == ptr)
      return node;
  }
  
  /* Not found :( */
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a node in a list and delete it.                                       *
 * Returns a node when found and deleted, NULL otherwise.                     *
 * -------------------------------------------------------------------------- */
sgNode *sgFindListDelete(sgList *list, void *ptr)
{
  sgNode *node;
  
  /* Loop through all nodes until we find the pointer */
  sgForeach(list, node)
  {
    if(node->data == ptr)
    {
      /* If there is a prev node, update its next-
         reference, otherwise update the head */
      if(node->prev)
        node->prev->next = node->next;
      else
        list->head = node->next;

      /* If there is a next node, update its prev-
         reference otherwise update the tail */
      if(node->next)
        node->next->prev = node->prev;
      else
        list->tail = node->prev;
  
      /* Zero references on this node */
      node->next = NULL;
      node->prev = NULL;
  
      /* Update list size */
      list->size--;
      
      return node;
    }    
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Index a node in a list.                                                    *
 * Returns a node when the index was valid, NULL otherwise.                   *
 * -------------------------------------------------------------------------- */
sgNode *sgIndexList(sgList *list, size_t index)
{
  sgNode *node;
  size_t  i = 0;
  
  /* Damn, index is invalid */
  if(index >= list->size)
    return NULL;
  
  /* Loop through list until index */
  sgForeach(list, node)
  {
    if(i == index)
      return node;
    
    i++;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Add node to the list before the specified node.                            *
 * -------------------------------------------------------------------------- */
void sgAddListBefore(sgList *list, sgNode *node, sgNode *before, void *ptr)
{  
  /* If <before> is the list head, then a sgAddListHead() does the job */
  if(before == list->head)    
  {
    sgAddListHead(list, node, ptr);
    return;
  }
  
  /* Set the data pointer */
  node->data = ptr;
  
  /* Make references on the new node */
  node->next = before;
  node->prev = before->prev;
  
  /* Update next-reference of the node before the <before> */
  before->prev->next = node;
  
  /* Update prev-reference of the <before> node */
  before->prev = node;
  
  /* Update list size */
  list->size++;
}

