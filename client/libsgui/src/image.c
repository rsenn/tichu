/* $Id: image.c,v 1.24 2005/05/18 22:45:56 smoli Exp $
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

/** @weakgroup sgImage 
 *  @{
 */

#include <libsgui/sgui.h>
#include <libsgui/png.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgImageType =
{
  .name = "sgImage",
  .size = sizeof(sgImage),
  .methods =
  {
    .recalc = sgRecalcImage,
    .redraw = sgRedrawImage,
    .handler = sgHandleImageEvent,
    .blit = sgBlitImage
  }
};

/* -------------------------------------------------------------------------- *
 * Creates a new image widget (splitted)                                      *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewImageSplitted(sgWidget *based, sgEdge edge, int pixels,
                             const char *caption)
{
  sgWidget *image;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  image = sgNewImage(based->parent, 
                     newrect.x, newrect.y, newrect.w, newrect.h, caption);

  return image;
}

/* -------------------------------------------------------------------------- *
 * Creates a new image widget (grouped)                                       *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewImageGrouped(sgWidget *group, sgEdge edge, sgAlign align, 
                            Uint16 w, Uint16 h, const char *caption)  
{  
  sgWidget *image = sgNewImage(group, 0, 0, w, h, caption);
  
  sgSubGroup(group, image, edge, align);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * Creates a new image widget from rectangle dimension                        *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewImageRect(sgWidget *parent, SDL_Rect rect, const char *caption)
{
  return sgNewWidget(&sgImageType, parent, rect.x, rect.y, rect.w, rect.h, caption);
}
  
/* -------------------------------------------------------------------------- *
 * Creates a new image widget                                                 *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewImage(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w,
                     Uint16 h, const char *caption) 
{
  sgWidget *image;
  
  image = sgNewWidget(&sgImageType, parent, x, y, w, h, caption);

  return image;
}

/* -------------------------------------------------------------------------- *
 * Recalcs image dimensions                                                  *
 * -------------------------------------------------------------------------- */
void sgRecalcImage(sgWidget *image)
{  
  sgFont   *font;
  Uint16    minh, minw;
  
  /* Get the font we're drawing on the image face */
  font = image->font[SG_FONT_BOLD];
  
  /* Calculate minimal dimensions based on font */
  if(image->caption[0])
  {
    minw = sgTextWidth(font, image->caption) + (image->border << 1) + 2;
    minh = sgFontHeight(font) + 3 + (image->border << 1) + 2;
    
    /* Apply them */
    if(image->rect.h < minh) image->rect.h = minh;
    if(image->rect.w < minw) image->rect.w = minw;
  }
  
  /* Calc rectangle for the outer image frame */
  sgImage(image)->outer.x = 0;
  sgImage(image)->outer.y = 0;
  sgImage(image)->outer.w = image->rect.w;
  sgImage(image)->outer.h = image->rect.h;
  
  if(image->border)
    sgSubBorder(&sgImage(image)->outer, image->border);
    
  /* Calc rectangle for the inner group frame */
  sgImage(image)->inner = sgImage(image)->outer;
  
  if(image->border)
    sgSubBorder(&sgImage(image)->inner, 2);
  
  /* Split off the caption rectangle */
  if(image->caption[0])
    sgSplitRect(&sgImage(image)->inner,
                &sgImage(image)->caption, SG_EDGE_TOP, sgFontHeight(font));
  
  /* Calc rectangle for the image image frame */
  sgImage(image)->body = sgImage(image)->inner;
  
  if(image->border)
    sgSubBorder(&sgImage(image)->body, 2);
  
  sgSetWidgetStatus(image, SG_REDRAW_NEEDED);
}

/* -------------------------------------------------------------------------- *
 * Redraws image look                                                         *
 * -------------------------------------------------------------------------- */
void sgRedrawImage(sgWidget *image) 
{
  if(sgRedrawWidgetBorder(image))
  {
    if(sgHasWidgetFocus(image))
      sgDrawWidgetBorder(image, &sgImage(image)->outer);
  }
  
  if(image->border && sgRedrawWidgetFrame(image))
  {
    sgDrawFrame(image->face.frame, &sgImage(image)->outer,
                SG_DRAW_NORMAL|SG_DRAW_FILL);

    sgDrawFrame(image->face.frame, &sgImage(image)->inner,
                SG_DRAW_INVERSE|SG_DRAW_CLEAR);
  }
  
  if(sgRedrawWidgetContent(image))
  {
    if(image->caption[0])
      sgDrawTextOutline(image->font[SG_FONT_BOLD],
                        image->face.content, &sgImage(image)->caption,
                        SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, image->caption);
  }
}

/* -------------------------------------------------------------------------- *
 * Sets the image to display inside the widget                                *
 * -------------------------------------------------------------------------- */
int sgSetImageSurface(sgWidget *image, SDL_Surface *surface, sgAlign align)
{
  if(sgImage(image)->image)
    SDL_FreeSurface(sgImage(image)->image);
      
  sgImage(image)->align = align;
  sgImage(image)->image = (surface->format->Amask ? 
                           SDL_DisplayFormatAlpha(surface) :
                           SDL_DisplayFormat(surface));
  
  if(sgImage(image)->image)
  {
    sgImage(image)->srect = sgImage(image)->image->clip_rect;
    
    if(sgImage(image)->srect.w > sgImage(image)->body.w || 
       sgImage(image)->srect.h > sgImage(image)->body.h)
    {
      sgImage(image)->srect.w = sgImage(image)->body.w;
      sgImage(image)->srect.h = sgImage(image)->body.h;
      
      sgAlignRect(&sgImage(image)->image->clip_rect, &sgImage(image)->srect, sgImage(image)->align);
    }
    
    sgImage(image)->drect = sgImage(image)->srect;
    
    sgAlignRect(&sgImage(image)->body, &sgImage(image)->drect, sgImage(image)->align);
    
    sgSetWidgetStatus(image, SG_REDRAW_CONTENT);
    return 1;
  }
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Loads an image from a file                                                 *
 * -------------------------------------------------------------------------- */
int sgLoadImageFile(sgWidget *image, const char *file, sgAlign align)
{
  SDL_Surface *surface = sgLoadPngFile(file);
  
  if(surface)
  {
    sgSetImageSurface(image, surface, align);
    SDL_FreeSurface(surface);
    return 0;
  }
  
  return -1;
}  
  
/* -------------------------------------------------------------------------- *
 * Loads an image from an SDL_RWops                                           *
 * -------------------------------------------------------------------------- */
int sgLoadImageRWops(sgWidget *image, SDL_RWops *rwops, sgAlign align)
{
  SDL_Surface *surface = sgLoadPngRWops(rwops);
  
  if(surface)
  {
    sgSetImageSurface(image, surface, align);
    SDL_FreeSurface(surface);
    return 0;
  }
  
  return -1;
}  
  
/* -------------------------------------------------------------------------- *
 * Loads an image from a file pointer                                         *
 * -------------------------------------------------------------------------- */
int sgLoadImageFp(sgWidget *image, FILE *fp, sgAlign align)
{
  SDL_Surface *surface = sgLoadPngFp(fp, 0);
  
  if(surface)
  {
    sgSetImageSurface(image, surface, align);
    SDL_FreeSurface(surface);
    return 0;
  }
  
  return -1;
}  
  
/* -------------------------------------------------------------------------- *
 * Blits an image widget                                                      *
 * -------------------------------------------------------------------------- */
int sgBlitImage(sgWidget *image, SDL_Surface *surface, Sint16 x, Sint16 y)
{
  SDL_Rect rect;
  /* Do not blit hidden widgets */
  if(image->status & SG_HIDDEN)
    return 0;

  /* Redraw widgets faces if needed */
  sgRedrawWidget(image);

  rect = image->rect;
  rect.x += x;
  rect.y += y;

  /* Blit the surfaces to screen */
  if(image->face.frame)
    SDL_BlitSurface(image->face.frame, NULL, surface, &rect);

  if(image->face.border)
    SDL_BlitSurface(image->face.border, NULL, surface, &rect);
  
  if(image->face.content)
    SDL_BlitSurface(image->face.content, NULL, surface, &rect);

  if(sgImage(image)->image)
  {
    SDL_Rect drect;
    
    drect = sgImage(image)->drect;
    
    drect.x += rect.x;
    drect.y += rect.y;
    
    SDL_BlitSurface(sgImage(image)->image, &sgImage(image)->srect, surface, &drect);
  }
  
  /* Reset redraw flags */
  sgClearWidgetStatus(image, SG_REDRAW_NEEDED);

  return 1;
}

/* -------------------------------------------------------------------------- *
 * Handle events concerning the image                                         *
 * -------------------------------------------------------------------------- */
int sgHandleImageEvent(sgWidget *image, SDL_Event *event)
{
  if(event->type == SDL_MOUSEMOTION)
  {
    if(sgMatchRect(&sgImage(image)->body, event->motion.x, event->motion.y))
      sgReportWidgetEvent(image, SG_EVENT_MOTION);
  }
  
  return 0;
}
  
/** @} */
