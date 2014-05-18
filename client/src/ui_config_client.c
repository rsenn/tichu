/* $Id: ui_config_client.c,v 1.7 2005/05/21 22:36:09 smoli Exp $
 * -------------------------------------------------------------------------- *
 *  .___.    .                                                                *
 *    |  * _.|_ . .        Portabler, SDL-basierender Client für das          *
 *    |  |(_.[ )(_|             Multiplayer-Kartenspiel Tichu.                *
 *  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . *
 *                                                                            *
 *               (c) 2004-2005 by Martin Zangger, Roman Senn                  *
 *                                                                            *
 *    Dieses Programm ist freie Software. Sie können es unter den Bedingungen *
 * der GNU General Public License, wie von der Free Software Foundation ver-  *
 * öffentlicht, weitergeben und/oder modifizieren, entweder gemäss Version 2  *
 * der Lizenz oder (nach Ihrer Option) jeder späteren Version.                *
 *                                                                            *
 *    Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, dass es  *
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die  *
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BE-    *
 * STIMMTEN ZWECK. Details finden Sie in der GNU General Public License.      *
 *                                                                            *
 *    Sie sollten eine Kopie der GNU General Public License zusammen mit      *
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free     *
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          *
 * 02111-1307, USA.                                                           *
 * -------------------------------------------------------------------------- */

#include <libsgui/sgui.h>

#include "client.h"
#include "sound.h"
#include "net.h"
#include "ini.h"
#include "ui_config.h"
#include "ui_sound.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget    *ui_config_client_group;
sgWidget    *ui_config_client_user;
sgWidget    *ui_config_client_pass;
sgWidget    *ui_config_client_host;
sgWidget    *ui_config_client_port;
sgWidget    *ui_config_client_fullscreen;
sgWidget    *ui_config_client_grab;
//sgWidget    *ui_config_client_bpp;
sgWidget    *ui_config_client_resolution;
sgWidget    *ui_config_client_cursor;
sgWidget    *ui_config_client_pattern;
sgWidget    *ui_config_client_hue;
sgWidget    *ui_config_client_sat;
sgWidget    *ui_config_client_val;
sgWidget    *ui_config_client_contrast;

/* -------------------------------------------------------------------------- *
 * 'event handling' des [config] Dialoges                                     *
 * -------------------------------------------------------------------------- */
int ui_config_client_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);
  
  if(event == SG_SEL_CHANGE)
  {
    /* Wenn ein anderes Cursor-Theme gewählt wurde, dann laden wir
       dieses und wechseln das Theme on-the-fly, aber ohne es in der
       Konfiguration zu speichern */
    if(widget == ui_config_client_cursor)
    {
      sgDropdownItem *item;
      
      if((item = sgSelectedDropdownItem(ui_config_client_cursor)))
        sgSetDialogCursorTheme(ui_config_dialog, item->value);
    }
    
    /* Dasselbe wie mit dem Cursor machen wir auch mit dem Hintergrundmuster */
    if(widget == ui_config_client_pattern)
    {
      sgDropdownItem *item;
      
      if((item = sgSelectedDropdownItem(ui_config_client_pattern)))
        sgSetDialogPattern(ui_config_dialog, item->value);
    }
  }
  
  /* Wenn Hintergrundfarbe geändert hat, dann diese neu setzen */
  if(event == SG_EVENT_CHANGE)
  {
    /* Farbton hat geändert */
    if(widget == ui_config_client_hue)
    {
      sgColor color;
      int hue;
      
      color = sgGetColorSelRGB(ui_config_client_hue);
      hue = sgGetColorSelPos(ui_config_client_hue, NULL);
      sgSetWidgetColor(ui_config_dialog, color);
      
      if(ui_config_client_sat)
        sgSetColorSelHue(ui_config_client_sat, hue);
      if(ui_config_client_val)
        sgSetColorSelHue(ui_config_client_val, hue);
    }
    
    /* Sättigung hat geändert */
    if(widget == ui_config_client_sat)
    {
      sgColor color;
      int sat;
      
      color = sgGetColorSelRGB(ui_config_client_sat);
      sat = sgGetColorSelPos(ui_config_client_sat, NULL);
      sgSetWidgetColor(ui_config_dialog, color);
      
      if(ui_config_client_hue)
        sgSetColorSelSaturation(ui_config_client_hue, sat);
      if(ui_config_client_val)
        sgSetColorSelSaturation(ui_config_client_val, sat);
    }
    
    /* Helligkeit hat geändert */
    if(widget == ui_config_client_val)
    {
      sgColor color;
      int value;
      
      color = sgGetColorSelRGB(ui_config_client_val);
      value = sgGetColorSelPos(ui_config_client_val, NULL);
      sgSetWidgetColor(ui_config_dialog, color);
      
      if(ui_config_client_hue)
        sgSetColorSelValue(ui_config_client_hue, value);
      if(ui_config_client_sat)
        sgSetColorSelValue(ui_config_client_sat, value);
    }
    if(widget == ui_config_client_contrast)
    {
      sgSetDialogContrast(ui_config_dialog, sgGetAdjustValue(ui_config_client_contrast, NULL) * 255 / 100);
    }
  }

  return 0;  
}

/* -------------------------------------------------------------------------- *
 * Lädt alle Konfigurationsdaten in die Widgets für ui_config                 *
 * -------------------------------------------------------------------------- */
void ui_config_client_load(void)
{
  char portstr[6];
  sgPattern *pattern;
  sgCursorTheme *theme;
  int item;
  sgList *themes;
  
  /* Mögliche Video-Modes ins Dropdown */
  client_addmodes(ui_config_client_resolution);
  
  /* Farbtiefen */
/*  sgAddDropdownItem(ui_config_client_bpp, "Auto", (void *)0);
  sgAddDropdownItem(ui_config_client_bpp, "8-bit Palette", (void *)8);
  sgAddDropdownItem(ui_config_client_bpp, "15-bit HiColor", (void *)15);
  sgAddDropdownItem(ui_config_client_bpp, "16-bit HiColor", (void *)16);
  sgAddDropdownItem(ui_config_client_bpp, "24-bit TrueColor", (void *)24);
  sgAddDropdownItem(ui_config_client_bpp, "32-bit TrueColor", (void *)32);
  
  if((item = sgGetDropdownItemByValue(ui_config_client_bpp, (void *)(int)client_config.depth)) >= 0)
    sgSelectDropdownItem(ui_config_client_bpp, item);*/

  
  /* Dann die Hintergrundmuster aus sgUI auslesen */
  pattern = sgGetPatternTable();
  
  for(item = 0; pattern->fn; item++)
  {
    sgAddDropdownItem(ui_config_client_pattern, pattern->name, pattern);
    
    if(pattern == client_bgnd)
      sgSelectDropdownItem(ui_config_client_pattern, item);
    
    pattern++;
  }
  
  /* Hintergrundfarbe */
  sgSetColorSelHSV(ui_config_client_hue, client_config.bgcolor);
  sgSetColorSelHSV(ui_config_client_sat, client_config.bgcolor);
  sgSetColorSelHSV(ui_config_client_val, client_config.bgcolor);
  
  /* Kontrast */
  sgSetAdjustValue(ui_config_client_contrast, client_config.contrast * 100 / 255);
  
  /* ...und die Cursor Themes */
  themes = sgGetCursorThemes();
  
  item = 0;
  
  sgForeach(themes, theme)
  {
    sgAddDropdownItem(ui_config_client_cursor, theme->name, theme);
    
    if(theme == client_cursor)
      sgSelectDropdownItem(ui_config_client_cursor, item);
    
    item++;
  }
  

  /* Fullscreen modus? */
  if(client_config.fullscreen)
    sgSetToggleState(ui_config_client_fullscreen, 1);
  
  /* In die Netzwerk-Sektion der INI gehen und die Editboxen mit der 
     Server-Information füllen */
  net_configure(client_ini);

  snprintf(portstr, sizeof(portstr), "%u", net_port);
  
  sgSetWidgetCaption(ui_config_client_host, net_host);
  sgSetWidgetCaption(ui_config_client_port, portstr);
  
  /* Client-Information einfüllen */
  sgSetWidgetCaption(ui_config_client_user, client_config.user);
  sgSetWidgetCaption(ui_config_client_pass, client_config.pass);
}

/* -------------------------------------------------------------------------- *
 * Speichert alle Konfigurationsdaten aus den Widgets von ui_config           *
 * -------------------------------------------------------------------------- */
void ui_config_client_save(void)
{
  sgDropdownItem *item;

  /* Netzwerk-konfiguration schreiben */
  ini_section(client_ini, "net");

  ini_puts(client_ini, "host", ui_config_client_host->caption);
  ini_puts(client_ini, "port", ui_config_client_port->caption);
  
  /* client config */
  ini_section(client_ini, "client");
  
  if((item = sgSelectedDropdownItem(ui_config_client_resolution)))
  {
    ini_putulong(client_ini, "width", ((SDL_Rect *)item->value)->w);
    ini_putulong(client_ini, "height", ((SDL_Rect *)item->value)->h);
  }
  
/*  if((item = sgSelectedDropdownItem(ui_config_client_bpp)))
    ini_putulong(client_ini, "depth", (long)item->value);*/
  
  if((item = sgSelectedDropdownItem(ui_config_client_pattern)))
    ini_puts(client_ini, "pattern", item->caption);
  
  ini_puthsv(client_ini, "bgcolor", sgGetColorSelHSV(ui_config_client_hue));

  ini_putulong(client_ini, "contrast", sgGetAdjustValue(ui_config_client_contrast, NULL) * 255 / 100);
  
  
  if((item = sgSelectedDropdownItem(ui_config_client_cursor)))
    ini_puts(client_ini, "cursor", item->caption);
  
  ini_putulong(client_ini, "fullscreen", sgGetToggleState(ui_config_client_fullscreen));
  
  ini_puts(client_ini, "user", ui_config_client_user->caption);
  ini_puts(client_ini, "pass", ui_config_client_pass->caption);
}

/* -------------------------------------------------------------------------- *
 * Erstellt die Widgets des Client-Tabs in der Konfiguration                  *
 * -------------------------------------------------------------------------- */
void ui_config_client(sgWidget *group)
{
  Uint16 width;
  Uint16 height;
  
  /* Gruppen-Widgets */
  sgWidget *group_net;
  sgWidget *group_video;
  sgWidget *group_bgnd;
  sgWidget *group_sound;
  
  /* Ereignissbehandlungsroutine für diese Tab-Gruppe setzen */
  sgSetWidgetProc(ui_config_client_group, ui_config_client_proc);
  
  /* Sound-Konfiguration kriegt die ganze rechte Seite */
  group_video = sgNewGroupFull(ui_config_client_group, "Video");
  
  /* Netzwerk-Konfiguration ist links oben, Video unten */
  group_net = sgNewGroupSplitted(group_video, SG_EDGE_LEFT, 
                                 group_video->rect.w * 5 / 11, "Netzwerk");
  
  group_bgnd = sgNewGroupSplitted(group_net, SG_EDGE_BOTTOM,
                                  group_net->rect.h * 6 / 11, "Hintergrund");
  
  group_sound = sgNewGroupSplitted(group_video, SG_EDGE_BOTTOM,
                                   group_video->rect.h * 7 / 11, "Sound");
  
  /* Gruppenhintergrund auf Grau setzen -> gibt etwas besseren Kontrast */
/*  sgSetWidgetBorderRGB(group_video, 100, 100, 100);
  sgSetWidgetBorderRGB(group_sound, 100, 100, 100);
  sgSetWidgetBorderRGB(group_net, 100, 100, 100);
  sgSetWidgetBorderRGB(group_bgnd, 100, 100, 100);*/

  /* Alle Dropdowns und einen Toggle-Button in der Video-Gruppe erstellen */
  height = UI_EDIT_HEIGHT * 3;
  width = 0; //(sgGroup(group_video)->body.h - height) / 2;
  
  ui_config_client_fullscreen = 
    sgNewToggleGrouped(group_video, SG_EDGE_LEFT, SG_ALIGN_MIDDLE,
                       sgGroup(group_video)->body.w - width, height, "Vollbild");
  
/*  sgSetWidgetPos(ui_config_client_fullscreen, sgGroup(group_video)->body.x + width / 2,  ui_config_client_fullscreen->rect.y,
                 SG_ALIGN_LEFT|SG_ALIGN_TOP);*/
  
  ui_config_client_resolution = 
    sgNewDropdownSplitted(ui_config_client_fullscreen, SG_EDGE_TOP, UI_EDIT_HEIGHT);
/*  ui_config_client_bpp = 
    sgNewDropdownSplitted(ui_config_client_fullscreen, SG_EDGE_TOP, UI_EDIT_HEIGHT);*/
  ui_config_client_cursor =
    sgNewDropdownSplitted(ui_config_client_fullscreen, SG_EDGE_TOP, UI_EDIT_HEIGHT);
  
  /* Die Dropdowns splitten und ein Label anhängen 
     sowie einen weiteren Toggle-Button */
  sgNewLabelSplitted(ui_config_client_resolution, SG_EDGE_LEFT, 
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Auflösung");
/*  sgNewLabelSplitted(ui_config_client_bpp, SG_EDGE_LEFT, 
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Farbtiefe");*/
  sgNewLabelSplitted(ui_config_client_cursor, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Mauszeiger");
  
  ui_config_client_grab =
    sgNewToggleSplitted(ui_config_client_fullscreen, SG_EDGE_RIGHT, 
                        ui_config_client_fullscreen->rect.w / 2, "Maus fangen");

  
  /* Dropdowns auf die richtige Maximalgrösse setzen */
  sgSetWidgetSize(ui_config_client_resolution, 
                  ui_config_client_resolution->rect.w, UI_DROPDOWN_HEIGHT);
/*  sgSetWidgetSize(ui_config_client_bpp, 
                  ui_config_client_resolution->rect.w, UI_DROPDOWN_HEIGHT);*/
  sgSetWidgetSize(ui_config_client_cursor, 
                  ui_config_client_resolution->rect.w, UI_DROPDOWN_HEIGHT);  
  
  /* Das Dropdown für das Muster und die Farbregler für den Hintergrund */
  height = UI_COLOR_HEIGHT * 3 + UI_EDIT_HEIGHT * 2;
  width = (sgGroup(group_bgnd)->body.h - height) / 2;
  
  ui_config_client_hue = sgNewColorSelGrouped(group_bgnd, SG_EDGE_LEFT, SG_ALIGN_MIDDLE,
                                       sgGroup(group_bgnd)->body.w - width, height,
                                       SG_COLORSEL_HUE, NULL);
  ui_config_client_contrast =
    sgNewAdjustSplitted(ui_config_client_hue, SG_EDGE_BOTTOM, UI_EDIT_HEIGHT,
                        0, 100, NULL);
  sgSetAdjustFormat(ui_config_client_contrast, "%.0lf%%");
  
  ui_config_client_val =
    sgNewColorSelSplitted(ui_config_client_hue, SG_EDGE_BOTTOM, UI_COLOR_HEIGHT,
                          SG_COLORSEL_VALUE, NULL);

  ui_config_client_sat =
    sgNewColorSelSplitted(ui_config_client_hue, SG_EDGE_BOTTOM, UI_COLOR_HEIGHT,
                          SG_COLORSEL_SATURATION, NULL);
  
  ui_config_client_pattern =
    sgNewDropdownSplitted(ui_config_client_hue, SG_EDGE_TOP, UI_EDIT_HEIGHT);
  
  
  sgNewLabelSplitted(ui_config_client_pattern, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Muster");
  sgNewLabelSplitted(ui_config_client_hue, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Farbton");
  sgNewLabelSplitted(ui_config_client_sat, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Sättigung");
  sgNewLabelSplitted(ui_config_client_val, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Wert");
  sgNewLabelSplitted(ui_config_client_contrast, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Kontrast");
  
  sgSetWidgetSize(ui_config_client_pattern, 
                  ui_config_client_pattern->rect.w, UI_DROPDOWN_HEIGHT);
  
  /* Editierbare Felder für die Client-Identifikation/Netzwerkkonfiguration */
  height = UI_EDIT_HEIGHT * 4;
  width = (sgGroup(group_net)->body.h - height) / 2;
  
  ui_config_client_port =
    sgNewEditGrouped(group_net, SG_EDGE_LEFT, SG_ALIGN_MIDDLE|SG_ALIGN_CENTER,
                     sgGroup(group_net)->body.w - width, height, NULL);
  
  sgSetWidgetPos(ui_config_client_port, width / 2,  ui_config_client_port->rect.y,
                 SG_ALIGN_LEFT|SG_ALIGN_TOP);
  
  ui_config_client_user =
    sgNewEditSplitted(ui_config_client_port, SG_EDGE_TOP, UI_EDIT_HEIGHT, NULL);
  ui_config_client_pass =
    sgNewEditSplitted(ui_config_client_port, SG_EDGE_TOP, UI_EDIT_HEIGHT, NULL);
  ui_config_client_host =
    sgNewEditSplitted(ui_config_client_port, SG_EDGE_TOP, UI_EDIT_HEIGHT, NULL);

  /* Und dann die Felder beschriften */
  sgNewLabelSplitted(ui_config_client_user, SG_EDGE_LEFT, 
                     80, SG_ALIGN_CENTER, "Benutzer");
  sgNewLabelSplitted(ui_config_client_pass, SG_EDGE_LEFT, 
                     80, SG_ALIGN_CENTER, "Passwort");
  sgNewLabelSplitted(ui_config_client_host, SG_EDGE_LEFT,
                     80, SG_ALIGN_CENTER, "Server");
  sgNewLabelSplitted(ui_config_client_port, SG_EDGE_LEFT, 
                     80, SG_ALIGN_CENTER, "Port");
  
  /* Sound-Player in die Sound-Gruppe */
  ui_sound(group_sound);
  
  sgSetWidgetColor(group_video, ui_group_color);
  sgSetWidgetColor(group_sound, ui_group_color);
  sgSetWidgetColor(group_net, ui_group_color);
  sgSetWidgetColor(group_bgnd, ui_group_color);
}

