/* $Id: tab.h,v 1.5 2005/05/18 10:05:54 smoli Exp $
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

#ifndef SGUI_TAB_H
#define SGUI_TAB_H

/** @weakgroup sgTab  sgTab: Tabbing compound widget
 *                           Tabbing compounds can contain several groups.
 *  @{
 */

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
/** Defines a specialised structure for a tab widget */
struct sgTab
{
  sgWidget    *group;        /**< the currently selected group */
  Uint16       offset;       /**< tab rectangle offset from the left edge */
};

/** Data type for widget-specific storage */
typedef struct sgTab sgTab;
  
/** A macro to access the specialised structure of a tab widget */
#define sgTab(w) (w)->data.tab
  
/** Configuration and initial methods of the tabing widget */
extern sgWidgetType sgTabType;
  
/** Creates a new tab widget using full dialog size */
sgWidget *sgNewTabFull          (sgWidget    *parent);

/** Creates a new tab widget from a rectangle */
sgWidget *sgNewTabRect          (sgWidget    *parent,
                                 SDL_Rect     rect);
  
/** Creates a new tab widget */
sgWidget *sgNewTab              (sgWidget    *parent,
                                 Sint16       x,
                                 Sint16       y,
                                 Uint16       w,
                                 Uint16       h);

/** Recalc tab dimensions */
void      sgRecalcTab           (sgWidget    *tab);

/** Draws tab look */
void      sgRedrawTab           (sgWidget    *tab);

/** Handles events for a group inside a tab widget */
int       sgHandleTabEvent      (sgWidget    *group,
                                 sgEvent      event);

/** Adds a widget to a tab */
void      sgAddTab              (sgWidget    *tab, 
                                 sgWidget    *add,
                                 sgEdge       edge,
                                 sgAlign      align);

/** Adds a group widget to the tab-control */
void      sgAddTabGroup         (sgWidget    *tab, 
                                 sgWidget    *group);

/** Returns the currently active tab group */
sgWidget *sgSelectedTabGroup    (sgWidget    *tab);
    
/** Sets the currently active tab group */
void      sgSelectTabGroup      (sgWidget    *tab, 
                                 sgWidget    *group);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_TAB_H */
