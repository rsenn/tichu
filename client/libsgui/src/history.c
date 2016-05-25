/* $Id: history.c,v 1.5 2005/05/04 17:35:04 smoli Exp $
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

/** @weakgroup sgHistory 
 *  @{
 */

#include <stdlib.h>
#include <string.h>

#include <libsgui/sgui.h>
#include <libsgui/history.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgNewHistory(sgHistory *history)
{
  history->entries = calloc(sizeof(char *), HISTORY_SIZE + 1);
  history->pos = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgAddHistoryEntry(sgHistory *history, const char *text)
{
  if(history->entries[HISTORY_SIZE])
  {
    free(history->entries[HISTORY_SIZE]);
    history->entries[HISTORY_SIZE] = NULL;
  }

  if(history->entries[0])
    free(history->entries[0]);
  
  history->entries[0] = strdup(text);
  
  memmove(&history->entries[1], history->entries, HISTORY_SIZE);

  if(history->entries[HISTORY_SIZE])
  {
    free(history->entries[HISTORY_SIZE]);
    history->entries[HISTORY_SIZE] = NULL;
  }
  
  history->pos = 0;
  history->entries[0] = strdup("");
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgClearHistory(sgHistory *history)
{
  int i;

  if(history->entries)
  {
    for(i = 0; i <= HISTORY_SIZE; i++)
    {
      if(history->entries[i])
      {
        free(history->entries[i]);
        history->entries[i] = NULL;
      }
    }
  }
  
  history->pos = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char **sgGetHistory(sgHistory *history)
{
  char **entries;
  
  entries = history->entries;
  
  history->entries = NULL;
  history->pos = 0;
  
  return entries;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *sgPrevHistoryEntry(sgHistory *history)
{
  if(history->pos && history->entries[history->pos - 1])
  {
    history->pos--;

    return history->entries[history->pos];
  }

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *sgNextHistoryEntry(sgHistory *history)
{
  if(history->entries[history->pos + 1])
  {
    history->pos++;

    return history->entries[history->pos];
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgFreeHistory(sgHistory *history)
{
  sgClearHistory(history);

  if(history->entries)
  {
    free(history->entries);
    history->entries = 0;
    history->pos = 0;
  }
}

/** @} */
