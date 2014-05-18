/* $Id: dialog.h,v 1.29 2005/05/26 10:48:34 smoli Exp $
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

#ifndef SGUI_DIALOG_H
#define SGUI_DIALOG_H 

/** @defgroup sgDialog sgDialog: Dialog widget
 *                     The dialog widget handles a whole screen.
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/pattern.h>
#include <libsgui/cursor.h>

/** A timer callback (which is set using sgSetDialogTimer) */
typedef int sgDialogTimer(Uint32 interval, void *arg);

/** A specialised structure for a dialog widget */
struct sgDialog
{
  sgWidget      *focus;     /**< child that has focus */  
  sgPattern     *pattern;   /**< background pattern generator */
  SDL_Surface   *screen;    /**< surface the dialog will be drawn to in sgRunDialog() */
  int            fps;       /**< suggested frame rate for sgRunDialog() */
  sgCursor       cursor;    /**< the dialogs cursor */
  Uint8          contrast;  /**< background pattern contrast */  
  Uint32         interval;  /**< timer interval */
  Uint32         last;      /**< last time the timer has been called */
  sgDialogTimer *timer;     /**< timer function */
  void          *arg;       /**< user-defined argument */
};

/** Data type for widget-specific storage */
typedef struct sgDialog sgDialog;

/** A macro to access the specialised dialog structure */
#define sgDialog(w) (w)->data.dialog

/** Configuration and initial methods of the button widget */
extern sgWidgetType sgDialogType;

/** Create a dialog
 * 
 *  @param screen   The screen surface the dialog will be drawn to
 *  @param proc     Procedure that handles events of the dialog and all its children
 *  @param border   Default border for any child widget 
 *  @param alpha    Default alpha for any child widget
 *  @param color    Default color for any child widget 
 *  @param fps      FPS rate at which the dialog should be displayed
 * 
 *  @return         A dialog widget
 */
sgWidget *sgNewDialog            (SDL_Surface      *screen, 
                                  sgWidgetProc     *proc,
                                  Uint16            border,
                                  Uint8             alpha, 
                                  sgColor           color,
                                  int               fps);

/** Recalcs dialog dimensions */
void      sgRecalcDialog         (sgWidget         *dialog);

/** Redraws dialog background */
void      sgRedrawDialog         (sgWidget         *dialog);

/** Handles an incoming event for the dialog widget */
int       sgHandleDialogEvent    (sgWidget         *dialog, 
                                  SDL_Event        *event);

/** Blits the current cursor to the screen */
int       sgBlitDialogCursor     (sgWidget         *dialog, 
                                  SDL_Surface      *surface);

/** Blits a dialog to the screen surface */
int       sgBlitDialog           (sgWidget         *dialog, 
                                  SDL_Surface      *surface);  

/** Sets a timer callback for the dialog which is run periodically from sgRunDialog() 
 *
 *  @param dialog   the dialog that will run the timer
 *  @param fn       callback function which is periodically called (has widget as first argument)
 *  @param interval time between timer calls in milliseconds
 *  @param arg      is passed to the callback as second argument
 */
void      sgSetDialogTimer       (sgWidget         *dialog,
                                  sgDialogTimer    *fn, 
                                  Uint32            interval,
                                  void             *arg);

/** Handles processing of events and redrawing in a loop */
void      sgRunDialog            (sgWidget         *dialog,
                                  Uint32            fade);

/** Sets the current cursor used in the dialog */
void      sgSetDialogCursor      (sgWidget         *dialog, 
                                  sgCursorType      type);  

/** Sets the cursor theme for a dialog */
void      sgSetDialogCursorTheme (sgWidget         *dialog, 
                                  sgCursorTheme    *theme);

/** Sets the dialog pattern */
void      sgSetDialogPattern     (sgWidget         *dialog,
                                  sgPattern        *pattern);
/** Sets the dialog contrast */
void      sgSetDialogContrast    (sgWidget         *dialog,
                                  Uint8             contrast);
/** @} */

#endif /* SGUI_DIALOG_H */
