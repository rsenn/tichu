#include <libchaos/io.h>
#include <libchaos/mem.h>
#include <libchaos/log.h>
#include <libchaos/gif.h>
#include <libchaos/image.h>

void imagetest_write(void)
{
  struct image *image;
  uint32_t      i;
  struct rect   rect;
  
  image = image_new(IMAGE_TYPE_8, 640, 480);

  image_putline(image, 20, 460, 620, 20, 60);
  
  image_putellipse(image, 320, 240, 300, 200, 64, 180);
  image_putcircle(image, 320, 240, 80, 64, 195);
  
  rect.x = 60;
  rect.y = 300;
  rect.w = 580;
  rect.h = 180;
  
  image_putrect(image, &rect, 80);
  
  for(i = 0; i < 256; i++)
    image_putpixel(image, 20 + i, 50, i);
  
/*  image_putstr(image, &image_font_6x10, 100, 40, 12, IMAGE_ALIGN_LEFT, "libchaos rocks!");
  image_putstr(image, &image_font_6x10, 160, 80, 11, IMAGE_ALIGN_LEFT, "dschoint");*/
  image_save_gif(image, "lala.gif");
  
  image_delete(image);
}

int main()
{
  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  gif_init();
  image_init();
  
  imagetest_write();

  image_shutdown();
  gif_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
  
  return 0;
}
 
