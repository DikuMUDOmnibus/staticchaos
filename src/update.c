/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void	regen_update	args( ( void ) );
void	save_wholist	args( ( void ) );
void	second_update	args( ( void ) );
void	save_clanboard	args( ( void ) );
void	vote_update	args( ( void ) );
void	bug_check	args( ( void ) );
void	chant_update	args( ( void ) );

/*
 * Advancement stuff.
 * no longer necessary, probly should junk this
 */
void advance_level( CHAR_DATA *ch )
{

    if ( !IS_NPC(ch) )
	REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
    if ( IS_NPC(ch) )
	return;

/*  taken out, no more leveling
    ch->exp = UMAX( 1000, ch->exp + gain );
    while ( ch->level < LEVEL_HERO && ch->exp >= 1000 * (ch->level+1) )
    {
	send_to_char( "You raise a level!!  ", ch );
	ch->level += 1;
	advance_level( ch );
    }
*/

  ch->exp += gain;
  ch->totalexp += gain;
    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	gain = ch->level * 3 / 2;
    }
    else
    {
	gain = ch->pcdata->body + 100;

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += ch->pcdata->body * 5;	break;
	case POS_RESTING:  gain += ch->pcdata->body * 2;	break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = ch->pcdata->mind;

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += ch->pcdata->mind * 20;	break;
	case POS_RESTING:  gain += ch->pcdata->mind * 5;	break;
	}

	if ( ch->class == CLASS_SORCERER )
	{ switch( ch->position )
	  { case POS_SLEEPING: gain += ch->pcdata->will * 20;
	    case POS_RESTING:  gain += ch->pcdata->will * 15;
	  }
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = ch->pcdata->body;

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += ch->pcdata->body * 5;		break;
	case POS_RESTING:  gain += ch->pcdata->body * 2;		break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_HERO )
	return;

    condition				= ch->pcdata->condition[iCond];
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch, AFF_CHARM) )
	    continue;

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	}

	/* That's all for sleeping / busy monster */
	if ( ch->position < POS_STANDING )
	    continue;

	/* MOBprogram random trigger */
	if (ch->in_room->area->nplayer>0)
	{
	    mprog_random_trigger(ch);
	                                     /* If ch dies or changes
						position due to it's random
						trigger continue - Kahn */
	    if ( ch->position < POS_STANDING )
	        continue;
	}

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 2 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max )
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL)
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->to_room->area == ch->in_room->area ) )
	{
	    move_char( ch, door );
	                                       /* If ch changes position due
						  to it's or someother mob's
						  movement via MOBProgs,
						  continue - Kahn */
	    if ( ch->position < POS_STANDING )
	        continue;
	}

	/* Flee */
	if ( ch->hit < ( ch->max_hit / 2 )
	&& ( !IS_SET(ch->act,ACT_SENTINEL) )
	&& ( door = number_bits( 3 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )
	{
	    CHAR_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = pexit->to_room->people;
		  rch != NULL;
		  rch  = rch->next_in_room )
	    {
		if ( !IS_NPC(rch) )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
		move_char( ch, door );
	}

    }

    return;
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The sun rises in the east.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The sun slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}



/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_save;
    CHAR_DATA *ch_quit;
    time_t save_time;

    OBJ_DATA *wield;
    save_time	= current_time;
    ch_save	= NULL;
    ch_quit	= NULL;
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	/* unnecessary
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	*/

	ch_next = ch->next;

	/*
	 * Find dude with oldest save time.
	 */
	if ( !IS_NPC(ch)
	&& ( ch->desc == NULL || ch->desc->connected == CON_PLAYING )
	&&   ch->level >= 1
	&&   ch->save_time < save_time )
	{
	    ch_save	= ch;
	    save_time	= ch->save_time;
	}

	if ( !IS_NPC(ch) )
	  ch->played++;

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += dice(1,4) + hit_gain(ch);

	    if ( ch->mana < ch->max_mana )
		ch->mana += dice(1,4) + mana_gain(ch);

	    if ( ch->move < ch->max_move )
		ch->move += dice(1,4) + move_gain(ch);
	}

	if ( ch->position == POS_STUNNED || ch->position == POS_INCAP || ch->position == POS_MORTAL )
	    update_pos( ch );

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	    }

	    if ( ++ch->timer >= 18 )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		    act( "$n disappears into the void.",
			ch, NULL, NULL, TO_ROOM );
		    send_to_char( "You disappear into the void.\n\r", ch );
		    save_char_obj( ch );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		}
	    }

	    if ( ch->timer > 30 )
		ch_quit = ch;

	    if ( ch->level == 1 )
	    { gain_condition( ch, COND_DRUNK,  -1 );
	      gain_condition( ch, COND_FULL,   -1 );
	      gain_condition( ch, COND_THIRST, -1 );
	    }
	}

	/* check for char wielding a laguna blade */
	if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_LAGUNABLADE) )
	{ act( "The Laguna Blade dwindles down to a speck of darkness and vanishes.", ch, NULL, NULL, TO_CHAR );
	  act( "The Laguna Blade dwindles down to a speck of darkness and vanishes.", ch, NULL, NULL, TO_ROOM );
	  REMOVE_BIT(ch->pcdata->actnew,NEW_LAGUNABLADE);
	  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
	  { obj_from_char( wield );
	    extract_obj( wield );
	  }
	}
	if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_RECOVERY) )
	{ REMOVE_BIT(ch->pcdata->actnew,NEW_RECOVERY);
	  send_to_char( "You feel the healing power leave your body.\n\r", ch );
	}
        if ( !IS_NPC(ch) && is_affected(ch,gsn_defense) )
        { affect_strip(ch,gsn_defense);
          send_to_char( "Your defensive barrier drops.\n\r", ch );
        }
	if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_VAN_REHL ) )
	{ REMOVE_BIT(ch->pcdata->actnew,NEW_VAN_REHL);
	  act( "The web of ice above you shatters.", ch, NULL, NULL, TO_CHAR );
	}
	if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_AIR_BLOCK) )
	{ REMOVE_BIT(ch->pcdata->actnew,NEW_AIR_BLOCK);
	  act( "The inverted runes of air fade away.", ch, NULL, NULL, TO_CHAR );
	}
	if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_FIRE_BLOCK) )
	{ REMOVE_BIT(ch->pcdata->actnew,NEW_FIRE_BLOCK);
	  act( "The inverted runes of fire crumble.", ch, NULL, NULL, TO_CHAR );
	}
	if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
	{
	  REMOVE_BIT(ch->pcdata->actnew,NEW_NEGATIVE_BLOCK);
	  act( "The helix of black energy fades froma round you.", ch, NULL, NULL, TO_CHAR);
	}
	if ( IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
	{ REMOVE_BIT(ch->pcdata->powers[M_SET],M_ASTRIKE);
	  act( "You slide back into the material plane.", ch, NULL, NULL, TO_CHAR );
	  act( "$n solidifies.", ch, NULL, NULL, TO_ROOM );
	}
	if ( IS_CLASS(ch,CLASS_FIST) && IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
	{ if ( ch->position != POS_FIGHTING )
	  { act( "Your phoenix aura collapses.", ch, NULL, NULL, TO_CHAR );
	    act( "The aura of flame around $n collapses.", ch, NULL, NULL, TO_ROOM );
	    REMOVE_BIT(ch->pcdata->actnew,NEW_PHOENIX_AURA);
	    ch->hitroll -= 15;
	    ch->damroll -= 15;
	    ch->pcdata->powers[F_KI] = 0;
	  }
	}

	if ( !IS_NPC(ch) && ch->pcdata->extras2[PKCOUNT] > 0 )
	{ ch->pcdata->extras2[PKCOUNT]--;
	  if ( ch->pcdata->extras2[PKCOUNT] == 9 )
	    send_to_char( "Your body has recovered from the beating you took.\n\r", ch );
	}

	if ( !IS_NPC(ch) && ch->pcdata->extras[TIE] > 0 )
	{ ch->pcdata->extras[TIE]--;
	  if ( ch->pcdata->extras[TIE] == 0 )
	  { act( "The ropes fall away from you.", ch, NULL, NULL, TO_CHAR );
	    act( "The ropes fall away from $n.", ch, NULL, NULL, TO_ROOM );
	  }
	}
	if ( !IS_NPC(ch) && ch->pcdata->extras2[QUEST_TYPE] == QUEST_FAILED )
	{ ch->pcdata->extras2[QUEST_INFO]--;
	  if ( ch->pcdata->extras2[QUEST_INFO] <= 0 )
	  { send_to_char( "You may now quest again.\n\r", ch );
	    ch->pcdata->extras2[QUEST_TYPE] = QUEST_NONE;
	    ch->pcdata->extras2[QUEST_INFO] = QUEST_NONE;
	  }
	}

	/* removed for affects based on seconds
	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}
	*/

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */
	if ( IS_AFFECTED(ch, AFF_POISON) )
	{
	    act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You shiver and suffer.\n\r", ch );
	    damage( ch, ch, 2, gsn_poison );
	}
	else if ( ch->position == POS_INCAP )
	{
	    /* damage( ch, ch, 1, TYPE_UNDEFINED ); */
	}
	else if ( ch->position == POS_MORTAL )
	{
	    /* damage( ch, ch, 2, TYPE_UNDEFINED ); */
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    if ( ch_save != NULL || ch_quit != NULL )
    {
	for ( ch = char_list; ch != NULL; ch = ch_next )
	{
	    ch_next = ch->next;
	    if ( ch == ch_save )
	    {
		save_char_fin( ch );
		save_char_obj( ch );
	    }
	    if ( ch == ch_quit )
		do_quit( ch, "" );
	}
    }

    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char buf[MAX_INPUT_LENGTH];

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p vanishes."; 
	  sprintf( buf, "Fucked up item decay thing. '%s'", obj->short_descr );
	                      bug( "Fucked up item decay thing.",0 );continue;	break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         	break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; 	break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; 	break;
	case ITEM_FOOD:       message = "$p decomposes.";		break;
	case ITEM_GATE:
	case ITEM_PORTAL:     message = "$p shimmers and disappears.";	break;
	case ITEM_WEAPON:     message = "$p dissipates into ether.";	break;
	}

	if ( obj->carried_by != NULL )
	{
	    act( message, obj->carried_by, obj, NULL, TO_CHAR );
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    act( message, rch, obj, NULL, TO_ROOM );
	    act( message, rch, obj, NULL, TO_CHAR );
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;

	/* MOBProgram ACT_PROG trigger */
        if ( IS_NPC( wch ) && wch->mpact != NULL )
	/* removed for memory leak
	    && wch->in_room->area->nplayer > 0 )
	*/
	{
            MPROG_ACT_LIST * tmp_act, *tmp2_act;
            for ( tmp_act = wch->mpact; tmp_act != NULL;
		 tmp_act = tmp_act->next )
	    {
		 mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
				      tmp_act->obj, tmp_act->vo, ACT_PROG );
		 free_string( tmp_act->buf );
	    }
            for ( tmp_act = wch->mpact; tmp_act != NULL; tmp_act = tmp2_act )
	    {
		 tmp2_act = tmp_act->next;
		 free_mem( tmp_act, sizeof( MPROG_ACT_LIST ) );
		 //free( tmp_act );
            }
            act_prog_num--;
            wch->mpactnum = 0;
            wch->mpact = NULL;
	}

	if ( IS_NPC( wch )
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL )
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( is_affected(wch,gsn_dark_blaze) && !IS_SUIT(wch) && ch->level < 30 )
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) )
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   ( !IS_SET(ch->act,ACT_VALHERU) || vch->pcdata->clan[CLAN] != 1 )
		&&   ( !IS_SET(ch->act,ACT_CITHDEUX) || vch->pcdata->clan[CLAN] != 2 )
		&&   ( !IS_SET(ch->act,ACT_SYNDICATE) || vch->pcdata->clan[CLAN] != 3 )
		&&   ( !IS_SET(ch->act,ACT_BROTHERHOOD) || vch->pcdata->clan[CLAN] != 4 )
		&&   ( !IS_SET(ch->act,ACT_EXODUS) || vch->pcdata->clan[CLAN] != 5 )
		&&   ( !IS_SET(ch->act,ACT_MERCENARY) || vch->pcdata->clan[CLAN] != 6 )
		&&   can_see( ch, vch ) 
		&&   vch->hit > 0)
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
	    {
		/* removed for tremendous spam
		bug( "Aggr_update: null victim.", count );
		*/
		continue;
	    }

	    stancecheck( ch, victim );
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }

    return;
}

void second_update()
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  CHAR_DATA *victim;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  char buf[MAX_INPUT_LENGTH];
  int dam, rank;
  extern time_t copyover_time;
  extern time_t copyover_warning;

  if ( copyover_warning == current_time )
  {
    sprintf( buf, "`RCopyover in `W%d`R seconds.`n\n\r", (int) (copyover_time - current_time ));
    send_to_all_char( buf );
  }
  if ( copyover_time == current_time )
  {
    copyover();
  }

  for ( ch = char_list; ch != NULL; ch = ch_next )
  {
    ch_next = ch->next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
        paf_next    = paf->next;
        if ( paf->duration > 0 )
            paf->duration--;
        else if ( paf->duration < 0 )
            ;
        else
        {
            if ( paf_next == NULL
            ||   paf_next->type != paf->type
            ||   paf_next->duration > 0 )
            {
                if ( paf->type > 0 && skill_table[paf->type].msg_off )
                {
                    if ( skill_table[paf->type].room )
                      act( skill_table[paf->type].msg_off, ch, NULL, NULL, TO_ALL );
                    else
                      act( skill_table[paf->type].msg_off, ch, NULL, NULL, TO_CHAR );
                }
            }

            affect_remove( ch, paf );
        }
    }

    if ( IS_NPC(ch) )
      continue;

    if ( IS_CLASS(ch,CLASS_SAIYAN) )
    {
      if ( IS_SET(ch->pcdata->actnew,NEW_MASENKOUHA) )
      {
        REMOVE_BIT(ch->pcdata->actnew,NEW_MASENKOUHA);
        do_yell( ch, "Ha!!" );
        dam = dice( ch->pcdata->spirit / 10, ch->pcdata->spirit * 10 );
        if ( (victim = get_full_char_room(ch,ch->pcdata->target)) == NULL )
        {
          if ( (victim = get_char_area(ch,ch->pcdata->target)) == NULL )
          {
            act( "You launch a bolt of ki flame into the air!", ch, NULL, NULL, TO_CHAR );
            act( "$n launches a bolt of ki flame into the air.", ch, NULL, NULL, TO_ROOM );
          }
          else
          {
            damage( ch, victim, dam, DAM_KIFLAME );
          }
        }
        else
        {
          damage( ch, victim, dam, DAM_KIFLAME );
        }
        free_string( ch->pcdata->target );
        ch->pcdata->target = str_dup( "" );
      }
      if ( IS_SET(ch->pcdata->actnew,NEW_KAME_2) )
      {
        REMOVE_BIT(ch->pcdata->actnew,NEW_KAME_2);
        do_yell( ch, "Ha!!" );
        dam = ch->pcdata->spirit * 105 + dice(15,ch->pcdata->spirit );  
        if ( (victim = get_full_char_room(ch,ch->pcdata->target)) == NULL )
	{
          if ( (victim = get_char_area(ch,ch->pcdata->target)) == NULL )
          {
            act( "You launch a bolt of ki flame into the air!", ch, NULL, NULL, TO_CHAR );
            act( "$n launches a bolt of ki flame into the air.", ch, NULL, NULL, TO_ROOM );
          }
          else
          {
            damage( ch, victim, dam, DAM_KIFLAME );
          }
        }
        else
        {
          damage( ch, victim, dam, DAM_KIFLAME );
        }
        free_string( ch->pcdata->target );
        ch->pcdata->target = str_dup( "" );
      }
      if ( IS_SET(ch->pcdata->actnew,NEW_KAME_1) )
      {
        do_say( ch, "Hame.." );
        REMOVE_BIT(ch->pcdata->actnew,NEW_KAME_1);
        SET_BIT(ch->pcdata->actnew,NEW_KAME_2);
      }
    }

    if ( IS_CLASS(ch,CLASS_SORCERER) )
    {
      if ( ch->pcdata->powers[SORC_MYSTIC] < ch->pcdata->will )
        ch->pcdata->powers[SORC_MYSTIC]++;

      if ( IS_SET(ch->pcdata->actnew,NEW_CONCENTRATE) )
      {
        if ( ch->pcdata->powers[SORC_MYSTIC] < ch->pcdata->will && ch->mana >= 100 )
        {
          ch->pcdata->powers[SORC_MYSTIC] += dice(3,3);
          ch->mana -= 100;
          if ( ch->pcdata->powers[SORC_MYSTIC] > ch->pcdata->will )
            ch->pcdata->powers[SORC_MYSTIC] = ch->pcdata->will;
        }
        else if ( ch->mana < 100 )
        {
          REMOVE_BIT(ch->pcdata->actnew,NEW_CONCENTRATE);
          stc( "You are too exhausted to concentrate.\n\r", ch );
        }
      }
      else if ( ch->position > POS_STUNNED )
      {
        if ( ch->position <= POS_RESTING )
          ch->mana += (ch->pcdata->will + ch->pcdata->mind) * 3;
        else
          ch->mana += ch->pcdata->will + (ch->max_mana / 85);
        ch->mana = UMIN( ch->max_mana, ch->mana );
      }
    }
    if ( IS_CLASS(ch,CLASS_MAZOKU) && ch->pcdata->powers[M_CTYPE] != 0 )
    {
      if ( ch->position < POS_STUNNED )
      {
	ch->pcdata->powers[M_CTIME] = 0;
	ch->pcdata->powers[M_CTYPE] = 0;
      }
      else if ( ch->pcdata->powers[M_ESSENSE] < 50 )
      {
        act( "Your exhaustion causes you to lose your charge.", ch, NULL, NULL, TO_CHAR );
        act( "The chaotic energies around $n fade away.", ch, NULL, NULL, TO_ROOM );
        ch->pcdata->powers[M_CTIME] = 0;
        ch->pcdata->powers[M_CTYPE] = 0;
      }
      else if ( ch->pcdata->powers[M_CTIME] == (15+ch->pcdata->powers[M_ASTRAL]/10) )
      {
        stc( "You cannot charge up any more!\n\r", ch );
        ch->pcdata->powers[M_ESSENSE] -= 50;
        ch->pcdata->powers[M_CTIME]++;
      }
      else if ( ch->pcdata->powers[M_CTIME] >= (16+ch->pcdata->powers[M_ASTRAL]/10) )
      {
        ch->pcdata->powers[M_ESSENSE] -= 50;
      }
      else
      {
        ch->pcdata->powers[M_ESSENSE] -= 50;
        ch->pcdata->powers[M_CTIME]++;
      }
    }

    if ( ch->position > POS_RESTING && IS_SET(ch->pcdata->actnew,NEW_DRAGON_SLAVE) )
    { rank = ch->pcdata->powers[SCHOOL_BLACK];
      for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
      { vch_next = vch->next_in_room;
        if ( !is_same_group( ch, vch ) )
        { dam = dice( rank, rank*3 );
          chant_damage( ch, vch, dam, CHANT_DRAGON_SLAVE );
        }
      }
    }
  }
  return;
}

void chant_update()
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHANT_DATA *cha;
  CHANT_DATA *cha_next;

  for ( ch = char_list; ch != NULL; ch = ch_next )
  {
    ch_next = ch->next;

    if ( IS_NPC(ch) )
      continue;
    if ( !IS_CLASS(ch,CLASS_SORCERER) )
      continue;

    for ( cha = ch->pcdata->chant; cha != NULL; cha = cha_next )
    {
      cha_next = cha->next;
      cha->wait--;
      if ( cha->wait % chant_table[cha->cn].wait == 0 && chant_table[cha->cn].lines > 1)
      {
        switch( cha->line )
        {
          case 2:
            act( "You chant '$T`n'", ch, NULL, chant_table[cha->cn].line2, TO_CHAR );
            act( "$n chants '$T`n'", ch, NULL, chant_table[cha->cn].line2, TO_ROOM );
            break;
          case 3:
            act( "You chant '$T`n'", ch, NULL, chant_table[cha->cn].line3, TO_CHAR );
            act( "$n chants '$T`n'", ch, NULL, chant_table[cha->cn].line3, TO_ROOM );
            break;
          case 4:
            act( "You chant '$T`n'", ch, NULL, chant_table[cha->cn].line4, TO_CHAR );
            act( "$n chants '$T`n'", ch, NULL, chant_table[cha->cn].line4, TO_ROOM );
            break;
          case 5:
            act( "You chant '$T`n'", ch, NULL, chant_table[cha->cn].line5, TO_CHAR );
            act( "$n chants '$T`n'", ch, NULL, chant_table[cha->cn].line5, TO_ROOM );
            break;
          default:
            act( "!Error!", ch, NULL, NULL, TO_CHAR );
            act( "!Error!", ch, NULL, NULL, TO_ROOM );
            break;
        }
        cha->line++;
      }
      if ( cha->wait <= 0 )
        chant_cast( ch, cha );
    }

  }

  return;
}

void regen_update()
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  sh_int hitgain, managain, movegain, runes;

  for ( ch = char_list; ch != NULL; ch = ch_next )
  {
    ch_next = ch->next;

    if ( IS_NPC(ch) )
    {
      if ( ch->level > 95 && ch->hit < ch->max_hit )
      { ch->hit += 1000 + dice( 10, 10 );
        ch->hit = UMIN( ch->hit, ch->max_hit );
      }
      if ( is_affected(ch,skill_lookup("flame breath")) )
        damage( ch, ch, dice(10,20), DAM_FLAME_BREATH );
      continue;
    }

    /* deal with spammers */
    if ( ch->pcdata->extras2[GLOBALS] > 0 )
      ch->pcdata->extras2[GLOBALS] = UMAX( 0, ch->pcdata->extras2[GLOBALS] - 2 );
    if ( ch->pcdata->extras2[GLOBALS] < 0 )
    {
      ch->pcdata->extras2[GLOBALS] = UMIN( 0, ch->pcdata->extras2[GLOBALS] + 2 );
      if ( ch->pcdata->extras2[GLOBALS] == 0 )
      {
        stc( "You may use global channels again.\n\r", ch );
        stc( "Show some restraint this time.\n\r", ch );
      }
    }

    /* Update evaluation */
    eval(ch);

    if ( IS_SUIT(ch) && ch->pcdata->suit[SUIT_FUEL] > 0 )
    { ch->pcdata->suit[SUIT_FUEL] -= dice(1,4);
      if ( ch->pcdata->suit[SUIT_FUEL] <= 0 )
      { ch->pcdata->suit[SUIT_FUEL] = 0;
        send_to_char( "Your suit has run out of fuel!\n\r", ch );
      }
    }


    /*
     * From this point on, all ch's are player characters.
     * Mobs dealt with previously.
     */
    hitgain = dice(1,4);
    managain = dice(1,4);
    movegain = dice(1,4);

    if ( ch->in_room != NULL && ch->position > POS_STUNNED &&
         IS_SET(ch->in_room->room_flags, ROOM_REGEN) )
    {
      hitgain += 100;
      managain += 100;
      movegain += 100;
    }

    if ( ch->pcdata->extras[TIMER] > 0 )
      ch->pcdata->extras[TIMER]--;

    /* mortal regen */
    if ( ch->level == 1 )
    { hitgain += dice( 4, 100 );
      managain += dice( 4, 100 );
      movegain += dice( 4, 100 );
    }

    if ( IS_CLASS(ch,CLASS_SAIYAN) && ch->level >= 2 )
    { if ( ch->pcdata->powers[S_POWER] > 6 )
	ch->pcdata->powers[S_POWER] -= URANGE( 5, ch->pcdata->powers[S_POWER] / 50, 200 - ch->pcdata->spirit );
      if ( !is_affected(ch,gsn_sleep) )
      {
        if ( ch->pcdata->powers[S_SPEED] > ch->pcdata->powers[S_SPEED_MAX]/2 )
  	  ch->pcdata->powers[S_SPEED] -= UMAX( ch->pcdata->powers[S_SPEED] / 50, 1 );
        if ( ch->pcdata->powers[S_STRENGTH] > ch->pcdata->powers[S_STRENGTH_MAX]/2 )
	  ch->pcdata->powers[S_STRENGTH] -= UMAX( ch->pcdata->powers[S_STRENGTH] / 50, 1 );
        if ( ch->pcdata->powers[S_AEGIS] > ch->pcdata->powers[S_AEGIS_MAX]/2 )
	  ch->pcdata->powers[S_AEGIS] -= UMAX( ch->pcdata->powers[S_AEGIS] / 50, 1 );
      }
      if ( ch->position == POS_SLEEPING )
      { hitgain += ( 50 + ch->pcdata->body * 8 ) * ( 100 -
		   ( 100 * ch->pcdata->powers[S_POWER] / ch->pcdata->powers[S_POWER_MAX] ) ) / 100;
	managain += hitgain / 5;
	movegain += hitgain * 6 / 5;
      }

    }
    else if ( IS_CLASS(ch,CLASS_FIST) && ch->level >= 2 )
    { if ( ch->fighting == NULL && ch->pcdata->powers[F_KI] > 0 
        && !IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA ) )
      { send_to_char( "The aura of Ki around you collapses.\n\r", ch );
	act( "The aura of Ki around $n collapses.", ch, NULL, NULL, TO_ROOM );
	ch->pcdata->powers[F_KI] = 0;
      }
      if ( ch->position < POS_FIGHTING && ch->position > POS_STUNNED )
      { hitgain += dice( 10, ch->pcdata->body );
	managain += dice( 10, ch->pcdata->mind );
	movegain += dice( 10, ch->pcdata->body );
      }
      if ( IS_SET(ch->pcdata->actnew,NEW_INNERFIRE) )
      { REMOVE_BIT(ch->pcdata->actnew,NEW_INNERFIRE);
        act( "Your Ki flares up, burning away all impurities!", ch, NULL, NULL, TO_CHAR );
        act( "$n explodes in ki flame!", ch, NULL, NULL, TO_ROOM );
        while ( ch->affected )
          affect_remove( ch, ch->affected );
      }

      if ( IS_SET(ch->pcdata->actnew,NEW_FIGUREEIGHT) && number_percent() < (100 - (ch->pcdata->body/2)))
	REMOVE_BIT(ch->pcdata->actnew,NEW_FIGUREEIGHT);
      if ( IS_SET(ch->pcdata->actnew,NEW_NOBLOCK ) )
        REMOVE_BIT(ch->pcdata->actnew,NEW_NOBLOCK );
      if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
      { if ( ch->move < 250 )
        { act( "Your phoenix aura collapses.", ch, NULL, NULL, TO_CHAR );
          act( "The aura of flame around $n collapses.", ch, NULL, NULL, TO_ROOM );
          REMOVE_BIT(ch->pcdata->actnew,NEW_PHOENIX_AURA);
          ch->hitroll -= 15;
          ch->damroll -= 15;
        }
        else
          ch->move -= 150+dice(10,10);
      }
    }
    else if ( IS_CLASS(ch,CLASS_PATRYN) && ch->level >= 2 )
    { if ( ch->position > POS_STUNNED )
      { runes = get_runes(ch,RUNE_WATER,TORSO);
	hitgain += runes * 75 + dice( runes*2, ch->pcdata->powers[P_WATER]/2+1 );
	managain += runes * 50 + dice( runes*2, ch->pcdata->powers[P_WATER]/2+1 );
	movegain += runes * 20 + dice( runes*2, ch->pcdata->powers[P_WATER]/2+1 );
      }
      if ( IS_SET(ch->pcdata->actnew,NEW_CIRCLE) )
      { if ( ch->position < POS_RESTING || ch->position > POS_RESTING || IS_SUIT(ch) )
        { send_to_char( "Your circle is broken.\n\r", ch  );
          REMOVE_BIT(ch->pcdata->actnew,NEW_CIRCLE);
        }
        else
        { hitgain += ch->pcdata->powers[P_WATER]*5;
          managain += ch->pcdata->powers[P_ENERGY]*8;
          movegain += ch->pcdata->powers[P_AIR]*4;
        }
      }
    }
    else if ( IS_CLASS(ch,CLASS_SORCERER) && ch->level >= 2 )
    {

      if ( IS_SET(ch->pcdata->actnew,NEW_DRAGON_SLAVE) )
        REMOVE_BIT(ch->pcdata->actnew,NEW_DRAGON_SLAVE);
    }
    else if ( IS_CLASS(ch,CLASS_MAZOKU) && ch->level >= 2)
    {
      if ( ch->position >= POS_STUNNED )
      {
        if ( IS_SET(ch->pcdata->powers[M_SET],M_TRUE) )
        { hitgain += 150 + dice(6,ch->pcdata->powers[M_ASTRAL]);
          managain += 150 + dice(6,ch->pcdata->powers[M_ASTRAL]);
          movegain += 150 + dice(6,ch->pcdata->powers[M_ASTRAL]);
        }
        else
        {
          hitgain += dice(6,10);
          managain += dice(6,10);
          movegain += dice(6,10);
        }
      }
      if ( ch->pcdata->powers[M_ESSENSE] < 10000 )
        ch->pcdata->powers[M_ESSENSE]++;
    }

    if ( is_affected(ch,skill_lookup("flame breath")) )
      damage( ch, ch, dice(10,20), DAM_FLAME_BREATH );

    if ( ch->position <= POS_STUNNED )
    { if ( IS_SET(ch->pcdata->actnew,NEW_RECOVERY) )
        REMOVE_BIT(ch->pcdata->actnew,NEW_RECOVERY);
    }
    if ( IS_SET(ch->pcdata->actnew,NEW_RECOVERY) )
    { 
      if ( ch->position <= POS_STUNNED )
      { if ( IS_SET(ch->pcdata->actnew,NEW_RECOVERY) )
          REMOVE_BIT(ch->pcdata->actnew,NEW_RECOVERY);
      }
      else if ( ch->position != POS_FIGHTING || ( IS_CLASS(ch,CLASS_SORCERER) 
             && ch->pcdata->powers[SORC_SPEC] == SCHOOL_WHITE ) )
      {
        send_to_char( "You feel your wounds healing at an accelerated rate.\n\r", ch );
        if ( IS_CLASS( ch, CLASS_SORCERER ) )
          if ( ch->position == POS_FIGHTING && !IS_NPC(ch->fighting) )
            hitgain += dice( (ch->pcdata->powers[SCHOOL_WHITE]/20)+1, ch->pcdata->will*3 );
          else
            hitgain += dice( (ch->pcdata->powers[SCHOOL_WHITE]/10)+1, ch->pcdata->will*3 );
        else
          hitgain += dice( 3, ch->pcdata->will*3 );
      }
    }


    if ( IS_SET(ch->pcdata->actnew,NEW_FLEEING) )
      REMOVE_BIT(ch->pcdata->actnew,NEW_FLEEING);
    if ( IS_SET(ch->pcdata->actnew,NEW_BALL_LIGHTNING) )
      REMOVE_BIT(ch->pcdata->actnew,NEW_BALL_LIGHTNING);

    ch->hit = UMIN( ch->hit + hitgain, ch->max_hit );
    ch->mana = UMIN( ch->mana + managain, ch->max_mana );
    ch->move = UMIN( ch->move + movegain, ch->max_move );

    if ( ch->position == POS_STUNNED || ch->position == POS_INCAP || ch->position == POS_MORTAL )
      update_pos( ch );
  }

  return;
}

void vote_update()
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int total = 0, i;
  int percent[5] = { 0, 0, 0, 0, 0 };

  /* if there isn't a vote going on, done */
  if ( !voting->active )
    return;

  /* if the vote isn't finished, done */
  voting->pulses--;
  if ( voting->pulses > 0 )
    return;

  for ( i = 1; i <= voting->num; i++ )
    total += voting->votes[i];

  if ( total > 0 )
  {
    for ( i = 1; i <= voting->num; i++ )
      percent[i-1] = voting->votes[1] * 100 / total;
  }

  sprintf( buf, "---*---*---*---*---*---*--- Results ---*---*---*---*---*---*---\n\r" );
  sprintf( buf2, "`B%s`n\n\r", voting->question );
  strcat( buf, buf2 );
  sprintf( buf2, " 1) <%d> %d%% %s\n\r", voting->votes[1], percent[0], voting->line1 );
  strcat( buf, buf2 );
  sprintf( buf2, " 2) <%d> %d%% %s\n\r", voting->votes[2], percent[1], voting->line2 );
  strcat( buf, buf2 );
  if ( voting->num >= 3 )
  { sprintf( buf2, " 3) <%d> %d%% %s\n\r", voting->votes[3], percent[2], voting->line3 );
    strcat( buf, buf2 );
  }
  if ( voting->num >= 4 )
  { sprintf( buf2, " 4) <%d> %d%% %s\n\r", voting->votes[4], percent[3], voting->line4 );
    strcat( buf, buf2 );
  }
  if ( voting->num >= 5 )
  { sprintf( buf2, " 5) <%d> %d%% %s\n\r", voting->votes[5], percent[4], voting->line5 );
    strcat( buf, buf2 );
  }

  sprintf( buf2, "---*---*---*---*---*---*--- Results ---*---*---*---*---*---*---\n\r" );
  strcat( buf, buf2 );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  { vch_next = vch->next;
    if ( !IS_NPC(vch) )
      send_to_char( buf, vch );
  }

  /* reset voting info */
  voting->active = FALSE;
  voting->pulses = 0;
  free_string( voting->question );
  voting->question = str_dup( "none" );
  for( i = 1; i <= voting->num; i++ )
    voting->votes[i] = 0;
  free_string( voting->line1 );
  free_string( voting->line2 );
  free_string( voting->line3 );
  free_string( voting->line4 );
  free_string( voting->line5 );
  voting->line1 = str_dup( "none" );
  voting->line2 = str_dup( "none" );
  voting->line3 = str_dup( "none" );
  voting->line4 = str_dup( "none" );
  voting->line5 = str_dup( "none" );
  voting->num = 0;

  return;
}

/* the auction update - another very important part*/

void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
        if (--auction->pulse <= 0) /* decrease pulse */
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
            if (auction->bet > 0)
                sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"), auction->bet);
            else
                sprintf (buf, "%s: going %s (no bid received yet).", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"));

            talk_auction (buf);
            break;

            case 3 : /* SOLD! */

            if (auction->bet > 0)
            {
                sprintf (buf, "%s sold to %s for %d.",
                    auction->item->short_descr,
                    IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
                    auction->bet);
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                act ("The auctioneer appears before you in a puff of smoke and hands you $p.",
                     auction->buyer,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n, and hands $m $p",
                     auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold += auction->bet; /* give him the money */

                auction->item = NULL; /* reset item */

            }
            else /* not sold */
            {
                sprintf (buf, "No bids received for %s - object has been removed.",auction->item->short_descr);
                talk_auction(buf);
                act ("The auctioneer appears before you to return $p to you.",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n to return $p to $m.",
                      auction->seller,auction->item,NULL,TO_ROOM);
                obj_to_char (auction->item,auction->seller);
                auction->item = NULL; /* clear auction */

            } /* else */

            } /* switch */
        } /* if */
} /* func */


void save_wholist()
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char startbuf[MAX_STRING_LENGTH];
    char whofile[MAX_INPUT_LENGTH];
    char *point;
    DESCRIPTOR_DATA *d;
    FILE *fp;
    int iLevelLower;
    int iLevelUpper;
    int nMatch;
    int stat, invised;
    bool fClassRestrict;
    bool fImmortalOnly;
 

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fImmortalOnly  = FALSE;

    sprintf( startbuf, "\n" );
    sprintf( buf, "<HTML>\n<HEAD>\n" );
    sprintf( buf2, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">\n" );
    strcat( buf, buf2 );
    sprintf( buf2, "<META NAME=\"GENERATOR\" CONTENT=\"Mozilla/4.03 [en] (Win95; I) [Netscape]\">" );
    strcat( buf, buf2 );
    sprintf( buf2, "<TITLE>Wholist</TITLE>\n</HEAD>\n" );
    strcat( buf, buf2 );
    sprintf( buf2, "<BODY TEXT=\"#008080\" BGCOLOR=\"#000000\" LINK=\"#FF0000\" VLINK=\"#990000\" ALINK=\"#FFFF00\">\n<H3><TT>\n" );
    strcat( buf, buf2 );
    sprintf( buf2, "\n\r---------------------------------------------------------------------------\n<FONT COLOR=\"#00FFFF\">\n" );
    strcat( buf, buf2 );


    /*
     * Now show matching chars.
     */
    nMatch = 0;
    invised = 0;
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *legend;
	char const *status;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || IS_SET(d->character->act,PLR_WIZINVIS) )
	{ invised++;
	  continue;
	}

	wch   = ( d->original != NULL ) ? d->original : d->character;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fImmortalOnly  && wch->level < LEVEL_HERO ) )
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for status.
	 */
	switch ( wch->level )
	{
          default: stat = wch->pcdata->extras2[EVAL];
                if ( stat <= 9 )        status = "freshmeat";
                else if ( stat <= 19 )  status = "2nd class";
                else if ( stat <= 29 )  status = "1st class";
                else if ( stat <= 39 )  status = "Powerful&nbsp;";
                else if ( stat <= 49 )  status = "Maniacal&nbsp;";
                else if ( stat <= 59 )  status = "Unholy&nbsp;&nbsp;&nbsp;";
                else if ( stat <= 64 )  status = "B.A.M.F.&nbsp;";
                else
                { switch( wch->class )
                  { default:    status = "W I E R D";       break;
                    case 1:     status = "&nbsp;-=SSJ=-&nbsp;";       break;
                    case 2:     status = "Rune Lord";       break;
                    case 3:     status = "&nbsp;Shaolin&nbsp;";       break;
                    case 4:
                      if ( wch->pcdata->powers[SORC_SPEC] == 2 )
                        status = "&nbsp;Warlock&nbsp;";
                      else if ( wch->pcdata->powers[SORC_SPEC] == 8 )
                        status = "Priestess";
                      else
                        status = "Archmagus";
                      break;
                  }
                }

                break;


	case 1:				status = "mortal&nbsp;&nbsp; ";	break;
	case 17:			status = "Hero&nbsp;&nbsp;&nbsp; ";	break;
	case MAX_LEVEL:			status = "Admin&nbsp;&nbsp; ";  break;
	}



	/*
	 * Figure out what to print for legend.
	 */
	switch ( wch->pcdata->extras[LEGEND] )
	{
	default:	legend = "->error<-";			break;
	case 0:		legend = "Peasant&nbsp; ";			break;
	case 1:		legend = "Citizen&nbsp; ";			break;
	case 2:		legend = "Celebrity";			break;
	case 3:		legend = "Champion ";			break;
	case 4:		legend = "Hero&nbsp;&nbsp;&nbsp;&nbsp; ";	break;
	case 5:		legend = "Superhero";			break;
	case 6:		legend = "Saviour&nbsp; ";			break;
	case 7:		legend = "Guardian ";			break;
	case 8:		legend = "Herald&nbsp;&nbsp; ";			break;
	case 9:		legend = "Ancient&nbsp; ";			break;
	case 10:	legend = "Messiah&nbsp; ";			break;
	}

	if ( wch->level == MAX_LEVEL )
	  legend = "Bastard!!";
	/*
	 * Format it up.
	 */

	sprintf( buf2, "<BR>" );
	strcat( buf, buf2 );
	sprintf( buf2, "\n[%s][%s] %s%s %s %s",
	    status,
	    legend,
	    ( wch->level > LEVEL_HERO ? "" : "" ),
	  wch->name, 
	    wch->pcdata->title,
	    ( wch->pcdata->clan[CLAN] > 0 ? clan_table[wch->pcdata->clan[CLAN]].title : "") );
	point = buf2;
	while ( *point != '\0' )
	{ if ( *point == '<' )
	    *point = '{';
	  if ( *point == '>' )
	    *point = '}';
	  point++;
	}
	strcat( buf, buf2 );
    }

    sprintf( buf2, "\n</FONT><FONT COLOR=\"#00FF00\"><BR><BR>There %s %d visible player%s out of %d in the game.\n",
    nMatch == 1 ? "is" : "are",
    nMatch, nMatch == 1 ? "" : "s",
    nMatch + invised );
    strcat( buf, buf2 );
    sprintf( buf2, "</FONT><BR>---------------------------------------------------------------------------\n" );
    strcat( buf, buf2 );

    /* end of htmlizing */
    sprintf( buf2, "</TT></H3></BODY>\n</HTML>\n" );
    strcat( buf, buf2 );
    colorize( startbuf, buf, FALSE );

    fclose( fpReserve );
    sprintf( whofile, "%s", "~/public_html/wholist.html" );

    if ( ( fp = fopen( whofile, "w" ) ) == NULL )
    { bug( "save_wholist: fopen", 0 );
      perror( whofile );
    }
    else
    {
      fprintf( fp, startbuf );
    }

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Save Leader- and Loserboards to data files in ~/public_html
 */
void save_llboards( bool leader)
{ char leaders[MAX_STRING_LENGTH];
  char losers[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char leaderpath[MAX_STRING_LENGTH];
  char loserpath[MAX_STRING_LENGTH];
  FILE *fp;
  int i;

  sprintf( leaderpath, "../area/leaders.txt" );
  sprintf( loserpath, "../area/losers.txt" );

  if ( leader )
  { sprintf( leaders, "%s~\n%s~\n%d %d %d %d\n",
      leaderboard[0].name,    leaderboard[0].clan,    leaderboard[0].pkills,
      leaderboard[0].pdeaths, leaderboard[0].assessment,
      (int)leaderboard[0].lastkill );
    for ( i = 1; i < MAX_LEADERS+5; i++ )
    { sprintf( buf, "%s~\n%s~\n%d %d %d %d\n",
  	leaderboard[i].name,	leaderboard[i].clan,	leaderboard[i].pkills,
    	leaderboard[i].pdeaths,	leaderboard[i].assessment,
        (int)leaderboard[i].lastkill );
      strcat( leaders, buf );
    }

    if ( ( fp = fopen( leaderpath, "w" ) ) == NULL )
    { bug( "save_llboards: fopen", 0 );
      perror( leaderpath );
    }
    else
    {
      fprintf( fp, leaders );
    }

    fclose( fp );
  }
  else
  { sprintf( losers, "%s~\n%s~\n%d %d %d\n",
      loserboard[0].name,    loserboard[0].clan,      loserboard[0].pkills,
      loserboard[0].pdeaths, loserboard[0].assessment );
    for ( i = 1; i < 10; i++ )
    { sprintf( buf, "%s~\n%s~\n%d %d %d\n",
	loserboard[i].name,    loserboard[i].clan,	loserboard[i].pkills,
	loserboard[i].pdeaths, loserboard[i].assessment );
      strcat( losers, buf );
    }

    if ( ( fp = fopen( loserpath, "w" ) ) == NULL )
    { bug( "save_llboards: fopen", 0 );
      perror( loserpath );
    }
    else
    {
      fprintf( fp, losers );
    }

    fclose( fp );
  }
  return;
}

void save_clanboard()
{ FILE *fp;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char clanpath[MAX_INPUT_LENGTH];
  int i, j;

  sprintf( clanpath, "../area/clan.txt" );

  buf[0] = '\0';
  for ( i = 0; i <= MAX_CLANS; i++ )
  {
    for ( j = 0; j <= MAX_CLANS; j++ )
    { sprintf( buf2, "%d ", clan_table[i].kills[j] );
      strcat( buf, buf2 );
    }
    sprintf( buf2, "%d %d %d %d %d\n", clan_table[i].pkills, clan_table[i].pdeaths,
    	clan_table[i].mobkills, clan_table[i].mobdeaths, clan_table[i].gold );
    strcat( buf, buf2 );
    for ( j = 0; j <= MAX_CLANS; j++ )
    {
      sprintf( buf2, "%d ", clan_table[i].diplomacy[j] );
      strcat( buf, buf2 );
    }
  }

  fclose( fpReserve );

  if ( ( fp = fopen( clanpath, "w" ) ) == NULL )
  { bug( "save_clanboards: fopen", 0 );
    perror( clanpath );
  }
  else
  {
    fprintf( fp, buf );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  return;
}

void bug_check()
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char log_buf[MAX_STRING_LENGTH];

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next;
    if ( !IS_NPC(vch) )
      continue;
    if ( vch->desc != NULL )
      continue;

    if ( vch->level - 5 > vch->pIndexData->level || vch->max_hit > 40000 )
    {
      sprintf( log_buf, "Bugged mob: %s (%d) lvl:%d hp:%d extracted.", vch->pIndexData->short_descr, vch->pIndexData->vnum, vch->level, vch->max_hit );
      log_string( log_buf );
      extract_char( vch, TRUE );
    }
  }
  return;
}


/*
 * Handle all kinds of epdates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_regen;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int     pulse_second;
    static int pulse_db_dump;	/* OLC 1.1b */

    /* OLC 1.1b */
    if ( --pulse_db_dump  <= 0 )
    {
	pulse_db_dump	= PULSE_DB_DUMP;
	do_asave( NULL, "" );
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( FALSE );
    }

    if ( --pulse_regen <= 0 )
    {   pulse_regen	= number_range( PULSE_REGEN - 4 , PULSE_REGEN + 4 );
	regen_update();
	vote_update();

	/* Needs modding for directories, if you even want to use it. */
	// save_wholist(); 

    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	pulse_point     = number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 );
	weather_update	( );
	char_update	( );
	obj_update	( );
	inf_update	( );
        save_clanboard	( );
        // save_llboards   ( (flipflop = !flipflop) );
        // bug_check	( );
        memory_check	( );
    }

    if ( --pulse_second <= 0 )
    {
      pulse_second = PULSE_PER_SECOND;
      second_update();
    }

    auction_update();
    aggr_update( );
    chant_update();
    tail_chain( );
    return;
}
