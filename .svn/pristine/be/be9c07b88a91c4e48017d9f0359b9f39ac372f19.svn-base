/* $Id: cursor.c,v 1.11 2005/05/17 15:12:34 smoli Exp $
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

/** @weakgroup sgCursor
 *  @{
 */

#include <ctype.h>
#include <dirent.h>

#include <libsgui/sgui.h>
#include <libsgui/file.h>
#include <libsgui/png.h>
#include <libsgui/cursor.h>

//static sgCursorTheme  sgDefaultCursorTheme = { .name = "Default" };

static sgList         sgCursorThemes = 
{
/*  .head = &sgDefaultCursorTheme.node,
  .tail = &sgDefaultCursorTheme.node*/
};

/* Allocates a new cursor surface, freeing potentially old ones */
void sgAllocCursorSurface(sgCursorFace *face, Sint16 w, Sint16 h)
{
  if(face->surface)
    SDL_FreeSurface(face->surface);
  
  face->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
                                       32, RMASK, GMASK, BMASK, AMASK);
}

/* Frees cursor face surface if present */
void sgFreeCursorSurface(sgCursorFace *face)
{
  if(face->surface)
  {
    SDL_FreeSurface(face->surface);
    face->surface = NULL;
  }
}

/* Sets a specific cursor surface and its hotspot */
void sgSetCursorSurface(sgCursorFace *face, SDL_Surface *surface,
                        Sint16 hotx, Sint16 hoty)
{
  face->surface = surface;
  face->hotx = hotx;  
  face->hoty = hoty;
}

/* Copies a specfic region of a surface to a cursor */
void sgCopyCursorSurface(sgCursorFace *face, SDL_Surface *surface)
{
  sgClip(surface, &face->rect);
  sgAllocCursorSurface(face, face->rect.w, face->rect.h);
  sgCopy(surface, &face->rect, face->surface, NULL);
}

/* Sets cursor information as parsed */
void sgSetCursorInfo(sgCursorFace *face, SDL_Rect *rect,
                    Sint16 hotx, Sint16 hoty)
{
  face->rect = *rect;
  face->hotx = hotx;
  face->hoty = hoty;
}
  
/* allocates new cursor theme */
sgCursorTheme *sgNewCursorTheme(void)
{
  sgCursorTheme *theme;
  
  if((theme = malloc(sizeof(sgCursorTheme))))
  {
    memset(theme, 0, sizeof(sgCursorTheme));
    sgAddList(&sgCursorThemes, &theme->node, theme);
  }
  
  return theme;
}

/* frees and unlinks all cursor themes */
void sgDeleteCursorTheme(sgCursorTheme *theme)
{
  sgUnloadCursorTheme(theme);
  
  sgDeleteList(&sgCursorThemes, &theme->node);
  
  free(theme);
}

/* frees and unlinks all cursor themes */
void sgDeleteCursorThemes(void)
{
  sgCursorTheme *theme;
  sgCursorTheme *next;
  
  sgForeachSafe(&sgCursorThemes, theme, next)
    sgDeleteCursorTheme(theme);
}

/* parses a line of a cursor theme and returns a non-zero value
   if successful */
int sgParseCursorTheme(sgCursorTheme *theme, char *line)
{
  int type, x, y, w, h, hotx, hoty;
  int ret;
  
  ret = sscanf(line, "%i:%i,%i/%ix%i/%i|%i", 
               &type, &x, &y, &w, &h, &hotx, &hoty);
  
  if(ret == 7)
  {
    SDL_Rect rect;
    
    rect.x = x; rect.y = y;
    rect.w = w; rect.h = h;
    
/*    sgLog("Loading cursor #%i @ %i,%i with dimensions %ux%u and hotspot %i|%i",
          type, rect.x, rect.y, rect.w, rect.h, hotx, hoty);*/
    
    sgSetCursorInfo(&theme->faces[type], &rect, hotx, hoty);    
    return ret;
  }
  
  return 0;
}

/* makes a cursor theme ready to use */
int sgLoadCursorTheme(sgCursorTheme *theme)
{
  int i;
  SDL_Surface *surface;
  
  surface = sgLoadPngFile(theme->image);
  
  if(surface == NULL)
    return -1;

  for(i = 0; i < SG_CURSOR_COUNT; i++)
    sgCopyCursorSurface(&theme->faces[i], surface);
  
  SDL_FreeSurface(surface);
  return 0;
}

/* unloads a cursor theme */
void sgUnloadCursorTheme(sgCursorTheme *theme)
{
  int i;
  
  for(i = 0; i < SG_CURSOR_COUNT; i++)
    sgFreeCursorSurface(&theme->faces[i]);
}

/* loads a cursor theme file and parses it */
sgCursorTheme *sgOpenCursorThemeFp(FILE *fp, int autoclose)
{
  char line[512];
  sgCursorTheme *theme;
  
  if((theme = sgNewCursorTheme()) == NULL)
    return NULL;
  
  while(fgets(line, sizeof(line) - 1, fp))
  {
    char *p = line;
    char *e;
    
    while(*p && isspace(*p)) 
      p++;
    
    if(*p == '\0' || *p == ';')
      continue;
    
    e = p + strlen(p);
    
    while(e >= p)
    {
      if(isspace(*--e))
        *e = '\0';
      else
        break;
    }
    
    if(!sgParseCursorTheme(theme, p))
    {
      char *name;
      
      if((name = strchr(line, ':')))
      {
        *name++ = '\0';
        sgStringCopy(theme->name, name);
        sgStringCopy(theme->image, line);
      }
      
      continue;
    }
    
  }
  
  sgLog("Loaded cursor theme \"%s\"", theme->name);
  
  return theme;
}

/* opens a cursor theme file and loads it */
sgCursorTheme *sgOpenCursorTheme(const char *file)
{
  FILE *fp;
  
  if((fp = sgOpenFileFp(file, "r")))
    return sgOpenCursorThemeFp(fp, 1);
  
  return NULL;
}

/* Loads all available cursor themes */
int sgOpenCursorThemes(void)
{
  DIR *d;
  struct dirent *de;
  int n = 0;
  unsigned int len;

#ifdef DATADIR
  if((d = opendir(DATADIR)) == NULL)
#else
  if((d = opendir("sounds")) == NULL)
#endif /* DATADIR */
      return -1;
  
  while((de = readdir(d)))
  {
    if((len = strlen(de->d_name)) <= 3)
      continue;
    
    if(strcmp(&de->d_name[len - 4], ".cur"))
      continue;

    if(sgOpenCursorTheme(de->d_name))
      n++;
  }
  
  return n;
}

/* Gets the current cursor theme */
sgCursorTheme *sgGetCursorTheme(sgCursor *cursor)
{
  return cursor->theme;
}

/* Gets the cursor theme list */
sgList *sgGetCursorThemes(void)
{
  return &sgCursorThemes;
}

/* Sets the current cursor theme (without loading it and unloading previous one) */
void sgSetCursorTheme(sgCursor *cursor, sgCursorTheme *theme)
{
  cursor->theme = theme;
}

/* Finds cursor theme by its name */
sgCursorTheme *sgFindCursorTheme(const char *name)
{
  sgCursorTheme *theme;
  
  sgForeach(&sgCursorThemes, theme)
  {
    if(!strcmp(theme->name, name))
      return theme;
  }
  
  return NULL;
}

/* Changes cursor theme */
void sgChangeCursorTheme(sgCursor *cursor, sgCursorTheme *theme)
{
  if(cursor->theme)
    sgUnloadCursorTheme(cursor->theme);
  
  cursor->theme = theme;
  sgLoadCursorTheme(theme);
  
  if(theme->image[0])
    SDL_ShowCursor(SDL_DISABLE);
  else
    SDL_ShowCursor(SDL_ENABLE);
}
  
/* Returns pointer to appropriate cursor face struct */
sgCursorFace *sgGetCursorFace(sgCursor *cursor, sgCursorType face)
{
  if(cursor->theme && face >= 0 && face < SG_CURSOR_COUNT)
    return &cursor->theme->faces[face];
  
  return NULL;
}

/* Returns pointer to current cursor surface */
SDL_Surface *sgGetCursorSurface(sgCursor *cursor, sgCursorType type)
{
  sgCursorFace *face = sgGetCursorFace(cursor, type);
  
  if(face)
    return face->surface;
  
  return NULL;
}

/* sets the cursor face */
int sgSetCursorFace(sgCursor *cursor, sgCursorType type)
{
  if(cursor->face != type)
  {
    sgCursorFace *face;

    cursor->face = type;
    
    if((face = sgGetCursorFace(cursor, type)) && face->surface)
    {
      cursor->crect.x = cursor->x - face->hotx;
      cursor->crect.y = cursor->y - face->hoty;
      cursor->crect.w = face->surface->w;
      cursor->crect.h = face->surface->h;
    }
    
    return 1;
  }
  
  return 0;
}

/* sets the cursor position */
int sgSetCursorPos(sgCursor *cursor, Sint16 x, Sint16 y)
{
  if(cursor->x != x || cursor->y != y)
  {
    sgCursorFace *face;
    
    cursor->x = x;
    cursor->y = y;
    
    if((face = sgGetCursorFace(cursor, cursor->face)) && face->surface)
    {
      cursor->crect.x = x - face->hotx;
      cursor->crect.y = y - face->hoty;
      cursor->crect.w = face->surface->w;
      cursor->crect.h = face->surface->h;
    }
    
    return 1;
  }
  
  return 0;
}  
  
/* allocates a large enough surface for the cursor backup */
void sgAllocCursorBackup(sgCursor *cursor, SDL_PixelFormat *format)
{
  SDL_Surface *surface;
  
  if((surface = sgGetCursorSurface(cursor, cursor->face)))
  {
    if(cursor->bgnd)
    {
      if(cursor->bgnd->w < surface->w ||  cursor->bgnd->h < surface->h)
      {
        SDL_FreeSurface(cursor->bgnd);
        cursor->bgnd = NULL;
      }
    }
    
    if(cursor->bgnd == NULL)
      cursor->bgnd =
      SDL_CreateRGBSurface(SDL_SWSURFACE,
                           surface->w, surface->h,
                           format->BitsPerPixel,
                           format->Rmask, format->Gmask,
                           format->Bmask, 0);
  }
}

/* clears the backupped cursor background so it is not restored
 * next time when blitting the cursor 
 */
void sgClearCursorBgnd(sgCursor *cursor)
{
  cursor->brect.x = 0;
  cursor->brect.y = 0;
  cursor->brect.w = 0;
  cursor->brect.h = 0;
}

/* restores the backupped background before blitting the cursor */
void sgRestoreCursorBgnd(sgCursor *cursor, SDL_Surface *surface)
{
  if(cursor->brect.w && cursor->brect.h)
    SDL_BlitSurface(cursor->bgnd, &cursor->brect, surface, NULL);
}

/* backs up the background area before blitting a cursor */
void sgBackupCursorBgnd(sgCursor *cursor, SDL_Surface *surface)
{
  sgAllocCursorBackup(cursor, surface->format);
  cursor->brect = cursor->crect;
  SDL_BlitSurface(surface, &cursor->brect, cursor->bgnd, NULL);
}

/* blits cursor to a surface after restoring the old background and 
   backing up the destination area */
int sgBlitCursor(sgCursor *cursor, SDL_Surface *surface)
{
  if(cursor->crect.x != cursor->brect.x ||
     cursor->crect.y != cursor->brect.y ||
     cursor->face != cursor->lastface)
  {
    SDL_Surface *sf;
    
    if(cursor->bgnd)
      sgRestoreCursorBgnd(cursor, surface);
    
    cursor->lastface = cursor->face;
    
    sgBackupCursorBgnd(cursor, surface);
    
    if((sf = sgGetCursorSurface(cursor, cursor->face)))
      SDL_BlitSurface(sf, NULL, surface, &cursor->crect);
    
    return 1;
  }
  
  return 0;
}

/* updates screen rectangles for the cursor */
void sgUpdateCursor(sgCursor *cursor, SDL_Surface *surface)
{
  SDL_Rect rect;
  
  sgMergeRect(&rect, cursor->brect, cursor->crect);
  
  sgClip(surface, &rect);
  
  if((surface->flags & SDL_DOUBLEBUF) == 0)
    SDL_UpdateRect(surface, rect.x, rect.y, rect.w, rect.h);
}

/** @} */
