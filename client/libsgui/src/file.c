/* $Id: file.c,v 1.8 2005/05/16 05:19:55 smoli Exp $
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

#include <stdlib.h>

/** @weakgroup sgFile
 *  @{
 */

#include <libsgui/file.h>
#include <libsgui/list.h>
#include <libsgui/common.h>

/** A list containing all search paths */
static sgList sgFilePaths;
static char   sgFilePath[PATH_MAX];

/* Get path of the last found file */
char *sgGetFilePath(void)
{
  return sgFilePath;
}

sgList *sgGetFilePaths(void)
{
  return &sgFilePaths;
}

/* Add a search path to the list */
void sgAddFilePath(const char *dir)
{
  sgPath *path;
  
  if((path = malloc(sizeof(sgPath))))
  {
    sgAddList(&sgFilePaths, &path->node, path);
    sgStringCopy(path->dir, dir);
  }
}

/* Clear the whole search path list */
void sgClearFilePaths(void)
{
  sgPath *path, *next;
  
  sgForeachSafe(&sgFilePaths, path, next)
    free(path);
  
  sgZeroList(&sgFilePaths);
}

/* Opens a file and returns a stdio file pointer */
FILE *sgOpenFileFp(const char *name, const char *mode)
{
  sgPath *path;
  FILE *fp = NULL;
  
  sgForeach(&sgFilePaths, path)
  {
    sgStringCopy(sgFilePath, path->dir);
    sgStringCat(sgFilePath, "/");
    sgStringCat(sgFilePath, name);
    
    if((fp = fopen(sgFilePath, mode)))
      return fp;
  }
  
  sgFilePath[0] = '\0';

  sgLog("Could not find the file \"%s\"", name);
  
  return NULL;
}

/* Opens a file and returns a SDL_rwops */
SDL_RWops *sgOpenFileRWops(const char *name, const char *mode)
{
  char buf[PATH_MAX];
  sgPath *path;
  SDL_RWops *rwops = NULL;
  
  sgForeach(&sgFilePaths, path)
  {
    sgStringCopy(buf, path->dir);
    sgStringCat(buf, "/");
    sgStringCat(buf, name);
    
    if((rwops = SDL_RWFromFile(buf, mode)))
      break;
  }
  
  return rwops;
}
  
/* Loads a files content into a string */
char *sgLoadFileText(const char *name)
{
  FILE *fp;
  char buf[4096];
  char *s = NULL;
  char *p;
  unsigned long n = 0;

  if((fp = sgOpenFileFp(name, "r")) == NULL)
    return NULL;

  while(fgets(buf, sizeof(buf), fp))
  {
    unsigned long len;
    buf[sizeof(buf) - 1] = '\0';
    
    if((p = strchr(buf, '\n')))
    {
      if(p > buf && p[-1] == '\r')
        p[-1] = '\0';
      else
        p[0] = '\0';
    }
    
    len = strlen(buf);
    
    if((s = realloc(s, n + len + 2)) == NULL)
      break;

    if(len)
      memcpy(&s[n], buf, len);

    n += len;
    s[n] = '\n';
  }
  
  fclose(fp);
  
  s[n + 1] = '\0';

  return s;
}

/** @} */
