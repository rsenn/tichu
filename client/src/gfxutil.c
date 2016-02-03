/* $Id: gfxutil.c,v 1.49 2005/05/21 08:27:20 smoli Exp $
 * -------------------------------------------------------------------------- *
 *  .___.    .                                                                *
 *    |  * _.|_ . .        Portabler, SDL-basierender Client für das          *
 *    |  |(_.[ )(_|             Multiplayer-Kartenspiel Tichu.                *
 *  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . *
 *                                                                            *
 *               (c) 2004-2005 by Martin Zangger, Roman Senn                  *
 *                                                                            *
 *    Dieses Programm ist freie Software. Sie können es unter den Bedingungen *
 * der GNU General Public License, wie von der Free Software Foundation ver-  *
 * öffentlicht, weitergeben und/oder modifizieren, entweder gemäss Version 2  *
 * der Lizenz oder (nach Ihrer Option) jeder späteren Version.                *
 *                                                                            *
 *    Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, dass es  *
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die  *
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BE-    *
 * STIMMTEN ZWECK. Details finden Sie in der GNU General Public License.      *
 *                                                                            *
 *    Sie sollten eine Kopie der GNU General Public License zusammen mit      *
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free     *
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          *
 * 02111-1307, USA.                                                           *
 * -------------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#include <math.h>

/* -------------------------------------------------------------------------- *
 * Das [gfxutil] Modul stellt verschiedene Pixel- und Geometrie Hilfs-        *
 * funktionen zur Verfügung                                                   *
 * -------------------------------------------------------------------------- */
#include "gfxutil.h"
#include "client.h"

 /* -------------------------------------------------------------------------- *
  * Nützliche Konstanten                                                       *
  * -------------------------------------------------------------------------- */
const struct color gfxutil_white   = { .r=0xff, .g=0xff, .b=0xff, .a=0xff };
const struct color gfxutil_white0  = { .r=0xff, .g=0xff, .b=0xff, .a=0x00 };
const struct color gfxutil_black   = { .r=0x00, .g=0x00, .b=0x00, .a=0xff };
const struct color gfxutil_red     = { .r=0xff, .g=0x00, .b=0x00, .a=0xff };
const struct color gfxutil_red128  = { .r=0xff, .g=0x00, .b=0x00, .a=0x80 };
const struct color gfxutil_grey    = { .r=0x80, .g=0x80, .b=0x80, .a=0x80 };
   
const sgHSV        gfxutil_defhsv  = { .h=0x00, .s=0xff, .v=0x80, .a=0x80 };

/* -------------------------------------------------------------------------- *
 * Zieht ganzzahlige Quadratwurzel, nur mithilfe von Integeradditionen        *
 * und Shifts                                                                 *
 * -------------------------------------------------------------------------- */
inline uint32_t gfxutil_isqrt(uint32_t x)
{
  uint32_t temp;
  uint32_t g = 0;
  uint32_t b = 0x8000;
  uint8_t bshft = 15;

  do
  {
    if(x >= (temp = (((g << 1) + b) << bshft--)))
    {
      g += b;
      x -= temp;
    }
  }
  while((b >>= 1));

  return g;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct color gfxutil_getpixel(SDL_Surface *sf, int16_t x, int16_t y)
{
  Uint32 value = 0;
  struct color color;
  
  if(x >= 0 && x < sf->w && y >= 0 && y < sf->h)
  {
    Uint8 *row;
    
    row = sf->pixels;
    row += sf->pitch * y;  
    
    switch(sf->format->BytesPerPixel)
    {
      case 1:
      {
        value = row[x];
      }
      case 2:
      {
        Uint16 *pixel = (Uint16 *)row;
        value = pixel[x];      
      }
      case 4:
      {
        Uint32 *pixel = (Uint32 *)row;
        value = pixel[x];
      }
    }
  }
  
  SDL_GetRGBA(value, sf->format, &color.r, &color.g, &color.b, &color.a);
  return color;
}

/* -------------------------------------------------------------------------- *
 * Skaliert den Wert 'now' innerhalb des Bereichs von 'start' bis 'end' auf   *
 * einen Integer-Wert von 0..255                                              *
 * -------------------------------------------------------------------------- */
uint8_t gfxutil_scale8(struct range *r)
{
  uint32_t n = r->end - r->start;
  uint32_t v = r->now - r->start;

  if(n)
    return (v * 255 / n);

  return 255;
}

/* -------------------------------------------------------------------------- *
 * Skaliert den Wert 'now' innerhalb des Bereichs von 'start' bis 'end' auf   *
 * einen Integer-Wert von 0..65535                                            *
 * -------------------------------------------------------------------------- */
uint16_t gfxutil_scale16(struct range *r)
{
  uint32_t n = r->end - r->start;
  uint32_t v = r->now - r->start;

  if(n)
    return (v * 65535 / n);

  return 65535;
}

/* -------------------------------------------------------------------------- *
 * Addiert einen Wert zu der Skala, gibt 1 zurück wenn das Ende erreicht ist  *
 * -------------------------------------------------------------------------- */
int gfxutil_scaleadd(struct range *rp, uint32_t t)
{
  rp->now += t;

  if(rp->now >= rp->end)
  {
    rp->now = rp->end;
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Skalawert, gibt 1 zurück wenn das Ende erreicht ist              *
 * -------------------------------------------------------------------------- */
int gfxutil_scaleset(struct range *rp, uint32_t t)
{
  rp->now = t;

  if(rp->now >= rp->end)
  {
    rp->now = rp->end;
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Skalawert auf eine Zeitspanne                                    *
 * -------------------------------------------------------------------------- */
int gfxutil_scaletime(struct range *rp, uint32_t t, uint32_t d)
{
  rp->start = rp->now = t;
  rp->end = t + d;

  if(rp->now >= rp->end)
  {
    rp->now = rp->end;
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Addiert zwei Positionen und gibt das Resultat zurück                       *
 * -------------------------------------------------------------------------- */
/*struct position gfxutil_posadd(struct position a,
                               struct position b)
{
  struct position c;

  c.x = a.x + b.x;
  c.y = a.y + b.y;

  return c;
}*/

/* -------------------------------------------------------------------------- *
 * Gibt Differenz zwischen zwei Positionen zurück                             *
 * -------------------------------------------------------------------------- */
/*struct position gfxutil_possub(struct position a,
                               struct position b)
{
  struct position c;

  c.x = a.x - b.x;
  c.y = a.y - b.y;

  return c;
}*/

/* -------------------------------------------------------------------------- *
 * Bereitet 'move' für eine Verschiebung nach 'end' vor und gibt die          *
 * relative Position zurück                                                   *
 * -------------------------------------------------------------------------- */
/*struct position gfxutil_moveto(struct move     *move,
                               struct position  end)
{
  move->end = end;

  return gfxutil_possub(end, move->now);
}*/
 
/* -------------------------------------------------------------------------- *
 * Lineare Translation von Position A nach Position B in 65535 Schritten (s)  *
 * -------------------------------------------------------------------------- */
/*void gfxutil_translate(struct position *p, struct position a, struct position b,
                       uint16_t s)
{
  p->x = ((int32_t)a.x * (65535 - s) + (int32_t)b.x * s) / 65535;
  p->y = ((int32_t)a.y * (65535 - s) + (int32_t)b.y * s) / 65535;
}*/

/* -------------------------------------------------------------------------- *
 * Transformationsparameter A nach B in 65535 Schritten (s)                   *
 * -------------------------------------------------------------------------- */
/*void gfxutil_transform(struct transform *t,
                       struct transform  a,
                       struct transform  b,
                       uint16_t          s)
{
  t->r = (a.r * (float)(65535 - s) + b.r * (float)s) / 65535.0;
  t->z = (a.z * (float)(65535 - s) + b.z * (float)s) / 65535.0;
  t->a = (a.a * (float)(65535 - s) + b.a * (float)s) / 65535.0;
}*/

/* -------------------------------------------------------------------------- *
 * Überblendet stufenweise von Farbe A nach Farbe B und speichert das         *
 * Resultat in Farbe C                                                        *
 * -------------------------------------------------------------------------- */
/*void gfxutil_blend(struct color *c, struct color a, struct color b, 
                   uint16_t s)
{
  c->r = ((uint32_t)a.r * (65535 - s) + (uint32_t)b.r * s) / 65535;
  c->g = ((uint32_t)a.g * (65535 - s) + (uint32_t)b.g * s) / 65535;
  c->b = ((uint32_t)a.b * (65535 - s) + (uint32_t)b.b * s) / 65535;
  c->a = ((uint32_t)a.a * (65535 - s) + (uint32_t)b.a * s) / 65535;
}*/

/* -------------------------------------------------------------------------- *
 * Zentriert Rechteck a innerhalb von Reckteck b                              *
 * -------------------------------------------------------------------------- */
/*inline void gfxutil_centerrect(SDL_Rect *a, const SDL_Rect *b)
{
  a->x = b->x + ((b->w - a->w) >> 1);
  a->y = b->y + ((b->h - a->h) >> 1);
}*/

/* -------------------------------------------------------------------------- *
 * Zentriert Rechteck auf die X/Y Koordinaten                                 *
 * -------------------------------------------------------------------------- */
/*inline void gfxutil_centerxy(SDL_Rect *rect, Sint16 x, Sint16 y)
{
  rect->x = x - (rect->w >> 1);
  rect->y = y - (rect->h >> 1);
}*/

/* -------------------------------------------------------------------------- *
 * Liefert 1 wenn Rechteck a und b sich überlappen, andernfalls 0             *
 * -------------------------------------------------------------------------- */
/*inline int gfxutil_overlap(const SDL_Rect *a, const SDL_Rect *b)
{
  int ax2, ay2, bx2, by2;
  
  ax2 = a->x + a->w;
  ay2 = a->y + a->h;
  bx2 = b->x + b->w;
  by2 = b->y + b->h;
  
  if(((a->x >= b->x && a->x < bx2) || (ax2 >= b->x && ax2 < bx2)) &&
     ((a->y >= b->y && a->y < by2) || (ay2 >= b->y && ay2 < by2)))
    return 1;
  
  return 0;
}*/

/* -------------------------------------------------------------------------- *
 * Gibt 1 zurück wenn x und y innerhalb des Rechtecks sind                    *
 * -------------------------------------------------------------------------- */
/*int gfxutil_matchrect(const SDL_Rect *rect, Sint16 xpos, Sint16 ypos)
{
  return (!((xpos) < rect->x || (xpos) > (rect->x + rect->w - 1) || (ypos) < rect->y || (ypos) > (rect->y + rect->h - 1)));
}*/

/* -------------------------------------------------------------------------- *
 * Verkleinert ein Rechteck so dass es in das Surface passt                   *
 * -------------------------------------------------------------------------- */
void gfxutil_clip(SDL_Surface *sf, SDL_Rect *rect)
{
  if(rect->x < 0)
  {
    rect->w -= -rect->x;
    rect->x = 0;
  }
  
  if(rect->x >= sf->w)
  {
    rect->w = 0;
    rect->x = sf->w;
  }
  
  if(rect->y < 0)
  {
    rect->h -= -rect->y;
    rect->y = 0;
  }
  
  if(rect->y >= sf->h)
  {
    rect->h = 0;
    rect->y = sf->h;
  }
  
  if(rect->x + rect->w >= sf->w)
    rect->w = sf->w - rect->x;

  if(rect->y + rect->h >= sf->h)
    rect->h = sf->h - rect->y;
}

/* -------------------------------------------------------------------------- *
 * Passt die Rechtecke fürs Surface-Kopieren oder Blitten an indem es Quell-  *
 * und Zielrechteck nötigenfalls auf den grössten gemeinsamen Bereich ver-    *
 * kleinert                                                                   *
 * -------------------------------------------------------------------------- */
void gfxutil_blitclip(SDL_Surface *src, SDL_Rect *srect, 
                      SDL_Surface *dst, SDL_Rect *drect)
{
  int srcx, srcy, w, h;  
  int maxw, maxh;
  SDL_Rect *clip = &dst->clip_rect;
  int dx, dy;
  
  /* source auf der x-achse clippen */
  srcx = srect->x;
  w = srect->w;
  
  /* source rechteck beginnt ausserhalb des bildes, 
     passe x und breite an */
  if(srcx < 0)
  {
    w += srcx;
    drect->x -= srcx;
    srcx = 0;
  }
  
  maxw = src->w - srcx;
  
  if(maxw < w)
    w = maxw;
  
  /* source auf der y-achse clippen */
  srcy = srect->y;
  h = srect->h;
  
  /* source rechteck beginnt ausserhalb des bildes, 
     passe y und höhe an */
  if(srcy < 0)
  {
    h += srcy;
    drect->y -= srcy;
    srcy = 0;
  }
  
  maxh = src->h - srcy;
  
  if(maxh < h)
    h = maxh;
  
  /* destination auf der x-achse clippen */
  dx = clip->x - drect->x;
  
  if(dx > 0)
  {
    w -= dx;
    drect->x += dx;
    srcx += dx;
  }
  
  dx = drect->x + w - clip->x - clip->w;
  
  if(dx > 0)
    w -= dx;
  
  /* destination auf der y-achse clippen */
  dy = clip->y - drect->y;
  
  if(dy > 0)
  {
    h -= dy;
    drect->y += dy;
    srcy += dy;
  }
  
  dy = drect->y + h - clip->y - clip->h;
  
  if(dy > 0)
    h -= dy;
  
  if(h > 0 && w > 0)
  {
    srect->x = srcx;
    srect->y = srcy;
    srect->w = drect->w = w;
    srect->h = drect->h = h;
  }
  else
  {
    srect->x = drect->x = 0;
    srect->y = drect->y = 0;
    srect->w = drect->w = 0;
    srect->h = drect->h = 0;
  }
}

/* -------------------------------------------------------------------------- *
 * Berechnet aus 2 Rechtecken die Überschneidung, lässt allerdings x und y    *
 * unverändert                                                                *
 * -------------------------------------------------------------------------- */
void gfxutil_intersect(SDL_Rect *a, SDL_Rect *b)
{
  if(a->w > b->w)
    a->w = b->w;
  else
    b->w = a->w;

  if(a->h > b->h)
    a->h = b->h;
  else
    b->h = a->h;
}

/* -------------------------------------------------------------------------- *
 * Kopiert ein Surface, der Alpha Layer von src wird mitkopiert.              *
 * -------------------------------------------------------------------------- */
void gfxutil_copy(SDL_Surface *src, SDL_Rect *sr,
                  SDL_Surface *dst, SDL_Rect *dr)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *drow;
  Uint16 y;
    
  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
  
  gfxutil_blitclip(src, &srect, dst, &drect);
  gfxutil_intersect(&srect, &drect);
  
  if(!drect.w || !drect.h || !srect.w || !srect.h)
    return;
  
  SDL_LockSurface(src);
  SDL_LockSurface(dst);
  
  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;

  for(y = 0; y < srect.h; y++)
  {
    memcpy(drow, srow, srect.w * src->format->BytesPerPixel);

    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/* -------------------------------------------------------------------------- *
 * Kopieren mit Alpha und Farbtönung                                          *
 * -------------------------------------------------------------------------- */
void gfxutil_tint(SDL_Surface *src, SDL_Rect *sr,
                  SDL_Surface *dst, SDL_Rect *dr, 
                  struct color tint)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *drow;
  Uint16 y;
  Uint16 x;
    
  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
  
  gfxutil_blitclip(src, &srect, dst, &drect);
  gfxutil_intersect(&srect, &drect);
  
  if(!drect.w || !drect.h || !srect.w || !srect.h)
    return;
  
  SDL_LockSurface(src);
  SDL_LockSurface(dst);
  
  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;

  for(y = 0; y < srect.h; y++)
  {
    for(x = 0; x < srect.w; x++)
    {
      uint32_t salpha = ((srow[x] & AMASK) >> ASHIFT); /* quell alpha */
      uint32_t talpha;                                 /* tönungs alpha */
      uint32_t r; /* finaler rot-wert */
      uint32_t g; /* finaler grün-wert */
      uint32_t b; /* finaler blau-wert */
      uint32_t a; /* finaler alpha-wert */
    
      /* pixel aus dem quellbild laden */
      r = ((srow[x] & RMASK) >> RSHIFT);
      g = ((srow[x] & GMASK) >> GSHIFT);
      b = ((srow[x] & BMASK) >> BSHIFT);

      /* alpha-wert für die tönung entspricht der differenz vom
         source alpha-wert zu 255 mal 2, d.h. was im source
         image halbtransparent ist wird getönt, und je weniger
         transparenz desto weniger tönung (damit nur der
         weisse bereich der karten eingefärbt wird) */
      talpha = (255 - salpha) << 1;
      
      if(talpha > 255)
        talpha = 255;

      /* Wenn pixel nicht schwarz (rand) dann volles alpha für das
         Quellpixel setzen, andernfalls */
      if(r && g && b)
        salpha = 255;
      else
        talpha = 0;
      
      a = (tint.a * talpha + salpha * (255 - talpha)) >> 8;

/*      if(talpha)
      {
        r = (r * (255 - talpha)) + ((r * tint.r * talpha) >> 8);
        g = (g * (255 - talpha)) + ((g * tint.g * talpha) >> 8);
        b = (b * (255 - talpha)) + ((b * tint.b * talpha) >> 8);
        r >>= 8;
        g >>= 8;
        b >>= 8;
      }*/
      
      /* Wohl nicht schneller, aber korrekter */
        if(talpha)
      {
        r = (r * (255 - talpha)) + ((r * tint.r * talpha) / 255);
        g = (g * (255 - talpha)) + ((g * tint.g * talpha) / 255);
        b = (b * (255 - talpha)) + ((b * tint.b * talpha) / 255);
        r /= 255;
        g /= 255;
        b /= 255;
      }
      
      drow[x] = (a << ASHIFT) | (r << RSHIFT) | (g << GSHIFT) | (b << BSHIFT);
    }

    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/* -------------------------------------------------------------------------- *
 * Blitten mit Weichzeichner                                                  *
 * -------------------------------------------------------------------------- */
void gfxutil_blur(SDL_Surface *src, SDL_Rect *sr, 
                  SDL_Surface *dst, SDL_Rect *dr, 
                  struct color tint, uint8_t rate)
{
#define GET(x, m, s) (uint8_t)(((x) & (m)) >> (s))
#define SET(x, m, s) (uint32_t)(((x) << (s)) & (m))
#define CLAMP(v)     (uint8_t)((v) > 255 ? 255 : (v))
  
#define PUT(c, v, d, m, s) \
    SET(CLAMP(GET(c, m, s) + (((d - GET(c, m, s)) * v) / 255)), m, s)

#define PUTRGBA(c, v) \
  cp = &(c); \
  if(cp >= start && cp < end) \
    { *cp = PUT(*cp, v, tint.r, RMASK, RSHIFT) | \
            PUT(*cp, v, tint.g, GMASK, GSHIFT) | \
            PUT(*cp, v, tint.b, BMASK, BSHIFT) | \
            PUT(*cp, v, tint.a, AMASK, ASHIFT); }

  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *drow;
  Uint16 x;
  Uint16 y;
  Uint32 *start;
  Uint32 *end;
  Uint32 *cp;

  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
  
  gfxutil_blitclip(src, &srect, dst, &drect);
  gfxutil_intersect(&srect, &drect);
  
  if(!drect.w || !drect.h || !srect.w || !srect.h)
    return;
  
  SDL_LockSurface(src);
  SDL_LockSurface(dst);
  
  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;

  start = dst->pixels;
  end = (Uint32 *)dst->pixels + ((dst->h * dst->pitch) >> 2);

  for(y = 0; y < srect.h && y < drect.h; y++)
  {
    for(x = 0; x < srect.w && x < drect.w; x++)
    {
      int sa = (srow[x] & AMASK) >> ASHIFT;
      int da;
      
      da = gfxutil_clamp(rate, 192, 255) - 192;
      da = sa * da / (256 - 192);
      
      PUTRGBA(drow[x - (dst->pitch >> 2) * 2 - 1], da);
      PUTRGBA(drow[x - (dst->pitch >> 2) * 2 + 1], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) * 2 - 1], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) * 2 + 1], da);      
      PUTRGBA(drow[x - (dst->pitch >> 2) - 2], da);
      PUTRGBA(drow[x - (dst->pitch >> 2) + 2], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) - 2], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) + 2], da);      
      
      da = gfxutil_clamp(rate, 128, 192) - 128;
      da = sa * da / (192 - 128);
      
      PUTRGBA(drow[x - 2], da);
      PUTRGBA(drow[x + 2], da);
      PUTRGBA(drow[x - (dst->pitch >> 2) * 2], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) * 2], da);
      
      da = gfxutil_clamp(rate, 64, 128) - 64;
      da = sa * da / (128 - 64);
      
      PUTRGBA(drow[x - (dst->pitch >> 2) - 1], da);
      PUTRGBA(drow[x - (dst->pitch >> 2) + 1], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) - 1], da);
      PUTRGBA(drow[x + (dst->pitch >> 2) + 1], da);

      da = gfxutil_clamp(rate, 0, 64);
      da = sa * da / 64;
      
      PUTRGBA(drow[x - 1], da);
      PUTRGBA(drow[x + 1], da);
      PUTRGBA(drow[x - (dst->pitch >> 2)], da);
      PUTRGBA(drow[x + (dst->pitch >> 2)], da);

      PUTRGBA(drow[x], sa);
    }
    
    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/* -------------------------------------------------------------------------- *
 * Additives Blitten mit Transparenz und Farbtönung (RGBA -> RGBA)            *
 * -------------------------------------------------------------------------- */
void gfxutil_blit(SDL_Surface *src, SDL_Rect *sr,
                  SDL_Surface *dst, SDL_Rect *dr,
                  int alpha, struct color *hue)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *drow;
  Uint16 x;
  Uint16 y;

  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);

  gfxutil_blitclip(src, &srect, dst, &drect);  
  
  if(!drect.w || !drect.h || !srect.w || !srect.h)
    return;
  
  SDL_LockSurface(src);
  SDL_LockSurface(dst);

  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;

  for(y = 0; y < srect.h; y++)
  {
    for(x = 0; x < srect.w; x++)
    {
      uint32_t dalpha = ((drow[x] & AMASK) >> ASHIFT); /* ziel alpha */
      uint32_t salpha = ((srow[x] & AMASK) >> ASHIFT); /* quell alpha */
      uint32_t talpha;                                 /* tönungs alpha */
      uint32_t balpha;                                 /* farbmisch alpha */
      uint32_t r; /* finaler rot-wert */
      uint32_t g; /* finaler grün-wert */
      uint32_t b; /* finaler blau-wert */
      uint32_t a; /* finaler alpha-wert */

      /* weder quell noch ziel bild haben ein nicht-transparentes pixel
         oder quelle hat ein transparentes pixel */
      if(!(salpha | dalpha) || !salpha)
        continue;

      /* pixel aus dem quellbild laden */
      r = ((srow[x] & RMASK) >> RSHIFT);
      g = ((srow[x] & GMASK) >> GSHIFT);
      b = ((srow[x] & BMASK) >> BSHIFT);

      /* alpha-wert für die tönung entspricht der differenz vom
         source alpha-wert zu 255 mal 2, d.h. was im source
         image halbtransparent ist wird getönt, und je weniger
         transparenz desto weniger tönung (damit nur der
         weisse bereich der karten eingefärbt wird) */
      talpha = (255 - salpha) << 1;
      
      if(talpha > 255)
        talpha = 255;

      /* Wenn pixel nicht schwarz (rand) dann volles alpha setzen */
      if(r && g && b)
      {
        salpha = 255;
      }
      else
      {
        talpha = 0;
      }
      
      salpha = (alpha * talpha + salpha * (255 - talpha)) >> 8;

      balpha = salpha;

      balpha += ((255 - balpha) * (255 - dalpha)) >> 8;
      
      /* Der neue Alphawert kann sich von dalpha bis 255 bewegen */
      a = dalpha;
      a += ((255 - a) * (salpha)) / 255;

      r *= balpha;
      g *= balpha;
      b *= balpha;

      if(hue && talpha)
      {
        r >>= 8;
        g >>= 8;
        b >>= 8;
        r = (r * (255 - talpha)) + ((r * hue->r * talpha) >> 8);
        g = (g * (255 - talpha)) + ((g * hue->g * talpha) >> 8);
        b = (b * (255 - talpha)) + ((b * hue->b * talpha) >> 8);
      }
      
      /* nice hack :) */
/*      if(r && g && b && !dalpha)
      {
        r += 255 * (255 - salpha);
        g += 255 * (255 - salpha);
        b += 255 * (255 - salpha);
      }
      else*/
          
      {
        r += ((drow[x] & RMASK) >> RSHIFT) * (255 - balpha);
        g += ((drow[x] & GMASK) >> GSHIFT) * (255 - balpha);
        b += ((drow[x] & BMASK) >> BSHIFT) * (255 - balpha);
      }
      
      r >>= 8;
      g >>= 8;
      b >>= 8;
      
      drow[x] = ((a << ASHIFT)) |
                ((r << RSHIFT)) |
                ((g << GSHIFT)) |
                ((b << BSHIFT));
    }
    
    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/* -------------------------------------------------------------------------- *
 * 32-bit Rotozoomer, basierend auf SDL_rotozoom.c - LGPL (c) A. Schiffler    *
 *                                                                            *
 * Reduziert auf das Nötigste und mit Alpha-Kanal Manipulation                *
 * -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- *
 * Rechnet benötigte Grösse der Zielsurface vor einer Transformation aus      *
 * -------------------------------------------------------------------------- */
void gfxutil_sizetrig(uint16_t sw, uint16_t sh, uint16_t *dw, uint16_t *dh,
                      float a, float z, float *sz, float *cz)
{
  float x, y, cx, cy, sx, sy;
  int dwh, dhh;
        
  /* Zielrechteck ausrechnen indem wir Quellrechteck um sein Zentrum 
     transformieren */
  *sz = -sin(a * M_PI / 180.0);
  *cz = cos(a * M_PI / 180.0);
  *sz *= z;
  *cz *= z;
  
  x = sw >> 1;
  y = sh >> 1;
  
  cx = *cz * x;
  cy = *cz * y;
  sx = *sz * x;
  sy = *sz * y;
  
  dwh = gfxutil_max((int)ceil(gfxutil_max
                              (gfxutil_max
                               (gfxutil_max
                                (fabs(cx + sy),
                                 fabs(cx - sy)),
                                fabs(-cx + sy)), 
                               fabs(-cx - sy))), 1);
  
  dhh = gfxutil_max((int)ceil(gfxutil_max
                              (gfxutil_max
                               (gfxutil_max
                                (fabs(sx + cy), 
                                 fabs(sx - cy)), 
                                fabs(-sx + cy)), 
                               fabs(-sx - cy))), 1);
  *dw = dwh << 1;
  *dh = dhh << 1;
}

/* -------------------------------------------------------------------------- *
 * Führt 32-bit Pixeltransformationen durch                                   *
 * -------------------------------------------------------------------------- */
void gfxutil_tf32(SDL_Surface *src, SDL_Surface *dst, int cx, int cy,
                  int isin, int icos, uint8_t alpha)
{
  int x, y, t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
  struct color c0, c1, c2, c3;
  struct color *pc, *sp;
  int gap;
  
  /* Initialisation */
  xd = ((src->w - dst->w) << 15);
  yd = ((src->h - dst->h) << 15);
  ax = (cx << 16) - (icos * cx);
  ay = (cy << 16) - (isin * cx);
  sw = src->w - 1;
  sh = src->h - 1;
  pc = dst->pixels;
  gap = dst->pitch - dst->w * 4;
  
  for(y = 0; y < dst->h; y++)
  {
    dy = cy - y;
    sdx = (ax + (isin * dy)) + xd;
    sdy = (ay - (icos * dy)) + yd;
    
    for(x = 0; x < dst->w; x++)
    {
      dx = (sdx >> 16);
      dy = (sdy >> 16);
      
      if((dx >= -1) && (dy >= -1) && (dx < src->w) && (dy < src->h))
      {
        if((dx >= 0) && (dy >= 0) && (dx < sw) && (dy < sh))
        {
          sp = (struct color *)((uint8_t *)src->pixels + src->pitch * dy);
          sp += dx;
          c0 = *sp;
          sp += 1;
          c1 = *sp;
          sp = (struct color *)((uint8_t *)sp + src->pitch);
          sp -= 1;
          c2 = *sp;
          sp += 1;
          c3 = *sp;
        }
        else if((dx == sw) && (dy == sh))
        {
          sp = (struct color *)((uint8_t *)src->pixels + src->pitch * dy);
          sp += dx;
          c0 = *sp;
          c1 = *sp;
          c2 = *sp;
          c3 = *sp;
        }
        else if((dx == -1) && (dy == -1))
        {
          sp = (struct color *)(src->pixels);
          c0 = *sp;
          c1 = *sp;
          c2 = *sp;
          c3 = *sp;
        }
        else if((dx == -1) && (dy == sh))
        {
          sp = (struct color *)(src->pixels);
          sp = (struct color *)((uint8_t *)src->pixels + src->pitch * dy);
          c0 = *sp;
          c1 = *sp;
          c2 = *sp;
          c3 = *sp;
        }
        else if((dx == sw) && (dy == -1))
        {          
          sp = (struct color *)(src->pixels);
          sp += dx;
          c0 = *sp;
          c1 = *sp;
          c2 = *sp;
          c3 = *sp;
        }
        else if (dx == -1)
        {
          sp = (struct color *)((uint8_t *)src->pixels + src->pitch * dy);
          c0 = *sp;
          c1 = *sp;
          c2 = *sp;
          sp = (struct color *)((uint8_t *)sp + src->pitch);
          c3 = *sp;
        }
        else if (dy == -1)
        {
          sp = (struct color *)(src->pixels);
          sp += dx;
          c0 = *sp;
          c1 = *sp;
          c2 = *sp;
          sp += 1;
          c3 = *sp;
        }
        else if (dx == sw)
        {          
          sp = (struct color *)((uint8_t *)src->pixels + src->pitch * dy);
          sp += dx;
          c0 = *sp;
          c1 = *sp;
          sp = (struct color *)((uint8_t *)sp + src->pitch);
          c2 = *sp;
          c3 = *sp;
        }
        else if (dy == sh)
        {
          sp = (struct color *)((uint8_t *)src->pixels + src->pitch * dy);
          sp += dx;
          c0 = *sp;
          sp += 1;
          c1 = *sp;
          c2 = *sp;
          c3 = *sp;
        }
        
        /* Farben interpolieren */
        ex = (sdx & 0xffff);
        ey = (sdy & 0xffff);
        t1 = ((((c1.r - c0.r) * ex) >> 16) + c0.r) & 0xff;
        t2 = ((((c3.r - c2.r) * ex) >> 16) + c2.r) & 0xff;
        pc->r = (((t2 - t1) * ey) >> 16) + t1;
        t1 = ((((c1.g - c0.g) * ex) >> 16) + c0.g) & 0xff;
        t2 = ((((c3.g - c2.g) * ex) >> 16) + c2.g) & 0xff;
        pc->g = (((t2 - t1) * ey) >> 16) + t1;
        t1 = ((((c1.b - c0.b) * ex) >> 16) + c0.b) & 0xff;
        t2 = ((((c3.b - c2.b) * ex) >> 16) + c2.b) & 0xff;
        pc->b = (((t2 - t1) * ey) >> 16) + t1;
        t1 = ((((c1.a - c0.a) * ex) >> 16) + c0.a) & 0xff;
        t2 = ((((c3.a - c2.a) * ex) >> 16) + c2.a) & 0xff;
        pc->a = (((t2 - t1) * ey) >> 16) + t1;
        
        if(alpha != 255)
          pc->a = (((uint32_t)pc->a) * ((uint32_t)alpha)) / 255;
      }
      
      sdx += icos;
      sdy += isin;
      pc++;
    }

    pc = (struct color *)((uint8_t *)pc + gap);
  }
}

/* -------------------------------------------------------------------------- *
 * Transformiert eine Surface und skaliert den Alphakanal                     *
 * -------------------------------------------------------------------------- */
SDL_Surface *gfxutil_rotozoom(SDL_Surface *sf, float a, float z, uint8_t alpha)
{
  uint16_t dw, dh;
  uint16_t dwh, dhh;
  float sz, cz;
  float szi, czi;
  SDL_Surface *df;
  
  float iz;
  
  if(z < 0.001) z = 0.001;
  iz = 65535 / (z * z);
  
  
  /* Zielgrösse ausrechnen */
  gfxutil_sizetrig(sf->w, sf->h, &dw, &dh, a, z, &sz, &cz);
    
  szi = sz * iz;
  czi = cz * iz;

  dwh = dw >> 1;
  dhh = dh >> 1;
  
  df = SDL_CreateRGBSurface(SDL_SWSURFACE, dw, dh, 
                            32, RMASK, GMASK, BMASK, AMASK);
  
  SDL_LockSurface(sf);
  gfxutil_tf32(sf, df, dwh, dhh, (int)szi, (int)czi, alpha);  
  SDL_UnlockSurface(sf);
  
  return df;
}  

/* -------------------------------------------------------------------------- *
 * Berechnet den Farbtönungswert aus Fliesskommazahlen von 0-100% für jede    *
 * Farbe.                                                                     *
 * -------------------------------------------------------------------------- */
struct color gfxutil_tintcolor(float r, float g, float b, float a)
{
  int ir;
  int ig;
  int ib;
  int ia;
  struct color ret;
  
  ir = (int)(r * 255) / 100;
  ig = (int)(g * 255) / 100;
  ib = (int)(b * 255) / 100;
  ia = (int)(a * 255) / 100;
  
  ir = ir > 255 ? 255 : (ir < 0 ? 0 : ir);
  ig = ig > 255 ? 255 : (ig < 0 ? 0 : ig);
  ib = ib > 255 ? 255 : (ib < 0 ? 0 : ib);
  ia = ia > 255 ? 255 : (ia < 0 ? 0 : ia);

  ret.r = (uint8_t)(255 - ig);
  ret.g = (uint8_t)(255 - ir);
  ret.b = (uint8_t)(255 - ib);
  ret.a = (uint8_t)(255 - ia);

#ifdef DEBUG
  gfxutil_dumpcolor(ret);
#endif /* DEBUG */
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Gibt einen Farbwert in Dezimalzahlen aus (0-255 für jeden Kanal)           *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void gfxutil_dumpcolor(struct color c)
{
  gfxutil_debug(INFO, "Farbe [ r = %3u, g = %3u, b = %3u, a = %3u ]",
                c.r, c.g, c.b, c.a);
}
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Konvertiert einen RGBA Wert in einen 6-8 stelligen Hexadezimal-String im   *
 * Format #RRGGBB(AA)                                                         *
 * -------------------------------------------------------------------------- */
char *gfxutil_strcolor(struct color c)
{
  static char color[10];
  static const char hexalphabet[] = "0123456789abcdef";
  int n = 0;
  
  color[n++] = '#';
  color[n++] = hexalphabet[c.r >> 4];
  color[n++] = hexalphabet[c.r & 15];
  color[n++] = hexalphabet[c.g >> 4];
  color[n++] = hexalphabet[c.g & 15];
  color[n++] = hexalphabet[c.b >> 4];
  color[n++] = hexalphabet[c.b & 15];
  
  if(c.a < 255)
  {
    color[n++] = hexalphabet[c.a >> 4];
    color[n++] = hexalphabet[c.a & 15];
  }
  
  color[n] = '\0';
  
  return color;
}

/* -------------------------------------------------------------------------- *
 * Konvertiert einen RGBA Wert in einen 6-8 stelligen Hexadezimal-String im   *
 * Format %HHSSVV(AA)                                                         *
 * -------------------------------------------------------------------------- */
char *gfxutil_strhsv(sgHSV hsv)
{
  static char color[10];
  static const char hexalphabet[] = "0123456789abcdef";
  int n = 0;
  
  color[n++] = '%';
  color[n++] = hexalphabet[hsv.h >> 4];
  color[n++] = hexalphabet[hsv.h & 15];
  color[n++] = hexalphabet[hsv.s >> 4];
  color[n++] = hexalphabet[hsv.s & 15];
  color[n++] = hexalphabet[hsv.v >> 4];
  color[n++] = hexalphabet[hsv.v & 15];
  
  if(hsv.a < 255)
  {
    color[n++] = hexalphabet[hsv.a >> 4];
    color[n++] = hexalphabet[hsv.a & 15];
  }
  
  color[n] = '\0';
  
  return color;
}

/* -------------------------------------------------------------------------- *
 * Parst einen 6-8 stelligen Hexadezimalen Farbwert zurück in den binären     *
 * Farbwert.                                                                  *
 * -------------------------------------------------------------------------- */
int gfxutil_parsecolor(struct color *c, const char *str)
{
  uint32_t r = 0, g = 0, b = 0, a = 0xff;
  int ret;
  
  ret = sscanf(str, "#%02x%02x%02x%02x", &r, &g, &b, &a);
  
  c->r = r;
  c->g = g;
  c->b = b;
  c->a = a;
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Parst einen 6-8 stelligen Hexadezimalen Farbwert zurück in den binären     *
 * Farbwert.                                                                  *
 * -------------------------------------------------------------------------- */
int gfxutil_parsehsv(sgHSV *hsv, const char *str)
{
  uint32_t h = 0, s = 0, v = 0, a = 0xff;
  int ret;
  
  ret = sscanf(str, "%%%02x%02x%02x%02x", &h, &s, &v, &a);
  
  hsv->h = h;
  hsv->s = s;
  hsv->v = v;
  hsv->a = a;
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Konvertiert vom RGB in das HSV Farbmodell                                  *
 * -------------------------------------------------------------------------- */
sgHSV gfxutil_rgb2hsv(struct color color)
{
  sgColor rgb;
  
  rgb.r = color.r;
  rgb.g = color.g;
  rgb.b = color.b;
  
  return sgRGBToHSV(rgb);
}

/* -------------------------------------------------------------------------- *
 * Konvertiert vom RGB in das HSV Farbmodell                                  *
 * -------------------------------------------------------------------------- */
struct color gfxutil_hsv2rgb(sgHSV hsv)
{
  sgColor rgb;
  struct color color;
  
  rgb = sgHSVToRGB(hsv);
  
  color.r = rgb.r;
  color.g = rgb.g;
  color.b = rgb.b;
  color.a = hsv.a;
  
  return color;
}

/* -------------------------------------------------------------------------- *
 * Gibt die Dimensionen eines Rechtecks aus                                   *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void gfxutil_dumprect(SDL_Rect *rect)
{
  gfxutil_debug(INFO, "Rechteck: x = %i, y = %i, w = %u, h = %u",
                rect->x, rect->y, 
                rect->w, rect->h);
}
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Füllt das Zielsurface mit einem "minimalen" Muster aus den Farben S und U. *
 *                                                                            *
 * Der Vorschlag für dieses Muster stammt von Stephan Urech,                  *
 * vielen Dank Steph!                                                         *
 * -------------------------------------------------------------------------- */
void gfxutil_patternfill(SDL_Surface *sf, SDL_Rect *rect,
                         struct color s, struct color u)
{
  Uint32 cs, cu;
  int x, y;
  int x2, y2;
  Uint32 *pixels;
  
  /* Rechteck clippen oder ganzes Surface füllen wenn keins angegeben */
  if(rect)
    gfxutil_clip(sf, rect);
  else
    rect = &sf->clip_rect;
  
  /* Farbwerte mappen */
  cs = SDL_MapRGBA(sf->format, s.r, s.g, s.b, 0xff);
  cu = SDL_MapRGBA(sf->format, u.r, u.g, u.b, 0xff);
  
  /* Surface locken für direkten Zugriff */
  SDL_LockSurface(sf);
  
  /* Initialer Pointer */
  pixels = (Uint32 *)sf->pixels;
  pixels += rect->y * sf->pitch;

  /* Endposition */
  x2 = rect->x + rect->w;
  y2 = rect->y + rect->h;
  
  for(y = rect->y; y < y2; y++)
  {
    for(x = rect->x; x < x2; x++)
    {
      /* Wenn x + y ein Vielfaches von 8 oder 0 ist, dann
         setzen wir die Farbe S, andernfalls Farbe U, womit
         wir folgendes Muster erhalten, welches beliebig
         wiederholt werden kann:
       
         U S S S S S S S
         S S S S S S S U
         S S S S S S U S
         S S S S S U S S
         S S S S U S S S
         S S S U S S S S
         S S U S S S S S
         S U S S S S S S */       
      if(((x + y) & 0x03) == 0)
        pixels[x] = cs;
      else
        pixels[x] = cu;      
    }
    
    pixels += sf->pitch >> 2;
  }

  SDL_UnlockSurface(sf);
}

