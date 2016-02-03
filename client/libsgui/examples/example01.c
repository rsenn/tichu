#include <math.h>
#include <libsgui/sgui.h>
#include <libsgui/png.h>

#define RADIUS 54

static SDL_Surface *screen;
static sgColor      color;
static SDL_Rect     rect1;
static SDL_Rect     rect2;
static sgWidget    *dialog;
static sgWidget    *image1;
static SDL_Surface *surface1;
static sgWidget    *adjust1;
static int          radius = RADIUS;

int handler(sgWidget *widget, sgEvent event)
{
  switch(event)
  {
    case SG_EVENT_QUIT:
    {
      sgClearWidgetStatus(dialog, SG_RUNNING);
      break;
    }
    
/*    case SG_EVENT_RESIZE:
      
      if(widget == dialog)
      {
        rect1 = widget->rect;
        
        screen = SDL_SetVideoMode(rect1.w, rect1.h, 0, 
                                  SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
      }

      break;*/
    case SG_EVENT_CHANGE:
    {
      double pos;
      int x, y;
      
      sgGetAdjustValue(adjust1, &pos);
      
      x = dialog->rect.w / 2 - sin(pos * M_PI / 180) * radius;
      y = dialog->rect.h / 2 - cos(pos * M_PI / 180) * radius;
            
      sgSetWidgetPos(image1, x, y, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
    
      break;
    }
    
    default:
    {
      break;
    }
  }
  
  return 0;
}

int main(int argc, char *argv[]) 
{
  sgCursorTheme *theme;
  
  /** Initialize the SDL video subsystem, without exception catching */
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE);

  /** Initialize the sgUI library */
  sgInit();

  /** Set the initial video mode (preferring 32-bit format) */
  screen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF);
  
  /** Set a dialog color */
  color.r = 0xdc;
  color.g = 0xb5;
  color.b = 0x1a;
  
  /** Create a dialog */
  dialog = sgNewDialog(screen, handler, 4, 128, color, 25);

  /** Load default fonts */
  sgLoadWidgetFont(dialog, SG_FONT_NORMAL, "font-bold.png");

  theme = sgOpenCursorTheme("ghost.cur");

  sgSetDialogCursorTheme(dialog, theme);
  
  surface1 = sgLoadPngFile("libsgui.png");

  if(surface1)
  {
    rect1 = surface1->clip_rect;
  }
  else
  {
    rect1.w = 200;
    rect1.h = 200;
  }
  
  sgAlignRect(&dialog->rect, &rect1, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  
  image1 = sgNewImage(dialog, rect1.x, rect1.y - radius, rect1.w, rect1.h, NULL);
  
  if(surface1)
  {
    sgSetWidgetBorder(image1, 0);
    sgSetImageSurface(image1, surface1, SG_ALIGN_CENTER);
  }
  
  /* Create some buttons */
/*  button1 = sgNewButton(dialog, rect1.x, rect1.y, rect1.w, rect1.h / 3, "Button");

  button2 = sgNewButton(dialog, rect1.x, rect1.y + rect1.h / 3, rect1.w, rect1.h / 3, "Lala"); */
 
  rect1.y += rect1.h;
  rect1.h = dialog->rect.h - rect1.y;
  rect1.x = 0;
  rect1.w = dialog->rect.w;

  rect2.w = 450;
  rect2.h = 32;
  
  sgAlignRect(&rect1, &rect2, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  
  adjust1 = sgNewAdjustRect(dialog, rect2, 0, 360, "Angle");

  sgSetAdjustFormat(adjust1, "%.0lf°");
  
  /* Set new dialog color */
  color.r = 0x7e;
  color.g = 0x7e;
  color.b = 0x7e;
  
  sgSetWidgetColor(dialog, color);
  
  sgRunDialog(dialog, 0);

  SDL_FillRect(dialog->face.frame, NULL, 0);  
  sgSetWidgetStatus(dialog, SG_REDRAW_FRAME);
    
  sgBlitDialog(dialog, screen);
  
  //sgSavePngFile(screen, "test.png");
  
  /* Shutdown the sgUI library */
  sgQuit();
  
  /* Shutdown the SDL library */
  SDL_Quit();

  return 1;
}

