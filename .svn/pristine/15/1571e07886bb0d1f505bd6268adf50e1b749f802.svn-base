#include <libsgui/sgui.h>

static SDL_Surface *screen;
static sgColor      color;
static SDL_Rect     rect;
static sgWidget    *dialog;
static sgWidget    *tab;
static sgWidget    *group1;
static sgWidget    *group2;
static sgWidget    *group3;
static sgWidget    *console1;
static sgWidget    *console2;
static sgWidget    *console3;

int handler(sgWidget *widget, sgEvent event)
{
  switch(event)
  {
    case SG_EVENT_QUIT:
    {
      sgClearWidgetStatus(dialog, SG_RUNNING);
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

  sgOpenCursorTheme("deep-sky.cur");
  
  /* Create a group rectangle */
  rect = dialog->rect;

  sgSubBorder(&rect, 64);

  tab = sgNewTabRect(dialog, rect);

  group1 = sgNewGroupFull(tab, "README");
  sgSetWidgetRGB(group1, 124, 84, 160);
  
  group2 = sgNewGroupFull(tab, "COPYING.LIB");
  sgSetWidgetRGB(group2, 192, 52, 10);
  
  group3 = sgNewGroupFull(tab, "ChangeLog");
  sgSetWidgetRGB(group2, 48, 160, 100);

  /* Set up the consoles */
  console1 = sgNewConsoleFull(group1, NULL);  
  sgLoadConsoleText(console1, "README");
  
  console2 = sgNewConsoleFull(group2, NULL);
  sgLoadConsoleText(console2, "COPYING.LIB");
  
  console3 = sgNewConsoleFull(group3, NULL);
  sgLoadConsoleText(console3, "ChangeLog");
  
  /* Set new dialog color */
  color.r = 96;
  color.g = 128;
  color.b = 160;

  sgSetWidgetColor(dialog, color);

  sgRunDialog(dialog, 0);
  
  /* Shutdown the sgUI library */
  sgQuit();
  
  /* Shutdown the SDL library */
  SDL_Quit();

  return 1;
}

