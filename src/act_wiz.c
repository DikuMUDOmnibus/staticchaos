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
#include <unistd.h>


/* global for peace */
bool peace = FALSE;

/* global for mob bug checking */
bool bugcheck = TRUE;

/* global for EZPass */
bool EZPass = FALSE;

extern int port, control;

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    buf1[0] = '\0';
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) )
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    strcat( buf1, buf );
	    if ( ++col % 6 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 6 != 0 )
	strcat( buf1, "\n\r" );
    send_to_char( buf1, ch );
    return;
}



void do_bamfin( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_lenter( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->extras[LEGEND] < 4 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < 100 )
  { send_to_char( "You need 100 primal to set your lenter.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Why pay for the default lenter?\n\r", ch );
    return;
  }
  
  smash_tilde( argument );
  if ( strlen( argument ) > 60 )
  { send_to_char( "Too long.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= 100;
  free_string( ch->pcdata->bamfin );
  ch->pcdata->bamfin = str_dup( argument );
  send_to_char( "Ok.\n\r", ch );
  return;
}


void do_bamfout( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	free_string( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_lexit( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->extras[LEGEND] < 4 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < 100 )
  { send_to_char( "You need 100 primal to set your lexit.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "Why pay for the default lexit?\n\r", ch );
    return;
  }

  smash_tilde( argument );
  if ( strlen( argument ) > 60 )
  { send_to_char( "Too long.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= 100;
  free_string( ch->pcdata->bamfout );
  ch->pcdata->bamfout = str_dup( argument );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    stop_fighting( victim, TRUE );

    /* put victim at market squre */
    char_from_room( victim );
    char_to_room( victim, get_room_index( 3011 ) );

    SET_BIT(victim->act, PLR_DENY);
    victim->pcdata->extras[TIMER] = 0;
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Echo what?\n\r", ch );
	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Recho what?\n\r", ch );
	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) )
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    /* this is stupid
    if ( room_is_private( location ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }
    */

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    /* fuck this shit
    if ( room_is_private( location ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }
    */

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );
    /* removed, invis movement.
    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
    {
	act( "$n $T.", ch, NULL,
	    (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
	    ? ch->pcdata->bamfout : "leaves in a swirling mist",  TO_ROOM );
    }
    */

    char_from_room( ch );
    char_to_room( ch, location );

    /* removed, invis movement.
    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
    {
	act( "$n $T.", ch, NULL,
	    (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
	    ? ch->pcdata->bamfin : "appears in a swirling mist", TO_ROOM );
    }
    */

    do_look( ch, "auto" );
    return;
}



void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    buf1[0] = '\0';

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'.\n\r",
	location->name,
	location->area->name );
    strcat( buf1, buf );

    sprintf( buf,
	"Vnum: %d.  Sector: %d.  Light: %d.\n\r",
	location->vnum,
	location->sector_type,
	location->light );
    strcat( buf1, buf );

    sprintf( buf,
	"Room flags: %d.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    strcat( buf1, buf );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "Extra description keywords: '" );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next != NULL )
		strcat( buf1, " " );
	}
	strcat( buf1, "'.\n\r" );
    }

    strcat( buf1, "Characters:" );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	strcat( buf1, " " );
	one_argument( rch->name, buf );
	strcat( buf1, buf );
    }

    strcat( buf1, ".\n\rObjects:   " );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	strcat( buf1, " " );
	one_argument( obj->name, buf );
	strcat( buf1, buf );
    }
    strcat( buf1, ".\n\r" );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		pexit->to_room != NULL ? pexit->to_room->vnum : 0,
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    strcat( buf1, buf );
	}
    }

    send_to_char( buf1, ch );
    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Ostat what?\n\r", ch );
	return;
    }

    buf1[0] = '\0';

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: %s.\n\r",
	obj->name );
    strcat( buf1, buf );

    sprintf( buf, "Vnum: %d.  Type: %s.\n\r",
	obj->pIndexData->vnum, item_type_name( obj->item_type ) );
    strcat( buf1, buf );

    sprintf( buf, "Short description: %s.\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    strcat( buf1, buf );

    sprintf( buf, "Wear bits: %d.  Extra bits: %s.\n\r",
	obj->wear_flags, expand_bits( obj->extra_flags ) );
    strcat( buf1, buf );

    sprintf( buf, "Number: %d/%d.  Weight: %d/%d.\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ) );
    strcat( buf1, buf );

    sprintf( buf, "Cost: %d.  Timer: %d.  Level: %d.\n\r",
	obj->cost, obj->timer, obj->level );
    strcat( buf1, buf );

    sprintf( buf,
	"In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
	obj->wear_loc );
    strcat( buf1, buf );
    
    sprintf( buf, "Values: %d %d %d %d.\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
    strcat( buf1, buf );

    sprintf( buf, "Extras: %d %d %d %d.\n\r",
    	obj->extra[0], obj->extra[1], obj->extra[2], obj->extra[3] );
    strcat( buf1, buf );

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "Extra description keywords: '" );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next != NULL )
		strcat( buf1, " " );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next != NULL )
		strcat( buf1, " " );
	}

	strcat( buf1, "'.\n\r" );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier );
	strcat( buf1, buf );
    }

    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier );
	strcat( buf1, buf );
    }

    send_to_char( buf1, ch );
    return;
}


void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    INF_DATA *inf;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Mstat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    buf1[0] = '\0';

    sprintf( buf, "Name: %s.\n\r",
	victim->name );
    strcat( buf1, buf );

    sprintf( buf, "Vnum: %d.  Sex: %s.  Room: %d.  Version: %d.  Played: %d.\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum,
	IS_NPC(victim) ? 0 : victim->pcdata->version,
	IS_NPC(victim) ? 0 : (victim->played/120)
	);
    strcat( buf1, buf );


    if ( !IS_NPC(victim) )
    {
      sprintf( buf, "Body: %d.  Mind: %d.  Spirit: %d.  Willpower: %d    Primal: %d.\n\r",
	victim->pcdata->body,
	victim->pcdata->mind,
	victim->pcdata->spirit,
	victim->pcdata->will,
	victim->pcdata->primal );
      strcat( buf1, buf );
      sprintf( buf, "Weapons: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d.\n\r",
	victim->pcdata->weapons[0], victim->pcdata->weapons[1], victim->pcdata->weapons[2],
	victim->pcdata->weapons[3], victim->pcdata->weapons[4],	victim->pcdata->weapons[5],
	victim->pcdata->weapons[6], victim->pcdata->weapons[7],	victim->pcdata->weapons[8],
	victim->pcdata->weapons[9], victim->pcdata->weapons[10], victim->pcdata->weapons[11],
	victim->pcdata->weapons[12] );
      strcat( buf1, buf );
      sprintf( buf, "Stances: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d.\n\r",
	victim->pcdata->stances[1], victim->pcdata->stances[2],	victim->pcdata->stances[3],
	victim->pcdata->stances[4], victim->pcdata->stances[5],	victim->pcdata->stances[6],
	victim->pcdata->stances[7], victim->pcdata->stances[8],	victim->pcdata->stances[9],
	victim->pcdata->stances[10] );
      strcat( buf1, buf );
    }

    sprintf( buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d.  Practices: %d.\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	victim->practice );
    strcat( buf1, buf );
	
    if ( !IS_NPC( victim ) )
    { sprintf( buf, "Class Powers: %d %d %d %d %d %d %d %d %d %d\n\r",
	victim->pcdata->powers[0], victim->pcdata->powers[1], victim->pcdata->powers[2],
	victim->pcdata->powers[3], victim->pcdata->powers[4], victim->pcdata->powers[5],
	victim->pcdata->powers[6], victim->pcdata->powers[7], victim->pcdata->powers[8],
	victim->pcdata->powers[9] );
      strcat( buf1, buf );
    }

    if ( !IS_NPC( victim) )
    { sprintf( buf, "Extra values: %d %d %d %d %d %d %d %d %d %d\n\r",
    	victim->pcdata->extras[0], victim->pcdata->extras[1], victim->pcdata->extras[2],
    	victim->pcdata->extras[3], victim->pcdata->extras[4], victim->pcdata->extras[5],
    	victim->pcdata->extras[6], victim->pcdata->extras[7], victim->pcdata->extras[8],
    	victim->pcdata->extras[9] );
      strcat( buf1, buf );
    }

    if ( !IS_NPC( victim) )
    { sprintf( buf, "Extr2 values: %d %d %d %d %d %d %d %d %d %d  Clan: %d %d\n\r",
        victim->pcdata->extras2[0], victim->pcdata->extras2[1], victim->pcdata->extras2[2],
        victim->pcdata->extras2[3], victim->pcdata->extras2[4], victim->pcdata->extras2[5],
        victim->pcdata->extras2[6], victim->pcdata->extras2[7], victim->pcdata->extras2[8],
        victim->pcdata->extras2[9], victim->pcdata->clan[0],	victim->pcdata->clan[1] );
      strcat( buf1, buf );
    }

    if ( !IS_NPC(victim) )
    {
      sprintf( buf, "Kills: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n\r",
        victim->pcdata->kills[0],   victim->pcdata->kills[1],
        victim->pcdata->kills[2],   victim->pcdata->kills[3],
        victim->pcdata->kills[4],   victim->pcdata->kills[5],
        victim->pcdata->kills[6],   victim->pcdata->kills[7],
        victim->pcdata->kills[8],   victim->pcdata->kills[9],
        victim->pcdata->kills[10],   victim->pcdata->kills[11],
        victim->pcdata->kills[12],   victim->pcdata->kills[13],
        victim->pcdata->kills[14] );
      strcat( buf1, buf );
    }

    sprintf( buf,
	"Lv: %d.  Class: %d.  AC: %d.  Gold: %d.  Exp: %d.",
	victim->level,       victim->class,
	GET_AC(victim),      victim->gold,         victim->exp );
    strcat( buf1, buf );

    if ( IS_NPC(victim) )
      sprintf( buf, "  Clan: %d.\n\r", victim->pIndexData->clan );
    else
      sprintf( buf, "\n\r" );
    strcat( buf1, buf );

    sprintf( buf, "Hitroll: %d.  Damroll: %d.  Position: %d.  Wimpy: %d.\n\r",
	victim->hitroll, victim->damroll,
	victim->position,    victim->wimpy );
    strcat( buf1, buf );

    sprintf( buf, "Fighting: %s.\n\r",
	    victim->fighting ? victim->fighting->name : "(none)" );
    strcat( buf1, buf );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d.  Full: %d.  Drunk: %d.  Saving throw: %d.  Eval: %d.\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->saving_throw,
	    victim->pcdata->extras2[EVAL]);
	strcat( buf1, buf );
    }

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf, "Page Lines: %d.  ", victim->pcdata->pagelen );
        strcat( buf1, buf );
    }
    sprintf( buf, "Carry number: %d.  Carry weight: %d.\n\r",
	victim->carry_number, victim->carry_weight );
    strcat( buf1, buf );

    sprintf( buf, "Age: %d.  Played: %d.  Timer: %d.  Act: %d.\n\r",
	get_age( victim ), (int) victim->played, victim->timer, victim->act );
    strcat( buf1, buf );


    sprintf( buf, "Master: %s.  Leader: %s.  Affected by: %s.\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	affect_bit_name( victim->affected_by ) );
    strcat( buf1, buf );

    if ( !IS_NPC(victim) )
    { sprintf( buf, "MMaster: %s  MName: %s.  Temp: %s  Security: %d.\n\r",
    	victim->pcdata->mmaster, victim->pcdata->mname,
	victim->pcdata->temp, victim->pcdata->security );
      strcat( buf1, buf );
    }
    else
    {
      sprintf( buf, "Short description: %s.\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none).\n\r" );
      strcat( buf1, buf );
    }

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
	strcat( buf1, "Mobile has spec fun.\n\r" );

    if ( IS_NPC(victim) && victim->mpactnum > 0 )
    {
      MPROG_ACT_LIST *actprog;
      int num = 0;
      sprintf( buf, "Mobile has %d act_progs queued.\n\r", victim->mpactnum );
      for( actprog = victim->mpact; actprog != NULL && num < 15; actprog = actprog->next )
      {
        num++;
        sprintf( buf, "%s: %s\n\r",
            (!IS_NPC(actprog->ch) ? actprog->ch->name : actprog->ch->short_descr),
            actprog->buf );
        strcat( buf1, buf );
      }
    }

    if ( !IS_NPC(victim) && victim->pcdata->inf != NULL )
    {
      for ( inf = victim->pcdata->inf; inf != NULL; inf = inf->next )
      {
        sprintf( buf, "Inflicted %d on %s (%d seconds ago)\n\r",
	  inf->damage,	inf->victim,	(int) (current_time - inf->time) );
	strcat( buf1, buf );
      }
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d seconds with bits %s.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector )
	    );
	strcat( buf1, buf );
    }

    send_to_char( buf1, ch );
    return;
}



void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mfind whom?\n\r", ch );
	return;
    }

    buf1[0] = '\0';
    fAll	= !str_cmp( arg, "all" );
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		strcat( buf1, buf );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    send_to_char( buf1, ch );
    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Ofind what?\n\r", ch );
	return;
    }

    buf1[0] = '\0';
    fAll	= !str_cmp( arg, "all" );
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( arg, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		strcat( buf1, buf );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    send_to_char( buf1, ch );
    return;
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mwhere whom?\n\r", ch );
	return;
    }

    found = FALSE;
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room != NULL
	&&   is_name( arg, victim->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
	    send_to_char(buf, ch);
	}
    }

    if ( !found )
    {
	act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
	return;
    }

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;

    argument = one_argument( argument, buf );
    if ( str_cmp( buf, "crash" ) )
    {
      sprintf( buf, "Reboot by %s.", ch->name );
      do_echo( ch, buf );
    }
    save_llboards( TRUE );
    save_llboards( FALSE );
    merc_down = TRUE;
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;


    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );
    save_llboards( TRUE );
    save_llboards( FALSE );
    merc_down = TRUE;
    return;
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    /*
     * Pointed out by Da Pub (What Mud)
     */
    if ( !IS_NPC( victim ) )
    {
        send_to_char( "You cannot switch into a player!\n\r", ch );
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}



void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: mload <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	    return;
        }
        level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
        {
	    send_to_char( "Limited to your trust level.\n\r", ch );
	    return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_char( obj, ch );
    }
    else
    {
	obj_to_room( obj, ch->in_room );
	act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && victim != ch )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}


void do_apurge( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  char log_buf[MAX_STRING_LENGTH];

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next;
    if ( IS_NPC(vch) && vch != ch 
      && vch->in_room != NULL
      && vch->in_room->area == ch->in_room->area )
    {
      extract_char( vch, TRUE );
    }
  }

  for ( obj = object_list; obj != NULL; obj = obj_next )
  {
    obj_next = obj->next;
    if ( obj->in_room != NULL
      && obj->in_room->area == ch->in_room->area
      && obj->carried_by == NULL )
    {
      extract_obj( obj );
    }
  }

  sprintf( log_buf, "Apurge by %s from %d.", ch->name, ch->in_room->vnum );
  log_string( log_buf );
  stc( "The area has been purged.\n\r", ch );
  return;
}

void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 20 )
    {
	send_to_char( "Level must be 1 to 20.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    victim->level = level;
    victim->trust = 0;
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > 40 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 40.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restore whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    { CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      for ( vch = char_list; vch != NULL; vch = vch_next )
      { vch_next = vch->next;
        if ( !IS_NPC(vch) )
        { vch->hit = vch->max_hit;
          vch->mana = vch->max_mana;
          vch->move = vch->max_move;
          update_pos( vch );
          act( "$n has restored you.", ch, NULL, vch, TO_VICT );
        }
      }
      send_to_char( "You have successfully restored them all.\n\r", ch );
      return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_EMOTE) )
    {
	REMOVE_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NO_EMOTE set.\n\r", ch );
    }

    return;
}



void do_nonote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Nonote whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->act, PLR_NO_NOTE) )
    {
        REMOVE_BIT(victim->act, PLR_NO_NOTE);
        send_to_char( "You can write notes again.\n\r", victim );
        send_to_char( "NO_NOTE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_NO_NOTE);
        send_to_char( "You can't write notes!\n\r", victim );
        send_to_char( "NO_NOTE set.\n\r", ch );
    }

    return;
}


void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_TELL) )
    {
	REMOVE_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}



void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Silence whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	REMOVE_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can use channels again.\n\r", victim );
	send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can't use channels!\n\r", victim );
	send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    if ( !str_cmp( argument, "world" ) )
    { if ( peace )
        peace = FALSE;
      else
        peace = TRUE;

      if ( peace )
        do_info( ch, "`WChaosium is now at peace.`n" );
      else
        do_info( ch, "`RChaosium has returned to war!`n" );
      return;
    }


    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );

	/* No prepped spells when peace is declared */
	if ( !IS_NPC(rch) && IS_CLASS(rch,CLASS_SORCERER) && rch->pcdata->chant != NULL )
	{
	  while ( rch->pcdata->chant != NULL )
	    lose_chant( rch );
	}
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



BAN_DATA *		ban_free;
BAN_DATA *		ban_list;
NEWLOCK_DATA *		newlock_free;
NEWLOCK_DATA *		newlock_list;

void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	strcpy( buf, "Banned sites:\n\r" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    strcat( buf, pban->name );
	    strcat( buf, "\n\r" );
	}
	send_to_char( buf, ch );
	return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    if ( ban_free == NULL )
    {
	pban		= alloc_perm( sizeof(*pban), PERM_OTHER );
    }
    else
    {
	pban		= ban_free;
	ban_free	= ban_free->next;
    }

    pban->name	= str_dup( arg );
    pban->next	= ban_list;
    ban_list	= pban;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !str_cmp( arg, curr->name ) )
	{
	    if ( prev == NULL )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    free_string( curr->name );
	    curr->next	= ban_free;
	    ban_free	= curr;
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
	send_to_char( "Game wizlocked.\n\r", ch );
    else
	send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}

void do_newlock( CHAR_DATA *ch, char *argument )
{ extern bool newlock;
  newlock = !newlock;

  if ( newlock )
    send_to_char( "Game is newlocked.\n\r", ch );
  else
    send_to_char( "Game un-newlocked.\n\r", ch );

  return;
}

void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slookup what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        buf1[0] = '\0';
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    strcat( buf1, buf );
	}
	send_to_char( buf1, ch );
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}



void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax: sset <victim> <skill> <value>\n\r",	ch );
	send_to_char( "or:     sset <victim> all     <value>\n\r",	ch );
	send_to_char( "Skill being any skill or spell.\n\r",		ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int value, max;
    int power = 0;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax: mset <victim> <field>  <value>\n\r",	ch );
	send_to_char( "or:     mset <victim> <string> <value>\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  str int wis dex con sex class level\n\r",	ch );
	send_to_char( "  gold hp mana move practice\n\r",		ch );
	send_to_char( "  thirst drunk full security",			ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "String being one of:\n\r",			ch );
	send_to_char( "  name short long description title spec\n\r",	ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "body" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	max = 1000;

	if ( value < 1 || value > max )
	{
	    sprintf( buf, "Body range is 1 to %d.\n\r", max );
	    send_to_char( buf, ch );
	    return;
	}

	victim->pcdata->body = value;
	return;
    }

  if ( !str_cmp( arg2, "mind" ) )
  {
	if ( IS_NPC(victim) )
	{ send_to_char( "Not on NPC's.\n\r", ch );
	  return;
	}

    max = 1000;
    if ( value < 1 || value > max )
    {
      sprintf( buf, "Mind range is 1 to %d.\n\r", max );
      send_to_char( buf, ch );
      return;
    }
    victim->pcdata->mind = value;
    return;
  }

  if ( !str_cmp( arg2, "spirit" ) )
  {
	if ( IS_NPC(victim) )
	{ send_to_char( "Not on NPC's.\n\r", ch );
	  return;
	}

    max = 1000;
    if ( value < 1 || value > max )
    { sprintf( buf, "Spirit range is 1 to %d.\n\r", max );
      send_to_char( buf, ch );
      return;
    }
    victim->pcdata->spirit = value;
    return;
  }

  if ( !str_cmp( arg2, "will" ) )
  {
	if ( IS_NPC(victim) )
	{ send_to_char( "Not on NPC's.\n\r", ch );
	  return;
	}

    max = 1000;
    if ( value < 1 || value > max )
    { sprintf( buf, "Willpower range is 1 to %d.\n\r", max );
      send_to_char( buf, ch );
      return;
    }
    victim->pcdata->will = value;
    return;
  }

  if ( !str_cmp( arg2, "primal" ) )
  {
	if ( IS_NPC(victim) )
	{ send_to_char( "Not on NPC's.\n\r", ch );
	  return;
	}
    max = 1000;
    if ( value < 1 || value > max )
    { sprintf( buf, "Primal range is 1 to %d.\n\r", max );
      send_to_char( buf, ch );
      return;
    }
    victim->pcdata->primal = value;
    return;
  }


    if ( !str_cmp( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	return;
    }

    if ( !str_cmp( arg2, "exp" ) )
    {
	if ( IS_NPC( victim) ) return;

	victim->exp = value;
	return;
    }


    if ( !str_cmp( arg2, "class" ) )
    {
	if ( value < 0 || value >= MAX_CLASS )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Class range is 0 to %d.\n", MAX_CLASS-1 );
	    send_to_char( buf, ch );
	    return;
	}
	victim->class = value;
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 125 )
	{
	    send_to_char( "Level range is 0 to 125.\n\r", ch );
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_cmp( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
	if ( value < -10 || value > 2000000000 )
	{
	    send_to_char( "Hp range is -10 to 2,000,0002,000,000,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
	return;
    }

    if ( !str_cmp( arg2, "mana" ) )
    {
	if ( value < 0 || value > 2000000000 )
	{
	    send_to_char( "Mana range is 0 to 2,000,000,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
	return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
	if ( value < 0 || value > 2000000000 )
	{
	    send_to_char( "Move range is 0 to 2,000,000,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
	return;
    }

    if ( !str_cmp( arg2, "practice" ) )
    {
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Practice range is 0 to 100 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_cmp( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Thirst range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Drunk range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Full range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	free_string( victim->name );
	victim->name = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
	free_string( victim->short_descr );
	victim->short_descr = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
	free_string( victim->long_descr );
	victim->long_descr = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "title" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	set_title( victim, arg3 );
	return;
    }

    if ( !str_cmp( arg2, "powers" ) )
    {
      if (IS_NPC(victim))
      { send_to_char( "NPC's don't have class powers.\n\r", ch );
	return;
      }
      argument = one_argument( argument, arg4 );
      power = is_number( arg4 ) ? atoi( arg4 ) : 0;

      if ( arg4[0] == '\0' || power == 0 )
      { send_to_char( "Need four arguments to set powers.\n\r", ch );
        return;
      }
     
      victim->pcdata->powers[value] = power;
      return;
    } 

    if ( !str_cmp( arg2, "spec" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	{
	    send_to_char( "No such spec fun.\n\r", ch );
	    return;
	}

	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax: oset <object> <field>  <value>\n\r",	ch );
	send_to_char( "or:     oset <object> <string> <value>\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  value0 value1 value2 value3\n\r",		ch );
	send_to_char( "  extra wear level weight cost timer\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "String being one of:\n\r",			ch );
	send_to_char( "  name short long ed\n\r",			ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_cmp( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_cmp( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    if ( !str_cmp( arg2, "name" ) )
    {
	free_string( obj->name );
	obj->name = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
	free_string( obj->short_descr );
	obj->short_descr = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
	free_string( obj->description );
	obj->description = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "owner" ) )
    {
    	free_string( obj->owner );
    	obj->owner = str_dup( arg3 );
    	return;
    }

    if ( !str_cmp( arg2, "ed" ) )
    {
	EXTRA_DESCR_DATA *ed;

	argument = one_argument( argument, arg3 );
	if ( argument == NULL )
	{
	    send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		ch );
	    return;
	}

	if ( extra_descr_free == NULL )
	{
	    ed			= alloc_perm( sizeof(*ed), PERM_DESC );
	}
	else
	{
	    ed			= extra_descr_free;
	    extra_descr_free	= extra_descr_free->next;
	}

	ed->keyword		= str_dup( arg3     );
	ed->description		= str_dup( argument );
	ed->next		= obj->extra_descr;
	obj->extra_descr	= ed;
	return;
    }

    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax: rset <location> <field> value\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_cmp( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_users( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char ips[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';
    buf2[0]     = '\0';
    sprintf( ips, "127.0.0.1" );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character != NULL && can_see( ch, d->character ) )
	{
	    count++;

	    /* Hiding from other users with the user command
	    if ( ch->level < MAX_LEVEL && !str_cmp( "Valgarv", d->character->name ) )
	    { sprintf( buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
	      d->descriptor,
	      d->connected,
	      d->original  ? d->original->name  :
	      d->character ? d->character->name : "(none)",
	      "128.118.128.134" );
	    }
	    else
	    */

	    { sprintf( buf + strlen(buf), "[%3d %2d] %-12s @ %s%s`n\n\r",
		d->descriptor,
		d->connected,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		(is_name( d->host, ips ) ? "`R" : "`n"),
		d->host
		);
	      strcat( ips, d->host );
	      strcat( ips, " " );
	    }
	}
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int trust;
    int cmd;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    /*
     * Look for command in command table.
     */
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( argument[0] == cmd_table[cmd].name[0]
	    &&   !str_prefix( argument, cmd_table[cmd].name )
	    &&   ( cmd_table[cmd].level > trust 
		  && cmd_table[cmd].level != 41 ) )
	{
	  send_to_char( "You cant even do that yourself!\n\r", ch );
	  return;
	}
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
	        MOBtrigger = TRUE;
		act( "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	MOBtrigger = TRUE;
	act( "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
	REMOVE_BIT(ch->act, PLR_WIZINVIS);
	act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_WIZINVIS);
	act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }

    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* Wizify and Wizbit sent in by M. B. King */

void do_wizify( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
  
    argument = one_argument( argument, arg1  );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: wizify <name>\n\r" , ch );
	return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r" , ch );
	return;
    }
    if ( IS_NPC( victim ) )
    {
	send_to_char( "Not on mobs.\n\r", ch );
	return;
    }
    victim->wizbit = !victim->wizbit;
    if ( victim->wizbit ) 
    {
	act( "$N wizified.\n\r", ch, NULL, victim, TO_CHAR );
	act( "$n has wizified you!\n\r", ch, NULL, victim, TO_VICT );
    }
    else
    {
	act( "$N dewizzed.\n\r", ch, NULL, victim, TO_CHAR );
	act( "$n has dewizzed you!\n\r", ch, NULL, victim, TO_VICT ); 
    }

    do_save( victim, "");
    return;
}

/* Idea from Talen of Vego's do_where command */

void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int obj_counter = 1;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  owhere <object>.\n\r", ch );
	return;
    }
    else
    {
	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
	    if ( !can_see_obj( ch, obj ) || !is_name( arg, obj->name ) )
	        continue;

	    found = TRUE;

	    for ( in_obj = obj; in_obj->in_obj != NULL;
		 in_obj = in_obj->in_obj )
	        ;

	    if ( in_obj->carried_by != NULL )
	    {
		sprintf( buf, "[%2d] %s carried by %s.\n\r", obj_counter,
			obj->short_descr, PERS( in_obj->carried_by, ch ) );
	    }
	    else
	    {
		sprintf( buf, "[%2d] %s in %s.\n\r", obj_counter,
			obj->short_descr, ( in_obj->in_room == NULL ) ?
			"somewhere" : in_obj->in_room->name );
	    }
	    
	    obj_counter++;
	    buf[0] = UPPER( buf[0] );
	    send_to_char( buf, ch );
	}
    }

    if ( !found )
	send_to_char(
		"Nothing like that in hell, earth, or heaven.\n\r" , ch );

    return;


}


void do_powerset( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int val1, val2;

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  { send_to_char( "Three values are required, a name, the power and the new value.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !is_number( arg2 ) || !is_number( arg3 ) )
  { send_to_char( "Both arguments must be positive integers.\n\r", ch );
    return;
  }

  val1 = atoi( arg2 );
  val2 = atoi( arg3 );

  sprintf( buf, "%s's power %d has been set to %d.\n\r", victim->name, val1, val2 );
  send_to_char( buf, ch );
  victim->pcdata->powers[val1] = val2;

  return;
}

void do_extraset( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int val1, val2;

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  { send_to_char( "Three values are required, a name, the extra and the new value.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !is_number( arg2 ) || !is_number( arg3 ) )
  { send_to_char( "Both arguments must be positive integers.\n\r", ch );
    return;
  }

  val1 = atoi( arg2 );
  val2 = atoi( arg3 );

  sprintf( buf, "%s's extra value %d has been set to %d.\n\r", victim->name, val1, val2 );
  send_to_char( buf, ch );
  victim->pcdata->extras[val1] = val2;

  return;
}


void do_cset( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int val1, val2;
 
   if ( IS_NPC(ch) )
   return;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  { send_to_char( "Three values are required, a name, the slot and the new value.\n\r", ch )
;
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !is_number( arg2 ) || !is_number( arg3 ) )
  { send_to_char( "Both arguments must be positive integers.\n\r", ch );
    return;
  }
  val1 = atoi( arg2 );
  val2 = atoi( arg3 );

  sprintf( buf, "%s's clan value %d has been set to %d.\n\r", victim->name, val1, val2 );
  send_to_char( buf, ch );
  victim->pcdata->clan[val1] = val2;

  return;
}

void do_llbackup( CHAR_DATA *ch, char *argument )
{ char path[MAX_INPUT_LENGTH];
  FILE *fp;
  int i;

  sprintf( path, "leaders.old" );
  fclose( fpReserve );
  if ( ( fp = fopen( path, "r" ) ) == NULL )
  { bug( "Leaderboard load: fopen", 0 );
    perror( path );
    return;
  }
  else
  { for ( i = 0; i < 10; i++ )
    { leaderboard[i].name = fread_string( fp );
      leaderboard[i].clan = fread_string( fp );
      leaderboard[i].pkills = fread_number( fp );
      leaderboard[i].pdeaths = fread_number( fp );
      leaderboard[i].assessment = fread_number( fp );
    }
  }
  fclose( fp );

  sprintf( path, "losers.old" );
  if ( ( fp = fopen( path, "r" ) ) == NULL )
  { bug( "Loserboard load: fopen", 0 );
    perror( path );
    return;
  }
  else
  { for ( i = 0; i < 10; i++ )
    { loserboard[i].name = fread_string( fp );
      loserboard[i].clan = fread_string( fp );
      loserboard[i].pkills = fread_number( fp );
      loserboard[i].pdeaths = fread_number( fp );
      loserboard[i].assessment = fread_number( fp );
    }
  }
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void do_llsave( CHAR_DATA *ch, char *argument )
{
  save_llboards( TRUE );
  save_llboards( FALSE );
  return;
}

void do_withdraw( CHAR_DATA *ch, char *argument )
{ char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char log_buf[MAX_STRING_LENGTH];
  int gold, clan;

  if ( IS_NPC(ch) || ch->level < 20 )
    return;
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( !is_number( arg1 ) || !is_number( arg2 ) )
  { send_to_char( "Syntax is withdraw <clan number> <gold>\n\r", ch );
    return;
  }

  clan = atoi( arg1 );
  gold = atoi( arg2 );
  if ( clan > MAX_CLANS || clan < 1 )
  { send_to_char( "Clan out of bounds.\n\r", ch );
    return;
  }
  if ( gold > 999999999 || gold < 1 )
  { send_to_char( "Be reasonable.\n\r", ch ); 
    return;
  }
  if ( clan_table[clan].gold < gold )
  { send_to_char( "They don't have that much.\n\r", ch ); 
    return;
  }
  clan_table[clan].gold -= gold;
  ch->gold += gold;
  sprintf( log_buf, "%d gold withdrawn from %s.", gold, clan_table[clan].name );
  log_string( log_buf );
  return;
}

void do_bits( CHAR_DATA *ch, char *argument )
{ char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int num;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || !is_number(arg) )
  { send_to_char( "Syntax is: bits <positive integer>\n\r", ch );
    return;
  }

  num = atoi( arg );

  sprintf( buf, "Bits in %d:", num );

  if ( IS_SET(num,1) )
    strcat( buf, " 1" );
  if ( IS_SET(num,2) )
    strcat( buf, " 2" );
  if ( IS_SET(num,4) )
    strcat( buf, " 4" );
  if ( IS_SET(num,8) )
    strcat( buf, " 8" );
  if ( IS_SET(num,16) )
    strcat( buf, " 16" );
  if ( IS_SET(num,32) )
    strcat( buf, " 32" );
  if ( IS_SET(num,64) )
    strcat( buf, " 64" );
  if ( IS_SET(num,128) )
    strcat( buf, " 128" );
  if ( IS_SET(num,256) )
    strcat( buf, " 256" );
  if ( IS_SET(num,512) )
    strcat( buf, " 512" );
  if ( IS_SET(num,1024) )
    strcat( buf, " 1024" );
  if ( IS_SET(num,2048) )
    strcat( buf, " 2048" );
  if ( IS_SET(num,4096) )
    strcat( buf, " 4096" );
  if ( IS_SET(num,8192) )
    strcat( buf, " 8192" );
  if ( IS_SET(num,16384) )
    strcat( buf, " 16384" );
  if ( IS_SET(num,32768) )
    strcat( buf, " 32768" );
  if ( IS_SET(num,65536) )
    strcat( buf, " 65536" );
  if ( IS_SET(num,131072) )
    strcat( buf, " 131072" );
  if ( IS_SET(num,262144) )
    strcat( buf, " 262144" );
  if ( IS_SET(num,524288) )
    strcat( buf, " 524288" );
  if ( IS_SET(num,1048576) )
    strcat( buf, " 1048576" );
  if ( IS_SET(num,2097152) )
    strcat( buf, " 2097152" );
  if ( IS_SET(num,4194304) )
    strcat( buf, " 4194304" );
  if ( IS_SET(num,8388608) )
    strcat( buf, " 8388608" );
  if ( IS_SET(num,16777216) )
    strcat( buf, " 16777216" );
  if ( IS_SET(num,33554432) )
    strcat( buf, " 33554432" );
  if ( IS_SET(num,67108864) )
    strcat( buf, " 67108864" );

  strcat( buf, "\n\r" );
  send_to_char( buf, ch );
  return;
}

void do_voteset( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MAX_STRING_LENGTH];
  char arg0[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char arg5[MAX_INPUT_LENGTH];
  char arg6[MAX_INPUT_LENGTH];
  int num, i;

  argument = one_argument( argument, arg0 );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg4 );
  argument = one_argument( argument, arg5 );
  argument = one_argument( argument, arg6 );

  if ( !is_number( arg0 ) )
  { send_to_char( "First argument is 1-5.\n\r", ch );
    return;
  }

  num = atoi( arg0 );
  if ( num < 2 || num > 5 )
  { send_to_char( "Between 2 and 5.\n\r", ch );
    return;
  }
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  { send_to_char( "At least two options and a question.\n\r", ch );
    return;
  }
  if ( num >= 3 && arg4[0] == '\0' )
  { send_to_char( "Less options than specified.\n\r", ch );
    return;
  }
  if ( num >= 4 && arg5[0] == '\0' )
  { send_to_char( "Less options than specified.\n\r", ch );
    return;
  }
  if ( num >= 5 && arg6[0] == '\0' )
  { send_to_char( "Less options than specified.\n\r", ch );
    return;
  }

  voting->num = num;
  voting->active = TRUE;
  voting->pulses = 8;
  free_string( voting->question );
  voting->question = str_dup( arg1 );
  for ( i = 1; i <= 5; i++ )
    voting->votes[i] = 0;
  free_string( voting->line1 );
  free_string( voting->line2 );
  voting->line1 = str_dup( arg2 );
  voting->line2 = str_dup( arg3 );
  if ( num >= 3 )
  { free_string( voting->line3 );
    voting->line3 = str_dup( arg4 );
  }
  if ( num >= 4 )
  { free_string( voting->line4 );
    voting->line4 = str_dup( arg5 );
  }
  if ( num >= 5 )
  { free_string( voting->line5 );
    voting->line5 = str_dup( arg6 );
  }

  do_echo( ch, "`d---*---*---*---*---*---*--- `WNew Vote`d ---*---*---*---*---*---*---`n" );
  sprintf( buf, "`B%s`n", voting->question );
  do_echo( ch, buf );
  sprintf( buf, " 1) %s", voting->line1 );
  do_echo( ch, buf );
  sprintf( buf, " 2) %s", voting->line2 );
  do_echo( ch, buf );
  if ( num >= 3 )
  { sprintf( buf, " 3) %s", voting->line3 );
    do_echo( ch, buf );
  }
  if ( num >= 4 )
  { sprintf( buf, " 4) %s", voting->line4 );
    do_echo( ch, buf );
  }
  if ( num >= 5 )
  { sprintf( buf, " 5) %s", voting->line5 );
    do_echo( ch, buf );
  }

  do_echo( ch, "`d---*---*---*---*---*---*--- `WNew Vote`d ---*---*---*---*---*---*---`n" );
  for ( vch = char_list; vch != NULL; vch = vch_next )
  { vch_next = vch->next;
    if ( !IS_NPC(vch) && IS_SET(vch->pcdata->actnew,NEW_VOTED) )
      REMOVE_BIT(vch->pcdata->actnew,NEW_VOTED);
  }

  return;
}

void do_copyover( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    extern time_t copyover_time;
    extern time_t copyover_warning;
    int wait = 0;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        copyover();
    }
    else if ( !str_prefix( arg, "abort" ) )
    {
        if ( copyover_time < current_time )
        {
            stc( "There is no copyover to abort.\n\r", ch );
            return;
        }
         else
        {
            copyover_time = 0;
            copyover_warning = 0;
            send_to_all_char( "`RCopyover aborted.`n\n\r" );
            return;
        }
    }
    else if ( !is_number( arg ) )
    {
        stc( "The only acceptable argument for copyover is an integer.\n\r", ch );
        return;
    }

    wait = atoi( arg );
    if ( wait < 10 || wait > 600 )
    {
        stc( "Wait must be between 10 and 600 seconds.\n\r", ch );
        return;
    }

    sprintf( buf, "`RCopyover in `W%d`R seconds.`n\n\r", wait );                 
    send_to_all_char( buf );                                                     
    copyover_time = current_time + wait;                                         
    copyover_warning = current_time + (wait / 2);
                                                                                 
    return;                                                                      
}  
void copyover()
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100];
	char log_buf[100];
	
	fp = fopen (COPYOVER_FILE, "w");
	
	if (!fp)
	{
		sprintf( log_buf, "Could not write to copyover file: %s", COPYOVER_FILE);
		log_string( log_buf );
		perror ("do_copyover:fopen");
		return;
	}
	
	/* Consider changing all saved areas here, if you use OLC */
	
	/* do_asave (NULL, ""); - autosave changed areas */
	
	
	sprintf (buf, "\n\rReality shivers and collapses into a singularity.\n\r" );
	
	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
			write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
			/*
			if (och->level == 1)
			{
				write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
				advance_level (och);
				och->level++; // Advance_level doesn't do that
			}
			*/
			save_char_obj (och);
			write_to_descriptor (d->descriptor, buf, 0);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	/* Save player corpses */
	save_copyover();


	/* Close reserve and other always-open files and release other resources */
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	
	// sprintf (buf, "%d", port);
	sprintf( buf, "%d", 1984 );
	sprintf (buf2, "%d", control);
	sprintf( log_buf, "%s %s %s %s %s %s", EXE_FILE, "merc", buf, "copyover", buf2, (char *) NULL );
	log_string( log_buf );
	execl (EXE_FILE, "merc", buf, "copyover", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */

	perror ("do_copyover: execl");

	/* Here you might want to reopen fpReserve */
}

void do_minionset( CHAR_DATA *ch, char *argument )
{
}

void do_fightlist( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  char buf[MAX_INPUT_LENGTH];

  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
    if ( !IS_NPC(vch) && vch->fighting != NULL )
    {
      sprintf( buf, "%s fighting %s in %d.\n\r",
        vch->name,
        (IS_NPC(vch->fighting) ? vch->fighting->short_descr : vch->fighting->name),
        vch->in_room->vnum );
      send_to_char( buf, ch );
    }
  }

  return;
}

void do_tard( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    stc( "Tard who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    stc( "You can't tard that which does not exist.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    stc( "NPC's are already retarded.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->pcdata->actnew,NEW_TARD) )
  {
    REMOVE_BIT(victim->pcdata->actnew,NEW_TARD);
    sprintf( buf, "%s is no longer retarded.\n\r", victim->name );
    stc( buf, ch );
  }
  else
  {
    SET_BIT(victim->pcdata->actnew,NEW_TARD);
    sprintf( buf, "%s is now retarded.\n\r", victim->name );
    stc( buf, ch );
  }

  return;
}

void do_token( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    stc( "Give who a token?\n\r", ch );
    return;
  }
  if ( (victim = get_char_world( ch, arg )) == NULL )
  {
    stc( "You can't find them.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  {
    stc( "Not on NPCs.\n\r", ch );
    return;
  }

  victim->pcdata->extras[TOKENS]++;
  act( "$N has granted you a token!", victim, NULL, ch, TO_CHAR );
  sprintf( buf, "%s now has %d tokens.\n\r", victim->name, victim->pcdata->extras[TOKENS]);
  stc( buf, ch );
  return;
}

void do_llsort( CHAR_DATA *ch, char *argument )
{
  sort_llboards();
  stc( "Leader and loser boards have been sorted once.\n\r", ch );
  return;
}

void do_clanedit( CHAR_DATA *ch, char *argument )
{
  int i, clan;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( !str_cmp( arg1, "zero" ) )
  {
    stc( "Attempting to zero out a clan.\n\r", ch );
  }
  else
  {
    stc( "That command is not yet supported.\n\r", ch );
    sprintf( buf, "You sent me '%s' and '%s'.\n\r", arg1, arg2 );
    stc( buf, ch );
    return;
  }

  if ( arg2[0] == '\0' )
  {
    stc( "Remove what clan?\n\r", ch );
    return;
  }
  if ( !is_number( arg2 ) )
  {
    stc( "Clans are denoted by slot.\n\r", ch );
    return;
  }
  clan = atoi( arg2 );
  if ( clan < 1 || clan > MAX_CLANS )
  {
    stc( "There is no such clan.\n\r", ch );
    return;
  }

  clan_table[clan].gold = 0;
  clan_table[clan].pkills = 0;
  clan_table[clan].pdeaths = 0;
  clan_table[clan].mobkills = 0;
  clan_table[clan].mobdeaths = 0;
  for ( i = 0; i <= MAX_CLANS; i++ )
    clan_table[clan].kills[i] = 0;
  for ( i = 0; i <= MAX_CLANS; i++ )
    clan_table[i].kills[clan] = 0;
  sprintf( buf, "Clan %s has been zeroed out.\n\r", clan_table[clan].name );
  stc( buf, ch );
  return;
}

void do_send( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg1 );
  if ( arg1[0] == '\0' )
  {
    stc( "Send a message to whom?\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  {
    stc( "Send what message?\n\r", ch );
    return;
  }

  if ( (victim = get_char_world( ch, arg1 )) == NULL )
  {
    stc( "They aren't around.\n\r", ch );
    return;
  }

  stc( argument, victim );
  sprintf( buf, "You send '%s' to $N.", argument );
  act( buf, ch, NULL, victim, TO_CHAR );
  return;
}

void do_squish( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int duration;
    time_t now;
    CHAR_DATA *victim;

    time( &now );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        stc( "Syntax: squish <asshole> <hours>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
      stc( "Syntax: squish <asshole> <hours>\n\r", ch );
      return;
    }

    duration = atoi( arg2 );

    stop_fighting( victim, TRUE );

    /* put victim at market squre */
    char_from_room( victim );
    char_to_room( victim, get_room_index( 3011 ) );

    victim->pcdata->extras2[SQUISH] = ((int)now + (duration*3600));
    victim->pcdata->extras[TIMER] = 0;
    sprintf( buf, "You have been squished for %d hours.\n\r", duration );
    stc( buf, victim );
    sprintf( buf, "%s has been squished for %d hours.\n\r", victim->name, duration );
    stc( buf, ch );
    do_quit( victim, "" );

    return;
}

void do_buggedmob( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MAX_STRING_LENGTH];

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next;
    if ( IS_NPC(vch) )
    {
      if ( vch->level > 100 )
      {
        sprintf( buf, "100+ %s (%d)\n\r", vch->short_descr, vch->pIndexData->vnum );
        stc( buf, ch );
      }
      if ( vch->max_hit > 30000 )
      {
        sprintf( buf, "hps  %s (%d)\n\r", vch->short_descr, vch->pIndexData->vnum );
        stc( buf, ch );
      }
    }
  }
  return;
}

void do_swave( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( (victim = ch->fighting) == NULL )
  {
    if ( (victim = get_char_room(ch,arg)) == NULL )
    {
      stc( "Swave who?\n\r", ch );
      return;
    }
  }

  act( "You launch a wave of black energy at $N.", ch, NULL, victim, TO_CHAR );
  act( "A wave of black energy slams into you!", ch, NULL, victim, TO_VICT );
  act( "A wave of black energy slams into $N.", ch, NULL, victim, TO_NOTVICT );
  if ( !IS_NPC(victim) )
    unstance( victim );
  return;
}

void do_sblast( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( (victim = ch->fighting) == NULL )
  {
    if ( (victim = get_char_room(ch,arg)) == NULL )
    {
      stc( "Sblast who?\n\r", ch );
      return;
    }
  }

  act( "You blast $N with shadowy energy!.", ch, NULL, victim, TO_CHAR );
  act( "You are hammered by a blast of shadowy energy!", ch, NULL, victim, TO_VICT );
  act( "$N is hammered by a blast of shadowy energy.", ch, NULL, victim, TO_ROOM );
  damage( ch, victim, dice(8,1000), DAM_OBSIDIAN );
  if ( victim != NULL && !IS_NPC(victim) )
  {
    if ( IS_CLASS(victim,CLASS_FIST) )
    {
      if ( victim->pcdata->powers[F_KI] > 0 )
      {
        victim->pcdata->powers[F_KI] -= dice(4,4)+1;
        victim->pcdata->powers[F_KI] = UMAX( 0, victim->pcdata->powers[F_KI] );
      }
      if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
        REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
      act( "Your shadow blast staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "$n's shadow blast breaks your rhythm!", ch, NULL, victim, TO_VICT );
      act( "$n's shadow blast staggers $N.", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( victim->class == CLASS_SAIYAN )
    { if ( is_affected(victim,gsn_kiwall) )
      { affect_strip(victim,gsn_kiwall);
        act( "Your shadow blast staggers $N!", ch, NULL, victim, TO_CHAR );
        act( "$n's shadow blast collapes your wall of Ki!", ch, NULL, victim, TO_VICT );
        act( "$n's shadow blast staggers $N.", ch, NULL, victim, TO_NOTVICT );
      }
    }
    else if ( IS_CLASS(victim,CLASS_SORCERER) )
    {
      act( "Your shadow blast staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "$n's shadow blast disrupts your casting!", ch, NULL, victim, TO_VICT );
      act( "$n's shadow blast staggers $N.", ch, NULL, victim, TO_NOTVICT );
      lose_chant( victim );
    }
    else if ( IS_CLASS(victim,CLASS_MAZOKU) && victim->pcdata->powers[M_CTYPE] != 0 )
    {
      act( "Your shadow blast staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "Stunned, you lose control of your charge!", ch, NULL, victim, TO_VICT );
      act( "$n's shadow blast staggers $N.", ch, NULL, victim, TO_NOTVICT );
      victim->pcdata->powers[M_CTYPE] = 0;
      victim->pcdata->powers[M_CTIME] = 0;
    }

  }
  return;
}

void do_sbolt( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( (victim = ch->fighting) == NULL )
  {
    if ( (victim = get_char_room(ch,arg)) == NULL )
    {
      stc( "Sbolt who?\n\r", ch );
      return;
    }
  }

  act( "You launch a bolt of black energy at $N.", ch, NULL, victim, TO_CHAR );
  act( "A bolt of black energy tears into you!", ch, NULL, victim, TO_VICT );
  act( "A bolt of black energy tears into $N.", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dice(20,800), DAM_OBSIDIAN );
  return;
}

void do_sbomb( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  act( "Shadow force explodes all around you!", ch, NULL, NULL, TO_CHAR );
  act( "Shadowy energy explodes outwards from $n!", ch, NULL, NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;
    if ( IS_NPC(vch) )
      continue;
    if ( !can_see(ch,vch) )
      continue;
    damage( ch, vch, dice(6,1000), DAM_OBSIDIAN );
  }

  return;
}


void do_mburst( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    if ( ch->fighting == NULL )
    {
      stc( "Mburst who?\n\r", ch );
      return;
    }
    else
      victim = ch->fighting;
  }
  else if ( (victim = get_char_room( ch, arg )) == NULL )
  {
    stc( "Mburst who?\n\r", ch );
    return;
  }

  act( "You open up on $N with a barrage of gunfire!", ch, NULL, victim, TO_CHAR );
  act( "$n sprays you with gunfire!", ch, NULL, victim, TO_VICT );
  act( "$n sprays $N with a barrage of gunfire.", ch, NULL, victim, TO_NOTVICT );
  while( victim && number_percent() > 15 )
  {
    damage( ch, victim, 2500+dice(1,1000), DAM_BULLETS );
  }
  return;
}

void do_mplist( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  char buf[MAX_STRING_LENGTH];
  MPROG_ACT_LIST *actprog;

  for ( vch = char_list; vch != NULL; vch = vch->next )
  {

    if ( IS_NPC(vch) && (vch->mpactnum > 0 || vch->mpact != NULL) )
    {
      for( actprog = vch->mpact; actprog != NULL; actprog = actprog->next )
      {
        sprintf( buf, "%s: %s\n\r", vch->short_descr, actprog->buf );
        stc( buf, ch );
      }
    }
  }
  return;
}

void do_karma( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int k;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )
  { 
    stc( "Syntax: karma <list/char name> [new karma value]\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "list" ) )
  {
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
      if ( IS_NPC(victim) )
        continue;
      if ( victim->pcdata->extras[PUSSY] != 0 )
      {
        sprintf( buf, "%12s:  %d\n\r", victim->name, victim->pcdata->extras[PUSSY] );
        stc( buf, ch );
      }
    }
  }
  else
  {
    if ( arg2[0] == '\0' || !is_number( arg2 ) )
    {
      stc( "Syntax: karma <list/char name> [new karma value]\n\r", ch );
      return;
    }

    k = atoi( arg2 );
    k = URANGE( -500, k, 500 );

    if ( (victim = get_char_world(ch,arg1)) == NULL )
    {
      stc( "Player not found.\n\r", ch );
      return;
    }
    if ( IS_NPC(victim) )
    {
      stc( "That is a mob.\n\r", ch );
      return;
    }
    victim->pcdata->extras[PUSSY] = k;
    sprintf( buf, "%s's karma has been set to %d.\n\r", victim->name, k );
    stc( buf, ch );
  }

  return;
}

void do_retir( CHAR_DATA *ch, char *argument )
{
  stc( "If you want to retire somebody, spell it out.\n\r", ch );
  return;
}

void do_retire( CHAR_DATA *ch, char *argument )
{
  return;
}


void do_changeclass( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int i;

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    stc( "Syntax: changeclass <saiyan/patryn/fist/sorcerer/mazoku>\n\r", ch );
    return;
  }
  else if ( !str_prefix( arg, "saiyan" ) )
  {
    ch->class = 1;
    ch->pcdata->powers[0] = 30000;
    ch->pcdata->powers[1] = 30000;
    ch->pcdata->powers[2] = 1000;
    ch->pcdata->powers[3] = 1000;
    ch->pcdata->powers[4] = 1000;
    ch->pcdata->powers[5] = 1000;
    ch->pcdata->powers[6] = 1000;
    ch->pcdata->powers[7] = 1000;
    ch->pcdata->powers[8] = 520191;
    ch->pcdata->powers[9] = 0;
    if ( ch->pcdata->weapons[0] > 200 )
    {
      for ( i = 0; i < 13; i++ )
        ch->pcdata->weapons[i] = 200;
      for ( i = 1; i <= 10; i++ )
        ch->pcdata->stances[i] = 200;
    }
    stc( "You are now a saiyan.\n\r", ch );
  }
  else if ( !str_prefix( arg, "patryn" ) )
  {
    ch->class = 2;
    ch->pcdata->powers[0] = 24575;
    ch->pcdata->powers[1] = 14;
    ch->pcdata->powers[2] = 100;
    ch->pcdata->powers[3] = 100;
    ch->pcdata->powers[4] = 100;
    ch->pcdata->powers[5] = 100;
    ch->pcdata->powers[6] = 100;
    ch->pcdata->powers[7] = 100;
    ch->pcdata->powers[8] = 1;
    ch->pcdata->powers[9] = 0;
    for ( i = 0; i < 5; i++ )
    {
      ch->pcdata->runes[LEFTARM][i] = 4;
      ch->pcdata->runes[RIGHTARM][i] = 4;
    }
    ch->pcdata->runes[RIGHTARM][4] = 1;
    ch->pcdata->runes[RIGHTARM][5] = 1;
    for ( i = 0; i < 8; i++ )
    {
      ch->pcdata->runes[LEFTLEG][i] = 2;
      ch->pcdata->runes[RIGHTLEG][i] = 2;
    }
    for ( i = 0; i < 14; i++ )
      ch->pcdata->runes[TORSO][i] = 2;
    ch->pcdata->runes[TORSO][14] = 4;

    if ( ch->pcdata->weapons[0] > 200 )
    {
      for ( i = 0; i < 13; i++ )
        ch->pcdata->weapons[i] = 200;
      for ( i = 1; i <= 10; i++ )
        ch->pcdata->stances[i] = 200;
    }

    stc( "You are now a patryn.\n\r", ch );
  }
  else if ( !str_prefix( arg, "fist" ) )
  {
    ch->class = 3;
    ch->pcdata->powers[0] = 0;
    ch->pcdata->powers[1] = 30;
    ch->pcdata->powers[2] = 1305;
    ch->pcdata->powers[3] = 100;
    ch->pcdata->powers[4] = 100;
    ch->pcdata->powers[5] = 100;
    ch->pcdata->powers[6] = 100;
    ch->pcdata->powers[7] = 10;
    ch->pcdata->powers[8] = 746523;
    ch->pcdata->powers[9] = 0;

    ch->pcdata->weapons[0] = 500;
    for ( i = 1; i <= 10; i++ )
      ch->pcdata->stances[i] = 300;

    stc( "You are now a fist.\n\r", ch );
  }
  else
  {
    stc( "Can't change into that.\n\r", ch );
  }

  return;
}

