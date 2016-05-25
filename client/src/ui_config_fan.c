/* $Id: ui_config_fan.c,v 1.6 2005/05/21 10:09:34 smoli Exp $
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
#include "fan.h"
#include "ini.h"
#include "ui_config.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget    *ui_config_fan_group;
sgWidget    *ui_config_fan_image;
sgWidget    *ui_config_fan_bend;
sgWidget    *ui_config_fan_spacing;
sgWidget    *ui_config_fan_raise;
struct list  ui_config_fan_cards;
Uint32       ui_config_fan_last;
int          ui_config_fan_redraw;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ui_config_fan_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);
  
  if(event == SG_EVENT_CHANGE)
  {
    if(widget == ui_config_fan_spacing)
    {
      fan_config.spacing = sgGetAdjustValue(ui_config_fan_spacing, NULL);
      ui_config_fan_redraw = 1;
    }
    
    if(widget == ui_config_fan_bend)
    {
      fan_config.bend = sgGetAdjustValue(ui_config_fan_bend, NULL);
      ui_config_fan_redraw = 1;
    }
  }
  
  return 0;  
}
  
/* -------------------------------------------------------------------------- *
 * Starte die Vorschau in der Fächerkonfiguration                             *
 * -------------------------------------------------------------------------- */
void ui_config_fan_show(void)
{
  SDL_Surface *tmp;
  
  /* Wenn wir noch keine 14 Karten in der Vorschau haben,
     dann ziehen wir bis wir 14 haben eine zufällige */
  while(ui_config_fan_cards.size < 14)
  {
    struct card *card;
    card = card_random(&ui_config_fan_cards);
    dlink_add_head(&ui_config_fan_cards, dlink_node_new(), card);
  }

  /* Vorschau generieren */  
  tmp = fan_preview(ui_config_dialog->face.frame, 
                    sgImage(ui_config_fan_image)->body,
                    &ui_config_fan_cards);
  
  /* Die Vorschau in das Widget kopieren und freigeben */
  sgSetImageSurface(ui_config_fan_image, tmp, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  SDL_FreeSurface(tmp);

  /* Redraw-Flag zurücksetzen */
  ui_config_fan_redraw = 0;
}

/* -------------------------------------------------------------------------- *
 * Starte die Vorschau im [config] Dialog                                    *
 * -------------------------------------------------------------------------- */
void ui_config_fan_preview(int delay)
{
  if(ui_config_fan_redraw)
    ui_config_fan_show();
}

/* -------------------------------------------------------------------------- *
 * Lädt alle Konfigurationsdaten in die Widgets für ui_config                 *
 * -------------------------------------------------------------------------- */
void ui_config_fan_load(void)
{
  /* Biegungsquotient */
  sgSetAdjustValue(ui_config_fan_bend, fan_config.bend);
  
  /* Kartenabstand */
  sgSetAdjustValue(ui_config_fan_spacing, fan_config.spacing);
}

/* -------------------------------------------------------------------------- *
 * Speichert alle Konfigurationsdaten aus den Widgets von ui_config           *
 * -------------------------------------------------------------------------- */
void ui_config_fan_save(void)
{
}

/* -------------------------------------------------------------------------- *
 * Erstellt die Widgets des Fächer-Tabs in der Konfiguration                  *
 * -------------------------------------------------------------------------- */
void ui_config_fan(sgWidget *group)
{
  sgWidget *settings;
  
  ui_config_fan_redraw = 1;
  
  sgSetWidgetProc(group, ui_config_fan_proc);
    
  /* fan group */
  ui_config_fan_image =
    sgNewImageRect(ui_config_fan_group, sgGroup(group)->body, "Vorschau");
  
  settings = sgNewGroupSplitted(ui_config_fan_image, SG_EDGE_LEFT,
                                ui_config_fan_image->rect.w * 2 / 5, 
                                "Einstellungen");
  
  /* Krümmungsquotient */
  ui_config_fan_bend = sgNewAdjustAligned(settings, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                       sgGroup(settings)->body.w, UI_EDIT_HEIGHT, 0.01, 1.0, NULL);
  sgSetAdjustFormat(ui_config_fan_bend, "÷%.2lf");
  /* Kartenabstände */
  ui_config_fan_spacing = sgNewAdjustAligned(settings, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                          sgGroup(settings)->body.w, UI_EDIT_HEIGHT, 1, 50, NULL);
  /* Amplitude */
  ui_config_fan_raise = sgNewAdjustAligned(settings, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                        sgGroup(settings)->body.w, UI_EDIT_HEIGHT, 0, 100, NULL);
  
  sgNewLabelSplitted(ui_config_fan_bend, SG_EDGE_LEFT, UI_LABEL_WIDTH, 
                     SG_ALIGN_CENTER, "Krümmung");
  sgNewLabelSplitted(ui_config_fan_spacing, SG_EDGE_LEFT, UI_LABEL_WIDTH,
                     SG_ALIGN_CENTER, "Abstand");
  sgNewLabelSplitted(ui_config_fan_raise, SG_EDGE_LEFT, UI_LABEL_WIDTH, 
                     SG_ALIGN_CENTER, "Anheben");
  
  sgSetWidgetColor(settings, ui_group_color);
  
  /* Preview vorbereiten */
  
  ui_config_resize(ui_config_fan_image, 4.0 / 3.0);
  
  fan_init();
  
}

