/* $Id: input.c,v 1.10 2005/05/23 02:45:23 smoli Exp $
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

/** @weakgroup sgInput
 *  @{
 */

#include <libsgui/sgui.h>
#include <libsgui/history.h>

/** widget type definition and its callbacks */
sgWidgetType sgInputType =
{
  .name = "sgInput",
  .size = sizeof(sgInput),
  .methods =
  {
    .delete = sgDeleteInput,
    .recalc = sgRecalcInput,
    .redraw = sgRedrawInput,
    .handler = sgHandleInputEvent,
    .blit = sgBlitWidget
  }
};

/* -------------------------------------------------------------------------- *
 * Creates a new input widget (grouped)                                       *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewInputGrouped(sgWidget *group, sgEdge edge, sgAlign align, 
                            Uint16 w, Uint16 h, const char *caption)
{
  sgWidget *input = sgNewInput(group, 0, 0, w, h, caption);

  sgSubGroup(group, input, edge, align);

  return input;
}

/* -------------------------------------------------------------------------- *
 * Creates a new input widget                                                 *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewInput(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h, 
                     const char *caption) 
{
  sgWidget *input;
  
  input = sgNewWidget(&sgInputType, parent, x, y, w, h, caption);
  
  /* translate to unicode */
  SDL_EnableUNICODE(1);
  
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  sgNewHistory(&sgInput(input)->history);
  
  return input;
}

/* -------------------------------------------------------------------------- *
 * Free history data associated with the widget                               *
 * -------------------------------------------------------------------------- */
void sgDeleteInput(sgWidget *input)
{
  sgFreeHistory(&sgInput(input)->history);
}

/* -------------------------------------------------------------------------- *
 * Recalcs widget dimensions                                                  *
 * -------------------------------------------------------------------------- */
void sgRecalcInput(sgWidget *input)
{
  sgFont *font;
  Uint16 minh, minw;
    
  /* Get the font we're drawing into the inputbox */
  font = input->font[SG_FONT_FIXED];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, input->caption) + (input->border << 1) + 2;
  minh = sgFontHeight(font) + 3 + (input->border << 1) + 2;
 
  /* Apply them */
  if(input->rect.h < minh) input->rect.h = minh;
  if(input->rect.w < minw) input->rect.w = minw;

  /* Calc rectangle for the outer input frame */
  sgInput(input)->outer.x = 0;
  sgInput(input)->outer.y = 0;
  sgInput(input)->outer.w = input->rect.w;
  sgInput(input)->outer.h = input->rect.h;

  sgSubBorder(&sgInput(input)->outer, input->border);

  /* Calc rectangle for the inner input frame */
  sgInput(input)->inner = sgInput(input)->outer;
  sgSubBorder(&sgInput(input)->inner, 2);

  /* And finally the rectangle that contains the caption */
  sgInput(input)->caption = sgInput(input)->inner;
  
  sgSubBorder(&sgInput(input)->caption, 2);
  sgPadRect(&sgInput(input)->caption, SG_EDGE_LEFT|SG_EDGE_RIGHT, 3);

  sgSetWidgetStatus(input, SG_REDRAW_NEEDED);

  sgInput(input)->max = sgInput(input)->caption.w / sgTextWidth(font, "A");
}

/* -------------------------------------------------------------------------- *
 * Redraws inputbox look                                                       *
 * -------------------------------------------------------------------------- */
void sgRedrawInput(sgWidget *input)
{
  SDL_Rect rect;
  
  if(sgRedrawWidgetBorder(input))
  {
    if(sgHasWidgetFocus(input))
      sgDrawWidgetBorder(input, NULL);
    else
      sgDrawFrame(input->face.border, &sgInput(input)->outer, SG_DRAW_FILL);
  }

  if(sgRedrawWidgetFrame(input))
  {
    /* Draw the frames around the input field */
    sgDrawFrame(input->face.frame, &sgInput(input)->outer, SG_DRAW_FILL);
    sgDrawFrame(input->face.frame, &sgInput(input)->inner, SG_DRAW_INVERSE|SG_DRAW_CLEAR);
    
    /* Clear cursor */
//    SDL_FillRect(input->face.frame, &sgInput(input)->caption, 0);
    
    /* Draw the cursor */
    {
      char tmp[2];
      char part[256];
      
      tmp[0] = input->caption[sgInput(input)->pos];
      tmp[1] = '\0';
      
      if(!tmp[0])
        tmp[0] = ' ';
      
      strcpy(part, input->caption);
      part[sgInput(input)->pos] = 0;
      
      rect = sgInput(input)->caption;
      rect.y += rect.h - 4;
      rect.h = 4;
      rect.x += sgTextWidth(input->font[SG_FONT_FIXED], part) - 1;
      rect.w = sgTextWidth(input->font[SG_FONT_FIXED], tmp) + 1;
      
      sgDrawFrame(input->face.frame, &rect, SG_DRAW_FILL);
    }
  }

  if(sgRedrawWidgetContent(input))
  {
    /* Clear text */
//    SDL_FillRect(input->face.content, &sgInput(input)->caption, 0);
    
    /* Make the text appear inside sgInput(input)->caption */
    sgDrawTextOutline(input->font[SG_FONT_FIXED], input->face.content,
                      &sgInput(input)->caption, SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, 
                      input->caption);
  }
}

/* -------------------------------------------------------------------------- *
 * Handle events concerning the inputbox                                      *
 * -------------------------------------------------------------------------- */
int sgHandleInputEvent(sgWidget *input, SDL_Event *event)
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
      
      return sgInsertInputChar(input, c);
    }
    
    /* Handle backspace and delete */
    if(event->key.keysym.sym == SDLK_BACKSPACE) 
      return sgRemoveInputChar(input);
    if(event->key.keysym.sym == SDLK_DELETE)
      return sgDeleteInputChar(input);
    
    /* Return has been pressed, emitt a user handleable event */
    if(event->key.keysym.sym == SDLK_RETURN && input->caption[0])
    {
      sgReportWidgetEvent(input, SG_RETURN);
        
      sgAddHistoryEntry(&sgInput(input)->history, input->caption);
      input->caption[0] = '\0';
      sgInput(input)->len = 0;
      sgInput(input)->pos = 0;
      
      sgSetWidgetStatus(input, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
    }
    
    /* Home has been pressed, set position to 0 */
    if(event->key.keysym.sym == SDLK_HOME) 
    {
      sgSetWidgetStatus(input, SG_REDRAW_FRAME);
      
      sgInput(input)->pos = 0;
      return 1;
    }
    
    /* End has been pressed, set position to length */
    if(event->key.keysym.sym == SDLK_END) 
    {
      sgSetWidgetStatus(input, SG_REDRAW_FRAME);
      
      sgInput(input)->pos = sgInput(input)->len;
      return 1;
    }
    
    /* Up has been pressed, get next history entry */
    if(event->key.keysym.sym == SDLK_UP)
    {
      char *p;
      
      if((p = sgNextHistoryEntry(&sgInput(input)->history)))
        sgSetWidgetCaption(input, p);
      
      return 1;
    }
    
    /* Down has been pressed, get previous history entry */
    if(event->key.keysym.sym == SDLK_DOWN)
    {
      char *p;
      
      if((p = sgPrevHistoryEntry(&sgInput(input)->history)))
        sgSetWidgetCaption(input, p);
      
      return 1;
    }
  }
  
  if(sgHasWidgetFocus(input))
  {
    /* Handle clicks inside the textrect */
    if(event->type == SDL_MOUSEBUTTONDOWN && 
       event->button.button == SDL_BUTTON_LEFT) 
    {
      SDL_Rect rc;
      
      rc = sgInput(input)->caption;
      
      rc.x += input->rect.x;
      rc.y += input->rect.y;
      
      /* Is it inside the rect? */
      if(sgMatchRect(&rc, event->button.x, event->button.y)) 
      {
        int x;
        char caption[256];
        int start, end;
        int xpos;
        
        memset(caption, 0, 256);
        
        start = 0;
        
        xpos = (event->button.x - rc.x);
        
        for(x = 0; x < sgInput(input)->len; x++)
        {
          caption[x] = input->caption[x];
          
          end = sgTextWidth(input->font[SG_FONT_NORMAL], caption);
          
          if(xpos >= start && xpos < end)
            break;
          
          start = end;
        }
        
        if(x > (int)sgInput(input)->len)
          x = sgInput(input)->len;
        
        if(x != (int)sgInput(input)->pos) 
        {
          sgInput(input)->pos = x;
          
          sgSetWidgetStatus(input, SG_REDRAW_FRAME);
          return 1;
        }
      }
    }
    
    if((event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_WHEELUP) ||
       (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_LEFT))
    {
      if(sgInput(input)->pos > 0) 
      {
        sgInput(input)->pos--;
        sgSetWidgetStatus(input, SG_REDRAW_FRAME);
        return 1;
      }        
    }
    
    if((event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_WHEELDOWN) ||
       (event->key.keysym.sym == SDLK_RIGHT))
    {
      if(sgInput(input)->pos < sgInput(input)->len) 
      {
        sgInput(input)->pos++;
        sgSetWidgetStatus(input, SG_REDRAW_FRAME);
        return 1;
      }
    }
  }

  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Insert a char at the current position                                      *
 * -------------------------------------------------------------------------- */
int sgInsertInputChar(sgWidget *input, char c) 
{ 
  size_t len;
  
/*  sgLog("inserting char: pos = %i, len = %i, max = %i",
        sgInput(input)->pos, sgInput(input)->len, sgInput(input)->max);*/
  
  /* Check if there is space for another char */
  if(sgInput(input)->len < sgInput(input)->max) 
  {
    len = sgInput(input)->len - sgInput(input)->pos + 1;
    
    /* Move ahead every char from current position */
    memmove(&input->caption[sgInput(input)->pos + 1], 
            &input->caption[sgInput(input)->pos], len);
    
    /* Write the char, increment position and length and schedule a redraw */
    input->caption[sgInput(input)->pos++] = c;
    
/*    if(input->history[input->history_pos])
      free(input->history[input->history_pos]);
    
    input->history[input->history_pos] = strdup(input->caption);*/
    
    sgInput(input)->len++;
    
    sgSetWidgetStatus(input, SG_REDRAW_FRAME|SG_REDRAW_CONTENT);
    
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove a char left of the current position (backspace)                     *
 * -------------------------------------------------------------------------- */
int sgRemoveInputChar(sgWidget *input) 
{
  /* Do this only when position is not zero */
  if(sgInput(input)->pos) 
  {
    /* Move left all characters past the deleted one */
    memmove(&input->caption[sgInput(input)->pos - 1],
            &input->caption[sgInput(input)->pos], sgInput(input)->len);
    sgInput(input)->len--;
    sgInput(input)->pos--;
    
    sgSetWidgetStatus(input, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
    
/*    if(input->history[input->history_pos])
      free(input->history[input->history_pos]);
    
    input->history[input->history_pos] = strdup(input->caption);*/
        
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove a char on the current position (delete)                             *
 * -------------------------------------------------------------------------- */
int sgDeleteInputChar(sgWidget *input) 
{
  /* Do this only when cursor is not past the last character */
  if(sgInput(input)->pos < sgInput(input)->len) 
  {
    /* Move left all characters past the deleted one */
    memmove(&input->caption[sgInput(input)->pos], 
            &input->caption[sgInput(input)->pos + 1], sgInput(input)->len);
    sgInput(input)->len--;
    
    sgSetWidgetStatus(input, SG_REDRAW_CONTENT);
    return 1;
  }
  return 0;
}

/** @} */
