/* $Id: edit.c,v 1.25 2005/05/19 23:08:32 smoli Exp $
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

/** @weakgroup sgEdit
 *  @{
 */

#include <libsgui/sgui.h>

/** widget type definition and its callbacks */
sgWidgetType sgEditType =
{
  .name = "sgEdit",
  .size = sizeof(sgEdit),
  .methods =
  {
    .recalc = sgRecalcEdit,
    .redraw = sgRedrawEdit,
    .handler = sgHandleEditEvent,
    .blit = sgBlitWidget
  }
};

/* -------------------------------------------------------------------------- *
 * Creates a new edit widget (splitted)                                       *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewEditSplitted(sgWidget *based, sgEdge edge, Uint16 pixels,
                            const char *caption)
{
  sgWidget *edit;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  edit = sgNewEdit(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, caption);
  
  return edit;
}
/* -------------------------------------------------------------------------- *
 * Creates a new edit widget (grouped)                                        *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewEditGrouped(sgWidget *group, sgEdge edge, sgAlign align, 
                           Uint16 w, Uint16 h, const char *caption)
{
  sgWidget *edit = sgNewEdit(group, 0, 0, w, h, caption);

  sgSubGroup(group, edit, edge, align);

  return edit;
}

/* -------------------------------------------------------------------------- *
 * Creates a new edit widget                                                  *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewEdit(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h, 
                    const char *caption) 
{
  sgWidget *edit;
  
  edit = sgNewWidget(&sgEditType, parent, x, y, w, h,  caption);
  
  sgSetEditCaption(edit, caption);
  
  /* translate to unicode */
  SDL_EnableUNICODE(1);
  
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  
  return edit;
}

/* -------------------------------------------------------------------------- *
 * Recalcs widget dimensions                                                  *
 * -------------------------------------------------------------------------- */
void sgRecalcEdit(sgWidget *edit)
{
  sgFont *font;
  Uint16 minh, minw;
    
  /* Get the font we're drawing into the editbox */
  font = edit->font[SG_FONT_FIXED];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, edit->caption) + (edit->border << 1) + 2;
  minh = sgFontHeight(font) + 3 + (edit->border << 1) + 2;
  
  /* Apply them */
  if(edit->rect.h < minh) edit->rect.h = minh;
  if(edit->rect.w < minw) edit->rect.w = minw;

  /* Calc rectangle for the outer edit frame */
  sgEdit(edit)->outer.x = 0;
  sgEdit(edit)->outer.y = 0;
  sgEdit(edit)->outer.w = edit->rect.w;
  sgEdit(edit)->outer.h = edit->rect.h;
  
  sgSubBorder(&sgEdit(edit)->outer, edit->border);
  
  /* Calc rectangle for the inner edit frame */
  sgEdit(edit)->inner = sgEdit(edit)->outer;
  sgSubBorder(&sgEdit(edit)->inner, 2);
  
  /* And finally the rectangle that contains the caption */
  sgEdit(edit)->caption = sgEdit(edit)->inner;
  
  sgSubBorder(&sgEdit(edit)->caption, 2);
  sgPadRect(&sgEdit(edit)->caption, SG_EDGE_LEFT|SG_EDGE_RIGHT, 3);
  
  sgSetWidgetStatus(edit, SG_REDRAW_NEEDED);
  
  sgEdit(edit)->max = sgEdit(edit)->caption.w / sgTextWidth(font, "A");
}

/* -------------------------------------------------------------------------- *
 * Redraws editbox look                                                       *
 * -------------------------------------------------------------------------- */
void sgRedrawEdit(sgWidget *edit)
{
  SDL_Rect rect;
  
  if(sgRedrawWidgetBorder(edit))
  {
    if(sgHasWidgetFocus(edit))
      sgDrawWidgetBorder(edit, &sgEdit(edit)->outer);
    else
      sgDrawFrame(edit->face.border, &sgEdit(edit)->inner, SG_DRAW_INVERSE|SG_DRAW_FILL);
  }
  
  if(sgRedrawWidgetFrame(edit))
  {
    /* Draw the frames around the edit field */
    sgDrawFrame(edit->face.frame, &sgEdit(edit)->outer, SG_DRAW_NORMAL);
    sgDrawFrame(edit->face.frame, &sgEdit(edit)->inner, SG_DRAW_INVERSE|SG_DRAW_FILL);
    
    /* Draw the cursor */
    if(sgHasWidgetFocus(edit))
    {
      char tmp[2];
      char part[256];
      
      tmp[0] = edit->caption[sgEdit(edit)->pos];
      tmp[1] = '\0';
      
      if(!tmp[0])
        tmp[0] = ' ';
      
      strcpy(part, edit->caption);
      part[sgEdit(edit)->pos] = 0;
      
      rect = sgEdit(edit)->caption;
      rect.y += rect.h - 4;
      rect.h = 4;
      rect.x += sgTextWidth(edit->font[SG_FONT_FIXED], part) - 1;
      rect.w = sgTextWidth(edit->font[SG_FONT_FIXED], tmp) + 1;
      
      sgDrawFrame(edit->face.frame, &rect, SG_DRAW_FILL);
    }
  }

  if(sgRedrawWidgetContent(edit))
  {
    /* Make the text appear inside sgEdit(edit)->text */
    sgDrawTextOutline(edit->font[SG_FONT_FIXED], edit->face.content,
                      &sgEdit(edit)->caption, SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, edit->caption);
  }
}

/* -------------------------------------------------------------------------- *
 * Changes the editbox text                                                   *
 * -------------------------------------------------------------------------- */
void sgSetEditCaption(sgWidget *edit, const char *caption)
{
  sgSetWidgetCaption(edit, caption);
  sgEdit(edit)->pos = sgEdit(edit)->len = strlen(edit->caption);
  
  sgSetWidgetStatus(edit, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
}

/* -------------------------------------------------------------------------- *
 * Gets the editbox text                                                      *
 * -------------------------------------------------------------------------- */
char *sgGetEditCaption(sgWidget *edit) 
{
  return edit->caption;
}

/* -------------------------------------------------------------------------- *
 * Insert a char at the current position                                      *
 * -------------------------------------------------------------------------- */
int sgInsertEditChar(sgWidget *edit, char c) 
{ 
  size_t len;
  
  /* Check if there is space for another char */
  if(sgEdit(edit)->len < sgEdit(edit)->max) 
  {
    len = sgEdit(edit)->len - sgEdit(edit)->pos + 1;
    
    /* Move ahead every char from current position */
    memmove(&edit->caption[sgEdit(edit)->pos + 1], 
            &edit->caption[sgEdit(edit)->pos], len);
    
    /* Write the char, increment position and length and schedule a redraw */
    edit->caption[sgEdit(edit)->pos++] = c;
    sgEdit(edit)->len++;
    sgSetWidgetStatus(edit, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
    
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove a char left of the current position (backspace)                     *
 * -------------------------------------------------------------------------- */
int sgRemoveEditChar(sgWidget *edit) 
{
  /* Do this only when position is not zero */
  if(sgEdit(edit)->pos) 
  {
    /* Move left all characters past the deleted one */
    memmove(&edit->caption[sgEdit(edit)->pos - 1],
            &edit->caption[sgEdit(edit)->pos], sgEdit(edit)->len);
    sgEdit(edit)->len--;
    sgEdit(edit)->pos--;
    sgSetWidgetStatus(edit, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove a char on the current position (delete)                             *
 * -------------------------------------------------------------------------- */
int sgDeleteEditChar(sgWidget *edit) 
{
  /* Do this only when cursor is not past the last character */
  if(sgEdit(edit)->pos < sgEdit(edit)->len) 
  {
    /* Move left all characters past the deleted one */
    memmove(&edit->caption[sgEdit(edit)->pos], 
            &edit->caption[sgEdit(edit)->pos + 1], sgEdit(edit)->len);
    sgEdit(edit)->len--;
    sgSetWidgetStatus(edit, SG_REDRAW_CONTENT);
    return 1;
  }
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Handle events concerning the editbox                                       *
 * -------------------------------------------------------------------------- */
int sgHandleEditEvent(sgWidget *edit, SDL_Event *event)
{
  if(event->type == SDL_MOUSEMOTION)
  {
    if(!sgEdit(edit)->cursor && sgHasWidgetFocus(edit) &&
       sgMatchRect(&sgEdit(edit)->caption, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(edit->dialog, SG_CURSOR_EDIT);
      sgSetWidgetStatus(edit->dialog, SG_REDRAW_MOUSE);
      sgEdit(edit)->cursor = 1;
      return 1;
    }
    
    if(sgEdit(edit)->cursor && 
       !sgMatchRect(&sgEdit(edit)->caption, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(edit->dialog, SG_CURSOR_DEFAULT);
            sgSetWidgetStatus(edit->dialog, SG_REDRAW_MOUSE);
      sgEdit(edit)->cursor = 0;
      return 1;
    }
    
    
  }
  
  /* Handle clicks inside the textrect */
  if(event->type == SDL_MOUSEBUTTONDOWN && 
     event->button.button == SDL_BUTTON_LEFT && sgEdit(edit)->cursor)
  {
    int x;
    char caption[256];
    int start, end;
    int xpos;
    
    
    sgLog("len: %i", sgEdit(edit)->len);
    sgLog("pos: %i|%i", event->button.x, event->button.y);
    sgLog("sgEdit(edit)->caption: %i|%i|%i|%i", sgEdit(edit)->caption.x, sgEdit(edit)->caption.y, sgEdit(edit)->caption.w, sgEdit(edit)->caption.h);
    
    memset(caption, 0, 256);
      
    start = 0;
    
    xpos = (event->button.x - sgEdit(edit)->caption.x);
    
    for(x = 0; x < sgEdit(edit)->len; x++)
    {
      caption[x] = edit->caption[x];
      
      end = sgTextWidth(edit->font[SG_FONT_FIXED], caption);
      
      if(xpos >= start && xpos < end)
        break;
      
      start = end;
    }
    
    if(x > (int)sgEdit(edit)->len)
      x = sgEdit(edit)->len;
    
    if(x != (int)sgEdit(edit)->pos) 
    {
      sgEdit(edit)->pos = x;
      sgSetWidgetStatus(edit, SG_REDRAW_FRAME);
      return 1;
    }
  }
    
    
  /* We only care of events when the editbox has focus */
  if(sgHasWidgetFocus(edit))
  {
    /* Handle keyboard input */
    if(event->type == SDL_KEYDOWN) 
    {
      /* A printable character was received from the keyboard, insert it! */
      if(((event->key.keysym.unicode & 0xff) >= ' ' &&
          (event->key.keysym.unicode & 0xff) <= '~') ||
          (event->key.keysym.unicode & 0xff) >= 192) 
      {
        char c = (char)(event->key.keysym.unicode);
        
        return sgInsertEditChar(edit, c);
      }
      
      /* Handle backspace and delete */
      if(event->key.keysym.sym == SDLK_BACKSPACE) 
        return sgRemoveEditChar(edit);
      if(event->key.keysym.sym == SDLK_DELETE)
        return sgDeleteEditChar(edit);
      
      /* Return has been pressed, emitt a user handleable event */
      if(event->key.keysym.sym == SDLK_RETURN)
        sgReportWidgetEvent(edit, SG_RETURN);
      
      /* Home has been pressed, set position to 0 */
      if(event->key.keysym.sym == SDLK_HOME) 
      {
        sgSetWidgetStatus(edit, SG_REDRAW_FRAME);
        sgEdit(edit)->pos = 0;
        return 1;
      }
      
      /* End has been pressed, set position to length */
      if(event->key.keysym.sym == SDLK_END) 
      {
        sgSetWidgetStatus(edit, SG_REDRAW_FRAME);
        sgEdit(edit)->pos = sgEdit(edit)->len;
        return 1;
      } 
    }
  
  
    if((event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_WHEELUP) ||
       (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_LEFT))
    {
      if(sgEdit(edit)->pos > 0) 
      {
        sgEdit(edit)->pos--;
        sgSetWidgetStatus(edit, SG_REDRAW_FRAME);
        return 1;
      }        
    }
    
    if((event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_WHEELDOWN) ||
       (event->key.keysym.sym == SDLK_RIGHT))
    {
      if(sgEdit(edit)->pos < sgEdit(edit)->len) 
      {
        sgEdit(edit)->pos++;
        sgSetWidgetStatus(edit, SG_REDRAW_FRAME);
        return 1;
      }
    }
  }
  
  return 0;
}

/** @} */
