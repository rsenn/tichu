/* $Id: label.c,v 1.13 2005/05/21 22:36:09 smoli Exp $
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

/** @weakgroup sgLabel 
 *  @{
 */

#include <libsgui/sgui.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgLabelType =
{
  .name = "sgLabel",
  .size = sizeof(sgLabel),
  .methods =
  {
    .recalc = sgRecalcLabel,
    .redraw = sgRedrawLabel,
    .handler = NULL,
    .blit = sgBlitWidget
  }
};

/* -------------------------------------------------------------------------- *
 * Creates a new label widget (splitted)                                      *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewLabelSplitted(sgWidget *based, sgEdge edge, int pixels,
                             sgAlign textalign, const char *caption)
{
  sgWidget *label;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  label = sgNewLabel(based->parent, newrect.x, newrect.y, 
                     newrect.w, newrect.h, textalign, caption);

  return label;
}

/* -------------------------------------------------------------------------- *
 * Creates a new label widget (grouped)                                       *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewLabelGrouped(sgWidget *group, sgEdge edge, sgAlign align, 
                            Uint16 w, Uint16 h, sgAlign textalign, const char *caption)
{  
  sgWidget *label = sgNewLabel(group, 0, 0, w, h, textalign, caption);
  
  sgSubGroup(group, label, edge, align);
  
  return label;
}

/* -------------------------------------------------------------------------- *
 * Creates a new label widget (aligned)                                       *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewLabelAligned(sgWidget *group, sgEdge edge, sgAlign align, 
                            Uint16 w, Uint16 h, sgAlign textalign, const char *caption)
{  
  sgWidget *label = sgNewLabel(group, 0, 0, w, h, textalign, caption);
  
  sgAddGroup(group, label, edge, align);
  
  return label;
}

/* -------------------------------------------------------------------------- *
 * Creates a new label widget                                                 *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewLabel(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w,
                     Uint16 h, sgAlign textalign, const char *caption) 
{
  sgWidget *label;
  
  label = sgNewWidget(&sgLabelType, parent, x, y, w, h, caption);

  sgLabel(label)->align = textalign;
  
  return label;
}

/* -------------------------------------------------------------------------- *
 * Recalcs label dimensions                                                  *
 * -------------------------------------------------------------------------- */
void sgRecalcLabel(sgWidget *label)
{  
  sgFont   *font;
  Uint16    minh, minw;
  
  /* Get the font we're drawing on the label face */
  font = label->font[SG_FONT_NORMAL];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, label->caption) + (label->border << 1) + 2;
  minh = sgFontHeight(font) + 2;
  
  /* Apply them */
  if(label->rect.h < minh) label->rect.h = minh;
  if(label->rect.w < minw) label->rect.w = minw;
  
  sgLabel(label)->text.x = 0;
  sgLabel(label)->text.y = 0;
  sgLabel(label)->text.w = label->rect.w;
  sgLabel(label)->text.h = label->rect.h - label->border;
  
  sgPadRect(&sgLabel(label)->text, SG_EDGE_LEFT|SG_EDGE_RIGHT, label->border);
  
  sgSetWidgetStatus(label, SG_REDRAW_NEEDED);
}

/* -------------------------------------------------------------------------- *
 * Redraws label look                                                         *
 * -------------------------------------------------------------------------- */
void sgRedrawLabel(sgWidget *label) 
{
  if(sgRedrawWidgetContent(label))
  {
    sgDrawTextOutline(label->font[SG_FONT_NORMAL],
                      label->face.content, &sgLabel(label)->text,
                      sgLabel(label)->align, label->caption);
  }
}

/** @} */
