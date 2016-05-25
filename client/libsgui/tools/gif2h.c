#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libsgui/gif.h>

/*extern int          IMG_isGIF(SDL_RWops *src);
extern SDL_Surface *IMG_LoadGIF_RW(SDL_RWops *src);
extern SDL_Surface *IMG_Load(const char *file);*/
  
uint8_t  *dest;
uint8_t   tempb = 0;
uint32_t  bitc  = 0;
uint32_t  bytec = 0;

void putbit(uint8_t bit) {
  
  dest[bytec] |= (bit & 0x01) << bitc;
  
  if(++bitc == 8) {
    bitc    = 0;
    bytec++;
  }  
} 

void gif2header(gif_t *gif, const char *name) {
 
  int          width, height;
  int          bw;
  uint8_t     *pixel, *pixels;
  int          x, y;
  gif_image_t *image;
  
  image = gif->images;

  if(image == NULL) { 
    printf("No image data!\n");
    exit(EXIT_FAILURE);
  }
  
  if(image->desc.width & 0x0f || image->desc.height & 0x0f) {
    printf("Inappropriate size\n");
    exit(EXIT_FAILURE);
  }
  
  width = image->desc.width >> 4;
  height = image->desc.height >> 4;
  
  printf("#include \"font.h\"\n\n");
  printf("sgFontData font_%s = {\n", name);
  printf("  %u, %u, {\n", width, height);
   
  bw = (image->desc.width + 7) >> 3;
  
  dest = calloc(bw, image->desc.height);

  pixel = image->bits;
  
  for(y = 0; y < image->desc.height; y++) {
    pixels = image->bits;
    pixel = &pixels[y * image->desc.width];

    for(x = 0; x < image->desc.width; x++, pixel++) {
      if(*pixel) {
        putbit(1);
      } else {
        putbit(0);
      }
    }
    printf("    ");
    
    for(x = 0; x < bw; x++) {
      printf("0x%02x,", (int)(unsigned char)dest[x]);
    }
    printf("\n");
    
    memset(dest, 0, bw * image->desc.height);
    bytec = 0;
    bitc = 0;
  }
  
  printf("  }\n};\n");
 
}

int main(int argc, char *argv[]) {
  
  gif_t *gif;
  char  *p;
  
  if(argv[1] == NULL) {
    fprintf(stderr, "Usage: %s <gif file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  gif = gif_open(argv[1], GIF_READ);
  
  if(gif == NULL) {
    printf("Cannot open %s: %s\n", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if(gif_slurp(gif)) { 
    printf("Cannot read %s: %s\n", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  p = strrchr(argv[1], '.');
  
  if(p != NULL) { 
    *p = '\0';
  }
  
  p = strrchr(argv[1], '/');
      
  if(p != NULL) { 
    p++;
  } else { 
    p = argv[1];
  }
  
  gif2header(gif, p);
  
  gif_close(gif);
  
  return 0;
}
  
