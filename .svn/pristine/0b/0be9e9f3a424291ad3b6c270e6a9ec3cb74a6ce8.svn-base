/* $Id: console.h,v 1.7 2005/05/14 08:53:08 smoli Exp $
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

#ifndef SGUI_CONSOLE_H
#define SGUI_CONSOLE_H

/** @defgroup sgConsole sgConsole: Console widget
 *                      A console widget displays a scrollable box which can
 *                      contain lines of text
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */            

/** Console line type */
typedef struct sgConsoleLine sgConsoleLine;

/** a specialised structure for a console widget */
struct sgConsole
{
  SDL_Rect       outer;      /**< the outer frame */
  SDL_Rect       inner;      /**< the inner frame */
  SDL_Rect       caption;    /**< contains the caption */
  SDL_Rect       body;       /**< contains the children */
  SDL_Rect       contents;   /**< contains the children */
  SDL_Rect       line;       /**< dimensions of an line */
  sgList         lines;      /**< list of lines */
  int            cols;       /**< number of characters that fit on a line */
  int            rows;       /**< number of rows that fit in the box */
  int            focus;      /**< line that has focus */
  int            select;     /**< the currently selected line */
  int            cursor;     /**< scrolling cursor face? */
  
  /* scrollbar stuff: to be moved */
  int            scroll;     /**< current scrolling position */
  int            clicky;     /**< where the scrollbar has been clicked on the y-axis */
  int            spushed;    /**< state of the selected line */
  int            spushed0;   /**< state of the up-button */
  int            spushed1;   /**< state of the down-button */
  SDL_Rect       sbar_up;    /**< the up-arrow of the scrollbar */
  SDL_Rect       sbar_down;  /**< the down-arrow of the scrollbar */
  SDL_Rect       sbar_bar;   /**< the «bar» of the scrollbar (the thing that actually moves) */
  SDL_Rect       sbar_space; /**< the space between up- and down-arrow */
};

/** Data type for console specific storage */
typedef struct sgConsole sgConsole;

/** A macro to access console specific widget data */
#define sgConsole(w) (w)->data.console

/** Configuration and initial methods of the console widget */
extern sgWidgetType sgConsoleType;

/** A console line contains only a caption */
struct sgConsoleLine 
{
  sgNode node;      /**< node which links to the line list */
  char   text[512]; /**< string */
};

/** Creates a new console widget using full dialog size
 *  
 *  @param parent   parent widget
 *  @param caption  console caption
 */
sgWidget      *sgNewConsoleFull        (sgWidget   *parent,
                                        const char *caption);  

/** Creates a new console widget by splitting another
 * 
 *  @param based    widget which we'll split
 *  @param edge     at which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  console caption
 */
sgWidget      *sgNewConsoleSplitted    (sgWidget   *based,
                                        sgEdge      edge,
                                        int         pixels,
                                        const char *caption);


/** Creates a new button widget and adds it to a group
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the button inside the group
 *  @param w        width of the button
 *  @param h        height of the button
 *  @param caption  initial button text
 *  
 *  @return         pointer to the newly created button or NULL on error.
 */
sgWidget      *sgNewConsoleGrouped     (sgWidget      *group,
                                        sgEdge         edge,
                                        sgAlign        align,
                                        Uint16         w, 
                                        Uint16         h,
                                        const char    *caption);

/** Creates a new console widget
 *
 *  @param parent   widget which will contain the console
 *  @param x        x-position of the console relative to the parent
 *  @param y        y-position of the console relative to the parent
 *  @param w        width of the console
 *  @param h        height of the console
 *  @param caption  console caption
 *
 *  @return         pointer to the newly created console or NULL on error
 */
sgWidget      *sgNewConsole            (sgWidget      *parent,
                                        Sint16         x, 
                                        Sint16         y,
                                        Uint16         w, 
                                        Uint16         h,
                                        const char    *caption);

/** Recalculates internal console rectangles
 *
 *  @param console  console to recalculate
 */
void           sgRecalcConsole         (sgWidget      *console);

/** Redraws console widget
 * 
 *  @param console  console widget to redraw
 */
void           sgRedrawConsole         (sgWidget      *console);

/** Handles an incoming event for a console widget 
 *
 *  @param console  console widget which receives the event
 *  @param event    an SDL_Event structure from SDL_PollEvent()
 * 
 *  @return         1 if some action has taken place, 0 otherwise
 */
int            sgHandleConsoleEvent    (sgWidget      *console, 
                                        SDL_Event     *event);
  
/** Create and insert line into console at the specified position
 * 
 *  @param console  the console which will have an line added
 *  @param caption  caption of the new line
 *  @param pos      insert position
 *
 *  @return         returns the new line
 */
sgConsoleLine *sgInsertConsoleLine     (sgWidget      *console,
                                        const char    *caption, 
                                        int            pos);

/** Create and append line to a console
 *  
 *  @param console  the console which will have a line added
 *  @param caption  caption of the new line
 * 
 *  @return         returns the new line
 */
sgConsoleLine *sgAddConsoleLine        (sgWidget      *console, 
                                        const char    *caption);

/** Create and append lines to a console. If the line is longer than the 
 *  column count it will be splitted up into several lines
 * 
 *  @param console  the console which will have lines added
 *  @param caption  text to add
 */
void           sgAddConsoleLineWrapped (sgWidget      *console,
                                        const char    *caption);
  
/** Deletes a line from a console
 */
void           sgDeleteConsoleLine     (sgWidget      *console,  
                                        sgConsoleLine *line);

/** Clears all console lines */
void           sgClearConsole          (sgWidget       *console);   

/** Adds text to a console while wrapping and splitting newlines
 */
void           sgAddConsoleText        (sgWidget      *console,
                                        const char    *text);

/** Sets a new console text while wrapping and splitting newlines */
void           sgSetConsoleText        (sgWidget      *console,
                                        const char    *text);

/** Converts the lines of a console into a string (imploding using \n)
 */
char          *sgGetConsoleText        (sgWidget       *console);

/** Gets console line by its index
 *  
 *  (Be careful, information about wrapped lines is lost!)
 * 
 *  @param console  where to get the line from
 *  @param num      line index
 */
sgConsoleLine *sgGetConsoleLineByNum   (sgWidget       *console,
                                        int             num);

/** Scrolls the console widget */
int            sgScrollConsole         (sgWidget       *console,
                                        int             pos,
                                        int             force,
                                        int             setrect);

/** Frees all data associated with a console */
void           sgFreeConsole           (sgWidget       *console);

/** Loads the text of a console from a file
 */
int            sgLoadConsoleText       (sgWidget       *console, 
                                        const char     *file);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_CONSOLE_H */
