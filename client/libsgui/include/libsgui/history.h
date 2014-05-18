/* $Id: history.h,v 1.2 2005/05/03 12:45:04 smoli Exp $
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

#ifndef SGUI_HISTORY_H
#define SGUI_HISTORY_H
 
/** @defgroup sgHistory sgHistory: a history for editing widgets
 *  @{
 */

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Structure containing the history data */
struct sgHistory 
{
  char        **entries;  /**< history entry string array */
  unsigned long pos;      /**< current position within array */
};
/** Datatype defining the history structure */
typedef struct sgHistory sgHistory;

/** Number of history entries */
#define HISTORY_SIZE 64

/** Creates a new history by initializing the specified structure */
void   sgNewHistory      (sgHistory  *history);

/** Adds an entry to the history */
void   sgAddHistoryEntry (sgHistory  *history,
                          const char *text);

/** Clears all history entries */
void   sgClearHistory    (sgHistory  *history);

/** Get the whole history array */
char **sgGetHistory      (sgHistory  *history);
  
/** Step back to the previous history entry */
char  *sgPrevHistoryEntry(sgHistory  *history);

/** Step forward to the next history entry */
char  *sgNextHistoryEntry(sgHistory  *history);

/** Free all history data */
void   sgFreeHistory     (sgHistory  *history);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SGUI_HISTORY_H */

