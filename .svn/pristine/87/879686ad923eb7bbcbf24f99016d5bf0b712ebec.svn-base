/* 
   The sgUI library
   Copyright (C) 2003  Roman Senn

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Roman Senn
   smoli@paranoya.ch

   $Id: sgui.h,v 1.18 2005/05/18 01:50:30 smoli Exp $
*/  

#ifndef SGUI_H
#define SGUI_H

#include <SDL.h>

#include <libsgui/stub.h>
#include <libsgui/common.h>
#include <libsgui/font.h>
#include <libsgui/dialog.h>
#include <libsgui/adjust.h>
#include <libsgui/button.h>
#include <libsgui/toggle.h>
#include <libsgui/image.h>
#include <libsgui/event.h>
#include <libsgui/picts.h>
#include <libsgui/listbox.h>
#include <libsgui/edit.h>
#include <libsgui/label.h>
#include <libsgui/console.h>
#include <libsgui/input.h>
#include <libsgui/group.h>
#include <libsgui/tab.h>
#include <libsgui/dropdown.h>
#include <libsgui/colorsel.h>
#include <libsgui/widget.h>

/** A structure for versioning of the compiled lib */
struct sgVersion 
{
  int major;  /**< Major release */
  int minor;  /**< Minor release */
  int patch;  /**< Patchlevel */
};

/** Version data type */
typedef struct sgVersion sgVersion;
  
int              sgInit    (void);
void             sgQuit    (void);
const sgVersion *sgLinkedVersion (void);

#endif /* SGUI_H */
