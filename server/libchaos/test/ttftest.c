#include <libchaos/mem.h>
#include <libchaos/log.h>
#include <libchaos/gif.h>
#include <libchaos/image.h>

#ifdef HAVE_FT2
#include <libchaos/ttf.h>

int ttftest_log;

void ttftest_write(const char *text, const char *font, const char *file, const char *bg, const char *color)
{
  struct image   *ttftext;
  struct image   *gradient;
  struct ttf     *ttf;
  struct color    fg;
  struct rect     drect;
  struct rect     srect;
  
  log(ttftest_log, L_status, "---------- TrueType GIF renderer ----------", text);
  log(ttftest_log, L_status, "text:  %s", text);
  log(ttftest_log, L_status, "font:  %s", font);
  log(ttftest_log, L_status, "bg:    %s", bg);
  log(ttftest_log, L_status, "file:  %s", file);
  log(ttftest_log, L_status, "color: %s", color);
  log(ttftest_log, L_status, "-------------------------------------------");
  
  ttf = ttf_new(font);
  
  if(ttf_open(ttf, font))
  {
    log(ttftest_log, L_fatal, "Could not load font: %s", font);
    return;
  }

  ttf_calc(ttf, 11);
  
  ttf->style |= TTF_STYLE_BOLD;
  
  image_color_parse(&fg, color);
    
  ttftext = ttf_text_blended(ttf, text, &fg);
  
  gradient = image_load_gif(bg);
  
  if(gradient == NULL)
  {
    log(ttftest_log, L_fatal, "Could not load background image: %s", bg);
    return;
  }  
  
  image_convert(gradient, IMAGE_TYPE_32);

  srect = ttftext->rect;

  drect = gradient->rect;
  drect.y = 1;
  drect.x = 6;
  
  image_blit_32to32(ttftext, &srect, gradient, &drect);
  
  image_convert(gradient, IMAGE_TYPE_8);
  
  if(gradient == NULL)
  {
    log(ttftest_log, L_warning, "Could not render test glyph.");
  }
  else
  {
    if(image_save_gif(gradient, file))
    {
      log(ttftest_log, L_fatal, "Could not save output: %s", file);
    }  
  
    image_delete(gradient);
  }
}
#endif

int main(int argc, char **argv)
{
#ifdef HAVE_FT2
  const char *text = "Test text!";
  const char *font = "arial.ttf";
  const char *file = "ttftest.gif";
  const char *bg = "heading-yellow.gif";
  const char *color = "#000000";
  
  log_init(STDOUT_FILENO, LOG_ALL, L_warning);
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  gif_init();
  image_init();
  ttf_init();
  
  ttftest_log = log_source_register("ttftest");
  
  log_level(LOG_ALL, L_verbose);
  
  if(argc > 1)
    text = argv[1];
  if(argc > 2)
    font = argv[2];
  if(argc > 3)
    file = argv[3];
  if(argc > 4)
    bg = argv[4];
  if(argc > 5)
    color = argv[5];

  ttftest_write(text, font, file, bg, color);

  log_level(LOG_ALL, L_warning);
  
  log_source_unregister(ttftest_log);
  
  ttf_shutdown();
  image_shutdown();
  gif_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
#endif  
  return 0;
}
 
