/* $Id: console.c,v 1.18 2005/05/23 02:45:23 smoli Exp $
 * ------------------------------------------------------------------------- *
 *                   /                                                       *
 *    ___  ___                                                               *
 *   |___ |   )|   )|        Simple and smooth GUI library :)                *
 *    __/ |__/ |__/ |        Copyright (C) 2003-2005  Roman Senn             *
 *        __/                                                                *
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

/** @weakgroup sgConsole
 *  @{
 */

#include <libsgui/sgui.h>
#include <libsgui/file.h>

/** widget type definition and its callbacks */
sgWidgetType sgConsoleType =
{
  .name = "sgConsole",
  .size = sizeof(sgConsole),
  .methods =
  {
    .recalc = sgRecalcConsole,
    .redraw = sgRedrawConsole,
    .handler = sgHandleConsoleEvent,
    .blit = sgBlitWidget,
    .delete = sgClearConsole
  }  
};

/* Creates a new console widget using full dialog size
 * -------------------------------------------------------------------------- */
sgWidget *sgNewConsoleFull(sgWidget *parent, const char *caption)
{
  SDL_Rect rect = parent->rect;
  
  if(parent->type == &sgGroupType)
  {
    rect = sgGroup(parent)->body;
  }
  else
  {
    sgSubBorder(&rect, parent->border);
  }

  return sgNewConsole(parent, rect.x - parent->rect.x, rect.y - parent->rect.y,
                      rect.w, rect.h, caption);
}

/* Creates a new console widget by splitting another
 * -------------------------------------------------------------------------- */
sgWidget *sgNewConsoleSplitted(sgWidget *based, sgEdge edge, int pixels,
                              const char *caption)
{
  sgWidget *console;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  console = sgNewConsole(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, caption);
  
  return console;
}

/* -------------------------------------------------------------------------- *
 * Creates a new console widget (grouped)                                     *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewConsoleGrouped(sgWidget *group, sgEdge edge, sgAlign align,
                              Uint16 w, Uint16 h, const char *caption)
{
  sgWidget *console = sgNewConsole(group, 0, 0, w, h, caption);
  
  sgSubGroup(group, console, edge, align);
    
  return console;
}

/* -------------------------------------------------------------------------- *
 * Creates a new console widget                                               *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewConsole(sgWidget *parent, Sint16 x, Sint16 y, 
                       Uint16 w, Uint16 h, const char *caption)
{
  sgWidget *console;
  
  console = sgNewWidget(&sgConsoleType, parent, x, y, w, h, caption);
  
  return console;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgRecalcConsole(sgWidget *console)
{
  sgFont *font;
  Uint16 minh, minw;
  
  /* Get the font we're drawing to the console title */
  font = console->font[SG_FONT_BOLD];
  
  /* Calculate minimal dimensions based on font */
  if(console->caption[0])
  {
    minw = sgTextWidth(font, console->caption) + (console->border << 1) + 2;
    minh = sgFontHeight(font) + 3 + (console->border << 1) + 2;
    
    /* Apply them */
    if(console->rect.h < minh) console->rect.h = minh;
    if(console->rect.w < minw) console->rect.w = minw;
  }
  
  /* Calc rectangle for the outer console frame */
  sgConsole(console)->outer.x = 0;
  sgConsole(console)->outer.y = 0;
  sgConsole(console)->outer.w = console->rect.w;
  sgConsole(console)->outer.h = console->rect.h;
  
  sgSubBorder(&sgConsole(console)->outer, console->border);
  
  /* Calc rectangle for the inner console frame */
  sgConsole(console)->inner = sgConsole(console)->outer;
  sgSubBorder(&sgConsole(console)->inner, 2);
  
  /* Split off the caption rectangle */
  if(console->caption[0])
    sgSplitRect(&sgConsole(console)->inner,
                &sgConsole(console)->caption, SG_EDGE_TOP, sgFontHeight(font));
  
  /* setup scrollbar rectangles */
  sgSplitRight(&sgConsole(console)->inner, &sgConsole(console)->sbar_space, 20);
  sgSplitTop(&sgConsole(console)->sbar_space, &sgConsole(console)->sbar_up, 20);
  sgSplitBottom(&sgConsole(console)->sbar_space, &sgConsole(console)->sbar_down, 20);
  
  sgConsole(console)->sbar_space.y--;
  sgConsole(console)->sbar_space.h += 2;
  
  sgConsole(console)->sbar_bar = sgConsole(console)->sbar_space;
  
  
  /* setup main rectangle */
  sgConsole(console)->body = sgConsole(console)->inner;
    
  sgSubBorder(&sgConsole(console)->body, 2);
    
    sgPadRect(&sgConsole(console)->body, SG_EDGE_LEFT|SG_EDGE_RIGHT, 4);
    sgPadRect(&sgConsole(console)->body, SG_EDGE_TOP|SG_EDGE_BOTTOM, 2);
  
  /* line dimensions */
  sgConsole(console)->line = sgConsole(console)->body;
    sgConsole(console)->line.h = sgFontHeight(console->font[SG_FONT_FIXED]) + 8;
  
  
  /* max. lines displayed at a time */
  sgConsole(console)->rows = sgConsole(console)->body.h / sgConsole(console)->line.h;
  
  sgConsole(console)->cols = (sgConsole(console)->body.w - 8) /
    sgTextWidth(console->font[SG_FONT_FIXED], "A");

  sgScrollConsole(console, sgConsole(console)->scroll, 1, 1);
  
  sgSetWidgetStatus(console, SG_REDRAW_NEEDED);
}

/* -------------------------------------------------------------------------- *
 * Redraw console look                                                        *
 * -------------------------------------------------------------------------- */
void sgRedrawConsole(sgWidget *console)
{
  sgConsoleLine *line;
  SDL_Rect linerect;
  int i;
  
  if(sgRedrawWidgetBorder(console))
  {
    if(sgHasWidgetFocus(console))
      sgDrawWidgetBorder(console, NULL);
    else
      sgDrawFrame(console->face.border, &sgConsole(console)->outer, SG_DRAW_FILL);
  }
  
  /* Redraw frames */
  if(sgRedrawWidgetFrame(console))
  {
    /* Draw console content frame */
    sgDrawFrame(console->face.frame, &sgConsole(console)->outer,
                SG_DRAW_FILL);
    sgDrawFrame(console->face.frame, &sgConsole(console)->inner,
                SG_DRAW_INVERSE|SG_DRAW_CLEAR);
    
    /* Draw up button/arrow */
    sgDrawFrame(console->face.frame, &sgConsole(console)->sbar_up,
                  (sgConsole(console)->spushed0 ? SG_DRAW_INVERSE : 0)
                  |SG_DRAW_FILL);
    
    sgPutPict(console->face.frame, &sgConsole(console)->sbar_up,
              &sgArrowUp, sgConsole(console)->spushed0);
    
    /* Draw down button/arrow */
    sgDrawFrame(console->face.frame, &sgConsole(console)->sbar_down,
                  (sgConsole(console)->spushed1 ? SG_DRAW_INVERSE : 0)
                  |SG_DRAW_FILL);
    
    sgPutPict(console->face.frame, &sgConsole(console)->sbar_down,
              &sgArrowDown, sgConsole(console)->spushed1);
    
    /* Draw scrollbar space frame */
    sgDrawFrame(console->face.frame, &sgConsole(console)->sbar_space,
                  SG_DRAW_INVERSE|SG_DRAW_FILL);
    
    /* Draw scrollbar bar frame */
    sgDrawFrame(console->face.frame, &sgConsole(console)->sbar_bar,
                (sgConsole(console)->spushed ? 0 : SG_DRAW_HIGH) | SG_DRAW_FILL);
  }

  /* Redraw contents */
  if(sgRedrawWidgetContent(console))
  {
    /* Draw console lines */
    line = sgGetConsoleLineByNum(console, sgConsole(console)->scroll);
    linerect = sgConsole(console)->outer;
  
    /* Clear textarea + some extra pixels above */
    SDL_FillRect(console->face.content, &linerect, 0);
    
    /* Draw until beyond textrect */
    linerect = sgConsole(console)->line;
    
    linerect.y += sgConsole(console)->body.h - sgConsole(console)->line.h;
    
    for(i = 0; line && linerect.y > sgConsole(console)->body.y; line = (line->node.next ? line->node.next->data : NULL), i++)
    {
      /* Draw the text */
      sgDrawTextOutline(console->font[SG_FONT_FIXED],
                        console->face.content, &linerect,
                        SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, line->text);
      
      linerect.y -= sgConsole(console)->line.h;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Handle console events                                                      *
 * -------------------------------------------------------------------------- */
int sgHandleConsoleEvent(sgWidget *console, SDL_Event *event) 
{
  if(event->type == SDL_MOUSEMOTION)
  {
    if(!sgConsole(console)->cursor &&
       sgMatchRect(&sgConsole(console)->sbar_bar, event->motion.x, event->motion.y))
    {
      
      sgSetDialogCursor(console->dialog, SG_CURSOR_RESIZE_V);
      sgSetWidgetStatus(console->dialog, SG_REDRAW_MOUSE);
      sgConsole(console)->cursor = 1;
      
      sgLog("activated");
      
      return 1;
    }

    if(sgConsole(console)->cursor && !sgConsole(console)->spushed &&
       !sgMatchRect(&sgConsole(console)->sbar_bar, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(console->dialog, SG_CURSOR_DEFAULT);
      sgSetWidgetStatus(console->dialog, SG_REDRAW_MOUSE);
      sgConsole(console)->cursor = 0;
      return 1;
    }
  }
  
  /* Console must have focus to generate events */
  if(sgHasWidgetFocus(console))
  {
    /* Mousewheel down */
    if(sgEventButton(event, SDL_BUTTON_WHEELUP, SG_PRESSED))
    {
      return sgScrollConsole(console, sgConsole(console)->scroll + 1, 0, 1);
    }
    /* Mousewheel up */
    else if(sgEventButton(event, SDL_BUTTON_WHEELDOWN, SG_PRESSED))
    {
      return sgScrollConsole(console, sgConsole(console)->scroll - 1, 0, 1);
    }
    else if(sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED))
    {
      sgLog("console button press");
      
      /* Check for click on the up arrow */
      if(!sgConsole(console)->spushed0 && 
         sgMatchRect(&sgConsole(console)->sbar_up, event->button.x, event->button.y))
      {
        sgConsole(console)->spushed0 = 1;
        sgSetWidgetStatus(console, SG_REDRAW_FRAME);
        return 1;
      }
      
      /* Check for click on the down arrow */
      if(!sgConsole(console)->spushed1 && 
         sgMatchRect(&sgConsole(console)->sbar_down, event->button.x, event->button.y))
      {
        sgConsole(console)->spushed1 = 1;
        sgSetWidgetStatus(console, SG_REDRAW_FRAME);
        return 1;
      }
      
      /* If we're not scrolling and there are more lines that 
         can be displayed once we're entering scrolling mode */
      if(!sgConsole(console)->spushed && sgConsole(console)->lines.size > sgConsole(console)->rows &&
         sgMatchRect(&sgConsole(console)->sbar_bar, event->button.x, event->button.y))
      {
        sgConsole(console)->clicky = event->button.y;
        sgConsole(console)->spushed = 1;
        sgSetWidgetStatus(console, SG_REDRAW_FRAME);
        return 1;
      }
    }
    /* Mouse button was released while console focused */
    else if(sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED))
    {
      /* Up arrow was pushed */
      if(sgConsole(console)->spushed0)
      {
        sgConsole(console)->spushed0 = 0;
        sgSetWidgetStatus(console, SG_REDRAW_FRAME);

        if(sgConsole(console)->scroll > 0)
          return sgScrollConsole(console, sgConsole(console)->scroll + 1, 0, 1);

        return 1;
      }
      
      /* Down arrow was pushed */
      if(sgConsole(console)->spushed1)
      {
        sgConsole(console)->spushed1 = 0;
        sgSetWidgetStatus(console, SG_REDRAW_FRAME);

        if(sgConsole(console)->scroll < sgConsole(console)->lines.size - sgConsole(console)->rows)
          return sgScrollConsole(console, sgConsole(console)->scroll - 1, 1, 1);

        return 1;
      }
      
      /* We we're scrolling */
      if(sgConsole(console)->spushed)
      {
        if(!sgMatchRect(&sgConsole(console)->sbar_bar, event->button.x, event->button.y))
        {
          sgSetDialogCursor(console->dialog, SG_CURSOR_DEFAULT);
          sgSetWidgetStatus(console->dialog, SG_REDRAW_MOUSE);
        }
        
        sgConsole(console)->spushed = 0;
        sgScrollConsole(console, sgConsole(console)->scroll, 1, 1);

        return 1;
      }
    }
    /* Mouse has moved */
    else if(event->type == SDL_MOUSEMOTION)
    {
      /* We're scrolling */
      if(sgConsole(console)->spushed)
      {
        int ydiff = event->motion.y - sgConsole(console)->clicky + (sgConsole(console)->sbar_bar.y - sgConsole(console)->sbar_space.y);
        int yrange = sgConsole(console)->sbar_space.h - sgConsole(console)->sbar_bar.h;
        int irange = sgConsole(console)->lines.size - sgConsole(console)->rows + 1;

        sgConsole(console)->clicky = event->motion.y;

        if(irange)
        {
          ydiff = ydiff < 0 ? 0 : (ydiff >= yrange ? yrange - 1 : ydiff);

          sgConsole(console)->sbar_bar.y = sgConsole(console)->sbar_space.y + ydiff;

          sgScrollConsole(console, irange - (ydiff * irange / yrange) - 1, 0, 0);

          sgSetWidgetStatus(console, SG_REDRAW_FRAME);
        }
        
        return 1;
      }
    }
  }
  
  if(!(console->status & SG_FOCUS))
  {
    if(sgConsole(console)->spushed0 || sgConsole(console)->spushed1)
    {
      sgConsole(console)->spushed0 = 0;
      sgConsole(console)->spushed1 = 0;
      sgSetWidgetStatus(console, SG_REDRAW_FRAME);
      return 1;
    }
  }
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgAddConsoleLineWrapped(sgWidget *console, const char *text)
{
  char str[1024];
  char *line_start, *backup;
  char *line_end;
  int len;
  
  if(strlen(text) <= sgConsole(console)->cols)
  {
    sgAddConsoleLine(console, text);
    return;
  }
  
  backup = line_start = strdup(text);
  
  while(1)
  {
    if(strlen(line_start) > sgConsole(console)->cols)
    {         
      line_end = line_start + sgConsole(console)->cols;
      while(*line_end != ' ' && line_end > line_start + 1)
        line_end--;
    }    
    else
    {      
      line_end = line_start + strlen(line_start);
    }

    len = line_end - line_start;
    
    if(len)
    {
      strncpy(str, line_start, len);
      str[len] = '\0';
      
      sgAddConsoleLine(console, str);
    }

    if(strlen(line_start) < sgConsole(console)->cols)
      break;
  
    line_start = line_end;
  }
  
  free(backup);
  
  return;
}

/* -------------------------------------------------------------------------- *
 * Adds a line to a console                                                   *
 * -------------------------------------------------------------------------- */
sgConsoleLine *sgAddConsoleLine(sgWidget *console, const char *text) 
{
  sgConsoleLine *ret;

  ret = malloc(sizeof(sgConsoleLine));
  
    sgLog("adding line: %s", text);
  
  sgStringCopy(ret->text, text);
  
  sgAddListHead(&sgConsole(console)->lines, &ret->node, ret);
  
  sgConsole(console)->scroll = 0;
  sgScrollConsole(console, sgConsole(console)->scroll, 1, 1);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Deletes a line from a console                                              *
 * -------------------------------------------------------------------------- */
void sgDeleteConsoleLine(sgWidget *console, sgConsoleLine *line)
{
  sgDeleteList(&sgConsole(console)->lines, &line->node);

  free(line);
  
  sgSetWidgetStatus(console, SG_REDRAW_FRAME|SG_REDRAW_CONTENT);
}

/* -------------------------------------------------------------------------- *
 * Clears all console lines                                                   *
 * -------------------------------------------------------------------------- */
void sgClearConsole(sgWidget *console)
{
  sgConsoleLine *line;
  sgNode *next;
  
  sgForeachSafe(&sgConsole(console)->lines, line, next)
    sgDeleteConsoleLine(console, line);
}   
  
/* -------------------------------------------------------------------------- *
 * Adds a string to a console                                                 *
 * -------------------------------------------------------------------------- */
void sgAddConsoleText(sgWidget *console, const char *text)
{
  char *copy;
  char *last;
  char *p;
  
  if(!text)
    return;
  
  p = last = copy = strdup(text);

  do
  {
    if((p = strchr(last, '\n')))
      *p++ = '\0';
    
    sgAddConsoleLineWrapped(console, last);
  } 
  while((last = p));
  
  free(copy);
}

void sgSetConsoleText(sgWidget *console, const char *text)
{
  sgClearConsole(console);
  
  sgAddConsoleText(console, text);
}
  
/* -------------------------------------------------------------------------- *
 * Converts the text of a console into a string                               *
 * -------------------------------------------------------------------------- */
char *sgGetConsoleText(sgWidget *console)
{
  int n = 0;
  char *text;
  sgConsoleLine *line;
  sgConsoleLine *last = NULL;

  if(sgConsole(console)->lines.size == 0)
    return NULL;
  
  sgForeach(&sgConsole(console)->lines, line)
  {
    n += strlen(line->text) + 1;
    last = line;
  }
  
  text = malloc(n + 1);
  
  text[0] = '\0';
  
  for(line = last; line; line = (line->node.next ? line->node.next->data : NULL))
  {
    strcat(text, line->text);
    strcat(text, "\n");
  }

  return text;
}

/* -------------------------------------------------------------------------- *
 * Loads the text of a console from a file                                    *
 * -------------------------------------------------------------------------- */
int sgLoadConsoleText(sgWidget *console, const char *file)
{
  char *s;
  
  if((s = sgLoadFileText(file)))
  {
    sgSetConsoleText(console, s);
    return -1;
  }
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Gets a line by its number                                                  *
 * -------------------------------------------------------------------------- */
sgConsoleLine *sgGetConsoleLineByNum(sgWidget *console, int num) 
{  
  return (sgConsoleLine *)sgIndexList(&sgConsole(console)->lines, num);
}

/* -------------------------------------------------------------------------- *
 * Scrolls a console                                                          *
 * -------------------------------------------------------------------------- */
int sgScrollConsole(sgWidget *console, int pos, int force, int setrect)
{
  SDL_Rect rect;
  int range;
  Sint16 x, y;

  sgGetWidgetMouse(console, &x, &y);
  
  /* Truncate position */
  pos = (pos > 0 ? (pos < sgConsole(console)->lines.size - sgConsole(console)->rows ?
                    pos : sgConsole(console)->lines.size - sgConsole(console)->rows) : 0);

  /* If scrollbar is useable */
  if(sgConsole(console)->lines.size > sgConsole(console)->rows)
  {
    rect = sgConsole(console)->sbar_space;

    /* Calculate scrollbar height */
    rect.h = ((rect.h - rect.w) * sgConsole(console)->rows) / sgConsole(console)->lines.size + rect.w;

    /* Range to scroll */
    range = sgConsole(console)->sbar_space.h - rect.h;

    /* Calculate scrollbar position */
    rect.y += range - (pos * range / (sgConsole(console)->lines.size - sgConsole(console)->rows));
  }
  /* Too few lines for scrolling */
  else
  {
    /* Disable scrollbar */
    pos = 0;
    rect = sgConsole(console)->sbar_space;
  }

  /* Scrolling position changed or rescroll was forced */
  if(pos != sgConsole(console)->scroll || force)
  {
    /* Position changed, generate an event */
    if(pos != sgConsole(console)->scroll)
      sgReportWidgetEvent(console, SG_SCROLL);

    /* Set new position */
    sgConsole(console)->scroll = pos;

    /* Should we set a new scrollbar rectangle? */
/*    if(setrect)
      sgConsole(console)->sbar_bar = rect;*/

    if(!sgConsole(console)->cursor &&
       sgMatchRect(&sgConsole(console)->sbar_bar, x, y))
    {
      sgSetDialogCursor(console->dialog, SG_CURSOR_RESIZE_V);
      sgSetWidgetStatus(console->dialog, SG_REDRAW_MOUSE);
      sgConsole(console)->cursor = 1;
    }

    if(sgConsole(console)->cursor && 
       !sgMatchRect(&sgConsole(console)->sbar_bar, x, y))
    {
      sgSetDialogCursor(console->dialog, SG_CURSOR_DEFAULT);
      sgSetWidgetStatus(console->dialog, SG_REDRAW_MOUSE);
      sgConsole(console)->cursor = 0;
    }    
    
    /* Redraw scrollbar stuff */
    sgSetWidgetStatus(console, SG_REDRAW_FRAME|SG_REDRAW_CONTENT);
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Frees all data associated with a console                                   *
 * -------------------------------------------------------------------------- */
void sgFreeConsole(sgWidget *console)
{
  sgConsoleLine *line;
  sgConsoleLine *next;
  
  sgForeachSafe(&sgConsole(console)->lines, line, next)
    sgDeleteConsoleLine(console, line);
}
