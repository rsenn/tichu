/* $Id: cursor.h,v 1.10 2005/05/18 10:05:54 smoli Exp $
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

#ifndef SGUI_CURSOR_H
#define SGUI_CURSOR_H

/** @defgroup sgCursor   sgCursor: Mouse cursor handling
 *  @{
 */
#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

typedef struct sgCursor sgCursor;          /**< per-dialog cursor data */
typedef struct sgCursorTheme sgCursorTheme;    /**< set of cursors */
typedef struct sgCursorFace sgCursorFace;  /**< cursor look */
typedef enum sgCursorType sgCursorType;    /**< cursor look id */
  
/** cursor type enum */
enum sgCursorType
{
  SG_CURSOR_DEFAULT = 0,   /**< the default mouse pointer */
  SG_CURSOR_MOVE,          /**< shown when over a moveable widget */
  SG_CURSOR_RESIZE_H,      /**< shown when over a resizeable vertical widget border */
  SG_CURSOR_RESIZE_V,      /**< shown when over a resizeable horizontal widget border */
  SG_CURSOR_RESIZE_HV,
  SG_CURSOR_RESIZE_VH,
  SG_CURSOR_EDIT,          /**< shown when over an editable widget */
    
  SG_CURSOR_COUNT
};

/** cursor face structure */
struct sgCursorFace
{
  sgCursorType type;       /**< type number */
  SDL_Rect     rect;       /**< source rectangle */
  SDL_Surface *surface;    /**< the actual surface data */
  Sint16       hotx;       /**< x-position of the hotspot */
  Sint16       hoty;       /**< y-position of the hotspot */
};
  
/** cursor as used in a dialog */
struct sgCursor
{
  sgCursorType   lastface;  /**< Previous cursor face */
  sgCursorType   face;      /**< Current cursor face */
  sgCursorTheme *theme;     /**< Current cursor theme */
  Sint16         x;         /**< current position on the x-axis */
  Sint16         y;         /**< current position on the y-axis */
  SDL_Surface   *bgnd;      /**< Backup of the background */
  SDL_Rect       brect;     /**< backup rectangle */
  SDL_Rect       crect;     /**< cursor rectangle */
};
  
/** a set of cursors */
struct sgCursorTheme
{
  sgNode        node;                   /**< Node which links to cursor set list */
  char          name[64];               /**< Full name of the cursor set */
  char          image[64];              /**< Image file */
  sgCursorFace  faces[SG_CURSOR_COUNT]; /**< All the faces of the set */
};  

/** Allocates a new cursor surface, freeing potentially old ones */
void           sgAllocCursorSurface  (sgCursorFace    *face,
                                      Sint16           w, 
                                      Sint16           h);

/** Frees cursor face surface if present */
void           sgFreeCursorSurface   (sgCursorFace    *face);
    
  
/** Sets a specific cursor surface and its hotspot */
void           sgSetCursorSurface    (sgCursorFace    *face,
                                      SDL_Surface     *surface,
                                      Sint16           hotx, 
                                      Sint16           hoty);

/** Copies a specfic region of a surface to a cursor */
void           sgCopyCursorSurface   (sgCursorFace    *face, 
                                      SDL_Surface     *surface);

/** Sets cursor information as parsed */
void           sgSetCursorInfo       (sgCursorFace    *face, 
                                      SDL_Rect        *rect,
                                      Sint16           hotx, 
                                      Sint16           hoty);    
  
/** Returns pointer to appropriate cursor face struct */
sgCursorFace  *sgGetCursorFace       (sgCursor        *cursor,
                                      sgCursorType     face);    
  
/** Returns pointer to current cursor surface */
SDL_Surface   *sgGetCursorSurface    (sgCursor        *cursor,
                                      sgCursorType     type);
    
  
/** allocates space for a new cursor set and links it to the list */
sgCursorTheme *sgNewCursorTheme      (void);
  
/** frees and unlinks a cursor theme */
void           sgDeleteCursorTheme   (sgCursorTheme   *theme);
  
/** frees and unlinks all cursor themes */
void           sgDeleteCursorThemes  (void);
  
/** frees and unlinks a cursor set */
void           sgFreeCursorTheme     (sgCursorTheme   *theme);
  
/** parses a line of a cursor set and returns a non-zero value
    if successful */
int            sgParseCursorTheme    (sgCursorTheme   *theme,
                                      char            *line);

/** Gets the current cursor theme */
sgCursorTheme *sgGetCursorTheme      (sgCursor        *cursor);

/** Gets the cursor theme list */
sgList        *sgGetCursorThemes     (void);
    
/** Sets the current cursor theme (without loading it and unloading previous one) */
void           sgSetCursorTheme      (sgCursor        *cursor,
                                      sgCursorTheme   *theme);
    
/** Finds cursor theme by its name */
sgCursorTheme *sgFindCursorTheme     (const char      *name);
    
  
/** Changes cursor theme */
void           sgChangeCursorTheme   (sgCursor        *cursor,
                                      sgCursorTheme   *theme);
        
  
/** loads a cursor set file and parses it */
sgCursorTheme *sgOpenCursorThemeFp   (FILE            *fp, 
                                      int              autoclose);

/** opens a cursor set file and loads it */
sgCursorTheme *sgOpenCursorTheme     (const char      *file);
  
/** Loads all available cursor sets */
int            sgOpenCursorThemes    (void);

/** Makes a cursor theme ready to use */
int            sgLoadCursorTheme     (sgCursorTheme   *theme);

/** Unloads a cursor theme */
void           sgUnloadCursorTheme   (sgCursorTheme   *theme);
  
/** sets the cursor face */
int            sgSetCursorFace       (sgCursor        *cursor, 
                                      sgCursorType     type);

/** sets the cursor position */
int            sgSetCursorPos        (sgCursor        *cursor, 
                                      Sint16           x, 
                                      Sint16           y);

/** allocates a large enough surface for the cursor backup */
void           sgAllocCursorBackup   (sgCursor        *cursor, 
                                      SDL_PixelFormat *format);

/** clears the backupped cursor background so it is not restored
 *  next time when blitting the cursor 
 */
void           sgClearCursorBgnd     (sgCursor        *cursor);

/** restores the backupped background before blitting the cursor */
void           sgRestoreCursorBgnd   (sgCursor        *cursor, 
                                      SDL_Surface     *surface);

/** backs up the background area before blitting a cursor */
void           sgBackupCursorBgnd    (sgCursor        *cursor, 
                                      SDL_Surface     *surface);

/** blits cursor to a surface after restoring the old background and 
 *  backing up the destination area 
 */
int           sgBlitCursor           (sgCursor        *cursor, 
                                      SDL_Surface     *surface);              

/** Updates screen rectangles for the cursor */
void          sgUpdateCursor         (sgCursor        *cursor, 
                                      SDL_Surface     *surface);  
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
  
#endif /* SGUI_CURSOR_H */
