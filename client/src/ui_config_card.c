/* $Id: ui_config_card.c,v 1.8 2005/05/22 01:25:47 smoli Exp $
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

#include "ini.h"
#include "card.h"
#include "dlink.h"
#include "ui_config.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget    *ui_config_card_group;
sgWidget    *ui_config_card_hue;            /* Farbton */
sgWidget    *ui_config_card_sat;            /* Sättigung */
sgWidget    *ui_config_card_val;            /* Wert */
sgWidget    *ui_config_card_border;         /* Rahmenstärke */
sgWidget    *ui_config_card_transparency;   /* Transparenz */
sgWidget    *ui_config_card_zoom;           /* Zoomfaktor */
sgWidget    *ui_config_card_select;         /* Auswählen */
sgWidget    *ui_config_card_hilite;         /* Hervorheben */
sgWidget    *ui_config_card_pick;           /* andere Karte ziehen */
sgWidget    *ui_config_card_image;          /* Vorschau */
Uint32       ui_config_card_last;           /* Zeitpunkt der letzten Vorschau */
struct card *ui_config_card_card;           /* in der Vorschau gezeigte Karte */
int          ui_config_card_redraw;         /* Vorschau neu zeichnen? */
Uint32       ui_config_card_counter;        /* Zeitzähler für Kartenrotation */
double       ui_config_card_angle;          /* Kartenwinkel in der Vorschau */
SDL_Rect     ui_config_card_preview_rect;

/* -------------------------------------------------------------------------- *
 * Erstellt die Widgets des Karten-Tabs in der Konfiguration                  *
 * -------------------------------------------------------------------------- */
void ui_config_card(sgWidget *group)
{
  sgWidget *group_effects;
  sgWidget *group_color;
  
  /* Ereignissbehandlungsroutine für den Kartenkonfigurations-Dialog */
  sgSetWidgetProc(group, ui_config_card_proc);
  
  /* Ein Bild-Widget zeigt die Vorschau einer Karte an */
  ui_config_card_image = sgNewImageRect(group, sgGroup(group)->body, "Vorschau");
  
  group_color =
    sgNewGroupSplitted(ui_config_card_image, SG_EDGE_LEFT,
                       ui_config_card_image->rect.w * 2 / 5, "Farbe");
  
  group_effects =
    sgNewGroupSplitted(group_color, SG_EDGE_BOTTOM,
                       group_color->rect.h / 2, "Effekte");
  
  /* Mit diesem Button kann eine andere Vorschaukarte gewählt werden */
  ui_config_card_pick =
    sgNewButtonSplitted(ui_config_card_image, SG_EDGE_BOTTOM,
                        UI_BUTTON_HEIGHT, "andere Karte");
  
  /* Mit diesen Knöpfen kann die Karte in den 
     selected/hilited Modus geschaltet werden */
  ui_config_card_select = 
    sgNewToggleSplitted(ui_config_card_pick, SG_EDGE_LEFT,
                        ui_config_card_pick->rect.w * 2 / 3, "Ausgewählt");
  
  sgSetToggleState(ui_config_card_select, 1);
  
  sgPadRect(&ui_config_card_select->rect, SG_EDGE_TOP|SG_EDGE_BOTTOM,
            (ui_config_card_pick->rect.h - UI_EDIT_HEIGHT) / 2);
  sgRecalcWidget(ui_config_card_pick);
  
  ui_config_card_hilite = 
    sgNewToggleSplitted(ui_config_card_select, SG_EDGE_RIGHT,
                        ui_config_card_select->rect.w / 2, "Hervorgehoben");
  
  /* Farbregler erstellen */
  ui_config_card_hue =
    sgNewColorSelAligned(group_color, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                         sgGroup(group_color)->body.w,
                         UI_COLOR_HEIGHT, SG_COLORSEL_HUE, NULL); 
  ui_config_card_sat =
    sgNewColorSelAligned(group_color, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                         sgGroup(group_color)->body.w,
                         UI_COLOR_HEIGHT, SG_COLORSEL_SATURATION, NULL); 
  ui_config_card_val =
    sgNewColorSelAligned(group_color, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                         sgGroup(group_color)->body.w,
                         UI_COLOR_HEIGHT, SG_COLORSEL_VALUE, NULL);
 
  /* ...und diese dann Beschriften */
  sgNewLabelSplitted(ui_config_card_hue, SG_EDGE_LEFT, 64, SG_ALIGN_CENTER, "Farbton");
  sgNewLabelSplitted(ui_config_card_sat, SG_EDGE_LEFT, 64, SG_ALIGN_CENTER, "Sättigung");
  sgNewLabelSplitted(ui_config_card_val, SG_EDGE_LEFT, 64, SG_ALIGN_CENTER, "Helligkeit");
  
  /* Regler für die Transparenz */
  ui_config_card_transparency =
    sgNewAdjustAligned(group_effects, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                       sgGroup(group_effects)->body.w,
                       28, 0, 100, NULL);
  
  sgSetAdjustFormat(ui_config_card_transparency, "%.0lf%%");

  /* Regler für die Rahmenstärke */
  ui_config_card_border =
    sgNewAdjustAligned(group_effects, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                       sgGroup(group_effects)->body.w,
                       28, 0.1, 5.0, NULL);
  
  sgSetAdjustFormat(ui_config_card_border, "%.1lfpt");
  
  /* Regler für den Zoomfaktor */
  ui_config_card_zoom =
    sgNewAdjustAligned(group_effects, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                       sgGroup(group_effects)->body.w,
                       28, 0.5, 2.5, NULL);
  
  sgSetAdjustFormat(ui_config_card_zoom, "x%.2lf");
  
  /* auch hier wieder Labels machen */
  sgNewLabelSplitted(ui_config_card_transparency, SG_EDGE_LEFT, 
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Transparenz");
  sgNewLabelSplitted(ui_config_card_border, SG_EDGE_LEFT, 
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Rahmenstärke");
  sgNewLabelSplitted(ui_config_card_zoom, SG_EDGE_LEFT,
                     UI_LABEL_WIDTH, SG_ALIGN_CENTER, "Zoomfaktor");
  
  /* Gruppen in grau */
  sgSetWidgetColor(group_effects, ui_group_color);
  sgSetWidgetColor(group_color, ui_group_color);
  sgSetWidgetColor(ui_config_card_image, ui_group_color);
}

/* -------------------------------------------------------------------------- *
 * Behandelt alle Ereignisse innerhalb der Kartenkonfiguration                *
 * -------------------------------------------------------------------------- */
int ui_config_card_proc(sgWidget *widget, sgEvent event)
{
  /* Allgemeine Ereignisbehandlung (für sound und so) */
  ui_generic_proc(widget, event);
  
  if(event == SG_EVENT_CHANGE)
  {
    /* Wenn die Rahmenstärke geändert hat, dann den Wert temporär in
       die Kartenconfig schreiben und die Vorschau neu zeichnen */
    if(widget == ui_config_card_border)
    {
      card_config.border = sgGetAdjustValue(ui_config_card_border, NULL) * 255 / 5;
      ui_config_card_redraw |= CARD_RENDER;
    }
    
    /* Wenn der Zoomfaktor geändert hat, dann den Wert temporär in 
       die Kartenconfig schreiben und die Vorschau neu zeichnen */
    if(widget == ui_config_card_zoom)
    {
      card_config.zoom = sgGetAdjustValue(ui_config_card_zoom, NULL);
      ui_config_card_redraw |= CARD_RENDER;
    }
    
    /* Wenn Kartenfarbe geändert hat, dann diese neu setzen: */
    /* Farbton hat geändert */
    if(widget == ui_config_card_hue)
    {
      sgColor color;      
      sgHSV hsv;
      
      hsv = sgGetColorSelHSV(ui_config_card_hue);
      color = sgHSVToRGB(hsv);
      
      card_tint.r = color.r;
      card_tint.g = color.g;
      card_tint.b = color.b;
      
      ui_config_card_redraw |= CARD_RENDER;
      
      if(ui_config_card_sat)
        sgSetColorSelHue(ui_config_card_sat, hsv.h);
      if(ui_config_card_val)
        sgSetColorSelHue(ui_config_card_val, hsv.h);
    }

    /* Sättigung hat geändert */
    if(widget == ui_config_card_sat)
    {
      sgColor color;
      sgHSV hsv;
      
      hsv = sgGetColorSelHSV(ui_config_card_sat);
      color = sgHSVToRGB(hsv);
      
      card_tint.r = color.r;
      card_tint.g = color.g;
      card_tint.b = color.b;
      
      ui_config_card_redraw |= CARD_RENDER;
      
      if(ui_config_card_hue)
        sgSetColorSelSaturation(ui_config_card_hue, hsv.s);
      if(ui_config_card_val)
        sgSetColorSelSaturation(ui_config_card_val, hsv.s);
    }
    
    /* Helligkeit hat geändert */
    if(widget == ui_config_card_val)
    {
      sgColor color;
      sgHSV hsv;
      
      hsv = sgGetColorSelHSV(ui_config_card_val);
      color = sgHSVToRGB(hsv);
      
      card_tint.r = color.r;
      card_tint.g = color.g;
      card_tint.b = color.b;
      
      ui_config_card_redraw |= CARD_RENDER;
      
      if(ui_config_card_hue)
        sgSetColorSelValue(ui_config_card_hue, hsv.v);
      if(ui_config_card_sat)
        sgSetColorSelValue(ui_config_card_sat, hsv.v);
    }
    
    /* Transparenz hat geändert */
    if(widget == ui_config_card_transparency)
    {
      card_tint.a = 255 - (sgGetAdjustValue(ui_config_card_transparency, NULL) * 255 / 100);
      
      ui_config_card_redraw |= CARD_RENDER;
    }
  }

  if(event == SG_EVENT_CLICK)
  {
    /* ..andere Karte wurde verlangt? */
    if(widget == ui_config_card_pick)
    {
      struct list list;
      
      /* Eine Liste mit der aktuellen Karte machen, 
         damit diese nicht wieder gewählt wird */
      dlink_list_zero(&list);
      dlink_add_head(&list, dlink_node_new(), ui_config_card_card);
      
      /* Alte Kartendaten freigeben */
      card_clean(ui_config_card_card);
      
      /* Neue Karte ziehen und redraw */
      ui_config_card_card = card_random(&list);
      ui_config_card_redraw |= CARD_RENDER;
      
      /* Liste wieder freigeben */
      dlink_destroy(&list);
    }
    
    /* Karte hervorheben in der Vorschau? */
    if(widget == ui_config_card_hilite)
      ui_config_card_redraw |= CARD_RENDER;
    
    /* Karte auswählen in der Vorschau? */
    if(widget == ui_config_card_select)
      ui_config_card_redraw |= CARD_RENDER;
  }
  
    
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ui_config_card_show(void)
{
  /* Wenn wir noch keine Karte in der Vorschau haben, 
     dann ziehen wir eine zufällige */
  if(ui_config_card_card == NULL)
    ui_config_card_card = card_random(NULL);
 
  /* Wenn eine Karte da ist, dann die Vorschau zeichnen */
  if(ui_config_card_card)
  {
    SDL_Surface *tmp;
    
    /* Ausgewählt/Hervorgehoben Status setzen */
    ui_config_card_card->status = 
      (sgGetToggleState(ui_config_card_hilite) ? CARD_HILITE : 0) | 
      (sgGetToggleState(ui_config_card_select) ? CARD_SEL : 0);
    
    /* Vorschau generieren */
    tmp = card_preview(ui_config_card_preview_rect, 
                       ui_config_card_card, 
                       ui_config_card_angle);

    sgSetImageSurface(ui_config_card_image, tmp, SG_ALIGN_CENTER);
    SDL_FreeSurface(tmp);
  }
}

/* -------------------------------------------------------------------------- *
 * Rotiert die Kartenvorschau und zeichnet sie neu, falls nötig               *
 * -------------------------------------------------------------------------- */
int ui_config_card_preview(int delay)
{
  /* Nur rotieren wenn das Image Widget Fokus hat (?!?) */
//  if(sgHasWidgetFocus(ui_config_card_image))
  {
    double newangle;
    
    /* Vergangene Zeit addieren */
    Uint32 now = SDL_GetTicks();
    ui_config_card_counter += delay;
    ui_config_card_last = now;
    
    /* Linear dazu einen Winkel berechnen */
    newangle = (double)(ui_config_card_counter % 10000) / ((double)10000 / (double)360);
    
    /* Auf eine Kommastelle genau vergleichen */
    if((int)(newangle * 10) != (int)(ui_config_card_angle * 10))
    {
      ui_config_card_angle = newangle;
      ui_config_card_redraw++;
    }
  }

  /* Muss die Kartenvorschau neu gezeichnet werden? */
  if(ui_config_card_redraw)
  {
    ui_config_card_show();
    ui_config_card_redraw = 0;
  }
  
  return 0;
  
}

/* -------------------------------------------------------------------------- *
 * Lädt alle Konfigurationsdaten in die Widgets für ui_config                 *
 * -------------------------------------------------------------------------- */
void ui_config_card_load(void)
{
  /* Kartentönung aus Konfiguration lesen */
  card_configure(client_ini);
  
  sgSetColorSelHSV(ui_config_card_hue, card_config.color);
  sgSetColorSelHSV(ui_config_card_sat, card_config.color);
  sgSetColorSelHSV(ui_config_card_val, card_config.color);
  
  /* Transparenz */
  sgSetAdjustValue(ui_config_card_transparency,
                   (double)(255 - card_tint.a) * 100 / 255);
  /* Zoom */
  sgSetAdjustValue(ui_config_card_zoom, 
                   (double)card_config.zoom);
  /* Rahmendicke */
  sgSetAdjustValue(ui_config_card_border, 
                   (double)card_config.border * 5 / 255);
}

/* -------------------------------------------------------------------------- *
 * Speichert alle Konfigurationsdaten aus den Widgets von ui_config           *
 * -------------------------------------------------------------------------- */
void ui_config_card_save(void)
{
  ini_section(client_ini, "card");

  /* Farbe und Transparenz speichern */
  ini_puthsv(client_ini, "color", sgGetColorSelHSV(ui_config_card_hue));
  ini_putulong(client_ini, "alpha", card_tint.a);
  
  /* Effekte */
  ini_putdouble(client_ini, "zoom", card_config.zoom);
  ini_putlong(client_ini, "border", card_config.border);  
}

