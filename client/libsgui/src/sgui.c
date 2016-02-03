/* $Id: sgui.c,v 1.19 2005/05/14 09:09:12 smoli Exp $
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

/** @weakgroup sgUI sgUI: main module
 * 
 *                  Initializes and shuts down the sgUI library and provides some
 *                  Global stuff.
 */

#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include <libsgui/sgui.h>
#include <libsgui/file.h>
#include <libsgui/cursor.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LIBSGUI_PRIVATE_H
#include "libsgui_private.h"
#endif /* LIBSGUI_PRIVATE_H */

static sgVersion version = { VER_MAJOR, VER_MINOR, VER_RELEASE };
  
int sgInit(void) 
{
  sgAddFilePath(".");
  sgAddFilePath("..");
#ifdef DATADIR
  sgAddFilePath(DATADIR);
#endif /* DATADIR */

  sgAddFilePath("fonts");
  sgAddFilePath("../fonts");
  sgAddFilePath("../../fonts");
  sgAddFilePath("cursors");
  sgAddFilePath("../cursors");
  sgAddFilePath("../../cursors");

  sgAddFilePath("libsgui/fonts");
  sgAddFilePath("../libsgui/fonts");
  sgAddFilePath("../../libsgui/fonts");
  sgAddFilePath("libsgui/cursors");
  sgAddFilePath("../libsgui/cursors");
  sgAddFilePath("../../libsgui/cursors");

  sgInitFonts();
  sgInitPatterns();
  return 1;
}

#ifdef DEBUG
static void sgLeaks(sgList *widgets)
{
  sgWidget *widget;
  sgWidget *next;
  
  sgForeachSafe(widgets, widget, next)
  {
    sgLog("Deleting leaked %s widget: %s", 
          widget->type->name, widget->caption);
    
    sgDeleteWidget(widget);
  }
}
#endif /* DEBUG */

void sgQuit(void) 
{
#ifdef DEBUG
  sgLeaks(&sgWidgetList);
#endif /* DEBUG */
  
  sgFreeFonts();
  
  sgClearFilePaths();
  sgDeleteCursorThemes();
}

const sgVersion *sgLinkedVersion(void)
{
  return &version;
}
