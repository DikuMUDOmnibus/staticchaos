
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

void do_quest( CHAR_DATA *ch, char *argument )
{ char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char areaname[MAX_INPUT_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char *name;
  MOB_INDEX_DATA *target;
  CHAR_DATA *mob;
  CHAR_DATA *vch;
  OBJ_DATA *obj;
  int vnum = 0, cost = 0,i;
  bool fMatch = FALSE;
  bool nMatch = FALSE;

  if ( IS_NPC(ch) )
    return;

  target = NULL;
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
  { if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
      break;
  }
  if ( mob == NULL )
  { send_to_char( "Find a quest master first.\n\r", ch );
    return;
  }

  if ( arg1[0] == '\0' )
  { send_to_char( "You may use the following quest commands:\n\r", ch );
    send_to_char( "QUEST REQUEST, INFO, COMPLETE, FAIL, LIST, BUY,\n\r", ch );
    send_to_char( "      AC, DAMAGE, RENAME, GLOW, HUM, NODROP, REPAIR.\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg1, "request" ) )
  { switch ( ch->pcdata->extras2[QUEST_TYPE] )
    { case QUEST_FAILED:
        do_say( mob, "It'll be a while before I give a failure like you another mission." );
        return;
      case QUEST_MOB:
      case QUEST_ITEM:
      case QUEST_FINISHED:
        do_say( mob, "You're already on another mission." );
        return;
      default:
        break;
    }

    while ( !fMatch )
    { fMatch = TRUE;
      vnum = dice(1,100) + (dice(1,100) * 100 );
      if ( vnum < 200 || vnum > 9500 )
        fMatch = FALSE;
      else if ( (target = get_mob_index(vnum)) == NULL ) 
        fMatch = FALSE;
    }

    name = target->short_descr;
    sprintf( buf1, "$n says '`mYour target is `M%s`m.`n'", name );
    switch( dice(1,10) )
    { case 1:
        sprintf( buf2, "$n says '`m%s is a menace to society and must be removed.`n'", name );
        break;
      case 2:
        sprintf( buf2, "$n says '`m%s has insulted my honor and must be made to pay!`n'", name );
        break;
      case 3:
        sprintf( buf2, "$n says '`m%s is scum, and I except them to be DEAD within the hour!`n'", name );
        break;
      case 4:
        sprintf( buf2, "$n says '`mI have suffered the existance of %s long enough.`n'", name );
        break;
      case 5:
        sprintf( buf2, "$n says '`mI want %s dead, you hear me? Dead Dead DEAD!!!`n'",name );
        break;
      case 6:
        sprintf( buf2, "$n says '`mVengeance shall be mine! Bwahahahaha!!`n'" );
        break;
      case 7:
        sprintf( buf2, "$n says '`mWhen you strike the finishing blow, tell %s that I sent you.`n'", name );
        break;
      case 8:
        sprintf( buf2, "$n says '`mJustice shall be served when %s is in the grave.`n'", name );
        break;
      case 9:
        sprintf( buf2, "$n says '`mI expect you to 'handle' %s professionally.`n'", name );
        break;
      case 10:
        sprintf( buf2, "$n says '`mPop a cap in the motherfucker!`n'" );
        break;
    }

  vch = char_list;
  while ( vch != NULL && !nMatch )
  { if ( IS_NPC(vch) && vch->pIndexData->vnum == target->vnum && vch->in_room != NULL )
      nMatch = TRUE;
    else
      vch = vch->next;
  }

  if ( nMatch )
  { for ( i = 16; vch->in_room->area->name[i] != '\0'; i++ )
      areaname[i-16] = vch->in_room->area->name[i];
    areaname[i-16] = '\0';
    switch( dice(1,5) )
    { case 1:
        sprintf( buf3, "$n says '`mOperatives have informed me that they may be found in `M%s`m.`n'", areaname );
        break;
      case 2:
        sprintf( buf3, "$n says '`mThey were last seen in `M%s`m.`n'", areaname );
        break;
      case 3:
        sprintf( buf3, "$n says '`mI've heard they hang around in `M%s`m.`n'", areaname );
        break;
      case 4:
        sprintf( buf3, "$n says '`mBegin your search in `M%s`m.`n'", areaname );
        break;
      case 5:
        sprintf( buf3, "$n says '`mI suggest you travel to `M%s`m.`n'", areaname );
        break;
    }
  }

    act( "$n says '`mI have a contract for you to fulfill.`n'", mob, NULL, NULL, TO_ROOM );
    act( buf1, mob, NULL, NULL, TO_ROOM );
    if ( nMatch )
      act( buf3, mob, NULL, NULL, TO_ROOM );
    else
      act( "$n says '`mI haven't seen them around lately, though.`n'", mob, NULL, NULL, TO_ROOM );
    act( buf2, mob, NULL, NULL, TO_ROOM );
    ch->pcdata->extras2[QUEST_TYPE] = QUEST_MOB;
    ch->pcdata->extras2[QUEST_INFO] = vnum;
    return;
  }
  else if ( !str_cmp( arg1, "info" ) )
  { switch ( ch->pcdata->extras2[QUEST_TYPE] )
    { case QUEST_NONE:
      case QUEST_FAILED:
        send_to_char( "You aren't currently on a quest.\n\r", ch );
        return;
      case QUEST_FINISHED:
        send_to_char( "You are just finishing up your previous quest.\n\r", ch );
        return;
      case QUEST_ITEM:
        return;
      default:
        break;
    }

    if ( (target = get_mob_index(ch->pcdata->extras2[QUEST_INFO])) == NULL )
    { send_to_char( "!Error!\n\r", ch );
      return;
    }
    sprintf( buf1, "You are currently on a mission to assassinate %s.\n\r", target->short_descr );
    send_to_char( buf1, ch );
    return;
  }
  else if ( !str_cmp( arg1, "fail" ) )
  { switch( ch->pcdata->extras2[QUEST_TYPE] )
    { case QUEST_NONE:
        send_to_char( "You aren't on a quest, laser brain.\n\r", ch );
        return;
      case QUEST_FAILED:
        send_to_char( "You already failed this quest.\n\r", ch );
        return;
      case QUEST_FINISHED:
        send_to_char( "Why fail when you've already won.. dumbshit.\n\r", ch );
        return;
      default:
        break;
    }

    send_to_char( "You shamefully admit defeat, and give up your quest.\n\r", ch );
    ch->pcdata->extras2[QUEST_TYPE] = QUEST_FAILED;
    if ( IS_CLASS(ch,CLASS_FIST) || IS_CLASS(ch,CLASS_PATRYN) )
      ch->pcdata->extras2[QUEST_INFO] = 6;
    else
      ch->pcdata->extras2[QUEST_INFO] = 10;
    return;
  }
  else if ( !str_cmp( arg1, "complete" ) )
  { switch( ch->pcdata->extras2[QUEST_TYPE] )
    { case QUEST_NONE:
      case QUEST_FAILED:
        send_to_char( "What are you thinking, you aren't even on one!\n\r", ch );
        return;
      case QUEST_MOB:
      case QUEST_ITEM:
        send_to_char( "You haven't finished your quest yet.\n\r", ch );
        return;
      default:
        break;
    }

    switch( dice(1,5) )
    { case 1:
        sprintf( buf1, "$n says '`mYour assistance in this matter is much appreciated.`n'" );
        break;
      case 2:
        sprintf( buf1, "$n says '`mGlad to see you back!.`n'" );
        break;
      case 3:
        sprintf( buf1, "$n says '`mThey're dead?  OOSHAAAAAAAAA!!`n'" );
        break;
      case 4:
        sprintf( buf1, "$n says '`mArigato!`n'" );
        break;
      case 5:
        sprintf( buf1, "$n says '`mVengeance is mine.. now.. I may rest.`n'" );
        break;
    }

    i = dice(50,100) + dice(UMIN(10,ch->pcdata->extras[LEGEND]),200)
    		     + dice(UMIN(100,ch->pcdata->extras2[QUEST_INFO]),25)
		     + dice(10,isquare(ch->played));
    if ( ch->pcdata->extras2[EVAL] >= 65 )
      i += i /3;
    i = URANGE(1,i,10000);
    sprintf( buf2, "$n gives you %d gold coins.", i );
    act( buf1, mob, NULL, ch, TO_ROOM );
    act( buf2, mob, NULL, ch, TO_VICT );
    ch->pcdata->extras2[QUEST_TYPE] = QUEST_NONE;
    ch->pcdata->extras2[QUEST_INFO] = QUEST_NONE;
    ch->pcdata->extras2[QUEST_COMPLETED] += 1;
    ch->gold += i;
    return;
  }
  else if ( !str_cmp( arg1, "list" ) )
  { send_to_char( "\n\r", ch );
    send_to_char( "Item                       Cost\n\r", ch );
    send_to_char( "a Tarnished Silver Bracer  400,000\n\r", ch );
    send_to_char( "a Medal of Honor           500,000\n\r", ch );
    send_to_char( "a Golden Band              600,000\n\r", ch );
    send_to_char( "a Mystic Robe              750,000\n\r", ch );
    send_to_char( "a Golden Tabard            750,000\n\r\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg1, "buy" ) )
  { if ( arg2[0] == '\0' )
    { send_to_char( "Buy what?\n\r", ch );
      return;
    }
    if ( !str_cmp( arg2, "tarnished" ) || !str_cmp( arg2, "bracer" ) )
    { vnum = 802;
      cost = 400000;
    }
    else if ( !str_cmp( arg2, "medal" ) || !str_cmp( arg2, "honor" ) )
    { vnum = 804;
      cost = 500000;
    }
    else if ( !str_cmp( arg2, "band" ) )
    { vnum = 801;
      cost = 600000;
    }
    else if ( !str_cmp( arg2, "mystic" ) || !str_cmp( arg2, "robe" ) )
    { vnum = 803;
      cost = 750000;
    }
    else if ( !str_cmp( arg2, "tabard" ) )
    { vnum = 800;
      cost = 750000;
    }
    else
    { send_to_char( "Buy what?\n\r", ch );
      return;
    }

    if ( ch->gold < cost )
    { send_to_char( "You don't have enough gold.\n\r", ch );
      return;
    }

    obj = create_object( get_obj_index(vnum), 0 );
    obj_to_char( obj, ch );
    act( "$N hands $p to you.", ch, obj, mob, TO_CHAR );
    act( "$N hands $p to $n.", ch, obj, mob, TO_ROOM );
    ch->gold -= cost;
    return;
  }
  else if ( !str_cmp( arg1, "ac" ) )
  { if ( arg2[0] == '\0' )
    { send_to_char( "Raise the armor class on what?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    { send_to_char( "You don't have that.\n\r", ch );
      return;
    }
    if ( obj->item_type != ITEM_ARMOR )
    { send_to_char( "That isn't armor.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE) )
    { send_to_char( "Uniques may not be quested.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj,ITEM_HARDENED) )
    {
      stc( "It's too hard.\n\r", ch );
      return;
    }
    if ( obj->value[0] == 15 )
    { send_to_char( "That item is as well made as possible.\n\r", ch );
      return;
    }

    cost = (15 - obj->value[0]) * 1000;
    if ( ch->gold < cost )
    { sprintf( buf1, "That will cost you %d gold.\n\r", cost ); 
      send_to_char( buf1, ch );
      return;
    }

    ch->gold -= cost;
    obj->value[0] = 15;
    act( "$N takes $p, taps it once, and returns it to you.", ch, obj, mob, TO_CHAR );
    act( "$N takes $p, taps it once, and returns it to $n.", ch, obj, mob, TO_ROOM );
    return;
  }
  else if ( !str_cmp( arg1, "damage" ) )
  { if ( arg2[0] == '\0' )
    { send_to_char( "Raise the damage on what?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    { send_to_char( "You don't have that.\n\r", ch );
      return;
    }
    if ( obj->item_type != ITEM_WEAPON )
    { send_to_char( "That isn't a weapon.\n\r", ch );
      return;
    }
    if ( obj->value[1] + obj->value[2] >= 30 )
    { send_to_char( "That weapon is as brutal as possible.\n\r", ch );
      return;
    }

    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE) )
    { send_to_char( "Uniques may not be quested.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj,ITEM_HARDENED) )
    {
      stc( "It's too hard.\n\r", ch );
      return;
    }
    cost = (10 - obj->value[1]) * 5000 + (20 - obj->value[2]) * 5000;
    if ( ch->gold < cost )
    { sprintf( buf1, "That will cost you %d gold.\n\r", cost ); 
      send_to_char( buf1, ch );
      return;
    }
    ch->gold -= cost;
    obj->value[1] = 10;
    obj->value[2] = 20;
    act( "$N takes $p, licks it, and returns it to you.", ch, obj, mob, TO_CHAR );
    act( "$N takes $p, licks it, and returns it to $n.", ch, obj, mob, TO_ROOM );
    return;
  }
  else if ( !str_cmp( arg1, "rename" ) )
  { if ( ch->gold < 1000 )
    { send_to_char( "Renames cost 1000 gold.\n\r", ch );
      return;
    }
    if ( arg2[0] == '\0' && arg3[0] == '\0' )
    { send_to_char( "Rename requires two arguments, an item and a new name?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    { send_to_char( "You don't have that.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE) )
    { send_to_char( "Uniques may not be quested.\n\r", ch );
      return;
    }
    if ( obj->pIndexData->vnum == 24 ) // quest trash item
    {
      stc( "Why bother with that trash?\n\r", ch );
      return;
    }
    if ( strlen( arg3 ) > 60 )
    { send_to_char( "Name too long.\n\r", ch );
      return;
    }

    smash_tilde( arg3 );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( arg3 ); 
    ch->gold -= 1000;
    return;
  }
  else if ( !str_cmp( arg1, "glow" ) )
  { if ( arg2[0] == '\0' )
    { send_to_char( "Make what glow?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    { send_to_char( "You don't have that.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE) )
    { send_to_char( "Uniques may not be quested.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj,ITEM_HARDENED) )
    {
      stc( "It's too hard.\n\r", ch );
      return;
    }

    if ( ch->gold < 5000 )
    { send_to_char( "It costs 5000 gold to make an item glow.\n\r", ch );
      return;
    }

    if ( !IS_SET(obj->extra_flags,ITEM_GLOW) )
    { act( "$N buffs $p to a perfect shine.", ch, obj, mob, TO_CHAR );
      SET_BIT(obj->extra_flags,ITEM_GLOW);
    }
    else
    { act( "$N drops $p, urinates on it, then returns it to you.", ch, obj, mob, TO_CHAR );
      REMOVE_BIT(obj->extra_flags,ITEM_GLOW);
    }
    ch->gold -= 5000;
    return;
  }
  else if ( !str_cmp( arg1, "nodrop" ) )
  { if ( arg2[0] == '\0' )
    { send_to_char( "Make what no_drop?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    { send_to_char( "You don't have that.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE) )
    { send_to_char( "Uniques may not be quested.\n\r", ch );
      return;
    }

    if ( ch->gold < 25000 )
    { send_to_char( "It costs 25000 gold to make an item no_drop.\n\r", ch );
      return;
    }

    if ( !IS_SET(obj->extra_flags,ITEM_NODROP) )
    { act( "$N scratches a tiny rune onto $p.", ch, obj, mob, TO_CHAR );
      SET_BIT(obj->extra_flags,ITEM_NODROP);
      ch->gold -= 25000;
    }
    else
      act( "$N says '`m$p is already undroppable.`n'", ch, obj, mob, TO_CHAR );

    return;
  }
  else if ( !str_cmp( arg1, "hum" ) )
  { if ( arg2[0] == '\0' )
    { send_to_char( "Make what hum?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    { send_to_char( "You don't have that.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE) )
    { send_to_char( "Uniques may not be quested.\n\r", ch );
      return;
    }
    if ( IS_OBJ_STAT(obj,ITEM_HARDENED) )
    {
      stc( "It's too hard.\n\r", ch );
      return;
    }
    if ( ch->gold < 5000 )
    { send_to_char( "It costs 5000 gold to make an item glow.\n\r", ch );
      return;
    }

    if ( !IS_SET(obj->extra_flags,ITEM_HUM) )
    { act( "$N smacks $p against $S head a couple times.", ch, obj, mob, TO_CHAR );
      SET_BIT(obj->extra_flags,ITEM_HUM);
    }
    else
    { act( "$N holds $p against an ass cheek for a couple moments.", ch, obj, mob, TO_CHAR );
      REMOVE_BIT(obj->extra_flags,ITEM_HUM);
    }
    ch->gold -= 5000;
    return;
  }
  else if ( !str_cmp( arg1, "repair" ) )
  { if ( !IS_SUIT(ch) )
    { send_to_char( "You don't have a mobile suit to repair.\n\r", ch );
      return;
    }
    i = ch->pcdata->suit[SUIT_NUMBER];
    if ( ch->pcdata->suit[SUIT_COND] == 0
      && ch->pcdata->suit[SUIT_ARMOR] == (suit_table[i].armor * 15) )
    { send_to_char( "Your mobile suit is already in tip-top shape.\n\r", ch );
      return;
    }

    /* misuse of a variable, but I don't feel like declaring another */
    vnum = 1;
    cost += 150;
    for ( i = 0; i < 12; i++ )
    { if ( IS_SET(ch->pcdata->suit[SUIT_COND],vnum) )
        cost += 250 * ch->pcdata->suit[SUIT_NUMBER];
      vnum *= 2;
    }
    if (ch->gold < cost )
    { sprintf( buf1, "It will cost %d gold to perform repairs.\n\r", cost );
      send_to_char( buf1, ch );
      return;
    }

    ch->pcdata->suit[SUIT_ARMOR] = suit_table[ch->pcdata->suit[SUIT_NUMBER]].armor * 15;
    ch->pcdata->suit[SUIT_COND] = 0;
    act( "$N gives your mobile suit a tune-up.", ch, NULL, mob, TO_CHAR );
    ch->gold -= cost;
    return;
  }
  else if ( !str_cmp( arg1, "time" ) )
  {
    switch( ch->pcdata->extras2[QUEST_TYPE] )
    { case QUEST_FAILED:
        sprintf( buf1, "You have less than %d minute%s left until you may quest",
          (ch->pcdata->extras2[QUEST_INFO]/2 + 1),
          ((ch->pcdata->extras2[QUEST_INFO]/2+1) == 1 ? "" : "s" ) );
        do_say( mob, buf1 );
        break;
      case QUEST_MOB:
      case QUEST_ITEM:
      case QUEST_FINISHED:
          do_say( mob, "You've got plenty of time to finish." );
        break;
      default:
        break;
    };
    return;
  }
}
