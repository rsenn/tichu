/* $Id: png.h,v 1.2 2005/05/03 12:45:05 smoli Exp $
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

#ifndef LIBSGUI_PNG_H
#define LIBSGUI_PNG_H

#include <stdio.h>
#include <SDL.h>

/** @defgroup sgPng sgPng: PNG image loading and saving functions
 */ 

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Load a PNG image from memory 
 *
 *  @param mem        Memory area containing the PNG image
 *  @param size       Size of the specified memory area
 * 
 *  @return           An SDL_Surface on success, NULL on error
 */
SDL_Surface *sgLoadPngMemory (const void *mem, 
                              Uint32      size);

/** Load a PNG image from a file 
 *
 *  @param file  Path to the PNG file
 * 
 *  @return           An SDL_Surface on success, NULL on error
 */
SDL_Surface *sgLoadPngFile   (const char *file);

/** Load a PNG image from a stdio file pointer 
 *
 *  @param fp         FILE pointer, opened for reading
 *  @param autoclose  close the file after the operation when this is non-zero
 * 
 *  @return           An SDL_Surface on success, NULL on error
 */
SDL_Surface *sgLoadPngFp     (FILE        *fp, 
                              int          autoclose);

/** Loads png from RWops into SDL_Surface 
 *
 *  sgLoadPngRWops is stolen from SDL_image which is 
 *  Copyright (C) by Sam Lantinga, Philippe Lavoie
 * 
 * 
 *  @param rwops      The data source
 * 
 *  @return           An SDL_Surface on success, NULL on error
 */
SDL_Surface *sgLoadPngRWops  (SDL_RWops   *rwops);

/** Save a surface as PNG image to memory
 * 
 *  @param surface    surface to save as .png
 *  @param mem        destination buffer
 *  @param size       max size of the buffer
 *
 *  @return           0 on success, non-zero otherwise
 */
int          sgSavePngMemory (SDL_Surface *surface,
                              void        *mem, 
                              Uint32       size);    

/** Save a surface to a PNG file 
 *
 *  @param surface    surface to save as .png
 *  @param file       path of the file to create
 * 
 *  @return           0 on success, non-zero otherwise
 */
int          sgSavePngFile   (SDL_Surface *surface, 
                              const char  *file);    
  
/** Save a surface to a stdio file pointer
 * 
 *  @param surface    surface to save as .png
 *  @param fp         path of the file to create
 *  @param autoclose  close the file after the operation when this is non-zero
 * 
 *  @return           0 on success, non-zero otherwise
 */
int          sgSavePngFp     (SDL_Surface *surface, 
                              FILE        *fp, 
                              int          autoclose);    
  
/** Saves png from SDL_Surface into RWops 
 *
 *  @param surface    Surface to save as .png
 *  @param rwops      SDL_rwops we'll write to
 * 
 *  @return           0 on success, non-zero otherwise
 */
int          sgSavePngRWops  (SDL_Surface *surface, 
                              SDL_RWops   *rwops);
    
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
  
#endif /* LIBSGUI_PNG_H */
