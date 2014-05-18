/* $Id: dnd.c,v 1.26 2005/05/21 08:27:20 smoli Exp $
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

#include <SDL.h>

/* -------------------------------------------------------------------------- *
 * Das DND Modul erledigt das Bewegen von Karten zwischen verschiedenen       *
 * Modulen (FAN, STACK, PLAYER)                                               *
 * -------------------------------------------------------------------------- */
#include "dnd.h"
#include "fan.h"
#include "stack.h"
#include "player.h"

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
struct list     dnd_list;         /* Liste aller gedraggten Karten */
int             dnd_status;
struct position dnd_origin;        /* DnD Startposition */
struct position dnd_rel;          /* Relative Mausposition zum Start */
int             dnd_mod;          /* Modul aus dem das Draggen gestartet hat */
int             dnd_target;       /* Modul in dem das Draggen enden soll */

/* -------------------------------------------------------------------------- *
 * Initialisiert Drag & Dropping                                              *
 * -------------------------------------------------------------------------- */
void dnd_init(void)
{
  dlink_list_zero(&dnd_list);
  
  dnd_mod = -1;
  dnd_status = DND_HIDDEN|DND_LOCKED|DND_FROZEN;
}

/* -------------------------------------------------------------------------- *
 * Gibt die Drag&Drop-Ressourcen wieder frei                                  *
 * -------------------------------------------------------------------------- */
void dnd_shutdown(void)
{
  dnd_mod = -1;
}  

/* -------------------------------------------------------------------------- *
 * Setzt Drag & Drop Statusflags                                              *
 * -------------------------------------------------------------------------- */
int dnd_set(int flags)
{
  /* Müssen wir noch flags setzen? */
  if((dnd_status & flags) != flags)
  {
    dnd_debug(VERBOSE, "Set [%s%s%s%s%s ] (%s%s%s%s%s )",
              ((~dnd_status) & flags & DND_REDRAW) ? " REDRAW" : "",
              ((~dnd_status) & flags & DND_LOCKED) ? " LOCKED" : "",
              ((~dnd_status) & flags & DND_HIDDEN) ? " HIDDEN" : "",
              ((~dnd_status) & flags & DND_FROZEN) ? " FROZEN" : "",
              ((~dnd_status) & flags & DND_RETURN) ? " RETURN" : "",
              ((dnd_status | flags) & DND_REDRAW) ? " REDRAW" : "",
              ((dnd_status | flags) & DND_LOCKED) ? " LOCKED" : "",
              ((dnd_status | flags) & DND_HIDDEN) ? " HIDDEN" : "",
              ((dnd_status | flags) & DND_FROZEN) ? " FROZEN" : "",
              ((dnd_status | flags) & DND_RETURN) ? " RETURN" : "");

    /* Redraw ist an Fächer gekoppelt */
    if(flags & DND_REDRAW)
      fan_set(FAN_REDRAW);
    
    /* Flags setzen */
    dnd_status |= flags;
  
    return 1;
  }

  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Löscht Drag & Drop Statusflags                                             *
 * -------------------------------------------------------------------------- */
int dnd_unset(int flags)
{
  /* Müssen wir noch Flags löschen? */
  if(dnd_status & flags)
  {
    dnd_debug(VERBOSE, "Unset [%s%s%s%s%s ] (%s%s%s%s%s )",
              (dnd_status & flags & DND_REDRAW) ? " REDRAW" : "",
              (dnd_status & flags & DND_LOCKED) ? " LOCKED" : "",
              (dnd_status & flags & DND_HIDDEN) ? " HIDDEN" : "",
              (dnd_status & flags & DND_FROZEN) ? " FROZEN" : "",
              (dnd_status & flags & DND_RETURN) ? " RETURN" : "",
              (dnd_status & (~flags) & DND_REDRAW) ? " REDRAW" : "",
              (dnd_status & (~flags) & DND_LOCKED) ? " LOCKED" : "",
              (dnd_status & (~flags) & DND_HIDDEN) ? " HIDDEN" : "",
              (dnd_status & (~flags) & DND_FROZEN) ? " FROZEN" : "",
              (dnd_status & (~flags) & DND_RETURN) ? " RETURN" : "");

    /* Flags löschen */
    dnd_status &= ~flags;
  
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Prüfe ob Drag & Drop ausgehend vom angegebenen Modus (vorläufig eigentlich * 
 * immer CARD_FAN) gestartet werden kann.                                     *
 *                                                                            *
 * DnD kann nicht gestartet werden wenn es bereits laufend ist oder wenn die  *
 * Distanz von der Startposition zur aktuellen zu klein ist.                  *
 * -------------------------------------------------------------------------- */
int dnd_start(int mod, int tgt, int x, int y)
{
  if(dnd_mod == -1)
  {
    /* Checken ob genügend Distanz zurückgelegt wurde */
    int dist = gfxutil_dist(dnd_origin.x - x, dnd_origin.y - y);
    
    dnd_debug(INFO, "DnD Distanz: %i, Trigger: %i", dist, DND_DISTANCE);
    
    if(dist >= DND_DISTANCE)
    {
      /* DnD initialisation */
      dnd_mod = mod;
      dnd_target = tgt;
      
      dnd_set(DND_REDRAW);
      dnd_unset(DND_HIDDEN|DND_LOCKED);
      
      /* Fächer-Bewegungen erstmal stoppen */
//      fan_set(FAN_FROZEN);
      
      dnd_debug(INFO, "DnD Modus aktiviert (%s -> %s)",
                card_mods[dnd_mod], card_mods[dnd_target]);
      
      return 1;
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Leitet einen Stop des Drag & Drops ein und gibt 1 zurück wenn der Modus    *
 * komplett deaktiviert ist                                                   *
 * -------------------------------------------------------------------------- */
int dnd_stop(void)
{
  struct card *card;
  struct node *node;

  if(dnd_mod == -1)
    return 1;
  
  /* Stop noch nicht eingeleitet? */
  if((dnd_status & DND_RETURN) == 0)
  {
    dnd_debug(INFO, "%u Karte%s zurückdraggen", 
              dnd_list.size, (dnd_list.size != 1 ? "n" : ""));
    
    /* Alle Karten an die Ausgangsposition zurück */
    dlink_foreach_data(&dnd_list, node, card)
    {
      card_setvector(card, CARD_DND,
                     card->is[CARD_DND].move.now,
                     card->is[CARD_DND].rotozoom.now);
      
      card_move(card, CARD_DND, 
                card->is[dnd_mod].move.now.x,
                card->is[dnd_mod].move.now.y, DND_SPEED);
      
      card_rotate(card, CARD_DND, card->is[dnd_mod].rotozoom.now.r);
      
      /* Wenn das Ziel der Stack ist, dann aus der Stack-Liste nehmen */
/*      if(dnd_target == CARD_STACK)
        stack_remove(card);*/
    }
    
    dnd_unset(DND_FROZEN);
    dnd_set(DND_LOCKED|DND_RETURN);
  }
  
  
  /* Ist noch eine Karte in Bewegung? */
  dlink_foreach_data(&dnd_list, node, card)
  {
    if(card->is[CARD_DND].scale.now < card->is[CARD_DND].scale.end)
      return 0;
  }

  dnd_debug(INFO, "DnD Modus deaktiviert");
  
  /* Drag & Drop Modus komplett ausschalten */
  dnd_mod = -1;
  
  dnd_set(DND_DISABLED);
  dnd_unset(DND_RETURN);
  
  stack_unset(STACK_DRAG);
  
  /* Fächer reaktivieren */
  fan_unset(FAN_DISABLED|FAN_DRAG);
  
  dlink_list_zero(&dnd_list);
  
  return 1;
}
  
/* -------------------------------------------------------------------------- *
 * Fügt eine Karte hinzu                                                      *
 * -------------------------------------------------------------------------- */
void dnd_add(struct card *card)
{
  /* Auf die Drag&Drop-Liste damit */
  dlink_add_tail(&dnd_list, &card->is[CARD_DND].node, card);
    
  /* Winkel und Position des Ausgangsmodus übernehmen */
  card_copyinst(card, dnd_mod, CARD_DND);

  dnd_debug(INFO, "Karte %s Ausgangsposition: %i|%i, Endposition: %i|%i",
            card->name, 
            (int)card->is[CARD_DND].move.start.x,
            (int)card->is[CARD_DND].move.start.y,
            (int)card->is[CARD_DND].move.end.x,
            (int)card->is[CARD_DND].move.end.y);
  
  /* Redrawen */
  dnd_set(DND_REDRAW);
}

/* -------------------------------------------------------------------------- *
 * Nimmt eine Karte von der DnD Liste                                         *
 * -------------------------------------------------------------------------- */
void dnd_remove(struct card *card)
{
  dlink_delete(&dnd_list, &card->is[CARD_DND].node);

  /* Instanz-Surface freigeben */
  card_freeinst(card, CARD_DND);
  
  /* Letzte Karte entfernt? */
  if(dnd_list.size == 0)
    dnd_stop();

  dnd_debug(INFO, "Karte %s entfernt", card->name);
}

/* -------------------------------------------------------------------------- *
 * Position für eine Karte kalkulieren                                        *
 * -------------------------------------------------------------------------- */
void dnd_calc(struct card *card, int dragx, int dragy)
{
  int rx, ry;
  
  if(dnd_target == CARD_PLAYER)
  {
    float angle;
    
    /* Ursprung der Karte relativ zum DnD-Start berechnen */
    rx = card->is[dnd_mod].move.now.x - dnd_origin.x;
    ry = card->is[dnd_mod].move.now.y - dnd_origin.y;
    
    /* Relative Dragposition dazurechnen und dort hin schieben */
    if(card_setpos(card, CARD_DND,  dragx + rx,  dragy + ry) & CARD_REDRAW)
      dnd_set(DND_REDRAW);
    
    rx = dragx - dnd_origin.x;
    ry = dragy - dnd_origin.y;
    
    /* Näherungswerte der Dragposition zu den Schupfkreisen und dem Ausgangs-
       punkt berechnen und dann die Winkel dieser Positionen relativieren um
       den Winkel der gedraggten Karte zu erhalten */
    angle = player_dragangle(dragx, dragy, gfxutil_dist(rx, ry), 
                             card->is[dnd_mod].rotozoom.now.r);
   
    if(card_setangle(card, CARD_DND, angle) & CARD_REDRAW)
      dnd_set(DND_REDRAW);
  }
  
  
  if(dnd_target == CARD_STACK)
  {
    int tdist, cdist, pdist;
    int step;

//    dnd_debug(INFO, "calc");
    
    /* Anfangsposition relativ zum Start machen */
    card->is[CARD_DND].move.start.x = card->is[dnd_mod].move.now.x - dnd_origin.x;
    card->is[CARD_DND].move.start.y = card->is[dnd_mod].move.now.y - dnd_origin.y;
    card->is[CARD_DND].rotozoom.start.r = card->is[dnd_mod].rotozoom.now.r;

//    dnd_debug(INFO, "startrel %i|%i",card->is[CARD_DND].move.start.x, card->is[CARD_DND].move.start.y);
    
    /* Endposition relativ zum Stapel machen */
    card->is[CARD_DND].move.end.x = card->is[dnd_target].move.end.x - stack_pos.x;
    card->is[CARD_DND].move.end.y = card->is[dnd_target].move.end.y - stack_pos.y;
    card->is[CARD_DND].rotozoom.end.r = card->is[dnd_target].rotozoom.end.r;

    
  dnd_debug(INFO, "stackpos %i|%i",card->is[CARD_STACK].move.end.x, card->is[CARD_STACK].move.end.y);
  dnd_debug(INFO, "stackrel %i|%i",card->is[CARD_DND].move.end.x, card->is[CARD_DND].move.end.y);
    
    /* Zurückgelegte Distanz berechnen */
    cdist = gfxutil_dist(dragx - dnd_origin.x, dragy - dnd_origin.y);
    
    /* Noch zurückzulegende Distanz berechnen */
    pdist = gfxutil_dist(stack_pos.x - dragx, stack_pos.y - dragy);
    
    /* Totale Distanz berechnen */
    tdist = cdist + pdist;

    dnd_debug(INFO, "drag dist %i, %i / %i",cdist, pdist, tdist);
    /* Zurückgelegte Distanz in Schritten von 0-65535 */
    step = cdist * 65535 / tdist;

    gfxutil_translate(&card->is[CARD_DND].move.now,
                      card->is[CARD_DND].move.start, 
                      card->is[CARD_DND].move.end, step);
    
    gfxutil_transform(&card->is[CARD_DND].rotozoom.now,
                      card->is[CARD_DND].rotozoom.start, 
                      card->is[CARD_DND].rotozoom.end, step);
    
    /* Relative Dragposition dazurechnen und dort hin schieben */
 /*   if(card_setpos(card, CARD_DND,  dragx + rx,  dragy + ry) & CARD_REDRAW)
      dnd_set(DND_REDRAW);*/
    
    
    /* Aktuelle Position ist relativ zur Dragposition */
    card->is[CARD_DND].move.now.x += dragx;
    card->is[CARD_DND].move.now.y += dragy;

    card->is[CARD_DND].status |= CARD_MOVE|CARD_TRANSFORM;
    
    dnd_set(DND_REDRAW);
/*    card->is[CARD_DND].move.start.x += dnd_origin.x;
    card->is[CARD_DND].move.start.y += dnd_origin.y;
    card->is[CARD_DND].move.end.x += stack_x;
    card->is[CARD_DND].move.end.y += stack_y;*/
  }
  
}
  
/* -------------------------------------------------------------------------- *
 * Drag & Drop Karten zeichnen                                                *
 * -------------------------------------------------------------------------- */
void dnd_redraw(SDL_Surface *surface)
{
  /* Karten werden in fan.c gezeichnet */
  
  
  dnd_unset(DND_REDRAW);
}
  
/* -------------------------------------------------------------------------- *
 * Behandelt Mausbewegungen                                                   *
 * -------------------------------------------------------------------------- */
int dnd_motion(uint8_t type, uint8_t state, int x, int y, int16_t xrel, int16_t yrel)
{
  struct card *card;
  struct node *node;

/*  if(!(state & SDL_BUTTON_LMASK))
  {
    dnd_stop();
    return dnd_status;
  }*/

  dnd_rel.x = x - dnd_origin.x;
  dnd_rel.y = y - dnd_origin.y;
  
  dnd_debug(INFO, "%u Karten bewegen..", dnd_list.size);
  
  dlink_foreach_data(&dnd_list, node, card)
  {
    dnd_calc(card, x, y);
  }
  
  
  return dnd_status;
}

/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int dnd_button(uint8_t type, uint8_t button, uint8_t state, int16_t x, int16_t y)
{
  /* Wurde Linke Maustaste losgelassen? */
/*  if(button == SDL_BUTTON_LEFT && !(state & SDL_BUTTON_LMASK))
  {
    dnd_stop();
  }*/
   
  return dnd_status;
}
           
/* -------------------------------------------------------------------------- *
 * Behandelt Events die das Drag & Drop betreffen                             *
 * -------------------------------------------------------------------------- */
int dnd_event(SDL_Event *event)
{
  /* Wenn Maustaste gedrückt wird bedeutet
     das eine neue Anfangsposition fürs DnD */
  if(event->type == SDL_MOUSEBUTTONDOWN && 
     event->button.button == SDL_BUTTON_LEFT)
  {
    dnd_origin.x = event->button.x;
    dnd_origin.y = event->button.y;
  }
  
  /* Alle anderen DnD Ereignisse behandeln wir nur wenn das DnD entsperrt ist */
  if(dnd_status & DND_LOCKED)
    return dnd_status;
  
  switch(event->type)
  {
    case SDL_MOUSEMOTION:
    {
      return dnd_motion(event->motion.type, 
                        event->motion.state, 
                        event->motion.x, event->motion.y,
                        event->motion.xrel, event->motion.yrel);
    }
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    {
      return dnd_button(event->button.type, 
                        event->button.button,
                        event->button.state,
                        event->button.x, event->button.y);
    }
  }
  
  return dnd_status;
}

/* -------------------------------------------------------------------------- *
 * Updatet Kartenpositionen und so                                            *
 * -------------------------------------------------------------------------- */
int dnd_update(uint32_t t)
{
  struct node *node;
  int astatus = CARD_FROZEN;
  int ostatus = 0;

  /* Wird nicht während der Benutzerinteraktion ausgeführt, sondern nur wenn
     das DnD Modul die Karten kontrolliert, also z.B. wenn diese an ihre 
     Ausgangsposition zurückkehren */
  if(dnd_status & DND_FROZEN)
    return dnd_status;
  
  /* Alle Karten updaten */
  dlink_foreach(&dnd_list, node)
  {
    int status;
    dnd_debug(VERBOSE, "Update [ t = %u, size = %u ] (%s)", 
              t, dnd_list.size, ((struct card*)node->data)->name);
  
    status = card_update(node->data, CARD_DND, t);
    astatus &= status;
    ostatus |= status;    
    
#ifdef DEBUG
//    card_dump(node->data, CARD_DND);
#endif /* DEBUG */
  }
  
  /* Irgend eine Karte muss neu gezeichnet werden */
  if(ostatus & CARD_REDRAW)
    dnd_set(DND_REDRAW);
  
  /* Alle Karten wieder eingefroren -> DnD stoppen */
  if(astatus & CARD_FROZEN)
  {
    dnd_set(DND_FROZEN);
    dnd_stop();
  }
  
  return dnd_status;
}

/* -------------------------------------------------------------------------- *
 * DND-Daten ausgeben                                                         *
 * -------------------------------------------------------------------------- */
#if 0
void dnd_dump(void)
{
}
#endif /* DEBUG */
