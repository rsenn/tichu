/* $Id: event.h,v 1.9 2005/05/07 15:21:19 smoli Exp $
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

#ifndef SGUI_EVENT_H
#define SGUI_EVENT_H

/** @defgroup sgEvent sgEvent: Event handling subsystem
 *  @{
 */

/* (The sgEvent enum is in stub.h) */

#define SG_PRESSED   1 /**< The button is beeing pressed */
#define SG_RELEASED  0 /**< The button has been released */

/*#define SG_BUTTON_LEFT  SDL_BUTTON_LEFT
#define SG_BUTTON_RIGHT SDL_BUTTON_RIGHT
#define SG_WHEEL_UP     SDL_BUTTON_WHEELUP
#define SG_WHEEL_DOWN   SDL_BUTTON_WHEELDOWN*/

/** Checks SDL_Event mouse clicks */
#define sgEventButton(event, b, s) \
  (event->type == ((s) == SG_PRESSED ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP) && \
   event->button.button == (b))


/** Checks SDL_Event structure for keypresses */
#define sgEventKey(event, k, s) \
   (event->type == ((s) == SG_PRESSED ? SDL_KEYDOWN : SDL_KEYUP) && \
    event->key.keysym.sym == (k))

/** @} */

#endif /* SGUI_EVENT_H */
