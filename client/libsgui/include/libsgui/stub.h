/* $Id: stub.h,v 1.7 2005/05/21 22:36:09 smoli Exp $
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

#ifndef SGUI_STUB_H
#define SGUI_STUB_H

/** @defgroup sgStub sgStub: Contains (incomplete) declarations of sgUI enums and
 *                           structures which are included before any other file.
 */

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ------------------------------------------------------------------------- */
  
/** @weakgroup sgEvent
 *  @{
 */
  
/** Enumerates event types */
enum sgEvent
{
  SG_EVENT_QUIT = 0, /**< dialog has been quitted */
  SG_EVENT_RESIZE,   /**< widget has been resized */
  SG_EVENT_CLICK,    /**< Button has been clicked */
  SG_EVENT_CHANGE,   /**< Adjust bar has been changed */
  SG_EVENT_MOTION,   /**< Motion inside image rectangle */
  SG_GAIN_FOCUS,     /**< A widget has gained focus */
  SG_LOOSE_FOCUS,    /**< A widget has lost focus */
  SG_RETURN,         /**< Return has been pressed in the input widget */
  SG_KEYPRESS,       /**< A key has been pressed */
  SG_SEL_CHANGE,     /**< Listbox selection has been changed */
  SG_SEL_CLICK,      /**< The current listbox selection has been clicked */
  SG_BUTTON_DOWN,    /**< A button has been released */
  SG_BUTTON_UP,      /**< A button has been released */
  SG_SCROLL          /**< A scrollbar has been adjusted */
};
  
typedef enum sgEvent sgEvent; /**< Event id */

/** @} */
  
/** @weakgroup sgWidget
 *  @{
 */
  
/** Generic widget type */
typedef struct sgWidget        sgWidget;

/** Widget surface data type */
typedef struct sgWidgetFace    sgWidgetFace;
  
/** Widget palette data type */
typedef struct sgWidgetPalette sgWidgetPalette;
  
/** Widget type initializer */
typedef struct sgWidgetType    sgWidgetType;

/** Widget callback structure */
typedef struct sgWidgetMethods sgWidgetMethods;
  
/** Widget status flags */
typedef enum   sgWidgetStatus  sgWidgetStatus;

/** A handler which catches events from parent widgets */
typedef int    sgWidgetProc    (sgWidget  *widget,
                                sgEvent    event);

/** Generic widget callback */
typedef void   sgWidgetCallback(sgWidget  *widget);

/** @} */
  
/* ------------------------------------------------------------------------- */
  
/** @weakgroup sgList
 *  @{
 */

/** A node on a sgList */
typedef struct sgNode sgNode;
  
/** A doubly-linked list consisting of sgNode structures */
typedef struct sgList sgList;
  
/** @} */
  
/* ------------------------------------------------------------------------- */

/** @weakgroup sgDialog
 *  @{
 */
  
//typedef struct sgDialog sgDialog; /**< Dialog type */
  
/** @} */
/** @weakgroup sgButton
 *  @{
 */
  
//typedef struct sgButton sgButton;
  
  
/** @} */

/** A prerendered bitmap type font */
typedef struct sgFont sgFont;

/** Low-level drawing mode */
typedef enum sgDraw sgDraw;

  
typedef struct sgPict sgPict;
//typedef struct sgGroup sgGroup;
//typedef struct sgListbox sgListbox;

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_STUB_H */
