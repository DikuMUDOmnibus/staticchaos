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

PKILL_DATA	*	pkill_free;

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};



/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
void	paradox		args( ( CHAR_DATA *ch ) );
bool	has_jboots	args( ( CHAR_DATA *ch ) );
void	record_pkill	args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool hum ) );


void move_char( CHAR_DATA *ch, int door )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->to_room      ) == NULL )
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
    }

    /* removed temporarily
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR) )
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }
    */

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( room_is_private( to_room ) && ch->level >= 2 )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	// int iClass;
	int move;

	/* removed, stupid.
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass != ch->class
	    &&   to_room->vnum == class_table[iClass].guild )
	    {
		send_to_char( "You aren't allowed in there.\n\r", ch );
		return;
	    }
	}
	*/

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_SUIT(ch)
	      && !(IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_WINGS)))
	    {
		send_to_char( "You can't fly.\n\r", ch );
		return;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    /*
	     * Suggestion for flying above water by Sludge
	     */
	    if ( IS_AFFECTED( ch, AFF_FLYING ) )
	        found = TRUE;
	    if ( IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_WINGS) )
	      found = TRUE;
	    if ( IS_SUIT(ch) )
	      found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

	if ( ch->move < move )
	{
	    send_to_char( "You are too exhausted.\n\r", ch );
	    return;
	}

	if ( IS_SUIT(ch) && ch->pcdata->suit[SUIT_FUEL] < 1 )
	{ send_to_char( "You don't have enough fuel!\n\r", ch );
	  return;
	}

	unstance( ch );
	if ( IS_CLASS(ch,CLASS_FIST) && ch->pcdata->powers[F_KI] > 0 )
	  ch->pcdata->powers[F_KI] = 0;
	if ( IS_SUIT(ch) )
	{ WAIT_STATE(ch, (suit_table[ch->pcdata->suit[SUIT_NUMBER]].speed - 160)/10);
	  ch->pcdata->suit[SUIT_FUEL]--;
	}
	else if ( !has_jboots( ch ) )
	  WAIT_STATE( ch, 1 );
	ch->move -= move;
    }

    /* this is some really poor logic right here......*/
    if ( IS_NPC(ch) )
    {
      if ( !IS_AFFECTED(ch,AFF_SNEAK) )
        act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );
    }
    else if ( !IS_SET(ch->act, PLR_WIZINVIS) )
    { 
      if ( !IS_SUIT(ch) )
      {
        if ( !IS_AFFECTED(ch,AFF_SNEAK) )
        {
  	  if ( is_cloaked(ch,ch) && number_percent() > 50 )
  	    act( "You notice a shimmering in the air to the $T.", ch, NULL, dir_name[door], TO_ROOM );
	  else
	    act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );
	}
      }
      else
      {
        if ( !is_cloaked(ch,ch) )
	  act( "$n leaves $T, piloting $s suit.", ch, NULL, dir_name[door], TO_ROOM );
        else if ( number_percent() > 50 )
          act( "You notice a shimmering in the air to the $T.",ch,NULL,dir_name[door],TO_ROOM);
      }
    }

    if ( IS_CLASS(ch,CLASS_SAIYAN) &&
        (IS_SET(ch->pcdata->actnew,NEW_KAME_1) || IS_SET(ch->pcdata->actnew,NEW_KAME_2)) )
    {
      stc( "You lose your concentration, and dissolve the attack.\n\r", ch );
      if ( IS_SET(ch->pcdata->actnew,NEW_KAME_1) )
        REMOVE_BIT(ch->pcdata->actnew,NEW_KAME_1);
      if ( IS_SET(ch->pcdata->actnew,NEW_KAME_2) )
        REMOVE_BIT(ch->pcdata->actnew,NEW_KAME_2);
    }

    char_from_room( ch );
    char_to_room( ch, to_room );
    if ( IS_NPC(ch) )
    {
      if ( !IS_AFFECTED(ch,AFF_SNEAK) )
        act( "$n has arrived.", ch, NULL, NULL, TO_ROOM );
    }
    else if ( !IS_SET(ch->act, PLR_WIZINVIS) )
    { 
      if ( !IS_SUIT(ch) )
      {
        if ( !IS_AFFECTED(ch,AFF_SNEAK) )
        {
          if ( is_cloaked(ch,ch) && number_percent() > 50 )
            act( "You notice a shimmering in the air to the $T.", ch, NULL, dir_name[door], TO_ROOM );
          else
            act( "$n has arrived.", ch, NULL, dir_name[door], TO_ROOM );
        }
      }
      else
      {
        if ( !is_cloaked(ch,ch) )
          act( "A $T suit has arrived, piloted by $n.", ch, NULL, suit_table[ch->pcdata->suit[SUIT_NUMBER]].name, TO_ROOM );
        else if ( number_percent() > 50 && !IS_AFFECTED(ch,AFF_SNEAK) )
          act( "You notice a shimmering in the air to the $T.",ch,NULL,dir_name[door],TO_ROOM);
      }
    }

    do_look( ch, "auto" );

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;
	if ( fch->master == ch && fch->position == POS_STANDING
	  && !IS_TIED( fch ) )
	{
	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door );
	}
    }

    mprog_entry_trigger(ch);
    mprog_greet_trigger(ch);
    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_pick_lock] )
    {
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* pick the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }

	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
    case POS_SITTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_STANDING: 
	send_to_char( "You sleep.\n\r", ch );
	act( "$n sleeps.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }
    if ( victim->position < POS_SLEEPING )
    {
     	stc( "You can't wake them.\n\r", ch );
     	return;
    }


    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    victim->position = POS_STANDING;
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_sneak] )
    {
	af.type      = gsn_sneak;
	af.duration  = 100 + dice(5,5);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
    }

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_hide] )
	SET_BIT(ch->affected_by, AFF_HIDE);

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_recall( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
  
    act( "$n begs Alathon for transportation!", ch, 0, 0, TO_ROOM );

    if ( IS_NPC(ch) )
    {  if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
      {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
      }
    }
    else if ( IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
    {
      stc( "Why bother?\n\r", ch );
      return;
    }
    else
    { if ( ( location = get_room_index( ch->pcdata->extras[6] ) ) == NULL )
      { location = get_room_index( 806 );
        ch->pcdata->extras[6] = 806;
        send_to_char( "Your home has been reset to Inner Ward.\n\r", ch );
      }
      if ( IS_SET(location->room_flags,ROOM_PRIVATE) )
      {
        stc( "Fucking pussy trash.\n\r", ch );
        WAIT_STATE(ch,1200);
        location = get_room_index( ROOM_VNUM_TEMPLE );
      }
      if ( ch->pcdata->extras[TIMER] > 16 )
      { send_to_char( "Alathon says '`mNo recalling right after capping.`n'\n\r", ch );
        return;
      }
    }

    if ( ch->in_room == location )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE) )
    {
	send_to_char( "Alathon is busy, please leave a message at the beep.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
    {
      send_to_char( "Duhhhhhh what? don't you mean FLEE?\n\r", ch );
      return;
    }

    /* No recalling with combos prepped */
    if ( !IS_NPC(ch) && IS_CLASS(ch,CLASS_FIST) )
      ch->pcdata->powers[F_KI] = 0;

    ch->move -= 50;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int *pAbility;
    char *pOutput;
    int cost, statcheck;
    int scost[4];
    int max = 0;
    int num, lcost;
    bool multi = FALSE;

    if ( IS_NPC(ch) )
	return;

    statcheck = 0;

    if ( ch->class == CLASS_SAIYAN )
    { scost[0] = ch->pcdata->powers[S_POWER_MAX];
      scost[1] = ch->pcdata->powers[S_SPEED_MAX] * 100;
      scost[2] = ch->pcdata->powers[S_AEGIS_MAX] * 100;
      scost[3] = ch->pcdata->powers[S_STRENGTH_MAX] * 100;
    }
    lcost = 2000000 * (ch->pcdata->extras[LEGEND]+1);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( is_number( arg2 ) )
      num = atoi( arg2 );
    else num = 0;

    if ( num > 25000 || num < 0 )
    { send_to_char( "Invalid entry, you may train from 1 to 25000 at a time.\n\r", ch );
      return;
    }
    /*
     *  No check for trainer.
     */
    if ( arg1[0] == '\0' )
    {
    sprintf( buf, "You can train the following:\n\r" );
    send_to_char( buf, ch );
    sprintf( buf, "    Hp: %6d   Mana: %6d   Move: %6d\n\r",
	ch->max_hit,	ch->max_mana,	ch->max_move );
    send_to_char( buf, ch );
    sprintf( buf, "  Body: %6d   Mind: %6d Spirit: %6d  Willpower: %6d\n\r",
	ch->pcdata->body * ch->pcdata->body * 10, ch->pcdata->mind * ch->pcdata->mind * 10,
	ch->pcdata->spirit * ch->pcdata->spirit * 10, ch->pcdata->will * ch->pcdata->will * 15 );
    send_to_char( buf, ch );
    if ( ch->class == CLASS_SAIYAN )
    { sprintf( buf, " Power: %6d  Speed: %6d  Aegis: %6d   Strength: %6d\n\r",
			scost[0], scost[1], scost[2], scost[3] );
      send_to_char( buf, ch );
    }
    sprintf( buf, "Avatar:  1000  Primal: %6d\n\rAll costs are in experience points.\n\r",
	(ch->pcdata->primal+1) * 500 );
    send_to_char( buf, ch );
    if ( ch->pcdata->extras[LEGEND] < 10 )
    { sprintf( buf, "Legend: %d experience.\n\r", lcost );
      send_to_char( buf, ch );
    }

    return;
    }

    cost = 0;

    if ( !str_cmp( arg1, "hp" ) )
    {
	if ( num > 0 )
	{ cost = num * ( (ch->max_hit + ch->max_hit + num) / 2 ); multi = TRUE; }
        else cost   = ch->max_hit;
	pAbility    = &ch->max_hit;
	pOutput     = "hp";
	max         = 25000;
    }

    else if ( !str_cmp( arg1, "mana" ) )
    {
	if ( num > 0 )
	{ cost = num * ( ( ch->max_mana + ch->max_mana + num) / 2 ); multi = TRUE; }
        else cost   = ch->max_mana;
	pAbility    = &ch->max_mana;
	pOutput     = "mana";
	max         = 25000;
    }

    else if ( !str_cmp( arg1, "move" ) )
    {
	if ( num > 0 )
	{ cost = num * ( ( ch->max_move + ch->max_move + num ) / 2 ); multi = TRUE; }
	else cost   = ch->max_move;
	pAbility    = &ch->max_move;
	pOutput     = "move";
	max	    = 25000;
    }

    else if ( !str_cmp( arg1, "body" ) )
    {
	cost		= ch->pcdata->body * ch->pcdata->body * 10;
	pAbility	= &ch->pcdata->body;
	pOutput		= "body";
	max		= class_table[ch->class].body;
    }

    else if ( !str_cmp( arg1, "mind" ) )
    {
	cost		= ch->pcdata->mind * ch->pcdata->mind * 10;
	pAbility	= &ch->pcdata->mind;
	pOutput		= "mind";
	max		= class_table[ch->class].mind;
    }

    else if ( !str_cmp( arg1, "spirit" ) )
    {
	cost		= ch->pcdata->spirit * ch->pcdata->spirit * 10;
	pAbility	= &ch->pcdata->spirit;
	max		= class_table[ch->class].spirit;
	pOutput		= "spirit";
    }

    else if ( !str_cmp( arg1, "willpower" ) )
    {
	cost		= ch->pcdata->will * ch->pcdata->will * 15;
	pAbility	= &ch->pcdata->will;
	pOutput		= "willpower";
	max		= class_table[ch->class].will;
    }

    else if ( !str_cmp( arg1, "power" ) )
    {
	if ( ch->class != CLASS_SAIYAN ) return;
	if ( num > 0 )
	{ cost = num * ( (ch->pcdata->powers[S_POWER_MAX]*2 + num) / 2 ); multi = TRUE; }
	else cost	= scost[0];
	pAbility	= &(ch->pcdata->powers[S_POWER_MAX]);
	pOutput		= "fighting power";
	max		= 30000;
    }

    else if ( !str_cmp( arg1, "speed" ) )
    {
	if ( ch->class != CLASS_SAIYAN ) return;
	if ( num > 0 )
	{ cost = num * ( (ch->pcdata->powers[S_SPEED_MAX]*200 + num*100) / 2 ); multi = TRUE; }
	else cost	= scost[1];
	pAbility	= &ch->pcdata->powers[S_SPEED_MAX];
	pOutput		= "maximum speed";
	max		= 1000;
    }

    else if ( !str_cmp( arg1, "aegis" ) )
    {
	if ( ch->class != CLASS_SAIYAN ) return;
	if ( num > 0 )
	{ cost = num * ( (ch->pcdata->powers[S_AEGIS_MAX]*200 + num*100) / 2 ); multi = TRUE; }
	else cost	= scost[2];
	pAbility	= &ch->pcdata->powers[S_AEGIS_MAX];
	pOutput		= "aegis";
	max		= 1000;
    }

    else if ( !str_cmp( arg1, "strength" ) )
    {
	if ( ch->class != CLASS_SAIYAN ) return;
	if ( num > 0 )
	{ cost = num * ( (ch->pcdata->powers[S_STRENGTH_MAX]*200 + num*100) / 2 ); multi = TRUE; }
	else cost	= scost[3];
	pAbility	= &ch->pcdata->powers[S_STRENGTH_MAX];
	pOutput		= "fighting strength";
	max		= 1000;
    }
    else if ( !str_cmp( arg1, "primal" ) )
    {
	if ( num > 0 )
	{ cost = num * ( ((ch->pcdata->primal+1)*500 + (ch->pcdata->primal+1+num)*500 ) / 2 ); multi = TRUE; }
	else cost       = (ch->pcdata->primal+1) * 500;
	pAbility	= &ch->pcdata->primal;
	pOutput		= "primal";
	max		= 500;
    }

    else if ( !str_cmp( arg1, "legend" ) )
    { if ( ch->pcdata->extras[LEGEND] == 10 )
      { send_to_char( "You're as famous as you'll ever be.\n\r", ch );
        return;
      }
      if ( ch->exp < lcost || ch->pcdata->extras2[EVAL] < ch->pcdata->extras[LEGEND]*5 )
      { sprintf( buf, "You need %d experience and/or more power to increase your fame.\n\r",
	lcost );
	send_to_char( buf, ch );
	return;
      }

      send_to_char( "You advance in history!\n\r", ch );
      sprintf( buf, "%s advances in history!", ch->name );
      do_info( ch, buf );
      ch->exp -= lcost;
      ch->pcdata->extras[LEGEND] += 1;
      sprintf( buf, "%s advanced to legend %d.", ch->name, ch->pcdata->extras[LEGEND] );
      log_string( buf );
      return;
    }

    else if ( !str_cmp( arg1, "avatar" ) )
    {
	if ( IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) )
	{ send_to_char( "Not in a private room.\n\r", ch );
	  return;
	}

	if ( ch->pcdata->extras2[PKCOUNT] >= 10 )
	{ send_to_char( "You've been capped too many times, wait a bit.\n\r", ch );
	  return;
	}

	if ( ch->level > 1 )
	{ send_to_char( "But you're already an avatar!\n\r", ch ); return; }

	if ( ch->level < 1 )
	{ send_to_char( "Uhhhh something is fucked up, tell Alathon.\n\r", ch ); return; }

	if ( ch->exp < 1000 )
	{ send_to_char( "You need more experience to become an avatar.\n\r", ch ); return; }

	ch->level = 2;
	ch->exp -= 1000;
	ch->pcdata->condition[COND_FULL] = 40;
	ch->pcdata->condition[COND_THIRST] = 40;
	sprintf( buf, "%s is now an avatar!", ch->name );
	do_info( ch, buf );
	do_save( ch, "" );
	WAIT_STATE( ch, 40 );
	return;
    }
    else
    {
      send_to_char( "Train what?\n\r", ch );
      return;
    }

    /* for the next check */
    if ( num == 0 )
	num = 1;

    if ( max != 0 && ((*pAbility)+num) > max )
    {
	act( "You cannot train your $T any more.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->exp )
    {
	send_to_char( "You don't have enough exp.\n\r", ch );
	return;
    }

  if ( max != 0 )
  { ch->exp -= cost;
    if ( multi )
      *pAbility		+= num;
    else
      *pAbility         += 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
  }
  return;
}


void do_stance( CHAR_DATA *ch, char *argument )
{
  int stance = 0;
  int slot;

  if (IS_NPC(ch))
    return;

  if ( IS_SUIT(ch) )
    return;

  slot = MAX_STANCES+1;

  if ( argument[0] == '\0' )
  {
    if (ch->pcdata->stances[slot])
    {
      ch->pcdata->stances[slot] = 0;
      act( "You relax from your fighting stance.", ch, NULL, NULL, TO_CHAR );
      act( "$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM );
      return;
    }
    else return;
  }
  else if (!str_prefix( argument, "lion" ))	stance = STANCE_LION;
  else if (!str_prefix( argument, "lynx" ))	stance = STANCE_LYNX;
  else if (!str_prefix( argument, "snake" ))	stance = STANCE_SNAKE;
  else if (!str_prefix( argument, "badger" ))	stance = STANCE_BADGER;
  else if (!str_prefix( argument, "ferret" ))	stance = STANCE_FERRET;
  else if (!str_prefix( argument, "hawk" ))	stance = STANCE_HAWK;
  else if (!str_prefix( argument, "eagle" ))	stance = STANCE_EAGLE;
  else if (!str_prefix( argument, "vulture" ))	stance = STANCE_VULTURE;
  else if (!str_prefix( argument, "sparrow" ))	stance = STANCE_SPARROW;
  else if (!str_prefix( argument, "stork" ))	stance = STANCE_STORK;
  else
  {
    send_to_char( "You don't know that stance!\n\r", ch );
    return;
  }
    
  if ( ch->pcdata->stances[slot] )
  {
    send_to_char( "You must unstance before entering another stance.\n\r", ch );
    return;
  }

  switch ( stance )
  {
  case STANCE_LION:
    act( "You charge into the Lion fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n charges into the Lion fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_LYNX:
    act( "You leap into the Lynx fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n leaps into the Lynx fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_SNAKE:
    act( "You slither into the Snake fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n slithers into the Snake fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_BADGER:
    act( "You crouch into the Badger fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n crouches into the Badger fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_FERRET:
    act( "You slink into the Ferret fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n slinks into the Ferret fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_HAWK:
    if ( ch->pcdata->stances[1] < 200 || ch->pcdata->stances[2] < 200 )
    { send_to_char( "You must master the Lion and Lynx stances first.\n\r", ch );
      return;
    }
    act( "You dive into the Hawk fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n dives into the Hawk fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_EAGLE:
    if ( ch->pcdata->stances[2] < 200 || ch->pcdata->stances[3] < 200 )
    { send_to_char( "You must master the Lynx and Snake stances first.\n\r", ch );
      return;
    }
    act( "You climb into the Eagle fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n climbs into the Eagle fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_VULTURE:
    if ( ch->pcdata->stances[3] < 200 || ch->pcdata->stances[4] < 200 )
    { send_to_char( "You must master the Snake and Badger stances first.\n\r", ch );
      return; }
    act( "You circle into the Vulture fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n circles into the Vulture fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_SPARROW:
    if ( ch->pcdata->stances[4] < 200 || ch->pcdata->stances[5] < 200 )
    { send_to_char( "You must master the Badger and Ferret stances first.\n\r", ch );
      return; }
    act( "You flitter into the Sparrow fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n flitters into the Sparrow fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  case STANCE_STORK:
    if ( ch->pcdata->stances[5] < 200 || ch->pcdata->stances[1] < 200 )
    { send_to_char( "You must master the Ferret and Lion stances first.\n\r", ch );
      return; }
    act( "You step into the Stork fighting stance.", ch, NULL, NULL, TO_CHAR );
    act( "$n steps into the Stork fighting stance.", ch, NULL, NULL, TO_ROOM );
    break;
  default:
    send_to_char( "Something is fucked up, better tell Alathon.\n\r", ch );
    return;
  }

  ch->pcdata->stances[slot] = stance;
  WAIT_STATE( ch, 6 );
  return;
}

void unstance( CHAR_DATA *ch )
{
  int stance;

  if ( IS_NPC(ch) )
    return;

  stance = ch->pcdata->stances[MAX_STANCES+1];
  if ( stance  == 0 )
    return;

  ch->pcdata->stances[MAX_STANCES+1] = 0;
  act( "You relax from your fighting stance.", ch, NULL, NULL, TO_CHAR );
  act( "$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM );

  return;
}

void do_autostance( CHAR_DATA *ch, char *argument )
{
  int i;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  if ( IS_SUIT(ch) )
    return;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  { send_to_char( "Pick one:  lion lynx snake badger ferret hawk eagle vulture sparrow stork.\n\r", ch );
    return;
  }

  for ( i = 1; i <= MAX_STANCES; i++ )
  { if ( !str_cmp( arg, stance_table[i].name ) )
    { sprintf( buf, "You now autostance to the %s stance.\n\r", stance_table[i].name );
      send_to_char( buf, ch );
      ch->pcdata->stances[0] = i;
      return;
    }
  }

  send_to_char( "That isn't a stance.\n\r", ch );
  return;
}

void do_finish( CHAR_DATA *ch, char *argument )
{
  char log_buf[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wield;
  int damtype, ceval, veval, i;
  bool pdox = FALSE;
  extern bool peace;
  time_t t;

  if ( peace && !IS_NPC(ch) )
  { send_to_char( "Sorry, peace has been declared.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->in_room->room_flags,ROOM_ARENA) )
  { send_to_char( "There is no capping in the Arena.\n\r", ch );
    return;
  }
  if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
  {
    stc( "Why not sit back and relax?\n\r", ch );
    return;
  }

  if ( ch->position < POS_STANDING && !IS_NPC(ch) )
  {
    send_to_char( "You're too busy.\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL && !IS_NPC(ch) )
  {
    stc( "You're too busy.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "Finish off who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, argument) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Just kill them!\n\r", ch );
    return;
  }
  if ( victim->level < 2 )
  { send_to_char( "Only avatars can be finished off.\n\r", ch );
    return;
  }
  /* Removed for now
  if ( !IS_NPC(ch) && (IS_CLASS(ch,CLASS_SORCERER) || IS_CLASS(victim,CLASS_SORCERER)) )
  {
    stc( "Sorry, sorcerers are currently in the test phase.\n\r", ch );
    return;
  }
  */
  if ( IS_NPC(ch) )
    ceval = 10;
  else
    ceval = ch->pcdata->extras2[EVAL];
  veval = victim->pcdata->extras2[EVAL];

  if ( (ceval < 10 || veval < 10) && !IS_NPC(ch)  )
  { send_to_char( "For your protection, newbies are prevented from pkilling and being pked.\n\r", ch );
    return;
  }
  if ( victim->level > 2 )
  { send_to_char( "You nearly dropped your crack pipe!\n\r", ch );
    return;
  }
  if ( victim->position != POS_MORTAL )
  { send_to_char( "You can only finish off mortally wounded players.\n\r", ch );
    return;
  }

  sprintf( log_buf, "%s finished by %s at %d with %d remaining, inf %d.",
  		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum,
		ch->hit,
		get_inf_dam( ch, victim->name ) );
  log_string( log_buf );
  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    damtype = -1;
  else
    damtype = wield->value[3];

  switch( damtype )
  {
    case -1:
      sprintf( buf, "%s just got their head ripped off by %s.", victim->name, ch->name );
      sprintf( buf1, "You drive your hand into $N's neck and rip off $S head!" );
      sprintf( buf2, "$n drives their hand into your neck and rips off your head!" );
      sprintf( buf3, "$n drives $m hand into $N's neck and rips off $S head!" );
      break;
    case 1:
    case 3:
      sprintf( buf, "%s has been decapitated by %s.", victim->name, ch->name );
      sprintf( buf1, "You growl 'There can be only One!' and decapitate $N!" );
      sprintf( buf2, "$n growls 'There can be only One!' and brings $s %s down on your neck!", wield->short_descr );
      sprintf( buf3, "$n growls 'There can be only One!' and decapitates $N!" );
      break;
    case 2:
    case 11:
      sprintf( buf, "%s just got their eyeballs and brain plucked out by %s.", victim->name, ch->name );
      sprintf( buf1, "You ram your weapon into $N's face and tear it out, removing eyes and brain." );
      sprintf( buf2, "$n rams $s weapon into your face and lobotomizes you." );
      sprintf( buf3, "$n rams $s weapon into $N's face and tears it out, removing eyes and brain." );
      break;
    case 7:
    case 8:
      sprintf( buf, "%s's head has been smashed in by %s.", victim->name, ch->name );
      sprintf( buf1, "You bring your weapon down on $N's head, and brains spray from $S ears!" );
      sprintf( buf2, "$n brings $s weapon down on your head, forcing your brain to vacate the premises!" );
      sprintf( buf3, "$n brings $s weapon down on $N's head, and brains spray from $S ears!" );
      break;
    default:
      sprintf( buf, "%s has been finished off by %s.", victim->name, ch->name );
      sprintf( buf1, "You finish $N off!" );
      sprintf( buf2, "$n finishes you off!" );
      sprintf( buf3, "$n finishes $N off!" );
      break;
  }
  /* use npc short description instead of name keyword */
  if ( IS_NPC(ch) )
    sprintf( buf, "%s just got their head ripped off by %s.", victim->name, ch->short_descr );

  if ( !IS_NPC(ch) && get_inf_dam( ch, victim->name ) < 1000 )
    sprintf( buf, "%s just got Nathored by %s.", victim->name, ch->name );

  act( buf1, ch, NULL, victim, TO_CHAR );
  act( buf2, ch, NULL, victim, TO_VICT );
  act( buf3, ch, NULL, victim, TO_NOTVICT );
  do_fatality( ch, buf );

  sprintf( buf, "%s capped at %d by %s with %d remaining (inflicted %d).",
                victim->name,
                ch->in_room->vnum,
                (IS_NPC(ch) ? ch->short_descr : ch->name),
                ch->hit,
                get_inf_dam( ch, victim->name ) );
  wiznet( buf );
  record_pkill( ch, victim, FALSE );

  if ( !IS_NPC(ch) )
  { 
    if ( ch->pcdata->extras[TIMER] > 10 )
      ch->pcdata->extras[TIMER] += 10;
    else
      ch->pcdata->extras[TIMER] += 20;

    /*
     * Check for Soul Reaver kicking in.
     */
    if ( ch->pcdata->extras[LEGEND] >= 9 )
    {
      act( "Lifeforce flows through your veins!", ch, NULL, NULL, TO_CHAR );
      act( "$n's body courses with power.", ch, NULL, NULL, TO_ROOM );
      i = URANGE( 1, get_inf_dam( ch, victim->name ) * veval / 100, ch->max_hit * veval / 100 );
      ch->hit = UMIN( ch->max_hit, ch->hit + i );
      ch->mana = UMIN( ch->max_mana, ch->mana + i );
      ch->move = UMIN( ch->max_move, ch->move + i );
    }

    /*
     * Check for paradoxing
     * Paradox if victim is in a lower category than the
     * character and NOT a gangster (30) or above.
     */
    if ( ((veval < 10 && ceval >= 10) || (veval < 20 && ceval >= 20)
      || (veval < 30 && ceval >= 30)) && !IS_SET(victim->pcdata->actnew,NEW_FAIR_GAME) )
    {
      pdox = TRUE;
      paradox( ch );
    }
    /* fair cap */
    else
    { victim->pcdata->kills[PD]++;
      if ( veval < 30 || veval > 39 || ceval < 40 )
      { ch->pcdata->kills[PK]++;
        clan_table[ch->pcdata->clan[CLAN]].pkills++;
        clan_table[ch->pcdata->clan[CLAN]].kills[victim->pcdata->clan[CLAN]]++;
        clan_table[victim->pcdata->clan[CLAN]].pdeaths++;
      }
      if ( (ceval <= veval+2) && (ceval+10 > veval) )
      { ch->exp += victim->exp / 2;
        victim->exp /= 2;
      }
      if ( victim->pcdata->extras2[BOUNTY] > 0 )
      {
        ch->gold += victim->pcdata->extras2[BOUNTY];
        sprintf( buf, "You collect a bounty of %d gold.\n\r", victim->pcdata->extras2[BOUNTY] );
        stc( buf, ch );
        victim->pcdata->extras2[BOUNTY] = 0;
      }
      /* check for lowbie capping higher player */
      if ( (veval >= 20 && ceval < 20) || (veval >= 30 && ceval < 30) )
      { if ( !IS_SET(ch->pcdata->actnew,NEW_FAIR_GAME) )
          SET_BIT(ch->pcdata->actnew,NEW_FAIR_GAME);
      }
      /* check for removing fair game */
      if ( IS_SET(victim->pcdata->actnew,NEW_FAIR_GAME) )
        REMOVE_BIT(victim->pcdata->actnew,NEW_FAIR_GAME);

      /* add extra information to kills */
      ceval /= 10;
      veval /= 10;
      if ( ceval == 6 )
        ceval = 5;
      if ( veval == 6 )
        veval = 5;
      ch->pcdata->kills[veval+3]++;
      time( &t );
      if ( ceval == veval )
        ch->pcdata->kills[LASTFAIR] = (int)t;
      ch->pcdata->kills[LASTKILL] = (int)t;
    }
  }


  /* for leaderboard updating */
  if ( victim->pcdata->extras[TIMER] < 20 )
    victim->pcdata->extras[TIMER] = 20;
  victim->pcdata->extras[TIE] = 0;
  victim->pcdata->extras2[PKCOUNT] += 5;
  victim->wait = 0;
  victim->level = 1;
  for ( i = 0; i < 10; i++ )
    victim->pcdata->suit[i] = 0;
  if ( IS_SET(victim->pcdata->actnew,NEW_DRAGON_SLAVE) )
    REMOVE_BIT(victim->pcdata->actnew,NEW_DRAGON_SLAVE);
  raw_kill( victim, pdox );
  update_llboards( ch, (ceval == veval) );
  do_save( ch, "auto" );
  return;
}

void do_escape( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->position == POS_FIGHTING )
  { send_to_char( "Try fleeing instead.\n\r", ch );
    return;
  }
  if ( ch->position > POS_STUNNED )
  { send_to_char( "Escape from what?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
  {
    stc( "Escape from what?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->in_room->room_flags,ROOM_ARENA) )
  { send_to_char( "You may not escape from the Arena.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[TIE] > 0 )
  { send_to_char( "You are tied up!\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[TIMER] > 10 )
  {
    stc( "No escaping with a high fight timer.\n\r", ch );
    return;
  }
  if ( ch->position < POS_MORTAL )
  { send_to_char( "I think it's a little late for that..\n\r", ch );
    return;
  }

  sprintf( buf, "%s has escaped defenseless from a fight.", ch->name );
  do_info( ch, buf );
  send_to_char( "Your vision fades to black..\n\r", ch );
  char_from_room( ch );
  char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
  WAIT_STATE( ch, 20 );
  return;
}

void do_bandage( CHAR_DATA *ch, char *argument )
{
  int sn = skill_lookup( "cure light" );
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;

  if ( argument[0] == '\0' )
  { send_to_char( "Bandage whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room(ch, argument) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    stc( "Why bother?\n\r", ch );
    return;
  }

  if ( ch->level < 2 && victim->level >= 2 )
  { send_to_char( "No interfering with avatars.\n\r", ch );
    return;
  }

  if ( ch->pcdata->extras2[EVAL] + 25 < victim->pcdata->extras2[EVAL] )
  {
    stc( "They really shouldn't need your help.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, 12 );
  act( "You bandage $N's wounds.", ch, NULL, victim, TO_CHAR );
  act( "$n bandages your wounds.", ch, NULL, victim, TO_VICT );
  act( "$n bandadges $N's wounds.", ch, NULL, victim, TO_NOTVICT );
  ( skill_table[sn].spell_fun) (sn, ch->pcdata->body, ch, victim);
  return;
}

void do_clearstats( CHAR_DATA *ch, char *argument )
{ OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if ( IS_NPC(ch) )
    return;

  /* remove all spells and other affects */
  while ( ch->affected )
    affect_remove( ch, ch->affected );

  /* remove all equipment */
  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
  { obj_next = obj->next_content;
    if ( obj->wear_loc != WEAR_NONE )
    { unequip_char( ch, obj );
      act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
      act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    }
  }

  /* remove bits */

  /* Was bugged, "NEW_TRUESIGHT", removed so it won't be annoying
  if ( IS_SET(ch->act,PLR_TRUESIGHT) )
  { REMOVE_BIT(ch->act,PLR_TRUESIGHT);
    send_to_char( "Your eyesight returns to normal.\n\r", ch );
  }
  */

  if ( IS_SET(ch->pcdata->actnew,NEW_RECOVERY) )
  { REMOVE_BIT(ch->pcdata->actnew,NEW_RECOVERY);
    send_to_char( "You feel the healing power leave your body.\n\r", ch );
  }
  if ( is_affected(ch,gsn_balus_wall) )
  { affect_strip(ch,gsn_balus_wall);
    send_to_char( "The flourescent ball hovering above you vanishes.\n\r", ch );
  }
  if ( is_affected(ch,gsn_defense) )
  { affect_strip(ch,gsn_defense);
    send_to_char( "Your defensive barrier drops.\n\r", ch );
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
  { REMOVE_BIT(ch->pcdata->actnew,NEW_PHOENIX_AURA);
    send_to_char( "Your phoenix aura collapses.\n\r", ch );
  }

  /* now, set stats to norms */
  ch->hitroll = ch->level;
  ch->damroll = ch->level;
  ch->armor = 100;

  send_to_char( "Your stats have been cleared.\n\r", ch );
  return;
}

void do_home( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;

  if ( IS_SET(ch->in_room->room_flags,ROOM_ARENA) )
  { send_to_char( "Not in the Arena.\n\r", ch );
    return;
  }
  if ( IS_SET(ch->in_room->room_flags,ROOM_PRIVATE) )
  {
    stc( "Not in a private room.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( !str_cmp( arg, "here" ) )
  { send_to_char( "This room is now your home.\n\r", ch );
    ch->pcdata->extras[HOME] = ch->in_room->vnum;
  }
  else
    send_to_char( "home here to set your home.\n\r", ch );

  return;
}

void paradox( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];

  sprintf( buf, "%s is struck by a paradox!", ch->name );
  do_info( ch, buf );
  do_follow( ch, "self" );
  if ( ch->in_room->vnum != ROOM_VNUM_TEMPLE )
  { char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
    sprintf( buf, "%s has escaped defenseless from a fight.\n\r", ch->name );
    do_info( ch, buf );
  }
  ch->hit = -10;
  ch->position = POS_RESTING;
  update_pos( ch );
  WAIT_STATE( ch, 120 );
  return;
}

void do_untie( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;
  if ( argument[0] == '\0' )
  { send_to_char( "Untie whom?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "That's a mob.\n\r", ch );
    return;
  }
  if ( victim->pcdata->extras[TIE] <= 0 )
  { send_to_char( "They're not tied up.\n\r", ch );
    return;
  }

  victim->pcdata->extras[TIE] = 0;
  act( "You untie $N.", ch, NULL, victim, TO_CHAR );
  act( "$n unties you.", ch, NULL, victim, TO_VICT );
  act( "$n unties $N.", ch, NULL, victim, TO_NOTVICT );
  WAIT_STATE( ch, 12 );
  return;
}

void do_tie( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
 
  if ( argument[0] == '\0' )
  { send_to_char( "Tie up whom?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Uhh.. that' a mob.\n\r", ch );
    return;
  }
  if ( victim->level < 2 )
  { send_to_char( "That's a mortal.\n\r", ch ); 
    return;
  }
  if ( victim->pcdata->extras[TIE] > 0 )
  { send_to_char( "They'r already tied up!\n\r", ch ); 
    return;
  }
  if ( victim->position >= POS_STUNNED )
  { send_to_char( "You'll need to weaken them first.\n\r", ch ); 
    return;
  }

  victim->pcdata->extras[TIE] = dice(3,4);
  act( "You tie up $N.", ch, NULL, victim, TO_CHAR );
  act( "You have been tied up by $n!", ch, NULL, victim, TO_VICT );
  act( "$n ties $N up.", ch, NULL, victim, TO_NOTVICT );
  sprintf( buf, "%s has been tied up by %s.", victim->name,
  	(IS_NPC(ch) ? ch->short_descr : ch->name) );
  do_info( ch, buf );
  sprintf( buf, "%s tied by %s in %d.", victim->name,
  	(IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->vnum );
  log_string( buf );
  return;
}

void do_flex( CHAR_DATA *ch, char *argument )
{ if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->extras[TIE] <= 0 )
    return;
  if ( number_percent() < (100 - (ch->pcdata->body / 4) ) )
  { act( "You flex your muscles, vainly trying to escape!", ch, NULL, NULL, TO_CHAR );
    act( "$n flexes $s muscles, but fails to escape.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, 8 );
  }
  else
  { act( "You flex your muscles and break the ropes!", ch, NULL, NULL, TO_CHAR );
    act( "$n flexes $s muscles and breaks the ropes.", ch, NULL, NULL, TO_ROOM );
    ch->pcdata->extras[TIE] = 0;
    WAIT_STATE( ch, 12 );
  }
  return;
}

void update_llboards( CHAR_DATA *ch, bool fair )
{ int assessment, i;
  bool found = FALSE;
  time_t t;
  
  if ( IS_NPC(ch) )
    return;

  time( &t );
  assessment = assess( ch );
  for ( i = 0; i < MAX_LEADERS+5; i++ )
  { if ( !str_cmp( leaderboard[i].name, ch->name ) && !found )
    { free_string( leaderboard[i].clan );
      leaderboard[i].clan = str_dup( clan_table[ch->pcdata->clan[CLAN]].title );
      leaderboard[i].pkills = ch->pcdata->kills[PK];
      leaderboard[i].pdeaths = ch->pcdata->kills[PD];
      leaderboard[i].assessment = assessment;
      leaderboard[i].lastkill = ch->pcdata->kills[LASTFAIR];
      found = TRUE;
    }
  }

  for ( i = 0; i < 10; i++ )
  { if ( !str_cmp( loserboard[i].name, ch->name ) )
    { free_string( loserboard[i].clan );
      loserboard[i].clan = str_dup( clan_table[ch->pcdata->clan[CLAN]].title );
      loserboard[i].pkills = ch->pcdata->kills[PK];
      loserboard[i].pdeaths = ch->pcdata->kills[PD];
      loserboard[i].assessment = assessment;
      found = TRUE;
    }
  }

  if ( !found && (assessment > leaderboard[MAX_LEADERS+5-1].assessment
               || leaderboard[MAX_LEADERS+5-1].lastkill + SECONDS_PER_WEEK < t) )
  { free_string( leaderboard[MAX_LEADERS+5-1].name );
    free_string( leaderboard[MAX_LEADERS+5-1].clan );
    leaderboard[MAX_LEADERS+5-1].name = str_dup( ch->name );
    leaderboard[MAX_LEADERS+5-1].clan = str_dup( clan_table[ch->pcdata->clan[CLAN]].title );
    leaderboard[MAX_LEADERS+5-1].pkills = ch->pcdata->kills[PK];
    leaderboard[MAX_LEADERS+5-1].pdeaths = ch->pcdata->kills[PD];
    leaderboard[MAX_LEADERS+5-1].assessment = assessment;
    leaderboard[MAX_LEADERS+5-1].lastkill = ch->pcdata->kills[LASTFAIR];
  }
  else if ( assessment < loserboard[9].assessment && !found )
  { free_string( loserboard[9].name );
    free_string( loserboard[9].clan );
    loserboard[9].name = str_dup( ch->name );
    loserboard[9].clan = str_dup( clan_table[ch->pcdata->clan[CLAN]].title );
    loserboard[9].pkills = ch->pcdata->kills[PK];
    loserboard[9].pdeaths = ch->pcdata->kills[PD];
    loserboard[9].assessment = assessment;
  }

  sort_llboards();
  return;
}

void sort_llboards()
{ int i, pkills, pdeaths, assessment, lastkill;
  char name[20];
  char clan[20];
  time_t t;
  time( &t );

  for ( i = (MAX_LEADERS+5-1); i > 0; i-- )
  { if ( (leaderboard[i].assessment > leaderboard[i-1].assessment
       && leaderboard[i].lastkill + SECONDS_PER_WEEK >= t)
      || (leaderboard[i-1].lastkill + SECONDS_PER_WEEK < t
       && leaderboard[i].lastkill + SECONDS_PER_WEEK >= t) )
    { sprintf( name, "%s", leaderboard[i-1].name );
      sprintf( clan, "%s", leaderboard[i-1].clan );
      pkills = leaderboard[i-1].pkills;
      pdeaths = leaderboard[i-1].pdeaths;
      assessment = leaderboard[i-1].assessment;
      lastkill = leaderboard[i-1].lastkill;
      free_string( leaderboard[i-1].name );
      free_string( leaderboard[i-1].clan );
      leaderboard[i-1].name = str_dup( leaderboard[i].name );
      leaderboard[i-1].clan = str_dup( leaderboard[i].clan );
      leaderboard[i-1].pkills = leaderboard[i].pkills;
      leaderboard[i-1].pdeaths = leaderboard[i].pdeaths;
      leaderboard[i-1].assessment = leaderboard[i].assessment;
      leaderboard[i-1].lastkill = leaderboard[i].lastkill;
      free_string( leaderboard[i].name );
      free_string( leaderboard[i].clan );
      leaderboard[i].name = str_dup( name );
      leaderboard[i].clan = str_dup( clan );
      leaderboard[i].pkills = pkills;
      leaderboard[i].pdeaths = pdeaths;
      leaderboard[i].assessment = assessment;
      leaderboard[i].lastkill = lastkill;
    }
  }

  for ( i = 9; i > 0; i-- )
  { if ( loserboard[i].assessment < loserboard[i-1].assessment )
    { sprintf( name, "%s", loserboard[i-1].name );
      sprintf( clan, "%s", loserboard[i-1].clan );
      pkills = loserboard[i-1].pkills;
      pdeaths = loserboard[i-1].pdeaths;
      assessment = loserboard[i-1].assessment;
      free_string( loserboard[i-1].name );
      free_string( loserboard[i-1].clan );
      loserboard[i-1].name = str_dup( loserboard[i].name );
      loserboard[i-1].clan = str_dup( loserboard[i].clan );
      loserboard[i-1].pkills = loserboard[i].pkills;
      loserboard[i-1].pdeaths = loserboard[i].pdeaths;
      loserboard[i-1].assessment = loserboard[i].assessment;
      free_string( loserboard[i].name );
      free_string( loserboard[i].clan );
      loserboard[i].name = str_dup( name );
      loserboard[i].clan = str_dup( clan );
      loserboard[i].pkills = pkills;
      loserboard[i].pdeaths = pdeaths;
      loserboard[i].assessment = assessment;
    }
  }

  return;
}

void do_enter( CHAR_DATA *ch, char *argument )
{ OBJ_DATA *obj;
  ROOM_INDEX_DATA *room;

  if ( IS_NPC(ch) )
    return;
  if ( IS_SET(ch->in_room->room_flags, ROOM_HQ) )
  { send_to_char( "You may not portal from or to a headquarters.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Enter what?\n\r", ch );
    return;
  }
  obj = get_obj_list( ch, argument, ch->in_room->contents );
  if ( obj == NULL )
  { send_to_char( "You attempt to enter thin air, and stumble.\n\r", ch );
    return;
  }
  switch( obj->item_type )
  { case ITEM_CONTAINER:
    case ITEM_CORPSE_PC:
    case ITEM_CORPSE_NPC:
      send_to_char( "You can't fit in there.\n\r", ch );
      return;
      break;
    case ITEM_PORTAL:
    case ITEM_GATE:
      break;
    default:
      send_to_char( "You can't enter that.\n\r", ch );
      return;
      break;
  }
  if ( obj->value[0] == 0 )
  { send_to_char( "Bonk! You smash your face against the portal.\n\r", ch );
    return;
  }
  if ( ( room = get_room_index( obj->value[0] ) ) == NULL )
  { send_to_char( "You think twice about entering a portal to a null pointer.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[TIMER] > 0 )
  { send_to_char( "Not with a fight timer.\n\r", ch );
    return;
  }
  act( "You step into $p.", ch, obj, NULL, TO_CHAR );
  act( "$n steps into $p.", ch, obj, NULL, TO_ROOM );
  if ( obj->item_type == ITEM_PORTAL )
  { act( "$p vanishes.", ch, obj, NULL, TO_ROOM );
    extract_obj( obj );
  }
  char_from_room( ch );
  char_to_room( ch, room );
  do_look( ch, "auto" );
  act( "$n steps out of thin air.", ch, NULL, NULL, TO_ROOM );
  return;
}

void do_dare( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->extras[LEGEND] < 6 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
  {
    stc( "The time for such things is past.\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < 75 )
  { send_to_char( "It costs 75 primal to issue a dare.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Dare who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
  { send_to_char( "You can't seem to find them.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Wow.. daring mobs.  How impressive.\n\r", ch );
    return;
  }
  if ( victim->level < 2 )
  { send_to_char( "No picking on mortals.\n\r", ch ); 
    return;
  }
  if ( victim->pcdata->extras[TIMER] > 15 )
  { send_to_char( "They're already gonna be sticking around.\n\r", ch );
    return;
  }

  victim->pcdata->extras[TIMER] = ch->pcdata->extras[LEGEND] + dice(1,6);
  ch->pcdata->extras[TIMER] = UMAX( ch->pcdata->extras[TIMER], 15 );
  ch->pcdata->primal -= 75;
  sprintf( buf, "You dare %s to stick around and take a beating.\n\r", victim->name );
  send_to_char( buf, ch );
  sprintf( buf, "%s dares you to stick around and take a beating.\n\r", ch->name );
  send_to_char( buf, victim );
  return;
}

void do_reduction( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  int cost;

  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->extras[LEGEND] < 9 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[STATUS] < 1 )
  { send_to_char( "You don't have any status to lose.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[STATUS] == 1 )
  { send_to_char( "The only way to become an avatar again is to be capped.\n\r", ch );
    return;
  }
  cost = ch->pcdata->extras[STATUS] * 3;
  if ( ch->pcdata->primal < cost )
  { sprintf( buf, "You need %d primal to reduce your status.\b\r", cost );
    send_to_char( buf, ch );
    return;
  }

  ch->pcdata->primal -= cost;
  ch->pcdata->extras[STATUS]--;
  send_to_char( "You have reduced your status.\n\r", ch );
  return;
}

void do_humiliation( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->extras[LEGEND] < 8 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < 200 )
  { send_to_char( "You need 200 primal to set your Humiliation.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Set your humiliation to what?\n\r", ch );
    return;
  }
  if ( strlen( argument ) < 10 )
  { send_to_char( "Too short.\n\r", ch );
    return;
  }
  if ( strlen( argument ) > 55 )
  { send_to_char( "Too long.\n\r", ch );
    return;
  }
  smash_tilde( argument );
  send_to_char( "Your humiliations will now appear as follows:\n\r", ch );
  sprintf( buf, "`YInfo ->`n Victim %s %s.`n\n\r", argument, ch->name );
  send_to_char( buf, ch );
  free_string( ch->pcdata->humiliate );
  ch->pcdata->humiliate = str_dup( argument );
  ch->pcdata->primal -= 200;
  return;
}

void do_humiliate( CHAR_DATA *ch, char *argument )
{ char log_buf[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wield;
  int damtype, ceval, veval, i;
  bool pdox = FALSE;
  extern bool peace;
  time_t t;

  if ( peace )
  { send_to_char( "Sorry, peace has been declared.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->in_room->room_flags,ROOM_ARENA) )
  { send_to_char( "There is no capping in the Arena.\n\r", ch );
    return;
  }
  if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
  {
    stc( "Why not sit back and relax?\n\r", ch );
    return;
  }

  if ( IS_NPC(ch) )
  { send_to_char( "NPC's cannot use lcommands.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[LEGEND] < 8 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( !ch->pcdata->humiliate || ch->pcdata->humiliate == "" )
  { send_to_char( "You haven't set your humiliation yet.\n\r", ch );
    return;
  }
  if ( ch->position < POS_STANDING || ch->fighting != NULL )
  {
    send_to_char( "You're too busy.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Humiliate who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, argument) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Just kill them!\n\r", ch );
    return;
  }
  if ( victim->level < 2 )
  { send_to_char( "Only avatars can be Humiliated.\n\r", ch );
    return;
  }
  /* Removed for now
  if ( IS_CLASS(ch,CLASS_SORCERER) || IS_CLASS(victim,CLASS_SORCERER) )
  {
    stc( "Sorry, sorcerers are currently in the test phase.\n\r", ch );
    return;
  }
  */
  if ( IS_NPC(ch) )
    ceval = 10;
  else
    ceval = ch->pcdata->extras2[EVAL];
  veval = victim->pcdata->extras2[EVAL];
  if ( ceval < 10 || veval < 10 )
  { send_to_char( "For your protection, newbies are prevented from pkilling and being pked.\n\r", ch );
    return;
  }
  if ( victim->level > 2 )
  { send_to_char( "Hahahaha.. I think not.\n\r", ch );
    return;
  }
  if ( victim->position != POS_MORTAL )
  { send_to_char( "You can only humiliate mortally wounded players.\n\r", ch );
    return;
  }

  sprintf( log_buf, "%s humiliated by %s at %d with %d remaining, inf %d.",
  		victim->name,
        	(IS_NPC(ch) ? ch->short_descr : ch->name),
        	victim->in_room->vnum,
        	ch->hit,
        	get_inf_dam( ch, victim->name ) );
  log_string( log_buf );

  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    damtype = -1;
  else
    damtype = wield->value[3];

  switch( damtype )
  {
    case -1:
      sprintf( buf1, "You drive your hand into $N's neck and rip off $N's head!" );
      sprintf( buf2, "$n drives their hand into your neck and rips off your head!" );
      sprintf( buf3, "$n drives $m hand into $N's neck and rips off $M head!" );
      break;
    case 1:
    case 3:
      sprintf( buf1, "You growl 'There can be only One!' and decapitate $N!" );
      sprintf( buf2, "$n growls 'There can be only One!' and brings $s %s down on your neck!", wield->short_descr );
      sprintf( buf3, "$n growls 'There can be only One!' and decapitates $N!" );
      break;
    case 2:
    case 11:
      sprintf( buf1, "You ram your weapon into $N's face and tear it out, removing eyes and brain." );
      sprintf( buf2, "$n rams $s weapon into your face and lobotomizes you." );
      sprintf( buf3, "$n rams $s weapon into $N's face and tears it out, removing eyes and brain." );
      break;
    case 7:
    case 8:
      sprintf( buf1, "You bring your weapon down on $N's head, and brains spray from $S ears!" );
      sprintf( buf2, "$n brings $s weapon down on your head, forcing your brain to vacate the premises!" );
      sprintf( buf3, "$n brings $s weapon down on $N's head, and brains spray from $S ears!" );
      break;
    default:
      sprintf( buf1, "You finish $N off!" );
      sprintf( buf2, "$n finishes you off!" );
      sprintf( buf3, "$n finishes $N off!" );
      break;
  }

  act( buf1, ch, NULL, victim, TO_CHAR );
  act( buf2, ch, NULL, victim, TO_VICT );
  act( buf3, ch, NULL, victim, TO_NOTVICT );
  sprintf( buf, "%s %s `n%s.", victim->name, ch->pcdata->humiliate, ch->name );

  if ( !IS_NPC(ch) && get_inf_dam( ch, victim->name ) < 1000 )
    sprintf( buf, "%s just got Nathored by %s.", victim->name, ch->name );

  do_fatality( ch, buf );

  sprintf( buf, "%s capped at %d by %s with %d remaining (inflicted %d).",
                victim->name,
                ch->in_room->vnum,
                (IS_NPC(ch) ? ch->short_descr : ch->name),
                ch->hit,
                get_inf_dam( ch, victim->name ) );
  wiznet( buf );
  record_pkill( ch, victim, TRUE );

  if ( !IS_NPC(ch) )
  { 
    if ( ch->pcdata->extras[TIMER] > 10 )
      ch->pcdata->extras[TIMER] += 10;
    else
      ch->pcdata->extras[TIMER] += 20;

    /*
     * Check for paradoxing
     * Paradox if victim is in a lower category than the
     * character and NOT a gangster (30) or above.
     */
    if ( ((veval < 10 && ceval >= 10) || (veval < 20 && ceval >= 20)
      || (veval < 30 && ceval >= 30)) && !IS_SET(victim->pcdata->actnew,NEW_FAIR_GAME) )
    {
      pdox = TRUE;
      paradox( ch );
    }
    /* fair cap */
    else
    { victim->pcdata->kills[PD]++;
      if ( veval < 30 || veval > 39 || ceval < 40 )
      { ch->pcdata->kills[PK]++;
        clan_table[ch->pcdata->clan[CLAN]].pkills++;
        clan_table[ch->pcdata->clan[CLAN]].kills[victim->pcdata->clan[CLAN]]++;
        clan_table[victim->pcdata->clan[CLAN]].pdeaths++;
      }
      if ( ceval <= veval && ceval+20 > veval )
      { ch->exp += victim->exp / 2;
        victim->exp /= 2;
      }
      if ( victim->pcdata->extras2[BOUNTY] > 0 )
      {
        ch->gold += victim->pcdata->extras2[BOUNTY];
        sprintf( buf, "You collect a bounty of %d gold.\n\r", victim->pcdata->extras2[BOUNTY] );
        stc( buf, ch );
        victim->pcdata->extras2[BOUNTY] = 0;
      }
      /* check for lowbie capping higher player */
      if ( (veval >= 20 && ceval < 20) || (veval >= 30 && ceval < 30) )
      { if ( !IS_SET(ch->pcdata->actnew,NEW_FAIR_GAME) )
          SET_BIT(ch->pcdata->actnew,NEW_FAIR_GAME);
      }
      /* check for removing fair game */
      if ( IS_SET(victim->pcdata->actnew,NEW_FAIR_GAME) )
        REMOVE_BIT(victim->pcdata->actnew,NEW_FAIR_GAME);

      /* add extra information to kills */
      ceval /= 10;
      veval /= 10;
      if ( ceval == 6 )
        ceval = 5;
      if ( veval == 6 )
        veval = 5;
      ch->pcdata->kills[veval+3]++;
      time( &t );
      if ( ceval == veval )
        ch->pcdata->kills[LASTFAIR] = (int)t;
      ch->pcdata->kills[LASTKILL] = (int)t;
    }
  }


  /* for leaderboard updating */
  if ( victim->pcdata->extras[TIMER] < 20 )
    victim->pcdata->extras[TIMER] = 20;
  victim->pcdata->extras[TIE] = 0;
  victim->pcdata->extras2[PKCOUNT] += 5;
  victim->wait = 0;
  victim->level = 1;
  for ( i = 0; i < 10; i++ )
    victim->pcdata->suit[i] = 0;
  if ( IS_SET(victim->pcdata->actnew,NEW_DRAGON_SLAVE) )
    REMOVE_BIT(victim->pcdata->actnew,NEW_DRAGON_SLAVE);
  raw_kill( victim, pdox );
  update_llboards( ch, (ceval == veval) );
  do_save( ch, "auto" );
  return;
}

void do_rub( CHAR_DATA *ch, char *argument )
{
  if ( !IS_AFFECTED(ch,AFF_BLIND) )
  { send_to_char( "You rub your eyes at the glare.\n\r", ch );
    return;
  }

  if ( number_percent() > 30 )
  { send_to_char( "You scrape at your eyes, to no avail.\n\r", ch );
    return;
  }
  affect_strip( ch, gsn_blindness );
  act( "You rub your eyes, restoring vision.", ch, NULL, NULL, TO_CHAR );
  act( "$n rubs $s eyes.", ch, NULL, NULL, TO_ROOM );
  return;
}

bool has_jboots( CHAR_DATA *ch )
{ OBJ_DATA *wield;
  int cost;
  if ( IS_NPC(ch) )
    return FALSE;
  if ( ( wield = get_eq_char( ch, WEAR_FEET ) ) == NULL )
    return FALSE;
  if ( wield->pIndexData->vnum != 33 )
    return FALSE;

  cost = dice(4,4);
  if ( ch->move <= cost )
    return FALSE;
  ch->move -= cost;
  return TRUE;
}

void do_challenge( CHAR_DATA *ch, char *argument )
{

  /*
   * Commented out, unimplemented.

  if ( IS_NPC(ch) )
    return;
  ceval = ch->pcdata->extras2[EVAL];
  if ( ceval < 10 )
  {
    stc( "Not yet.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    stc( "Challenge who?\n\r", ch );
    return;
  }
  if ( (victim = get_char_world( ch, arg )) == NULL )
  {
    stc( "You can't find them.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  {
    stc( "Why bother challenging a mob, just kill it.\n\r", ch );
    return;
  }

  if ( challenge->active )
  {
    stc( "A challenge is already underway.\n\r", ch );
    return;
  }
  veval = victim->pcdata->extras2[EVAL];
  if ( veval < 10 )
  {
    stc( "Uh, they're a newbie.\n\r", ch );
    return;
  }
  ceval /= 10;
  veval /= 10;
  if ( ceval >= 6 )
    ceval = 5;
  if ( veval >= 6 )
    veval = 5;

  if ( ceval > veval+1 )
  {
    stc( "Why bother, they're no challenge.\n\r", ch );
    return;
  }
  if ( ceval < veval-1 )
  {
    stc( "They're out of your league.\n\r", ch );
    return;
  }
  */

}

void do_smother( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int dam;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  { 
    stc( "Smother who?\n\r", ch );
    return;
  }

  if ( (victim = get_char_room(ch,arg)) == NULL )
  {
    stc( "They aren't here.\n\r", ch );
    return;
  }

  if ( !is_affected(victim,skill_lookup("flame breath")) )
  {
    act( "You smother $N with love.\n\r", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( number_percent() < 25 )
  {
    affect_strip(victim,skill_lookup("flame breath"));
    act( "You smother the flames burning $N.", ch, NULL, victim, TO_CHAR );
    act( "$N smothers your flames!", ch, NULL, victim, TO_VICT );
    act( "$n smothers the flames burning $N.", ch, NULL, victim, TO_NOTVICT );
  }
  else
  {
    dam = dice(10,50);
    if ( !IS_NPC(ch) && ch->level >= 1 )
      dam *= 3;
    damage( ch, ch, dam, DAM_FLAME_BREATH );
    act( "You burn your hands! OUCH!!", ch, NULL, victim, TO_CHAR );
    act( "$n burns $s hands attempting to smother $N", ch, NULL, victim, TO_ROOM );
  }

  WAIT_STATE(ch,6);
  return;
}


void record_pkill( CHAR_DATA *ch, CHAR_DATA *victim, bool hum )
{
  PKILL_DATA *pkill = new_pkill();

  if ( IS_NPC(ch) || IS_NPC(victim) )
    return;

  pkill->victim = str_dup( victim->name );
  pkill->inf = get_inf_dam( ch, victim->name );
  pkill->lost = get_inf_dam( victim, ch->name );
  pkill->vnum = ch->in_room->vnum;
  pkill->time = (int) current_time;
  pkill->humil = hum;
  pkill->suit = IS_SUIT(ch);

  pkill->next = ch->pcdata->pkills;
  ch->pcdata->pkills = pkill;

  return;
}

