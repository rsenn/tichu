/* $Id: file.h,v 1.7 2005/05/16 05:19:55 smoli Exp $
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

#ifndef SGUI_FILE_H
#define SGUI_FILE_H

#include <stdio.h>
#include <limits.h>
#include <SDL.h>

/** @defgroup sgFile sgFile: Routines for file searching/opening
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
#ifndef PATH_MAX
/** PATH_MAX defaults to 1024, which is less than it would be on most systems */
# define PATH_MAX 1024
#endif /* PATH_MAX */

/** A search path entry */
struct sgPath
{
  sgNode node;           /**< a node linking to the sgPathList */
  char   dir[PATH_MAX];  /**< the actual search path */
};

/** search path entry type */
typedef struct sgPath sgPath;

/** Get path of the last found file */
char      *sgGetFilePath    (void);

/** Get path list */
sgList    *sgGetFilePaths   (void);

/** Add a search path to the list 
 *  
 *  @param dir  The path to add (preferrably without trailing slash)
 */
void       sgAddFilePath    (const char *dir);

/** Clear the whole search path list 
 */
void       sgClearFilePaths (void);
  
/** Opens a file and returns a stdio file pointer 
 *
 *  @param name Filename
 *  @param mode mode as passed to fopen()
 * 
 *  @return     valid FILE * on success or NULL on error
 */
FILE      *sgOpenFileFp     (const char *name, 
                             const char *mode);
    
/** Opens a file and returns an SDL_rwops
 *
 *  @param name Filename
 *  @param mode mode as passed to fopen()
 * 
 *  @return     valid SDL_RWops * on success or NULL on error
 */
SDL_RWops *sgOpenFileRWops  (const char *name, 
                             const char *mode);
  
/** Loads a files content into a string */
char      *sgLoadFileText   (const char *name);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */
    
/** @} */

#endif /* SGUI_FILE_H */
