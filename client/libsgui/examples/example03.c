#include <libsgui/sgui.h>

static SDL_Surface *screen;
static SDL_Event    event;
static sgColor      color;
static SDL_Rect     rect;
static sgWidget    *dialog;
static sgWidget    *group1;
static sgWidget    *console1;
static sgWidget    *colorsel1;
static int          terminated;

int handler(sgWidget *widget, sgEvent event)
{
  switch(event)
  {
    case SG_EVENT_QUIT:
    {
      terminated = 1;
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
  /* Initialize the SDL video subsystem, without exception catching */
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE);

  /* Initialize the sgUI library */
  sgInit();

  /* Set the initial video mode */
  screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF);
  
  /* Set a dialog color */
  color.r = 160;
  color.g = 224;
  color.b = 32;
  
  /* Create a dialog */
  dialog = sgNewDialog(screen, handler, 4, 128, color, 25);

  /* Load default fonts */
  sgLoadWidgetFonts(dialog, "font-normal.png", "font-bold.png", "font-fixed.png");

  sgOpenCursorTheme("grounation.cur");
  
  /* Create a group rectangle */
  rect = dialog->rect;

  sgSubBorder(&rect, 32);
  
  group1 = sgNewGroup(dialog, rect.x, rect.y, rect.w, rect.h, "Example03");
  
  /* Create a console */
  console1 = sgNewConsoleGrouped(group1, SG_EDGE_TOP, SG_ALIGN_CENTER,
                                 400, 200, "Console 1");
  
  /* Add some lines to the console */
  sgAddConsoleLine(console1, "blah");
  
  /* Create a line-editing widget */
/*  input1 = sgNewInputGrouped(group1, SG_EDGE_TOP, SG_ALIGN_CENTER,
                             400, 40, "Input 1");*/
/*  toggle1 = sgNewToggleGrouped(group1, SG_EDGE_TOP, SG_ALIGN_CENTER,
                               200, 40, "Toggle 1");*/

  colorsel1 = sgNewColorSelGrouped(group1, SG_EDGE_TOP, SG_ALIGN_CENTER,
                                   320, 48, SG_COLORSEL_HUE, "Color");
  
/*  dropdown1 = sgNewDropdownGrouped(group1, SG_EDGE_TOP, SG_ALIGN_LEFT,
                                   240, 28);
  
  sgAddDropdownItem(dropdown1, "640x480", NULL);
  sgAddDropdownItem(dropdown1, "800x600", NULL);
  sgAddDropdownItem(dropdown1, "1024x768", NULL);
  sgAddDropdownItem(dropdown1, "1280x1024", NULL);*/
  
  /* Set new dialog color */
  color.r = 96;
  color.g = 128;
  color.b = 160;
  
  sgSetWidgetColor(dialog, color);
  
  /* Main loop */
  while(!terminated)
  {
    /* Fill the event queue and let the dialog handle them */
    SDL_PumpEvents();
    
    while(SDL_PollEvent(&event))
      sgHandleWidgetEvent(dialog, &event);
    
    sgBlitDialog(dialog, screen);
    
    SDL_Flip(screen);    
    SDL_Delay(100);
  }
  
  /* Shutdown the sgUI library */
  sgQuit();
  
  /* Shutdown the SDL library */
  SDL_Quit();

  return 1;
}

