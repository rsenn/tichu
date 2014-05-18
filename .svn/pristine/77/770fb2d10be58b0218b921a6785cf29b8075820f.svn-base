#include <libchaos/io.h>
#include <libchaos/mem.h>
#include <libchaos/log.h>
#include <libchaos/gif.h>

int giftest_write(void)
{
  struct gif     *gif;
  struct palette *pal;
  struct color    colors[4] = {
    {  0,    0,   0 },
    { 64,   64,  64 },
    { 128, 128, 128 },
    { 192, 192, 192 }
  };
  uint32_t        i;
  uint8_t         data;
  
  gif = gif_new("test.gif", GIF_WRITE);
  
  pal = gif_palette_make(4, colors);
  
  gif_screen_put(gif, 100, 100, 2, 0, pal);
  
  gif_image_put(gif, 0, 0, 100, 100, 0, pal);
  
  for(i = 0; i < 100 * 100; i++)
  {
    data = i & 0x03;
    gif_data_put(gif, &data, 1);
  }
  
  gif_close(gif);
  gif_save(gif);
  
  return 0;
}

int main()
{
  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  gif_init();
  
  giftest_write();
  
  gif_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
  
  return 0;
}
 
