#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/dlink.h>
#include <libchaos/hook.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/player.h>
#include <tichu/combo.h>
#include <tichu/structs.h>
#include <tichu/cnode.h>
#include <tichu/game.h>
#include <tichu/card.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int          combo_log;
uint32_t     combo_serial;

struct sheap combo_heap;

/* -------------------------------------------------------------------------- *
 * Initialize the combo module                                                *
 * -------------------------------------------------------------------------- */
void combo_init(void)
{
  combo_log = log_source_register("combo");

  combo_serial = 0;

  mem_static_create(&combo_heap, sizeof(struct combo),
                    COMBO_BLOCK_SIZE);
  mem_static_note(&combo_heap, "combo heap");
  
  
  log(combo_log, L_status, "Initialised [combo] module.");
}

/* -------------------------------------------------------------------------- *
 * Shut down the combo module                                                 *
 * -------------------------------------------------------------------------- */
void combo_shutdown(void)
{
  log(combo_log, L_status, "Shutting down [combo] module...");

  mem_static_destroy(&combo_heap);
  
  log_source_unregister(combo_log);
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct combo *combo_new()
{
  struct combo *combo;
  
  combo = mem_static_alloc(&combo_heap);
  memset(combo, 0, sizeof(struct combo));
  
  return combo;
}
void combo_delete(struct combo *combo)
{
    /* Free the block */
    mem_static_free(&combo_heap, combo);
}


/* -------------------------------------------------------------------------- *
 * checks if the combo can be played                                          *
 *                                                                            *
 * return values:                                                             *
 * NULL = Kombination kann nicht gespielt werden                                 *
 *                                                                            *
 * sonst den typ der kombination                                              *
 * -------------------------------------------------------------------------- */
struct combo *combo_check(struct player *player, struct list *cards)
{
  struct game *game = player->game;
  struct turn *last_turn = NULL; /* leer wenn gepasst wurde */
  struct node *last_valid_node = NULL;
  struct turn *last_valid_turn = NULL; /* letzter gültiger zug */
  struct node *node;
  struct cnode *cnode;
  
  struct combo *combo;
  
  
  /* letzter zug welcher nicht gepasst wurde ermitteln */
  if((last_valid_node = game->prick->turns.tail))
  {       
    /* Pointer auf letzten Zug, falls vorhanden */
    last_turn = last_valid_node->data;
    
    do 
    {
      last_valid_turn = last_valid_node->data;
    } 
    while(last_valid_turn->combo.type == COMBO_ABANDON && (last_valid_node = last_valid_node->prev));
  }
  /* boah ugly */
  if(last_valid_turn && last_valid_turn->combo.type == COMBO_ABANDON)
    last_valid_turn = NULL;
  
  /* 
   * Zuerst wird die Liste sortiert, damit der Typ der gespielten Kombination
   * ermittelt werden kann... 
   */

  /* Liste sortieren (nach wertigkeit) */
  combo_sort_list(cards);
  
  /* Typ der erhaltenen Kombination herausfinden */
  combo = combo_get_type(cards);  
    
  /* Falls keine Bombe gespielt wurde, muss der spieler auch tatsächlich an der Reihe sein */
  if(((combo && combo->type != COMBO_BOMB) ||  !combo) && game->actor != player)
  {
    player_send(player, "PLAYCARDS FAIL :Du bist nicht an der Reihe...");
    return NULL;
  }  
  
  
  /* SONDERKARTEN */
  dlink_foreach_data(cards, node, cnode)
  {
    if(cnode->card->name[0] == 'x')
    {
      if(cnode->card->name[1] == 'd')
      {
        /* Drache (kann nur als einzelkarte gespielt werden */
        if(cards->size != 1)
        {          
          player_send(player, "PLAYCARDS FAIL :Drache kann nur als Einzelkarte gespielt werden...");
          return NULL;
        }        
      }
      else if(cnode->card->name[1] == 'p')
      {
        /* Phoenix (kann als einzelkarte und in kombination gespielt werden */
        if(cards->size == 1)
        {
          /* der value vom Phoenix darf nicht höher sein, als der vom Drachen */
          if(last_valid_turn && last_valid_turn->cards.size == 1 &&
             ((struct cnode *)last_valid_turn->cards.head->data)->value <  30)
          {
            cnode->value = ((struct cnode *)last_valid_turn->cards.head->data)->value + 1;
            combo->value = cnode->value;
          } 
          else
          {
            cnode->value = 1;
            combo->value = cnode->value;
          }                    
        }
        else
        {
          /* pärchen oder trio (bei beiden sind alle werte gleich) */                      
          if(cards->size == 2 || cards->size == 3)
          {
            cnode->value = get_value_x(cards, 0);
            
            combo_sort_list(cards);
            combo = combo_get_type(cards);
          }
          /* sonst ermitteln mit welchem wert, die höchste kombination möglich ist */
          else
          {
            int i;
            int hi_val = 0; // wert des Phoenix während der höchsten Karten kombination
            int hi_com = 0; // wert der höchsten erreichten Kombination
              
            
            /*
             * Wir gehen jetzt einfach jeden möglichen wert durch, und schauen welcher die 
             * höchste kombination ergibt, welche der vorherigen (falls vorhanden)
             * entspricht.
             */            
            for(i = 2;  i <= 28; i += 2)
            {
              cnode->value = i;
              
              combo_sort_list(cards);
              combo = combo_get_type(cards);
              
              /* keine gültige Kombination */
              if(!combo)
                continue;
              
              /* ein vorheriger zug existiert und der kombination wird nicht lei(?) gehalten */
              if(last_valid_turn && last_valid_turn->combo.type != combo->type)
                continue;
              
              /* ein vorheriger zug existiert und die kombination ist schwächer */
              if(last_valid_turn && last_valid_turn->combo.value >= combo->value)
                continue;
              
              if(combo->value > hi_com)
              {
                hi_com = combo->value;
                hi_val = i;
              }                                  
            }
            
            if(hi_val == 0)
            {                
              combo = NULL;
            }              
            else
            {
              cnode->value = hi_val;
              combo_sort_list(cards);
              combo = combo_get_type(cards);
            }
          }
          
          /* darf nicht in bombe vorkommen */
          if(combo && combo->type == COMBO_BOMB)
          {
            player_send(player, "PLAYCARDS FAIL :Phoenix darf nicht in einer Bombe vorkommen...");
            return NULL;
          }                    
          
        }
        
      }
      else if(cnode->card->name[1] == 'h')
      {
        if(cards->size != 1 || last_turn)
        {
          player_send(player, "PLAYCARDS FAIL :Hund kann nur als Einzelkarte ausgespielt werden...");
          return NULL;
        }
        else
        {
          /* der Zug ist somit ok, es sind keine Weiteren prüfungen mehr nötig */
          combo->type  = COMBO_HUND;
          combo->value = 0;
          return combo;
        }                  
      }
      else if(cnode->card->name[1] == '1')
      {
        player_send(player, "WÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜNSCHEN dörftest dü nöch... (so neben bei)");        
      }
      
    }
  }
   
  /* ungültige kombination */
  if(!combo)
  {    
    player_send(player, "PLAYCARDS FAIL :Keine gültige Kombination gespielt...");
    return NULL;    
  }
  
  /* Ist dies der erste Zug in dieser Runde, und die Kombination keine bombe ist alles ok */
  if(!last_turn || !last_valid_turn)
  {    
    /* Falls die Kombination doch eine Bombe ist... */
    if(combo->type == COMBO_BOMB)
    {
      player_send(player, "PLAYCARDS FAIL :Mit einer Bombe ausspielen ist nicht erlaubt...");
      return NULL;
    }
    
    return combo;    
  }
  else
  {
    /* 
     * Es ist nicht der erste Zug, 
     * d.h. Der Kombination muss der vorherigen entsprechen
     * Ausname: BOMBE 
     */
    if(combo->type == COMBO_BOMB)
    {
      /* der stich wurde schon gebombt, bombe muss also höher sein */
      if(last_valid_turn->combo.type == combo->type)
      {
        if(last_valid_turn->cards.size == cards->size)
        {          
          if(get_value_x(&last_valid_turn->cards, 0) >= get_value_x(cards, 0))
          {
            player_send(player, "PLAYCARDS FAIL :Die gespielte Bombe ist zu schwach...");
            return NULL;
          }                   
        if(last_valid_turn->cards.size >= cards->size)
          {
            player_send(player, "PLAYCARDS FAIL :Die gespielte Bombe ist zu klein...");
            return NULL;
          }                             
        }                
      }
      return combo;      
    }
    
    /* keine bombe */      
    
    /* Kombination muss die selbe sein */
    if(last_valid_turn->combo.type != combo->type && last_valid_turn->cards.size != cards->size)
    {
      player_send(player, "PLAYCARDS FAIL :Die gespielte Kombination entspricht nicht der vorherigen... "
                          "(typ: %i/%i anzahl: %i/%i)", last_valid_turn->combo.type, combo->type, last_valid_turn->cards.size, cards->size);
      return NULL;
    }      
    
    /* jetzt muss die Kombination nur nöch einen höheren Wert habe */
//    if(!combo_is_higher(cards, &last_valid_turn->cards, combo->type))
    if(combo->value <= last_valid_turn->combo.value)
    {      
      player_send(player, "PLAYCARDS FAIL :Wert der gespielten Kombination ist zu tief...");
      return NULL;
    }
    
    return combo;
  }  
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void combo_sort_list(struct list *list)
{
  struct cnode *cnode;
  struct node  *node1;
  struct node  *node2;
  struct cnode *array[list->size];
  int i, j, size;
  
  /* backup list->size in size */
  size = list->size;
  
  /* array erstellen */
  i = 0;

  dlink_foreach_safe_data(list, node1, node2, cnode)
  {
    array[i] = cnode;
    i++;
    cnode_unlink(cnode, list);
  }
  
  /* array sortieren */
  for (i = 1; i < size; i++)
    for (j = size - 1; j >= i; j--)
      if (array[j]->value < array[j - 1]->value)
      {
        cnode = array[j];
        array[j] = array[j - 1];
        array[j - 1] = cnode;
      }
   
  /* list erstellen */
  for(i = size-1; i >= 0; i--)
    cnode_link(array[i], list);  
}

/* -------------------------------------------------------------------------- *
 * returns the type of the played card or card-combination                    *
 * else it returns NULL                                                       *
 *                                                                            *
 * NOTE: this function requires a sorted list (sorted by value)               *
 * -------------------------------------------------------------------------- */
struct combo *combo_get_type(struct list *list)
{
  struct combo *combo;
  
  combo = combo_new();
  
  /* 1 card -> SINGLE_CARD */
  if(list->size == 1)
  {    
    combo->type  = COMBO_SINGLE_CARD;
    combo->value = get_value_x(list, 0);
  }  
  /* 2 cards -> PAIR */
  else if(list->size == 2)
  {    
    if(get_value_x(list, 0) == get_value_x(list, 1))
    {      
      combo->type = COMBO_PAIR;
      combo->value = get_value_x(list, 0);
    }  
  }  
  /* 3 cards -> TRIO */
  else if(list->size == 3)
  {    
    if(get_value_x(list, 0) == get_value_x(list, 1) &&
       get_value_x(list, 1) == get_value_x(list, 2))
    {
      combo->type  = COMBO_TRIO;
      combo->value = get_value_x(list, 0);
    }    
  }  
  /* 4 cards -> FOLLOWING_PAIR, BOMB */
  else if(list->size == 4)
    {    
      if(combo_is_following_pair(list))
      {      
        combo->type  = COMBO_FOLLOWING_PAIR;         
        combo->value = get_value_x(list, 0);    
      }          
      else if(get_value_x(list, 0) == get_value_x(list, 1) && 
              get_value_x(list, 1) == get_value_x(list, 2) && 
              get_value_x(list, 2) == get_value_x(list, 3))
      {
        combo->type  = COMBO_BOMB;
        combo->value = get_value_x(list, 0);    
      }
  }    
   /* 5 cards -> FULLHOUSE, STAIR, BOMB */
  else if(list->size == 5)
  {
    /* check first possibility of a FULLHOUSE */
    if((get_value_x(list, 0) == get_value_x(list, 1) && get_value_x(list, 1) == get_value_x(list, 2)) && 
       (get_value_x(list, 3) == get_value_x(list, 4)))
    {
      combo->type  = COMBO_FULLHOUSE;
      combo->value = get_value_x(list, 0);
    }    
    /* check secound possibility of a FULLHOUSE */
    else if((get_value_x(list, 2) == get_value_x(list, 3) && get_value_x(list, 3) == get_value_x(list, 4)) &&
       (get_value_x(list, 0) == get_value_x(list, 1)))
    {
      combo->type  = COMBO_FULLHOUSE;
      combo->value = get_value_x(list, 2);
    }  
    /* check if STAIR or BOMB */
    else if(combo_is_stair(list))
    {
      if(combo_is_same_type(list))
        combo->type = COMBO_BOMB;
      else
        combo->type = COMBO_STAIR;
      
      combo->value = get_value_x(list, 0);
    }
  }  
  /* 6-14 cards -> FOLLOWING_PAIR, STAIR, BOMB */
  else if(list->size >= 6 && list->size <= 14)
  {
    /* check FOLLOWING_PAIR */
    if(combo_is_following_pair(list))
    {      
      combo->type = COMBO_FOLLOWING_PAIR;
    }    
    /* check STAIR, BOMB */
    else if(combo_is_stair(list))
    {
      if(combo_is_same_type(list))
        combo->type = COMBO_BOMB;
      else
        combo->type = COMBO_STAIR;           
    }
    combo->value = get_value_x(list, 0);
  }
 
  if(combo->type)
    return combo;
  else 
    return NULL;
}

int combo_is_higher(struct list *list, struct list *last_turn, int combo)
{
  /* 
   * COMBO_SINGLE_CARD,  COMBO_PAIR, COMBO_TRIO, COMBO_FOLLOWING_PAIR, COMBO_STAIR
   * -> card->value muss höher sein als bei de(r/n) vorherigen Karte
   */
  if(combo == COMBO_SINGLE_CARD || 
     combo == COMBO_PAIR || 
     combo == COMBO_TRIO || 
     combo == COMBO_FOLLOWING_PAIR || 
     combo == COMBO_STAIR)
  {
    if(get_value_x(list, 0) > get_value_x(last_turn, 0))
      return 1;
    else
      return 0;
  }
  
  /* 
   * COMBO_FULLHOUSE
   * -> card->value der 3 karten muss höher sein als bei den vorherigen
   */
  if(combo == COMBO_FULLHOUSE)
  {
    int value1;
    int value2;
    
    /* wertigkeit der kombination herausfinden */
    if(get_value_x(list, 0) == get_value_x(list, 1) && get_value_x(list, 1) == get_value_x(list, 2))
      value1 = get_value_x(list, 0);
    else
      value1 = get_value_x(list, 3);
    
    /* wertigkeit der kombination herausfinden */    
    if(get_value_x(last_turn, 0) == get_value_x(last_turn, 1) && get_value_x(last_turn, 1) == get_value_x(last_turn, 2))
      value2 = get_value_x(last_turn, 0);
    else
      value2 = get_value_x(last_turn, 3);
    
    if(value1 > value2)
      return  1;
    else 
      return 0;
  }
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Checks if the cards in the list are a valid stair                          *
 * if valid it returns 1 otherweis 0                                          *
 * -------------------------------------------------------------------------- */
int combo_is_stair(struct list *list)
{
  struct node  *node1;
  struct node  *node2;
  struct cnode *cnode;
  
  dlink_foreach_safe_data(list, node1, node2, cnode)
  {
    if(node2 == NULL)
      return 1;
    
    if((int)cnode->value + 2 != (int)((struct cnode *)node2->data)->value)
      return 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Checks if the cards in the list are all from the same card-type            *
 * if all the same,  it returns 1 otherweis 0                                 *
 * -------------------------------------------------------------------------- */
int combo_is_same_type(struct list *list)
{
  struct node  *node1;
  struct node  *node2;
  struct cnode *cnode;
  
  dlink_foreach_safe_data(list, node1, node2, cnode)
  {
    if(node2 == NULL)
      return 1;
    
    if(*cnode->card->name != *((struct cnode *)node2->data)->card->name)
      return 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Checks if the cards in the list are a valid following pair                 *
 * if valid it returns 1 otherweis 0                                          *
 * -------------------------------------------------------------------------- */
int combo_is_following_pair(struct list *list)
{
  struct node  *node1;
  struct node  *node2;
  struct cnode *cnode;
  
  int same = 1;
  
  if(list->size % 2)
    return 0;
  
  dlink_foreach_safe_data(list, node1, node2, cnode)
  {
    if(node2 == NULL)
      return 1;
    
    if(same) 
    { 
      if((int)cnode->value  != (int)((struct cnode *)node2->data)->value) 
        return 0;
      same = 0;
    }
    else 
    {
      if((int)cnode->value + 2 != (int)((struct cnode *)node2->data)->value)
        return 0;
      same = 1;      
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * returns the value (card->value) of the card <x> from <list>                *
 * -------------------------------------------------------------------------- */
int get_value_x(struct list *list, int x)
{
  struct node *node;
  
  /* node auf listen anfang setzten */
  node = list->head;
  
  /* x elemente weiter (maximal bis zu list->tail) */
  for(; x > 0; x--)
    if(node->next)
      node = node->next;
  
  /* den Value aus der cnode-struct nehmen und nicht aus der cards-struct (wegen phoenix) */
  return (int)((struct cnode *)node->data)->value;
}  

/* -------------------------------------------------------------------------- *
 * returns the type (*card->name) of the card <x> from <list>                 *
 * -------------------------------------------------------------------------- */
char get_type_x(struct list *list, int x)
{
  struct node *node;
  
  /* node auf listen anfang setzten */
  node = list->head;
  
  /* x elemente weiter (maximal bis zu list->tail) */
  for(; x > 0; x--)
    if(node->next)
      node = node->next;
  
  return *(((struct cnode *)node->data)->card->name);
}  

        
        
        
