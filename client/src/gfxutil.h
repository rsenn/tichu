/* $Id: gfxutil.h,v 1.40 2005/05/21 08:27:20 smoli Exp $
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

#ifndef GFXUTIL_H
#define GFXUTIL_H

#include <stdint.h>
#include <SDL.h>
#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define gfxutil_log(l, s...) writelog(MOD_GFXUTIL, (l), s)

#ifdef DEBUG_GFXUTIL
#define gfxutil_debug(l, s...) gfxutil_log((l), s)
#else
#define gfxutil_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten                                                                 *
 * -------------------------------------------------------------------------- */
#define AMASK  0xff000000
#define ASHIFT 24
#define RMASK  0x00ff0000
#define RSHIFT 16
#define GMASK  0x0000ff00
#define GSHIFT 8
#define BMASK  0x000000ff
#define BSHIFT 0

/* -------------------------------------------------------------------------- *
 * Nützliche Makros                                                           *
 * -------------------------------------------------------------------------- */

struct color gfxutil_getpixel(SDL_Surface *sf, int16_t x, int16_t y);
  

/* Pixel aus einer 32-Bit Surface indexieren 
 * -------------------------------------------------------------------------- */
#define gfxutil_getpixel32(sf, x, y) (((Uint32 *)(sf)->pixels)[(y) * \
                                                ((sf)->pitch >> 2) + (x)])

/* Pixel aus einer 8-Bit Surface indexieren 
 * -------------------------------------------------------------------------- */
#define gfxutil_getpixel8(sf, x, y) (((Uint8  *)(sf)->pixels)[(y) * \
                                                (sf)->pitch + (x)])

/* Von 2 Werten den kleineren/grösseren
 * -------------------------------------------------------------------------- */
#define gfxutil_min(a, b) (((a) < (b)) ? (a) : (b))
#define gfxutil_max(a, b) (((a) > (b)) ? (a) : (b))
  
/* Von 3 Werten den kleinsten/grössten zurückgeben
 * -------------------------------------------------------------------------- */
#define gfxutil_smallest(a, b, c) ((a) < (b) ? \
                                  ((a) < (c) ? (a) : (c)) : \
                                  ((b) < (c) ? (b) : (c))) 

#define gfxutil_biggest(a, b, c)  ((a) > (b) ? \
                                  ((a) > (c) ? (a) : (c)) : \
                                  ((b) > (c) ? (b) : (c)))


/* Gibt 'value' zurück wenn der Wert im Bereich von 'min' bis 'max' liegt,
 * andernfalls 'min' wenn er kleiner ist, 'max' wenn er grösser ist' 
 * -------------------------------------------------------------------------- */
#define gfxutil_clamp(value, min, max) ((value) > (min) ? \
                                       ((value) < (max) ? \
                                        (value) : (max)) : (min))

/* Skaliert Wert 'value' der im Bereich von 0 bis 
 * 'from' liegt auf einen Wert im Bereich 0 bis 'to'
 * -------------------------------------------------------------------------- */
#define gfxutil_scale(value, from, to) ((value) * (to) / (from))

/* Vektorstruktur auf die Standardwerte initialisieren 
 * -------------------------------------------------------------------------- */
#define gfxutil_vinit(vector) (vector)->x = 0; \
                              (vector)->y = 0; \
                              (vector)->z = 1.0; \
                              (vector)->a = 0.0; \
                              (vector)->t = 0;

/* -------------------------------------------------------------------------- *
 * Vergleicht 2 Farbwerte                                                     *
 * -------------------------------------------------------------------------- */
#define gfxutil_diffcolor(x, y) ((x).r != (y).r || (x).g != (y).g || (x).b != (y).b)

/* -------------------------------------------------------------------------- *
 * Vergleicht 2 Positionen                                                    *
 * -------------------------------------------------------------------------- */
#define gfxutil_diffposition(a, b) ((a).x != (b).x || (a).y != (b).y)
  
/* -------------------------------------------------------------------------- *
 * Vergleicht 2 Transformationen                                              *
 * -------------------------------------------------------------------------- */
#define gfxutil_difftransform(x, y) ((x).r != (y).r || (x).z != (y).z || ((int)(x).a * 255 != (int)(y).a * 255))  

/* Blending: Farbtönungswert
 * -------------------------------------------------------------------------- */
struct color
{
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
};

/* Translation: Position 
 * -------------------------------------------------------------------------- */
struct position
{
  int16_t x;
  int16_t y;
};

/* Transformation: Rotation, Zoom und Alpha 
 * -------------------------------------------------------------------------- */
struct transform
{
  float   r;     /* Rotationswinkel */
  float   z;     /* Zoomfaktor */
  float   a;     /* Alpha-Blending */
};

/* Ranged Scaling: Start, Position und Ende
 * -------------------------------------------------------------------------- */
struct range
{
  uint32_t start;   /* Unteres Ende der Skala */
  uint32_t now;     /* start <= now <= end  */
  uint32_t end;     /* Oberes Ende der Skala */
};

/* -------------------------------------------------------------------------- */

struct move
{
  struct position start;
  struct position now;
  struct position end;
};

struct rotozoom
{
  struct transform start;
  struct transform now;
  struct transform end;
};

struct blend
{
  struct color start;
  struct color now;
  struct color end;
};

/* -------------------------------------------------------------------------- *
 * Ein paar Farbkonstanten                                                    *
 * -------------------------------------------------------------------------- */
extern const struct color gfxutil_white;
extern const struct color gfxutil_white0;   /* Transparentes Weiss */
extern const struct color gfxutil_black;
extern const struct color gfxutil_red;
extern const struct color gfxutil_red128;   /* Halbtransparentes Rot */
extern const struct color gfxutil_grey;

extern const sgHSV        gfxutil_defhsv;

/* -------------------------------------------------------------------------- *
 * Zieht ganzzahlige Quadratwurzel, nur mit Hilfe von Integeradditionen       *
 * und Shifts                                                                 *
 * -------------------------------------------------------------------------- */
extern inline uint32_t gfxutil_isqrt(uint32_t x)
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
 * Kalkuliert eine Distanz aus x- und y-Abweichung                            *
 * -------------------------------------------------------------------------- */
#define gfxutil_dist(x, y) gfxutil_isqrt(((x) * (x)) + ((y) * (y)))

/* -------------------------------------------------------------------------- *
 * Skaliert den Wert 'now' innerhalb des Bereichs von 'start' bis 'end' auf   *
 * einen Integer-Wert von 0..255                                              *
 * -------------------------------------------------------------------------- */
uint8_t gfxutil_scale8(struct range *r);

/* -------------------------------------------------------------------------- *
 * Skaliert den Wert 'now' innerhalb des Bereichs von 'start' bis 'end' auf   *
 * einen Integer-Wert von 0..65535                                            *
 * -------------------------------------------------------------------------- */
uint16_t gfxutil_scale16(struct range *r);

/* -------------------------------------------------------------------------- *
 * Addiert einen Wert zu der Skala, gibt 1 zurück wenn das Ende erreicht ist  *
 * -------------------------------------------------------------------------- */
int gfxutil_scaleadd(struct range *rp, uint32_t t);

/* -------------------------------------------------------------------------- *
 * Setzt den Skalawert, gibt 1 zurück wenn das Ende erreicht ist              *
 * -------------------------------------------------------------------------- */
int gfxutil_scaleset(struct range *rp, uint32_t t);

/* -------------------------------------------------------------------------- *
 * Setzt den Skalawert auf eine Zeitspanne                                    *
 * -------------------------------------------------------------------------- */
int gfxutil_scaletime(struct range *rp, uint32_t t, uint32_t d);

/* -------------------------------------------------------------------------- *
 * Addiert zwei Positionen und gibt das Resultat zurück                       *
 * -------------------------------------------------------------------------- */
/*extern inline void gfxutil_posadd(struct position *p,
                                  struct position  a,
                                  struct position  b)
{
  p->x = a.x + b.x;
  p->y = a.y + b.y;
}*/

/* -------------------------------------------------------------------------- *
 * Gibt Differenz zwischen zwei Positionen zurück                             *
 * -------------------------------------------------------------------------- */

/*extern inline void gfxutil_possub(struct position *p,
                                  struct position  a,
                                  struct position  b)
{
  p->x = a.x - b.x;
  p->y = a.y - b.y;
}*/

/* -------------------------------------------------------------------------- *
 * Bereitet 'move' für eine Verschiebung nach 'end' vor und gibt die          *
 * relative Position zurück                                                   *
 * -------------------------------------------------------------------------- */
/*extern inline struct position gfxutil_moveto(struct position *rel,
                                             struct move     *move,
                                             struct position  end)
{
  move->end = end;
  
  return gfxutil_possub(end, move->now);
}*/

/* -------------------------------------------------------------------------- *
 * Lineare Translation von Position A nach Position B in 65535 Schritten (s)  *
 * -------------------------------------------------------------------------- */
#define gfxutil_translate(p, a, b, s) \
do \
{ \
  (p)->x = ((int32_t)(a).x * (65535 - (s)) + (int32_t)(b).x * (s)) / 65535; \
  (p)->y = ((int32_t)(a).y * (65535 - (s)) + (int32_t)(b).y * (s)) / 65535; \
} \
while(0);

/* -------------------------------------------------------------------------- *
 * Transformationsparameter A nach B in 65535 Schritten (s)                   *
 * -------------------------------------------------------------------------- */
/*void gfxutil_transform(struct transform *t,
                       struct transform  a,
                       struct transform  b,
                       uint16_t          s);*/

#define gfxutil_transform(t, x, y, s) \
do \
{ \
  (t)->r = (((float)((x).r)) * (float)(65535 - (s)) + ((float)((y).r)) * (float)(s)) / 65535.0; \
  (t)->z = (((float)((x).z)) * (float)(65535 - (s)) + ((float)((y).z)) * (float)(s)) / 65535.0; \
  (t)->a = (((float)((x).a)) * (float)(65535 - (s)) + ((float)((y).a)) * (float)(s)) / 65535.0; \
} \
while(0);

/* -------------------------------------------------------------------------- *
 * Überblendet stufenweise von Farbe A nach Farbe B und speichert das         *
 * Resultat in Farbe C                                                        *
 * -------------------------------------------------------------------------- */
/*void gfxutil_blend(struct color *c, struct color a, struct color b, 
                   uint16_t s);*/
#define gfxutil_blend(c, x, y, s) \
do \
{ \
  (c)->r = ((uint32_t)(x).r * (65535 - (s)) + (uint32_t)(y).r * (s)) / 65535; \
  (c)->g = ((uint32_t)(x).g * (65535 - (s)) + (uint32_t)(y).g * (s)) / 65535; \
  (c)->b = ((uint32_t)(x).b * (65535 - (s)) + (uint32_t)(y).b * (s)) / 65535; \
  (c)->a = ((uint32_t)(x).a * (65535 - (s)) + (uint32_t)(y).a * (s)) / 65535; \
} \
while(0);

/* -------------------------------------------------------------------------- *
 * Zentriert Rechteck a innerhalb von Reckteck b                              *
 * -------------------------------------------------------------------------- */
#define gfxutil_centerrect(a, b) \
do \
{ \
  (a)->x = (b)->x + (((b)->w - (a)->w) >> 1); \
  (a)->y = (b)->y + (((b)->h - (a)->h) >> 1); \
} \
while(0);

/* -------------------------------------------------------------------------- *
 * Padding                                                                    *
 * -------------------------------------------------------------------------- */
#define gfxutil_padrect(r, p) \
do \
{ \
  (r)->x += (p); (r)->w -= (p) << 1; \
  (r)->y += (p); (r)->h -= (p) << 1; \
} \
while(0);

/* -------------------------------------------------------------------------- *
 * Zentriert Rechteck auf die X/Y Koordinaten                                 *
 * -------------------------------------------------------------------------- */
#define gfxutil_centerxy(rect, xpos, ypos) \
do \
{ \
  (rect)->x = (xpos) - ((rect)->w >> 1); \
  (rect)->y = (ypos) - ((rect)->h >> 1); \
} \
while(0);

/* -------------------------------------------------------------------------- *
 * Liefert 1 wenn Rechteck a und b sich überlappen, andernfalls 0             *
 * -------------------------------------------------------------------------- */
#define gfxutil_overlap(a, b) \
    ((((a)->x >= (b)->x && (a)->x < (b)->x + (b)->w) || ((a)->x + (a)->w >= (b)->x && (a)->x + (a)->w < (b)->x + (b)->w)) && \
     (((a)->y >= (b)->y && (a)->y < (b)->y + (b)->h) || ((a)->y + (a)->h >= (b)->y && (a)->y + (a)->h < (b)->y + (b)->h)))

/* -------------------------------------------------------------------------- *
 * Gibt 1 zurück wenn x und y innerhalb des Rechtecks sind                    *
 * -------------------------------------------------------------------------- */
#define gfxutil_matchrect(rect, xpos, ypos) \
  (!((xpos) < (rect)->x || (xpos) > ((rect)->x + (rect)->w - 1) || (ypos) < (rect)->y || (ypos) > ((rect)->y + (rect)->h - 1)))

/* -------------------------------------------------------------------------- *
 * Verkleinert ein Rechteck so dass es in das Surface passt                   *
 * -------------------------------------------------------------------------- */
void         gfxutil_clip          (SDL_Surface    *sf,
                                    SDL_Rect       *rect);

/* -------------------------------------------------------------------------- *
 * Passt die Rechtecke fürs Surface-Kopieren oder Blitten an indem es Quell-  *
 * und Zielrechteck nötigenfalls auf den grössten gemeinsamen Bereich ver-    *
 * kleinert                                                                   *
 * -------------------------------------------------------------------------- */
void         gfxutil_blitclip      (SDL_Surface    *src,
                                    SDL_Rect       *srect,
                                    SDL_Surface    *dst,
                                    SDL_Rect       *drect);

/* -------------------------------------------------------------------------- *
 * Berechnet aus 2 Rechtecken die Überschneidung, lässt allerdings x und y    *
 * unverändert                                                                *
 * -------------------------------------------------------------------------- */
void         gfxutil_intersect     (SDL_Rect       *a,
                                    SDL_Rect       *b);

/* -------------------------------------------------------------------------- *
 * Kopiert ein Surface, der Alpha Layer von src wird mitkopiert.              *
 * -------------------------------------------------------------------------- */
void         gfxutil_copy          (SDL_Surface    *src,
                                    SDL_Rect       *sr,
                                    SDL_Surface    *dst, 
                                    SDL_Rect       *dr);

/* -------------------------------------------------------------------------- *
 * Kopieren mit Alpha und Farbtönung                                          *
 * -------------------------------------------------------------------------- */
void         gfxutil_tint          (SDL_Surface    *src, 
                                    SDL_Rect       *sr,
                                    SDL_Surface    *dst, 
                                    SDL_Rect       *dr,
                                    struct color    tint);
  
/* -------------------------------------------------------------------------- *
 * Additives Blitten mit Transparenz und Farbtönung (RGBA -> RGBA)            *
 * -------------------------------------------------------------------------- */
void         gfxutil_blit          (SDL_Surface    *src, 
                                    SDL_Rect       *sr,
                                    SDL_Surface    *dst, 
                                    SDL_Rect       *dr,
                                    int             alpha, 
                                    struct color   *hue);
  
/* -------------------------------------------------------------------------- *
 * Blitten mit Weichzeichner                                                  *
 * -------------------------------------------------------------------------- */
void         gfxutil_blur          (SDL_Surface    *src,
                                    SDL_Rect       *sr,
                                    SDL_Surface    *dst,
                                    SDL_Rect       *dr,
                                    struct color    tint,
                                    uint8_t         rate);

/* -------------------------------------------------------------------------- *
 * Rechnet benötigte Grösse der Zielsurface vor einer Transformation aus      *
 * -------------------------------------------------------------------------- */
void         gfxutil_sizetrig      (uint16_t        sw,
                                    uint16_t        sh,
                                    uint16_t       *dw,
                                    uint16_t       *dh,
                                    float           a,
                                    float           z,
                                    float          *sz, 
                                    float          *cz);

/* -------------------------------------------------------------------------- *
 * Führt 32-bit Pixeltransformationen durch                                   *
 * -------------------------------------------------------------------------- */
void         gfxutil_tf32          (SDL_Surface    *src,
                                    SDL_Surface    *dst,
                                    int             cx, 
                                    int             cy,
                                    int             icos,
                                    int             isin, 
                                    uint8_t         alpha);  

/* -------------------------------------------------------------------------- *
 * Transformiert eine Surface und skaliert den Alphakanal                     *
 * -------------------------------------------------------------------------- */
SDL_Surface *gfxutil_rotozoom      (SDL_Surface    *sf,
                                    float           r,
                                    float           z,
                                    uint8_t         a);

/* -------------------------------------------------------------------------- *
 * Berechnet den Farbtönungswert aus Fliesskommazahlen von 0-100% für jede    *
 * Farbe.                                                                     *
 * -------------------------------------------------------------------------- */
struct color gfxutil_tintcolor     (float           r,
                                    float           g, 
                                    float           b,
                                    float           a);

/* -------------------------------------------------------------------------- *
 * Gibt einen Farbwert in Dezimalzahlen aus (0-255 für jeden Kanal)           *
 * -------------------------------------------------------------------------- */
void         gfxutil_dumpcolor     (struct color    c);

/* -------------------------------------------------------------------------- *
 * Konvertiert einen RGB Wert in einen 6-8 stelligen Hexadezimal-String im    *
 * Format #RRGGBB(AA)                                                         *
 * -------------------------------------------------------------------------- */
char        *gfxutil_strcolor      (struct color    c);

/* -------------------------------------------------------------------------- *
 * Konvertiert einen HSV Wert in einen 6-8 stelligen Hexadezimal-String im    *
 * Format #HHSSVV(AA)                                                         *
 * -------------------------------------------------------------------------- */
char        *gfxutil_strhsv        (sgHSV           hsv);

/* -------------------------------------------------------------------------- *
 * Parst einen 6-stelligen Hexadezimalen Farbwert zurück in den binären Farb- *
 * wert.                                                                      *
 * -------------------------------------------------------------------------- */
int          gfxutil_parsecolor    (struct color   *c, 
                                    const char     *str);

/* -------------------------------------------------------------------------- *
 * Parst einen 6-8 stelligen Hexadezimalen Farbwert zurück in den binären     *
 * Farbwert.                                                                  * 
 * -------------------------------------------------------------------------- */
int          gfxutil_parsehsv      (sgHSV          *hsv,
                                    const char     *str);

/* -------------------------------------------------------------------------- *
 * Konvertiert vom HSV in das RGB Farbmodell                                  *
 * -------------------------------------------------------------------------- */
sgHSV        gfxutil_rgb2hsv       (struct color    color);
  
/* -------------------------------------------------------------------------- *
 * Konvertiert vom RGB in das HSV Farbmodell                                  *
 * -------------------------------------------------------------------------- */
struct color gfxutil_hsv2rgb       (sgHSV           hsv);
  
/* -------------------------------------------------------------------------- *
 * Gibt die Dimensionen eines Rechtecks aus                                   *
 * -------------------------------------------------------------------------- */
void         gfxutil_dumprect      (SDL_Rect       *rect);

/* -------------------------------------------------------------------------- *
 * Gibt 1 zurück wenn x und y innerhalb des Rechtecks sind                    *
 * -------------------------------------------------------------------------- */
/*int          gfxutil_matchrect     (const SDL_Rect *rect,
                                    int16_t         x,
                                    int16_t         y);*/

/* -------------------------------------------------------------------------- *
 * Füllt das Zielsurface mit einem "minimalen" Muster aus den Farben S und U. *
 *                                                                            *
 * Der Vorschlag für dieses Muster stammt von Stephan Urech,                  *
 * vielen Dank Steph!                                                         *
 * -------------------------------------------------------------------------- */
void         gfxutil_patternfill   (SDL_Surface    *sf,
                                    SDL_Rect       *rect,
                                    struct color    s, 
                                    struct color    u);
#endif /* GFXUTIL_H */
