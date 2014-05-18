/* $Id: dialog.c,v 1.46 2005/05/26 10:48:35 smoli Exp $
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

/** @weakgroup sgDialog 
 *  @{
 */

#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#ifndef PATH_MAX
#include <limits.h>
#endif

#include <libsgui/sgui.h>
#include <libsgui/list.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgDialogType =
{
  .name = "sgDialog",
  .size = sizeof(struct sgDialog),
  .methods =
  {
    .recalc  = sgRecalcDialog,
    .redraw  = sgRedrawDialog,
    .handler = sgHandleDialogEvent,
    .blit    = sgBlitWidget
  }
};

/* Create a dialog
 * ------------------------------------------------------------------------- */
sgWidget *sgNewDialog(SDL_Surface *screen, sgWidgetProc *proc,
                      Uint16 border, Uint8 alpha,
                      sgColor color, int fps)
{
  sgWidget *dialog;
  SDL_Rect rect;
  
  SDL_GetClipRect(screen, &rect);
  
  dialog = sgNewWidgetRect(&sgDialogType, NULL, rect, NULL);

  /* Set some supplied initial values */
  dialog->proc = proc;
  dialog->alpha = alpha;
  dialog->border = border;
  dialog->status |= SG_FOCUS;

  sgDialog(dialog)->screen = screen;
  sgDialog(dialog)->fps = fps;

  /* Set some constant initial values */
  sgDialog(dialog)->pattern = sgGetPatternTable();
  sgDialog(dialog)->contrast = 255;
  
  sgSetWidgetColor(dialog, color);
  sgSetDialogCursor(dialog, SG_CURSOR_DEFAULT);
  
  return dialog;
}

/* Recalcs dialog dimensions
 * -------------------------------------------------------------------------- */
void sgRecalcDialog(sgWidget *dialog)
{
  sgFreeWidgetFrame(dialog);
  sgSetWidgetStatus(dialog, SG_REDRAW_FRAME);
}
  
/* Redraws dialog background
 * -------------------------------------------------------------------------- */
void sgRedrawDialog(sgWidget *dialog)
{
  if(dialog->status & SG_REDRAW_FRAME)
  {
    if(dialog->face.frame)
    {
      sgFillPattern(dialog->face.frame, sgDialog(dialog)->pattern, sgDialog(dialog)->contrast);
   
      /* disable color keying */
      SDL_SetColorKey(dialog->face.frame, 0, 0);
    }
  }
}

/* Handles an incoming event for the dialog widget
 * -------------------------------------------------------------------------- */
int sgHandleDialogEvent(sgWidget *dialog, SDL_Event *event)
{
  switch(event->type)
  {
    /* Application focus is dialog focus */
    case SDL_ACTIVEEVENT:
    {
      if(event->active.state & SDL_APPINPUTFOCUS)
      {
        if(event->active.gain)
          sgSetWidgetStatus(dialog, SG_FOCUS);
        else
          sgClearWidgetStatus(dialog, SG_FOCUS),
        
        sgSetWidgetStatus(dialog, SG_REDRAW_SCREEN|SG_REDRAW_MOUSE);
        
        
      }

      return 1;
    }
    
    /* When the window gets resized we generate an SG_EVENT_RESIZE */
    case SDL_VIDEORESIZE:
    {
      /* Set the new dialog size */
      sgSetWidgetSize(dialog, event->resize.w, event->resize.h);
     
      /* Redraw everything before setting the new video mode */
      sgRedrawWidget(dialog);
      
      return sgReportWidgetEvent(dialog, SG_EVENT_RESIZE);
    }
    
    /* When the window gets exposed, we need a redraw */
    case SDL_VIDEOEXPOSE:
    {
      sgSetWidgetStatus(dialog, SG_REDRAW_SCREEN);
      return 1;
    }
    
    /* Closing the window exits the dialog */
    case SDL_QUIT:
    {
      return sgReportWidgetEvent(dialog, SG_EVENT_QUIT);
    }
    
    /* Pressing escape exits the dialog */
    case SDL_KEYUP:
    {
      if(event->key.keysym.sym == SDLK_ESCAPE)
        return sgReportWidgetEvent(dialog, SG_EVENT_QUIT);
      
      break;
    }
    
    case SDL_MOUSEMOTION:
    {
      sgSetWidgetStatus(dialog, SG_REDRAW_MOUSE);

      /* If we have focus ourselves, then check children focus */
      if(dialog->status & SG_FOCUS)
      {
        sgWidget *widget;
        
        /* but not while a mouse button is hold */
        Uint8 state = SDL_GetMouseState(NULL, NULL);
        
        if(state & (SDL_BUTTON(SDL_BUTTON_LEFT)|SDL_BUTTON(SDL_BUTTON_RIGHT)))
          break;
        
        sgForeach(&dialog->list, widget)
        {
          if(widget->status & SG_DISABLED)
            continue;
          
          if(sgMatchRect(&widget->area, event->motion.x, event->motion.y))
            break;
        }
        
        /* Mouse is over a widget but not the one in dialog->focus */
        if(widget && sgDialog(dialog)->focus != widget)
        {
          if(sgDialog(dialog)->focus)
          {
            sgClearWidgetStatus(sgDialog(dialog)->focus, SG_FOCUS);
            sgSetWidgetStatus(sgDialog(dialog)->focus, SG_REDRAW_NEEDED);
          }
          
          sgDialog(dialog)->focus = widget;
          
          sgLog("%s widget receives focus", widget->type->name);
          
          sgSetWidgetStatus(widget, SG_FOCUS|SG_REDRAW_BORDER);
          sgSetWidgetStatus(dialog, SG_REDRAW_NEEDED);
        }
      }      
    }
    
    default:
    {
/*      sgLog("unhandled dialog event: %i", event->type);*/
      break;
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- */
int sgBlitDialogCursor(sgWidget *dialog, SDL_Surface *surface)
{
    return sgBlitCursor(&sgDialog(dialog)->cursor, surface);
  
  return 0;  
}

/* -------------------------------------------------------------------------- */
int sgBlitDialog(sgWidget *dialog, SDL_Surface *surface)
{
  int ret;
  int x, y;
  
  SDL_GetMouseState(&x, &y);
  
  /* in double-buffering mode we should be redrawing everything when the cursor
     has changed */
  if(sgSetCursorPos(&sgDialog(dialog)->cursor, x, y))
  {
    if(surface->flags & SDL_DOUBLEBUF)
      sgSetWidgetStatus(dialog, SG_REDRAW_NEEDED);
    
    sgSetWidgetStatus(dialog, SG_REDRAW_MOUSE);
  }
  
  ret = dialog->methods.blit(dialog, surface, dialog->rect.x, dialog->rect.y);
  
  /* if anything has been blitted then the backupped cursor background
     is obsoleted */
  if(ret)
    sgClearCursorBgnd(&sgDialog(dialog)->cursor);
  
  if(ret || (dialog->status & (SG_REDRAW_NEEDED|SG_REDRAW_MOUSE)))
  {
    if(sgBlitDialogCursor(dialog, surface))
    {
      if(ret == 0)
        sgUpdateCursor(&sgDialog(dialog)->cursor, surface);
    }
  }
  
  return ret;
}

/* -------------------------------------------------------------------------- */
void sgSetDialogTimer(sgWidget *dialog, sgDialogTimer *fn, Uint32 interval, void *arg)
{
  sgDialog(dialog)->arg = arg;
  sgDialog(dialog)->timer = fn;
  sgDialog(dialog)->interval = interval;
}  
  
/* -------------------------------------------------------------------------- */
void sgRunDialog(sgWidget *dialog, Uint32 fade)
{
  Uint32 interval = 1000 / sgDialog(dialog)->fps;
  
  /* mark the dialog as running */
  sgSetWidgetStatus(dialog, SG_RUNNING);
  
  /* set focus to the first child widget */
  sgDialog(dialog)->focus = NULL; 
  
  /* loop as long as the dialog is running */
  while(dialog->status & SG_RUNNING)
  {
    SDL_Event event;
    Uint32 elapsed;
    Uint32 blit;                   /* blit time of the current iteration */
    Uint32 end;                    /* end time of the current iteration */
    Uint32 start = SDL_GetTicks(); /* start time of the current iteration */
    Uint32 events = 0;             /* number of events we handled */
    
    /* Check if the timer is present and pending */
    if(sgDialog(dialog)->timer)
    {
      elapsed = start - sgDialog(dialog)->last;
      
      if(elapsed > sgDialog(dialog)->interval)
      {
        sgDialog(dialog)->last = start;
        sgDialog(dialog)->timer(sgDialog(dialog)->interval, sgDialog(dialog)->arg);
      }
    }
    
    /* Check for incoming events */
    while(SDL_PollEvent(&event))
    {
      sgHandleWidgetEvent(dialog, &event);
      
      /* Do not handle more than 32 events per loop */
      if(++events == 32)
        break;
    }
     
    /* Redraw the dialog and blit it to screen if needed. */
    if(sgBlitDialog(dialog, sgDialog(dialog)->screen))
    {
      /* When something has been blitted we need to flip */
      if(sgDialog(dialog)->screen->flags & SDL_DOUBLEBUF)
        SDL_Flip(sgDialog(dialog)->screen);
      else
        SDL_UpdateRect(sgDialog(dialog)->screen,
                       dialog->rect.x, dialog->rect.y,
                       dialog->rect.w, dialog->rect.h);
    }
    
    /* If less time than the suggested frame interval elapsed
       we throttle it down a bit */
    blit = SDL_GetTicks();
    elapsed = blit - start;
    
    if(interval > elapsed)
      SDL_Delay(interval - elapsed);
    
    /* Calculate the effective time spent in this iteration */
    end = SDL_GetTicks();
    elapsed = end - start;
    
#ifdef DEBUG    
/*    sgLog("redrawing @ %f fps", 1000.0 / (float)elapsed);*/
#endif /* DEBUG */
  }
}  

/* -------------------------------------------------------------------------- */
void sgSetDialogCursorTheme(sgWidget *dialog, sgCursorTheme *theme)
{
  sgChangeCursorTheme(&sgDialog(dialog)->cursor, theme);
}

/* -------------------------------------------------------------------------- */
void sgSetDialogCursor(sgWidget *dialog, sgCursorType type)
{
  sgSetCursorFace(&sgDialog(dialog)->cursor, type);  
}  

/* -------------------------------------------------------------------------- */
void sgSetDialogPattern(sgWidget *dialog, sgPattern *pattern)
{
  sgDialog(dialog)->pattern = pattern;
  sgSetWidgetStatus(dialog, SG_REDRAW_FRAME);
}

/* -------------------------------------------------------------------------- */
void sgSetDialogContrast(sgWidget *dialog, Uint8 contrast)
{
  sgDialog(dialog)->contrast = contrast;
  sgSetWidgetStatus(dialog, SG_REDRAW_FRAME);
}

/** @} */
