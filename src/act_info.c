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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"


char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              "
};



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(`bInvis`n) "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(`RRed Aura`n) "  );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(`CMagical`n) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(`YGlowing`n) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(`WHumming`n) "   );
    if ( IS_OBJ_STAT(obj, ITEM_UNIQUE)    )   strcat( buf, "(`dU`nn`Wiq`nu`de`n) " );
    if ( IS_OBJ_STAT(obj, ITEM_ASTRAL)    )   strcat( buf, "(`dAstral`n) "    );
    if ( IS_OBJ_STAT(obj, ITEM_HARDENED)  )   strcat( buf, "(`rH`n) "	      );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL )
	    strcat( buf, obj->description );
    }

    if ( !IS_NPC(ch) && ch->level >= LEVEL_HERO )
    {
      sprintf( buf2, " (%d)", obj->pIndexData->vnum );
      strcat( buf, buf2 );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(`gInvis`n) "      );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(`mHide`n) "       );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "(`mCharmed`n) "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(`dTranslucent`n) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(`MPink Aura`n) "  );
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "(`WWhite Aura`n) " );
    if ( IS_AFFECTED(victim, AFF_STEELY)      ) strcat( buf, "(`dS`nt`Wee`nl`dy`n) " );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
						strcat( buf, "(KILLER) "     );
    if ( IS_NPC(victim) && !IS_NPC(ch) && ch->pcdata->extras2[QUEST_TYPE] == QUEST_MOB
      && victim->pIndexData->vnum == ch->pcdata->extras2[QUEST_INFO] )
      						strcat( buf, "[`rTARGET`n] "     );

    if ( victim->position == POS_STANDING && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	return;
    }

    strcat( buf, PERS( victim, ch ) );
    if ( IS_SUIT(victim) )
    { sprintf( buf2, " piloting a %s suit ", suit_table[victim->pcdata->suit[SUIT_NUMBER]].name );
      strcat( buf, buf2 );
    }
    else
    if ( !IS_NPC(victim) && !IS_SET(ch->act, PLR_BRIEF) )
    { strcat( buf, " " ); 
      strcat( buf, victim->pcdata->title );
    }

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: strcat( buf, " is sleeping here." );      break;
    case POS_RESTING:  strcat( buf, " is resting here." );       break;
    case POS_STANDING: strcat( buf, " is here." );               break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "somone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' && (IS_NPC(ch) || !IS_SET(ch->act,PLR_NODESC)) )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

         if ( percent >= 100 ) strcat( buf, " is in perfect health.\n\r"  );
    else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\n\r" );
    else if ( percent >=  80 ) strcat( buf, " has a few bruises.\n\r"     );
    else if ( percent >=  70 ) strcat( buf, " has some cuts.\n\r"         );
    else if ( percent >=  60 ) strcat( buf, " has several wounds.\n\r"    );
    else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\n\r" );
    else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\n\r"    );
    else if ( percent >=  30 ) strcat( buf, " is covered in blood.\n\r"   );
    else if ( percent >=  20 ) strcat( buf, " is leaking guts.\n\r"       );
    else if ( percent >=  10 ) strcat( buf, " is almost dead.\n\r"        );
    else                       strcat( buf, " is DYING.\n\r"              );

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < ch->pcdata->learned[gsn_peek] )
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
	&&   IS_SET(rch->act, PLR_WIZINVIS)
	&&   get_trust( ch ) < get_trust( rch ) )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}



void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char rname[MAX_STRING_LENGTH];
    char rdesc[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door, i = 0, j = 0, count = 0;

    if ( !IS_NPC( ch ) && ch->desc == NULL ) 
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   !IS_SET(ch->act, PLR_TRUESIGHT)
    &&	 !(IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_EYES) )
    &&	 !IS_SUIT(ch)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	sprintf( rname, "`B%s`n", ch->in_room->name );
	send_to_char( rname, ch );
	/* send_to_char( ch->in_room->name, ch ); */
	send_to_char( "\n\r", ch );

	// if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	do_exits( ch, "auto" );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) )
 	{

	  rdesc[j] = '`'; j++;
	  rdesc[j] = 'd'; j++;
	  while ( ch->in_room->description[i] != '\0' )
	  {
	    rdesc[j] = ch->in_room->description[i];
	    count++;
	    if ( rdesc[j] == '\n' || rdesc[j] == '\r' )
	      count = 0;
	    i++; j++;
	    if ( count >= 80 && ch->in_room->description[i] == ' ' )
	    {
	      i++;
	      rdesc[j] = '\n'; j++;
	      rdesc[j] = '\r'; j++;
	      count = 0;
	    }
	  }
	  rdesc[j] = '`'; j++;
	  rdesc[j] = 'n'; j++;
	  rdesc[j] = '\0';
	  send_to_char( rdesc, ch ); 
	}

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char( obj->description, ch );
	    return;
	}
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char( obj->description, ch );
	    return;
	}
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
	send_to_char( pdesc, ch );
	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}



void do_examine( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int sn = skill_lookup( "identify" );

    if ( IS_NPC(ch) )
      return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }


    if ( (victim = get_char_room( ch, arg )) != NULL )
    { 
	if ( IS_NPC(victim) )
	{ send_to_char( "It's a mob.  Big whoop.\n\r", ch );
	  return;
	}

	sprintf( buf, "You examine %s..\n\r", victim->name );
	if ( is_affected( victim, skill_lookup( "armor" ) ) )
	  strcat( buf, "They are well armored.\n\r" );
	if ( is_affected( victim, skill_lookup( "shield" ) ) )
	  strcat( buf, "They are protected by a thin force shield.\n\r" );
	if ( is_affected( victim, skill_lookup( "stone skin" ) ) )
	  strcat( buf, "Their skin is as hard as rock.\n\r" );
	if ( is_affected( victim, skill_lookup( "bless" ) ) )
	  strcat( buf, "They are blessed.\n\r" );
	if ( is_affected( victim, skill_lookup( "visfarank" ) ) )
	  strcat( buf, "Their hands are glowing with power.\n\r" );
	if ( is_affected( victim, skill_lookup( "mos varim" ) ) )
	  strcat( buf, "A flourescent ball of light hovers above them.\n\r" );
	if ( is_affected( victim, skill_lookup( "holy resist" ) ) )
	  strcat( buf, "They are limned with holy power.\n\r" );
	if ( is_affected( victim, skill_lookup( "vas gluudo" ) ) )
	  strcat( buf, "They are encased in a translucent energy shield.\n\r" );
	if ( is_affected( victim, skill_lookup( "windy shield" ) ) )
	  strcat( buf, "The wind swirls smoothly around them.\n\r" );
	if ( IS_AFFECTED( victim, AFF_RAYWING) )
	  strcat( buf, "They are flying freely.\n\r" );
	else if ( is_affected( victim, skill_lookup( "fly" ) ) )
	  strcat( buf, "Their feet are several inches off the ground.\n\r" );
	if ( is_affected( victim, skill_lookup( "sanctuary" ) ) )
	  strcat( buf, "They have a white aura about them.\n\r" );

	if ( IS_SET(victim->pcdata->actnew,NEW_PHOENIX_AURA) )
	  strcat( buf, "They are limned in deep red flames.\n\r" );
	if ( IS_SET(victim->pcdata->actnew,NEW_RECOVERY) )
	  strcat( buf, "Their wounds are regenerating.\n\r" );
	if ( is_affected(victim,gsn_kiwall) )
	  strcat( buf, "They are surrounded by a wall of Ki flame.\n\r" );
	if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	  strcat( buf, "Their hands are moving rapidly in a figureeight defense.\n\r" );

	if ( strlen( buf ) < 35 )
	  strcat( buf, "You notice nothing unusual.\n\r" );

	act( "$n stares at $N for a moment, then turns away.", ch, NULL, victim, TO_NOTVICT );
	act( "You notice $n sizing you up.", ch, NULL, victim, TO_VICT );
	send_to_char( buf, ch );
        return;
   } 


    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    { send_to_char( "You aren't carrying that.\n\r", ch );
      return;
    }

    ( skill_table[sn].spell_fun) ( sn, ch->pcdata->mind, ch, obj );

    /* old examine code, replaced with 'identify' spell
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }
    */

    return;
}



/* colorized dir names */
char *	const	color_dir	[]	=
{
  "`Gnorth`n", "`Beast`n", "`Csouth`n", "`Mwest`n", "`Rup`n", "`Ydown`n"
};

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "`W[`dExits:`B" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, color_dir[door] );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name
		    );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "`W]`n\n\r" );

    send_to_char( buf, ch );
    return;
}


void do_score( CHAR_DATA *ch, char *argument )
{
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
  {
    return;
  }

  sprintf( buf1, "+-----------------------------------------------------------------+\n\r" );
  sprintf( buf2, "| `R%16s`n the `C%-10s`n   | `B%3d`n years old, `B%5d`n hours  |\n\r",
  	ch->name, class_table[ch->class].who_name, get_age(ch), (ch->played/120) );
  strcat( buf1, buf2 );
  sprintf( buf2, "+-----------------------------------------------------------------+\n\r" );
  strcat( buf1, buf2 );
  sprintf( buf2, "|   Hp: `G%5d`n/`G%-5d`n  |   Body: `B%-3d`n  |     Items: `B%5d`n/`B%-5d`n      |\n\r",
  	ch->hit, ch->max_hit, ch->pcdata->body, ch->carry_number, can_carry_n(ch) );
  strcat( buf1, buf2 );
  sprintf( buf2, "| Mana: `C%5d`n/`C%-5d`n  |   Mind: `B%-3d`n  |    Weight: `B%5d`n/`B%-8d`n   |\n\r",
  	ch->mana, ch->max_mana, ch->pcdata->mind, ch->carry_weight, can_carry_w(ch) );
  strcat( buf1, buf2 );
  sprintf( buf2, "| Move: `c%5d`n/`c%-5d`n  | Spirit: `B%-3d`n  |    Primal: `B%5d`n            |\n\r",
  	ch->move, ch->max_move, ch->pcdata->spirit, ch->pcdata->primal );
  strcat( buf1, buf2 );
  sprintf( buf2, "| Page: `B%5d`n        |   Will: `B%-3d`n  |    Tokens: `B%5d`n            |\n\r",
  	ch->pcdata->pagelen, ch->pcdata->will, ch->pcdata->extras[TOKENS] );
  strcat( buf1, buf2 );
  sprintf( buf2, "+-----------------------------------------------------------------+\n\r" );
  strcat( buf1, buf2 );
  sprintf( buf2, "|   Hitroll: `B%-5d`n   | Experience: `W%-10d`n |  Mobkills: `B%-6d`n |\n\r",
  	ch->hitroll, ch->exp, ch->pcdata->kills[MK] );
  strcat( buf1, buf2 );
  sprintf( buf2, "|   Damroll: `B%-5d`n   |       Gold: `B%-10d`n | Mobdeaths: `B%-6d`n |\n\r",
  	ch->damroll, ch->gold, ch->pcdata->kills[MD] );
  strcat( buf1, buf2 );
  sprintf( buf2, "|  Autoexit: %3s     |   Autoloot: %3s        |   Autosac: %-3s    |\n\r",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "`Byes`n" : "`dno `n",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "`Byes`n" : "`dno `n",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "`Byes`n" : "`dno `n" );
  strcat( buf1, buf2 );
  sprintf( buf2, "+-----------------------------------------------------------------+\n\r" );
  strcat( buf1, buf2 );

  if ( ch->pcdata->extras2[EVAL] >= 30 )
  { if ( ch->pcdata->extras2[EVAL] >= 60 && ch->pcdata->extras2[EVAL] < 65 )
    {
      sprintf( buf2, "|  You are `B%2d`n percent of the way to the next assessment.          |\n\r", ((ch->pcdata->extras2[EVAL] % 10)*20) );
      strcat( buf1, buf2 );
    }
    else if ( ch->pcdata->extras2[EVAL] < 60 )
    {
      sprintf( buf2, "|  You are `B%2d`n percent of the way to the next assessment.          |\n\r", ((ch->pcdata->extras2[EVAL] % 10)*10) );
      strcat( buf1, buf2 );
    }
  }

  if ( GET_AC(ch) < -750 )
    sprintf( buf2, "|  Your armor is godlike!                                         |\n\r" );
  if ( GET_AC(ch) < -500 )
    sprintf( buf2, "|  Your armor is well nigh impenetrable!                          |\n\r" );
  else if ( GET_AC(ch) < -450 )
    sprintf( buf2, "|  You're armored like the Sandrock!                              |\n\r" );
  else if ( GET_AC(ch) < -400 )
    sprintf( buf2, "|  You're armored like the Heavyarms!                             |\n\r" );
  else if ( GET_AC(ch) < -350 )
    sprintf( buf2, "|  You're armored like the Shenlong!                              |\n\r" );
  else if ( GET_AC(ch) < -300 )
    sprintf( buf2, "|  You're armored like a Taurus.                                  |\n\r" );
  else if ( GET_AC(ch) < -250 )
    sprintf( buf2, "|  You're armored like a Leo.                                     |\n\r" );
  else if ( GET_AC(ch) < -200 )
    sprintf( buf2, "|  You are massively armored.                                     |\n\r" );
  else if ( GET_AC(ch) < -150 )
    sprintf( buf2, "|  You are heavily armored.                                       |\n\r" );
  else if ( GET_AC(ch) < -100 )
    sprintf( buf2, "|  You are well armored.                                          |\n\r" );
  else if ( GET_AC(ch) < -50 )
    sprintf( buf2, "|  You are armored.                                               |\n\r" );
  else if ( GET_AC(ch) < 0 )
    sprintf( buf2, "|  You are mostly armored.                                        |\n\r" );
  else if ( GET_AC(ch) < 25 )
    sprintf( buf2, "|  You are somewhat armored.                                      |\n\r" );
  else if ( GET_AC(ch) < 50 )
    sprintf( buf2, "|  You are slightly armored.                                      |\n\r" );
  else if ( GET_AC(ch) < 75 )
    sprintf( buf2, "|  You are barely armored at all.                                 |\n\r" );
  else if ( GET_AC(ch) < 100 )
    sprintf( buf2, "|  You might as well be naked.                                    |\n\r" );
  else if ( GET_AC(ch) < 150 )
    sprintf( buf2, "|  You're worse than naked!                                       |\n\r" );
  else if ( GET_AC(ch) < 250 )
    sprintf( buf2, "|  You're a big fat target!                                       |\n\r" );
  else
    sprintf( buf2, "|  Your unprotected body is screaming 'Kill me, Kill me!'.        |\n\r" );

  strcat( buf1, buf2 );

  sprintf( buf2, "+-----------------------------------------------------------------+\n\r" );
  strcat( buf1, buf2 );

  send_to_char( buf1, ch );
  return;
}




void do_old_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf,
	"You are %s the %s, %d years old (%d hours).\n\r",
	ch->name,
	class_table[ch->class].who_name,
	get_age(ch),
	(ch->played / 120 ) );
    send_to_char( buf, ch );

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );
	send_to_char( buf, ch );
    }

    sprintf( buf,
	"You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->practice );
    send_to_char( buf, ch );

    sprintf( buf,
	"You are carrying %d/%d items with weight %d/%d kg.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );
    send_to_char( buf, ch );

    sprintf( buf,
	"Body: %d  Mind: %d  Spirit: %d  Willpower: %d     Primal: %d\n\r",
	ch->pcdata->body,
	ch->pcdata->mind,
	ch->pcdata->spirit, 
	ch->pcdata->will,
	ch->pcdata->primal );
    send_to_char( buf, ch );

    sprintf( buf,
	"You have scored %d exp, %d gold coins, and %d tokens.\n\r",
	ch->exp,  ch->gold, ( !IS_NPC(ch) ? ch->pcdata->extras[TOKENS] : 0) );
    send_to_char( buf, ch );

    sprintf( buf,
	"Autoexit: %s.  Autoloot: %s.  Autosac: %s.\n\r",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no" );
    send_to_char( buf, ch );
    
    sprintf( buf, "Wimpy set to %d hit points.\n\r", ch->wimpy );
    send_to_char( buf, ch );

    if ( !IS_NPC( ch ) )
    {
	sprintf( buf, "Page pausing set to %d lines of text.\n\r",
		ch->pcdata->pagelen );
	send_to_char( buf, ch );
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "You are hungry.\n\r",  ch );

    switch ( ch->position )
    {
    case POS_DEAD:     
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    }


    send_to_char( "You are ", ch );
         if ( GET_AC(ch) >=  101 ) send_to_char( "WORSE than naked!\n\r", ch );
    else if ( GET_AC(ch) >=   80 ) send_to_char( "naked.\n\r",            ch );
    else if ( GET_AC(ch) >=   60 ) send_to_char( "wearing clothes.\n\r",  ch );
    else if ( GET_AC(ch) >=   40 ) send_to_char( "slightly armored.\n\r", ch );
    else if ( GET_AC(ch) >=   20 ) send_to_char( "somewhat armored.\n\r", ch );
    else if ( GET_AC(ch) >=    0 ) send_to_char( "armored.\n\r",          ch );
    else if ( GET_AC(ch) >= - 50 ) send_to_char( "well armored.\n\r",     ch );
    else if ( GET_AC(ch) >= -100 ) send_to_char( "strongly armored.\n\r", ch );
    else if ( GET_AC(ch) >= -150 ) send_to_char( "heavily armored.\n\r",  ch );
    else if ( GET_AC(ch) >= -250 ) send_to_char( "superbly armored.\n\r", ch );
    else if ( GET_AC(ch) >= -400 ) send_to_char( "divinely armored.\n\r", ch );
    else                           send_to_char( "invincible!\n\r",       ch );

    sprintf( buf, "You have exterminated %d monsters, and gotten stomped by %d of 'em.\n\r",
		ch->pcdata->kills[MK],	ch->pcdata->kills[MD] );
    send_to_char( buf, ch );
    /* removed
    sprintf( buf, "You have %d status.\n\r", ch->pcdata->extras[STATUS] );
    send_to_char( buf, ch );
    */

	sprintf( buf, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );
	send_to_char( buf, ch );
    
    if ( !IS_NPC(ch) && ch->pcdata->extras2[EVAL] >= 30 )
    { if ( ch->pcdata->extras2[EVAL] >= 60 )
        sprintf( buf, "You are %d percent of the way to the next assessment.\n\r",
          ((ch->pcdata->extras2[EVAL] % 10)*20) );
      else
        sprintf( buf, "You are %d percent of the way to the next assessment.\n\r",
          ((ch->pcdata->extras2[EVAL] % 10)*10) );

      send_to_char( buf, ch );
    }


    /* in affects now
    if ( ch->affected != NULL )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
	    send_to_char( buf, ch );

	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
		    " modifies %s by %d for %d hours",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );
		send_to_char( buf, ch );
	    }

	    send_to_char( ".\n\r", ch );
	}
    }
    */

    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\rMerc started up at %s\rThe system time is %s\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time )
	);

    send_to_char( buf, ch );
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if ( argument[0] == '\0' )
	argument = "summary";

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argument, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) )
	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\n\r", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		send_to_char( pHelp->text+1, ch );
	    else
		send_to_char( pHelp->text  , ch );
	    return;
	}
    }

    send_to_char( "No help on that word.\n\r", ch );
    return;
}




/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char startbuf[MAX_STRING_LENGTH];
    char *sigil;
    DESCRIPTOR_DATA *d;
    int nMatch, invised, stat;
    int min = 0, max = 999;
    bool clan = FALSE;
 
    sprintf( startbuf, "\n\r`d---------------------------------------------------------------------------`n" );

    if ( !IS_NPC(ch) && !str_cmp( argument, "target" ) )
    { if ( ch->pcdata->extras2[EVAL] >= 30 )
      { min = 30;
        max = 999;
      }
      else if ( ch->pcdata->extras2[EVAL] < 10 )
      { send_to_char( "You can't finish anybody off, yet.\n\r", ch );
        return;
      }
      else
      { min = (ch->pcdata->extras2[EVAL]/10) * 10;
        max = (ch->pcdata->extras2[EVAL]/10)*10 + 9;
      }
    }
    else if ( !str_cmp(argument,"clan") || !str_cmp(argument,"clanned") )
      clan = TRUE;

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    invised = 0;
    buf[0] = '\0';
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *legend;
	char const *status;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected == CON_PLAYING && !can_see( ch, d->character ) )
	{ invised++;
	  continue;
	}
	if ( d->connected != CON_PLAYING )
	{ continue;
	}


	wch   = ( d->original != NULL ) ? d->original : d->character;
	if ( (wch->pcdata->extras2[EVAL] < min
	  || wch->pcdata->extras2[EVAL] > max) 
	  && !IS_SET(wch->pcdata->actnew,NEW_FAIR_GAME) )
	    continue;

	if ( clan && (wch->pcdata->clan[CLAN] <= 0) )
	  continue;

	nMatch++;

	/*
	 * Figure out what to print for status.
	 */
	switch ( wch->level )
	{
       default: stat = wch->pcdata->extras2[EVAL];
		if ( stat <= 9 )	status = "`cfreshmeat`n";
		else if ( stat <= 19 )	status = "`g2nd class`n";
		else if ( stat <= 29 )	status = "`y1st class`n";
		else if ( stat <= 39 )	status = "`WPowerful `n";
		else if ( stat <= 49 )	status = "`RManiacal `n";
		else if ( stat <= 59 )	status = "`dUnholy   `n";
		else if ( stat <= 64 )  status = "`RB`d.`RA`d.`RM`d.`RF`d. `n";
		else 
		{ switch( wch->class )
		  { default:	status = "`dW `WI `dE `WR `dD`n";	break;
		    case 1:	status = "`d -`n=`WSSJ`n=`d- `n";	break;
		    case 2:	status = "`dR`nu`Wne Lo`nr`dd`n";	break;
		    case 3:	status = "`d S`nh`Waol`ni`dn `n";	break;
		    case 4:
		      if ( wch->pcdata->powers[SORC_SPEC] == 2 )
		        status = "`d W`na`Wrlo`nc`dk `n";
		      else if ( wch->pcdata->powers[SORC_SPEC] == 8 )
		        status = " `dR`ny`Wozo`nk`du `n";
		      else
		        status = "`dAr`nc`Whma`ng`dus`n";
		      break;
		    case 5:	status = " `dDe`nm`Wo`nn`dic`n ";	break;
		  }
		}

		break;
		

	case 1:				status = "`nmortal   `n";	break;
	case 16:			status = "`YHero     `n";	break;
	case MAX_LEVEL:			status = "`YAdmin    `n";  break;
	}

	if ( IS_SET(wch->pcdata->actnew,NEW_FAIR_GAME) )
	  status = "`MFair Game`n";
	if ( IS_SET(wch->pcdata->actnew,NEW_RETIRED) && wch->level <= 2 )
	  status = "`d-`WR`ne`dtir`ne`Wd`d-`n";


	/*
	 * Figure out what to print for legend.
	 */
	switch ( wch->pcdata->extras[LEGEND] )
	{
	default:	legend = "->error<-";				break;
	case 0:		legend = "`dPeasant  `n";			break;
	case 1:		legend = "`dC`Wi`dt`Wi`dz`We`dn  `n";		break;
	case 2:		legend = "`nC`de`nl`de`nb`dr`ni`dt`ny`n";	break;
	case 3:		legend = "`RC`rh`Ra`rm`Rp`ri`Ro`rn `n";		break;
	case 4:		legend = "`rH`Re`rr`Ro     `n";			break;
	case 5:		legend = "`dS`ru`dp`re`dr`rh`de`rr`do`n";	break;
	case 6:		legend = "`dS`Ra`dv`Ri`do`Ru`dr  `n";		break;
	case 7:		legend = "`dG`ru`Rardi`ra`dn `n";		break;
	case 8:		legend = "`WHerald   `n";			break;
	case 9:		legend = "`WA`nn`rcie`nn`Wt  `n";		break;
	case 10:	legend = "`bM`Be`Cssi`Ba`bh  `n";		break;
	}

	if ( wch->level == MAX_LEVEL )
	  legend = "`GBastard`W!!`n";
	if ( IS_SET(wch->pcdata->actnew,NEW_LEGEND) || wch->level == 20 )
	  legend = wch->pcdata->legend;
	if ( wch->pcdata->extras2[GLOBALS] < 0 )
	  legend = "`MSpammered`n";
	if ( IS_SET(wch->pcdata->actnew,NEW_TARD) )
	  legend = "`MTard     `n";
	if ( IS_SET(wch->act,PLR_SILENCE) )
	  legend = "`MSilenced `n";




	/* create a formatted name and title string */


	if ( strlen( wch->pcdata->cflag ) > 0 && strcmp(wch->pcdata->cflag,"(null)") )
	  sigil = wch->pcdata->cflag;
	else
	  sigil = clan_table[wch->pcdata->clan[CLAN]].title;

	/*
	 * Format it up.
	 */
	sprintf( buf + strlen(buf), "\n\r[%s][%s] %s%s`n %s  %s`n",
	    status,
	    legend,
	    ( wch->level > LEVEL_HERO ? "`W" : "`R" ),
	    wch->name, 
	    wch->pcdata->title,
    	    sigil );
    }

    sprintf( buf + strlen(buf), "\n\r\n\r`dThere %s `W%d `dvisible player%s `dout of `W%d `din the game.`n",
    nMatch == 1 ? "is" : "are",
    nMatch, nMatch == 1 ? "" : "s",
    nMatch + invised );
    sprintf( buf2, "\n\r`d---------------------------------------------------------------------------`n\n\r" );
    strcat( startbuf, buf );
    strcat( startbuf, buf2 );
    send_to_char( startbuf, ch );
    return;
}



void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content )
	{
	    if ( obj2->wear_loc != WEAR_NONE
	    &&   can_see_obj( ch, obj2 )
	    &&   obj1->item_type == obj2->item_type
	    && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( obj2 == NULL )
	{
	    send_to_char( "You aren't wearing anything comparable.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}
    }
	    
    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:
	    value1 = obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char areaname[MAX_STRING_LENGTH];
    int i;
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( i = 16; ch->in_room->area->name[i] != '\0'; i++ )
	  areaname[i-16] = ch->in_room->area->name[i];
	areaname[i-16] = '\0';

	sprintf( buf, "Players in %s:\n\r", areaname );
	send_to_char( buf, ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !is_cloaked(victim, victim)
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg = '\0';
    char *buf = '\0';
    int diff;
    int hpdiff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) )
    {
	send_to_char( "The gods do not accept this type of sacrafice.\n\r",
		     ch );
	return;
    }

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR );

    /* additions by king@tinuviel.cs.wcu.edu */
    hpdiff = ( ch->hit - victim->hit );

    if ( ( ( diff >= 0) && ( hpdiff <= 0 ) )
	|| ( ( diff <= 0 ) && ( hpdiff >= 0 ) ) )
    {
        send_to_char( "Also,", ch );
    }
    else
    {
        send_to_char( "However,", ch );
    }

    if ( hpdiff >= 101 )
        buf = " you are currently much healthier than $E.";
    if ( hpdiff <= 100 )
        buf = " you are currently healthier than $E.";
    if ( hpdiff <= 50 ) 
        buf = " you are currently slightly healthier than $E.";
    if ( hpdiff <= 25 )
        buf = " you are a teensy bit healthier than $E.";
    if ( hpdiff <= 0 )
        buf = " $E is a teensy bit healthier than you.";
    if ( hpdiff <= -25 )
        buf = " $E is slightly healthier than you.";
    if ( hpdiff <= -50 )
        buf = " $E is healthier than you.";
    if ( hpdiff <= -100 )
        buf = " $E is much healthier than you.";

    act( buf, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    /* removed  if ( isalpha(title[0]) || isdigit(title[0]) ) */
    if ( 0 )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    char *point;
    int counter = 0;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    point = argument;
    while ( *point != '\0' )
    {
      if ( counter >= (45 - strlen(ch->name)) )
	*point = '\0';
      else if ( *point == '`' && *(point+1) != '`' && *(point+1) != '\0' )
      { point++;
        point++;
      }
      else
      {
        counter++;
        point++;
      }
    }
    /* stupid off by one error */
    counter++;
    for ( ; counter < (45 - strlen(ch->name)); counter++ )
    { *point = ' ';
      point++;
    }
    *point = '\0';
 
    /* old style length check 
    if ( strlen(argument) > 45 )
	argument[45] = '\0';
    */

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH*2];
    char buf3[MAX_STRING_LENGTH*2];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

	colorize( buf2, buf, TRUE );
	colorize( buf3, argument, TRUE );

	if ( strlen(buf2) + strlen(buf3) >= MAX_STRING_LENGTH - 2 )
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

    buf1[0] = '\0';


    if ( argument[0] == '\0' )
    {
	int col;

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class] )
		continue;

	    sprintf( buf, "%18s %3d%%  ",
		skill_table[sn].name, ch->pcdata->learned[sn] );
	    strcat( buf1, buf );
	    if ( ++col % 3 == 0 )
		strcat( buf1, "\n\r" );
	}

	if ( col % 3 != 0 )
	    strcat( buf1, "\n\r" );

	sprintf( buf, "You have %d experience points left.\n\r",
	    ch->exp );
	strcat( buf1, buf );
	send_to_char( buf1, ch );
    }
    else
    {
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	if ( ch->exp < 5000 )
	{
	    send_to_char( "You don't have enough experience.\n\r", ch );
	    return;
	}

	if ( ( sn = skill_lookup( argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   ch->level < skill_table[sn].skill_level[ch->class] ) )
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	adept = 100;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already an adept of %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->exp = ch->exp - 5000;
	    ch->pcdata->learned[sn] = 100;
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now an adept of $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now an adept of $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    sprintf( log_buf, "%s password changed from %s to %s.", ch->name, ch->pcdata->pwd, pwdnew );
    log_string( log_buf );
    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
 
    col = 0;
    for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
    {
	sprintf( buf, "%-12s", social_table[iSocial].name );
	send_to_char( buf, ch );
	if ( ++col % 6 == 0 )
	    send_to_char( "\n\r", ch );
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}



/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    buf1[0] = '\0';
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
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



void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
	{
	    send_to_char( "You are silenced.\n\r", ch );
	    return;
	}

	send_to_char( "Channels:", ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_AUCTION)
	    ? " +AUCTION"
	    : " -auction",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_CHAT)
	    ? " +CHAT"
	    : " -chat",
	    ch );

	if ( ch->level == MAX_LEVEL )
	  send_to_char( !IS_SET(ch->deaf, CHANNEL_WIZNET)
	    ? " +WIZNET"
	    : " -wiznet",
	    ch );


	if ( IS_HERO(ch) )
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_IMMTALK)
		? " +IMMTALK"
		: " -immtalk",
		ch );
	}

	send_to_char( !IS_SET(ch->deaf, CHANNEL_MUSIC)
	    ? " +MUSIC"
	    : " -music",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_QUESTION)
	    ? " +QUESTION"
	    : " -question",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_SHOUT)
	    ? " +SHOUT"
	    : " -shout",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_YELL)
	    ? " +YELL"
	    : " -yell",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_INFO )
	    ? " +INFO"
	    : " -info",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_TELL )
	    ? " +TELL"
	    : " -tell",
	    ch );

	send_to_char( ".\n\r", ch );
    }
    else
    {
	bool fClear;
	int bit;

	     if ( arg[0] == '+' ) fClear = TRUE;
	else if ( arg[0] == '-' ) fClear = FALSE;
	else
	{
	    send_to_char( "Channels -channel or +channel?\n\r", ch );
	    return;
	}

	     if ( !str_cmp( arg+1, "auction"  ) ) bit = CHANNEL_AUCTION;
        else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
	else if ( !str_cmp( arg+1, "wiznet" ) &&
	          ch->level == MAX_LEVEL        ) bit = CHANNEL_WIZNET;
	else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
	else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
	else if ( !str_cmp( arg+1, "question" ) ) bit = CHANNEL_QUESTION;
	else if ( !str_cmp( arg+1, "shout"    ) ) bit = CHANNEL_SHOUT;
	else if ( !str_cmp( arg+1, "yell"     ) ) bit = CHANNEL_YELL;
	else if ( !str_cmp( arg+1, "info"     ) ) bit = CHANNEL_INFO;
	else if ( !str_cmp( arg+1, "tell"     ) ) bit = CHANNEL_TELL;
	else
	{
	    send_to_char( "Set or clear which channel?\n\r", ch );
	    return;
	}

	if ( fClear )
	    REMOVE_BIT (ch->deaf, bit);
	else
	    SET_BIT    (ch->deaf, bit);

	send_to_char( "Ok.\n\r", ch );
    }

    return;
}



/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "[ Keyword  ] Option\n\r", ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOEXIT)
            ? "[+AUTOEXIT ] You automatically see exits.\n\r"
	    : "[-autoexit ] You don't automatically see exits.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOLOOT)
	    ? "[+AUTOLOOT ] You automatically loot corpses.\n\r"
	    : "[-autoloot ] You don't automatically loot corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOSAC)
	    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\n\r"
	    : "[-autosac  ] You don't automatically sacrifice corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BLANK)
	    ? "[+BLANK    ] You have a blank line before your prompt.\n\r"
	    : "[-blank    ] You have no blank line before your prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BRIEF)
	    ? "[+BRIEF    ] You see brief descriptions.\n\r"
	    : "[-brief    ] You see long descriptions.\n\r"
	    , ch );
         
	send_to_char(  IS_SET(ch->act, PLR_COMBINE)
	    ? "[+COMBINE  ] You see object lists in combined format.\n\r"
	    : "[-combine  ] You see object lists in single format.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_PROMPT)
	    ? "[+PROMPT   ] You have a prompt.\n\r"
	    : "[-prompt   ] You don't have a prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_TELNET_GA)
	    ? "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
	    : "[-telnetga ] You don't receive a telnet GA sequence.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_NOFOLLOW)
	    ? "[+NOFOLLOW ] You may not be followed.\n\r"
	    : "[-nofollow ] You may be followed.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_FIREBACK)
	    ? "[+FIREBACK ] You can fire back at ranged attacks.\n\r"
	    : "[-fireback ] You will not enter combat when range attacked.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_NODESC)
	    ? "[+NODESC   ] You will not see descs looking at players.\n\r"
	    : "[-nodesc   ] You will see players' descs when you look at them.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_SILENCE)
	    ? "[+SILENCE  ] You are silenced.\n\r"
	    : ""
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_EMOTE)
	    ? ""
	    : "[-emote    ] You can't emote.\n\r"
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_TELL)
	    ? ""
	    : "[-tell     ] You can't use 'tell'.\n\r"
	    , ch );
	send_to_char( !IS_SET(ch->act, PLR_NO_NOTE)
	    ? ""
	    : "-note      ] You can't write notes.\n\r"
	    , ch );
    }
    else
    {
	bool fSet;
	int bit;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

             if ( !str_cmp( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_cmp( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !str_cmp( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !str_cmp( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_cmp( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_cmp( arg+1, "combine"  ) ) bit = PLR_COMBINE;
        else if ( !str_cmp( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_cmp( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !str_cmp( arg+1, "nofollow" ) ) bit = PLR_NOFOLLOW;
	else if ( !str_cmp( arg+1, "fireback" ) ) bit = PLR_FIREBACK;
	else if ( !str_cmp( arg+1, "nodesc"   ) ) bit = PLR_NODESC;
	else
	{
	    send_to_char( "Config which option?\n\r", ch );
	    return;
	}

	if ( fSet )
	    SET_BIT    (ch->act, bit);
	else
	    REMOVE_BIT (ch->act, bit);

	send_to_char( "Ok.\n\r", ch );
    }

    return;
}

void do_wizlist ( CHAR_DATA *ch, char *argument )
{

    do_help ( ch, "wizlist" );
    return;

}

void do_spells ( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int sn;
    int col;

    if ( ( !IS_NPC( ch ) &&
	   !class_table[ch->class].fMana )
    ||  IS_NPC ( ch ) )
    {  
       send_to_char ( "You do not know how to cast spells!\n\r", ch );
       return;
    }

    buf1[0] = '\0';

    col = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        if ( skill_table[sn].name == NULL )
	   break;
	if ( ( ch->level < skill_table[sn].skill_level[ch->class] )
	|| ( skill_table[sn].skill_level[ch->class] > LEVEL_HERO ) )
	   continue;

	sprintf ( buf, "%18s %3dpts ",
           skill_table[sn].name, MANA_COST( ch, sn ) );
	strcat( buf1, buf );
	if ( ++col % 3 == 0 )
	   strcat( buf1, "\n\r" );
    }

    if ( col % 3 != 0 )
      strcat( buf1, "\n\r" );

    send_to_char ( buf1, ch );
    return;

}

void do_slist ( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int sn;
    int col;
    int level;
    bool pSpell;

    if ( ( !IS_NPC( ch ) &&
	   !class_table[ch->class].fMana )
    ||  IS_NPC ( ch ) )
    {  
       send_to_char ( "You do not need any stinking spells!\n\r", ch );
       return;
    }

    buf1[0] = '\0';

    strcat ( buf1, "ALL Spells available for your class.\n\r\n\r" );
    strcat ( buf1, "Lv          Spells\n\r\n\r" );

    for ( level = 1; level < LEVEL_IMMORTAL; level++ )
    {

      col = 0;
      pSpell = TRUE;

      for ( sn = 0; sn < MAX_SKILL; sn++ )
      {
	if ( skill_table[sn].name == NULL )
	  break;
	if ( skill_table[sn].skill_level[ch->class] != level )
	  continue;

	if (pSpell)
	{
	  sprintf ( buf, "%2d:", level );
	  strcat ( buf1, buf );
	  pSpell = FALSE;
	}

	if ( ++col % 5 == 0 )
	  strcat ( buf1, "   " );

	sprintf ( buf, "%18s", skill_table[sn].name );
	strcat ( buf1, buf );

	if ( col % 4 == 0 )
	  strcat ( buf1, "\n\r" );

      }

      if ( col % 4 != 0 )
	strcat ( buf1, "\n\r" );

    }

    send_to_char( buf1, ch );
    return;

}

/* by passing the conf command - Kahn */

void do_autoexit ( CHAR_DATA *ch, char *argument )
{

    ( IS_SET ( ch->act, PLR_AUTOEXIT )
     ? do_config( ch, "-autoexit" )
     : do_config( ch, "+autoexit" ) );

}

void do_autoloot ( CHAR_DATA *ch, char *argument )
{

    ( IS_SET ( ch->act, PLR_AUTOLOOT )
     ? do_config( ch, "-autoloot" )
     : do_config( ch, "+autoloot" ) );

}

void do_autosac ( CHAR_DATA *ch, char *argument )
{

    ( IS_SET ( ch->act, PLR_AUTOSAC )
     ? do_config( ch, "-autosac" )
     : do_config( ch, "+autosac" ) );

}

void do_blank ( CHAR_DATA *ch, char *argument )
{

    ( IS_SET ( ch->act, PLR_BLANK )
     ? do_config( ch, "-blank" )
     : do_config( ch, "+blank" ) );

}

void do_brief ( CHAR_DATA *ch, char *argument )
{

    ( IS_SET ( ch->act, PLR_BRIEF )
     ? do_config( ch, "-brief" )
     : do_config( ch, "+brief" ) );

}

 
void do_pagelen ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int lines;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	lines = 20;
    else
	lines = atoi( arg );

    if ( lines < 1 || lines > 10000 )
    {
	send_to_char(
		"Negative or Zero values for a page pause is not legal.\n\r",
		     ch );
	return;
    }

    ch->pcdata->pagelen = lines;
    sprintf( buf, "Page pause set to %d lines.\n\r", lines );
    send_to_char( buf, ch );
    return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void do_prompt( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   buf[0] = '\0';

   if ( argument[0] == '\0' )
   {
       ( IS_SET ( ch->act, PLR_PROMPT )
	? do_config( ch, "-prompt" )
	: do_config( ch, "+prompt" ) );
       return;
   }

   if( !str_cmp( argument, "default" ) )
     strcpy( buf, "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV]> ");
   else if ( !str_cmp( argument, "fist" ) )
     strcpy( buf, "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`W%k`n Ki}> " );
   else if ( !str_cmp( argument, "saiyan" ) )
     strcpy( buf, "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`W%P`nP `R%S`nS `R%s`ns `R%A`nA}> ");
   else if ( !str_cmp( argument, "sorcerer" ) )
     strcpy( buf, "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`W%c`nM %p %C}> " );
   else if ( !str_cmp( argument, "mazoku" ) )
     strcpy( buf, "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`M%e`nEss `M%E`nEgo `M%n`nNil}> " );
   else if ( !str_cmp( argument, "admin" ) )
     strcpy( buf, "`m<`M[`W%R`n %z`M]`m>`n%i " );
   else
   {
      if ( strlen(argument) > MAX_STRING_LENGTH/10 )
         argument[MAX_STRING_LENGTH/10] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
   }

   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   send_to_char( "Ok.\n\r", ch );
   return;
} 

void do_auto( CHAR_DATA *ch, char *argument )
{

    do_config( ch, "" );
    return;

}

void do_level( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ( IS_NPC( ch) )
    return;

  sprintf( buf, "\n\r`d-----------------------------------------------------------------------------`n\n\r" );

  sprintf( buf2, "Hit:  %3d  Slice:%3d  Stab:  %3d Slash: %3d  Whip:  %3d  Claw: %3d  Blast:%3d\n\r",
		ch->pcdata->weapons[0], ch->pcdata->weapons[1], ch->pcdata->weapons[2],
		ch->pcdata->weapons[3], ch->pcdata->weapons[4], ch->pcdata->weapons[5],
		ch->pcdata->weapons[6] );
  strcat( buf, buf2 );
  send_to_char( buf, ch );
  sprintf( buf,  "Pound:%3d  Crush:%3d  Grep:  %3d Bite:  %3d  Pierce:%3d  Blow: %3d\n\r\n\r",
		ch->pcdata->weapons[7], ch->pcdata->weapons[8], ch->pcdata->weapons[9],
		ch->pcdata->weapons[10], ch->pcdata->weapons[11], ch->pcdata->weapons[12] );
  send_to_char( buf, ch );
  sprintf( buf,  "Lion:   %3d  Lynx:   %3d  Snake:  %3d  Badger: %3d  Ferret: %3d\n\r",
	ch->pcdata->stances[1], ch->pcdata->stances[2], ch->pcdata->stances[3],
	ch->pcdata->stances[4], ch->pcdata->stances[5] );
  send_to_char( buf, ch );
  sprintf( buf,  "Hawk:   %3d  Eagle:  %3d  Vulture:%3d  Sparrow:%3d  Stork:  %3d\n\r",
	ch->pcdata->stances[6], ch->pcdata->stances[7], ch->pcdata->stances[8],
	ch->pcdata->stances[9], ch->pcdata->stances[10] );

  if ( ch->class == CLASS_SAIYAN )
  { sprintf( buf2, "\n\rMax Power:   %5d  Max Strength:  %4d  Max Speed:  %4d  Max Aegis:  %4d\n\r",
		ch->pcdata->powers[S_POWER_MAX], ch->pcdata->powers[S_STRENGTH_MAX],
		ch->pcdata->powers[S_SPEED_MAX], ch->pcdata->powers[S_AEGIS_MAX] );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    sprintf( buf,      "    Power:   %5d      Strength:  %4d      Speed:  %4d      Aegis:  %4d\n\r",
		ch->pcdata->powers[S_POWER],	ch->pcdata->powers[S_STRENGTH],
		ch->pcdata->powers[S_SPEED],	ch->pcdata->powers[S_AEGIS] );
  }

  if ( ch->class == CLASS_FIST )
  { sprintf( buf2, "\n\rTorso:  %3d  Hands:  %3d  Arms:   %3d  Legs:   %3d  Max Ki: %3d\n\r",
		ch->pcdata->powers[F_TORSO],	ch->pcdata->powers[F_HANDS],
		ch->pcdata->powers[F_ARMS],	ch->pcdata->powers[F_LEGS],
		ch->pcdata->powers[F_KI_MAX] );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    sprintf( buf, "`n" );
  }

  if ( ch->class == CLASS_PATRYN )
  { sprintf( buf2, "\n\rAir:    %3d  Earth:    %3d  Fire:   %3d  Water:  %3d\n\r",
  		ch->pcdata->powers[P_AIR],	ch->pcdata->powers[P_EARTH],
  		ch->pcdata->powers[P_FIRE],	ch->pcdata->powers[P_WATER] );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    sprintf( buf, "Energy: %3d  Negative: %3d\n\r",
    		ch->pcdata->powers[P_ENERGY],	ch->pcdata->powers[P_NEGATIVE] );
  }

  if ( ch->class == CLASS_SORCERER )
  { sprintf( buf2, "\n\rRanks in schools of magic.\n\r" );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    sprintf( buf, "Black:  %2d  Earth:  %2d  Wind:  %2d  Fire: %2d  Water: %2d  Astral: %2d  White: %2d\n\r",
    ch->pcdata->powers[SCHOOL_BLACK],	ch->pcdata->powers[SCHOOL_EARTH],
    ch->pcdata->powers[SCHOOL_WIND],	ch->pcdata->powers[SCHOOL_FIRE],
    ch->pcdata->powers[SCHOOL_WATER],	ch->pcdata->powers[SCHOOL_ASTRAL],
    ch->pcdata->powers[SCHOOL_WHITE] );
  }

  if ( ch->class == CLASS_MAZOKU )
  { sprintf( buf2, "\n\rEssense:  %7d  Ego:     %3d  Nihilism:  %3d\n\r",
  	ch->pcdata->powers[M_ESSENSE],	ch->pcdata->powers[M_EGO],
  	ch->pcdata->powers[M_NIHILISM] );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    sprintf( buf, "Matter:       %3d  Astral:  %3d  Focus:     %3d\n\r",
    	ch->pcdata->powers[M_MATTER],	ch->pcdata->powers[M_ASTRAL],
    	ch->pcdata->powers[M_FOCUS] );
  }


  sprintf( buf2, "`d-----------------------------------------------------------------------------`n\n\r" );
  strcat( buf, buf2 );

  send_to_char( buf, ch );
  return;
}

void do_scan( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *scan_room;
  EXIT_DATA *pexit;
  char buf[MAX_STRING_LENGTH];
  int dir;

  char	*	const	dir_name[]	=
  {
    "north", "east", "south", "west", "up", "down"
  };

  send_to_char( "Here:\n\r", ch );
  show_char_to_char( ch->in_room->people, ch );

  for ( dir = 0; dir < 6; dir++ )
  { if ( (pexit = ch->in_room->exit[dir]) == NULL || (scan_room = pexit->to_room) == NULL )
      continue;
    else
    { sprintf( buf, "\n\rTo the %s:\n\r", dir_name[dir] );
      send_to_char( buf, ch );
      if ( scan_room->people == NULL )
	continue;
      else
	show_char_to_char( scan_room->people, ch );
    }
  }

  return;
}

char *skill_message[10] =
{	"slightly skilled",
	"reasonable",
	"fairly competent",
	"highly skilled",
	"very dangerous",
	"extremely deadly",
	"an expert",
	"a master",
	"a grand master",
	"supremely skilled"
};



void do_skills( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  OBJ_DATA *wield;
  int stance, weap;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  

  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
  { weap = UMIN( ch->pcdata->weapons[wield->value[3]] / 25, 8 );
    sprintf( buf, "You are %s with %s.\n\r", skill_message[weap], wield->short_descr );
    send_to_char( buf, ch );
  }
  else
  { weap = UMIN( ch->pcdata->weapons[0] / 25, 8 );
    sprintf( buf, "You are %s in unarmed combat.\n\r", skill_message[weap] );
    send_to_char( buf, ch );
  }

  if ( stanced( ch ) > 0 && stanced( ch ) <= 10 )
  { stance = UMIN( ch->pcdata->stances[stanced(ch)] / 25, 8 );
    sprintf( buf, "You are %s in the %s stance.\n\r", skill_message[stance], stance_table[stanced(ch)].name );
    send_to_char( buf, ch );
  }

  if ( ( victim = ch->fighting ) != NULL  && !IS_NPC(victim) )
  { if ( ( wield = get_eq_char( victim, WEAR_WIELD ) ) != NULL )
    { weap = UMIN( victim->pcdata->weapons[wield->value[3]] / 25, 8 );
      sprintf( buf, "%s is %s with %s.\n\r", victim->name, skill_message[weap], wield->short_descr );
      send_to_char( buf, ch );
    }
    else
    { weap = UMIN( victim->pcdata->weapons[0] / 25, 8 );
      sprintf( buf, "%s is %s in unarmed combat.\n\r", victim->name, skill_message[weap] );
      send_to_char( buf, ch );
    }
  
    if ( stanced( victim ) > 0 && stanced( victim ) <= 10 )
    { stance = UMIN( victim->pcdata->stances[stanced(victim)] / 25, 8 );
      sprintf( buf, "%s is %s in the %s stance.\n\r", victim->name, skill_message[stance], stance_table[stanced(victim)].name );
      send_to_char( buf, ch );
    }
  }

  return;
}  


void do_timer( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  if ( ch->pcdata->extras[TIMER] == 0 )
    sprintf( buf, "You do not have a fight timer.\n\r" );
  else
    sprintf( buf, "Your fight timer is currently at %d.\n\r", ch->pcdata->extras[TIMER] );

  send_to_char( buf, ch );
  return;
}

void do_finger( CHAR_DATA *ch, char *argument )
{
  FILE *fp;
  char finfile[50];
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *word = "none";
  char *name = "none";
  char *title = "none";
  char *laston = "none";
  char *created = "none";
  char *sex = "none";
  char *class = "none";
  char *status = "none";
  char *rating = "none";
  char *courage = "none";
  int kills[4];
  int i, level = 0, stat = 0, ostat = 0, ratio = 0, clan = 0, rank = 0, line = 0, cnum = 0;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  { send_to_char( "Finger who?\n\r", ch );
    return;
  }
  i = 0;
  while ( arg[i] != '\0' )
  {
    if ( !isalpha(arg[i]) )
    {
      stc( "Stop trying to crash the mud, cocksmoker.\n\r", ch );
      return;
    }
    i++;
  }

  sprintf( finfile, "../finger/%s", capitalize( arg ) );
  if ( ( fp = fopen( finfile, "r" ) ) == NULL )
  { send_to_char( "No finger file for that character.\n\r", ch );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  else
  {
    for ( line = 0; line < MAX_FINGER_LINES && !feof(fp) ; line++ )
    {
      word = fread_word( fp );
      if ( !str_cmp( word, "Name" ) )
        name = fread_string( fp );
      else if ( !str_cmp( word, "Title" ) )
        title = fread_string( fp );
      else if ( !str_cmp( word, "Sex" ) )
        sex = fread_string( fp );
      else if ( !str_cmp( word, "Class" ) )
        class = fread_string( fp );
      else if ( !str_cmp( word, "Laston" ) )
        laston = fread_string( fp );
      else if ( !str_cmp( word, "Created" ) )
        created = fread_string( fp );
      else if ( !str_cmp( word, "Clan" ) )
      {
        clan = fread_number( fp );
        rank = fread_number( fp );
      }
      else if ( !str_cmp( word, "Level" ) )
        level = fread_number( fp );
      else if ( !str_cmp( word, "PkPdMkMd" ) )
      {
        kills[0] = fread_number( fp );
        kills[1] = fread_number( fp );
        kills[2] = fread_number( fp );
        kills[3] = fread_number( fp );
      }
      else if ( !str_cmp( word, "Ostat" ) )
        ostat = fread_number( fp );
      else if ( !str_cmp( word, "Status" ) )
        stat = fread_number( fp );
      else if ( !str_cmp( word, "Courage" ) )
        cnum = fread_number( fp );
      else
      {
        fclose( fp );
        stc( "Bugged finger file.\n\r", ch );
        return;
      }
    } // end of for loop
  } // end of else statement

  fclose( fp );

  if ( level == 20 || stat < 50 )
    cnum = -1;

  if ( stat < 10 )
    status = "`cfreshmeat`n";
  else if ( stat < 20 )
    status = "`g2nd class`n";
  else if ( stat < 30 )
    status = "`y1st class`n";
  else if ( stat < 40 )
    status = "`WPowerful`n";
  else if ( stat < 50 )
    status = "`RManiacal`n";
  else if ( stat < 60 )
    status = "`dUnholy`n";
  else
    status = "`RB`d.`RA`d.`RM`d.`RF`d.`n";

  if ( kills[0] < 10 && kills[1] < 10)
    rating = "    none    ";
  else
  { if ( kills[0] >= kills[1] )
    { ratio = kills[0] * 10 / UMAX( 1, kills[1] );
      if ( ratio <= 10 )	rating = "`bmediocre`n";
      else if ( ratio < 15 )	rating = "`Bfair`n";
      else if ( ratio < 20 )	rating = "`Ggood`n";
      else if ( ratio < 25 )	rating = "`Gstrong`n";
      else if ( ratio < 30 )	rating = "`Ypowerful`n";
      else if ( ratio < 40 )	rating = "`Rincredible`n";
      else if ( ratio < 50 )	rating = "`rmaniacal`n";
      else if ( ratio < 60 )	rating = "`dUnholy`n";
      else			rating = "`RB`d.`RA`d.`RM`d.`RF`d.`n";
    }
    else
    { ratio = kills[1] * 10 / UMAX( 1, kills[0] );
      if ( ratio < 15 )		rating = "`bpoor`n";
      else if ( ratio < 20 )	rating = "`mweakling`n";
      else if ( ratio < 25 )	rating = "`mcontemptable`n";
      else if ( ratio < 30 )	rating = "`Mpathetic`n";
      else if ( ratio < 40 )	rating = "`Mwuss`n";
      else if ( ratio < 50 )	rating = "`WWoD WT`n";
      else			rating = "`MGremio`n";
    }
  }

  if ( cnum == -1 )
    courage = "`n";
  else if ( cnum < 20 )
    courage = "`Ycum dumpster`n";
  else if ( cnum < 44 )
    courage = "`Mdouchebag`n";
  else if ( cnum < 54 )
    courage = "`mwuss`n";
  else if ( cnum < 60 )
    courage = "`yyellow-belly`n";
  else if ( cnum < 66 )
    courage = "`yrookie`n";
  else if ( cnum < 72 )
    courage = "`nsoldier";
  else if ( cnum < 76 )
    courage = "`gwarrior`n";
  else if ( cnum < 80 )
    courage = "`Gadventurer`n";
  else if ( cnum < 84 )
    courage = "`rswashbuckler`n";
  else if ( cnum < 88 )
    courage = "`Rcavalier`n";
  else if ( cnum < 92 )
    courage = "`Wknight`n";
  else if ( cnum < 96 )
    courage = "`WChampion`n";
  else
    courage = "`dP`na`Wlad`ni`dn`n";

  if ( level == 20 )
    rating = "`CJesus Fuckin' Christ`n";

  sprintf( buf, "\n\r`c------------------------------------------------------------`n\n\r" );
  send_to_char( buf, ch );
  sprintf( buf, "`R%s`n %s\n\r", name, title );
  send_to_char( buf, ch );
  /* different message for me ;p */
  if ( level == 20 )
    sprintf( buf, "He is the Implementor and doesn't want status.\n\r" );
  else
    sprintf( buf, "`n%s is a %s %s.\n\r", sex, status, class );
  send_to_char( buf, ch );
  if ( clan > 0 )
  { sprintf( buf, "%s is %s %s of clan %s.\n\r",
  	sex,
  	( rank == 1 ? "the" : "a"),
  	clan_table[clan].rank[rank],
  	clan_table[clan].name );
    send_to_char( buf, ch );
  }
  if ( get_char_world( ch, arg ) != NULL )
    sprintf( buf, "%s is currently logged on.\n\r", sex );
  else
    sprintf( buf, "Last logged on %s\n\r", laston );
  send_to_char( buf, ch );
  sprintf( buf, "Created %s\n\r", created );
  stc( buf, ch );
  sprintf( buf, "                                      Assessment:\n\r" );
  send_to_char( buf, ch );
  sprintf( buf, "Mobkills: `G%5d`n  Mobdeaths: `g%4d`n      %s %s\n\r",
	kills[2], kills[3], rating, courage );
  send_to_char( buf, ch );
  sprintf( buf, "`c------------------------------------------------------------`n\n\r" );
  send_to_char( buf, ch );
  return;
}

void do_affects( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;

  if ( ch->affected != NULL )
  {
      send_to_char( "You are affected by:\n\r", ch );
      for ( paf = ch->affected; paf != NULL; paf = paf->next )
      {
        if ( paf->type == gsn_kiwall )
          continue;

 	if ( paf->duration > 59 )
 	{
 	  sprintf( buf,
            "Spell: %s modifies %s by %d for %d:%s%d.\n\r",
            skill_table[paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            (paf->duration / 60 ),
            ((paf->duration % 60) < 10 ? "0" : ""),
            (paf->duration % 60 ) );
        }
        else
        {
          sprintf( buf,
            "Spell: %s modifies %s by %d for %d seconds.\n\r",
            skill_table[paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            paf->duration );
        }

          stc( buf, ch );
      }

  }
  else
    send_to_char( "You are not affected by anything.\n\r", ch );

  return;
}


void do_leaderboards( CHAR_DATA *ch, char *argument)
{ char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i;

  if ( IS_NPC(ch) )
    return;

  sprintf( buf, "                `c-`C=`WTop Ten Warriors of Chaosium`C=`c-\n\r" );
  sprintf(buf2, "-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );
  sprintf(buf2, "`C Name             Pkills    Pdeaths    Rating         Clan`c\n\r" );
  strcat( buf, buf2 );
  sprintf(buf2, "-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );
  for ( i = 0; i < 3; i++ )
  { sprintf(buf2, " `R%-15s`n  %-8d  %-9d  %-13d  %s\n\r", leaderboard[i].name,
  	leaderboard[i].pkills,	leaderboard[i].pdeaths,
  	leaderboard[i].assessment,	leaderboard[i].clan );
    strcat( buf, buf2 );
  }
  for ( i = 3; i < 6; i++ )
  { sprintf(buf2, " `r%-15s`n  %-8d  %-9d  %-13d  %s\n\r", leaderboard[i].name,
        leaderboard[i].pkills,  leaderboard[i].pdeaths,
        leaderboard[i].assessment,      leaderboard[i].clan );
    strcat( buf, buf2 );
  }
  for ( i = 6; i < MAX_LEADERS; i++ )
  { sprintf(buf2, " `d%-15s`n  %-8d  %-9d  %-13d  %s\n\r", leaderboard[i].name,
        leaderboard[i].pkills,  leaderboard[i].pdeaths,
        leaderboard[i].assessment,      leaderboard[i].clan );
    strcat( buf, buf2 );
  }
  if ( ch->level >= 17 )
  {
    for ( i = MAX_LEADERS; i < MAX_LEADERS+5; i++ )
    { sprintf(buf2, " `n%-15s`n  %-8d  %-9d  %-13d  %s\n\r", leaderboard[i].name,
          leaderboard[i].pkills,  leaderboard[i].pdeaths,
          leaderboard[i].assessment, leaderboard[i].clan );
      strcat( buf, buf2 );
    }
  }


  sprintf(buf2, "`c-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );
  sprintf( buf2, "`C Retired leaders`n: `REuthanasia`n.\n\r" );
  strcat( buf, buf2 );
  sprintf( buf2, "`c-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );

  send_to_char( buf, ch );
  return;
}


void do_lamerboards( CHAR_DATA *ch, char *argument )
{
  do_help( ch, "lamers" );
  return;
}

void do_loserboards( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i;

  if ( IS_NPC(ch) )
    return;

  sprintf( buf, "                `m-`M=`WTop Ten Pansies of Chaosium`M=`m-`g\n\r" );
  sprintf(buf2, "-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );
  sprintf(buf2, "`G Name             Pkills    Pdeaths    Rating         Clan\n\r" );
  strcat( buf, buf2 );
  sprintf(buf2, "-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );
  for ( i = 0; i < 3; i++ )
  { sprintf(buf2, " `M%-15s`n  %-8d  %-9d  %-13d  %s\n\r", loserboard[i].name,
        loserboard[i].pkills,  loserboard[i].pdeaths,
        loserboard[i].assessment, loserboard[i].clan );
    strcat( buf, buf2 );
  }
  for ( i = 3; i < 6; i++ )
  { sprintf(buf2, " `m%-15s`n  %-8d  %-9d  %-13d  %s\n\r", loserboard[i].name,
        loserboard[i].pkills,  loserboard[i].pdeaths,
        loserboard[i].assessment, loserboard[i].clan );
    strcat( buf, buf2 );
  }
  for ( i = 6; i < 10; i++ )
  { sprintf(buf2, " `b%-15s`n  %-8d  %-9d  %-13d  %s\n\r", loserboard[i].name,
        loserboard[i].pkills,  loserboard[i].pdeaths,
        loserboard[i].assessment, loserboard[i].clan );
    strcat( buf, buf2 );
  }

  sprintf( buf2, "\n\r`nHonorable mention: `MColumbo`n\n\r" );
  strcat( buf, buf2 );
  sprintf(buf2, "`g-------------------------------------------------------------------\n\r" );
  strcat( buf, buf2 );
  sprintf( buf2, "\n\r`n" );
  strcat( buf, buf2 );

  send_to_char( buf, ch );
  return;
}

void do_spend( CHAR_DATA *ch, char *argument )
{ int i;
  bool fail = FALSE;

  if ( IS_NPC(ch) )
    return;

  if ( ch->pcdata->extras[TOKENS] <= 0 )
  { send_to_char( "Buf you don't have any remort tokens!\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "You can spend a token on the following things:\n\r", ch );
    send_to_char( "Experience          Gain five million exp.\n\r", ch );
    send_to_char( "Basicstances        Set all basic stances to 200.\n\r", ch );
    send_to_char( "Advancedstances     Set advanced stances to 200, reqs. basics.\n\r", ch );
    send_to_char( "Weapons1            Set hit, slash, stab, slice, whip to 200.\n\r", ch );
    send_to_char( "Weapons2            Set claw, blast, pound, crush to 200.\n\r", ch );
    send_to_char( "Weapons3            Set grep, bite, pierce, blow to 200.\n\r", ch );
    send_to_char( "Restore             Get a full restore.\n\r", ch );
    send_to_char( "\n\r", ch );
  }
  else if ( !str_cmp( argument, "experience" ) )
  { ch->exp += 5000000;
    ch->totalexp += 5000000;
    send_to_char( "You have been credited with 5 million exp.\n\r", ch );
    ch->pcdata->extras[TOKENS]--;
    do_save( ch, "auto" );
    return;
  }
  else if ( !str_cmp( argument, "basicstances" ) )
  { for ( i = 1; i <= 5; i++ )
      if ( ch->pcdata->stances[i] < 200 )
        ch->pcdata->stances[i] = 200;
    send_to_char( "your basic stances have been set to 200.\n\r", ch );
    ch->pcdata->extras[TOKENS]--;
    do_save( ch, "auto" );
    return;
  }
  else if ( !str_cmp( argument, "advancedstances" ) )
  { for ( i = 1; i <= 5; i++ )
      if ( ch->pcdata->stances[i] < 200 )
        fail = TRUE;
    if ( fail )
    { send_to_char( "You must have 200 in all your basic stances first.\n\r", ch );
      return;
    }
    for ( i = 6; i <= 10; i++ )
      if ( ch->pcdata->stances[i] < 200 )
        ch->pcdata->stances[i] = 200;
    send_to_char( "Your advanced stances have all been set to 200.\n\r", ch );
    ch->pcdata->extras[TOKENS]--;
    do_save( ch, "auto" );
    return;
  }
  else if ( !str_cmp( argument, "weapons1" ) )
  { for ( i = 0; i < 5; i++ )
      if ( ch->pcdata->weapons[i] < 200 )
        ch->pcdata->weapons[i] = 200;
    send_to_char( "Some of your weapon ratings have been set to 200.\n\r", ch );
    ch->pcdata->extras[TOKENS]--;
    do_save( ch, "auto" );
    return;
  }
  else if ( !str_cmp( argument, "weapons2" ) )
  { for ( i = 5; i < 9; i++ )
      if ( ch->pcdata->weapons[i] < 200 )
        ch->pcdata->weapons[i] = 200;
    send_to_char( "Some of your weapon ratings have been set to 200.\n\r", ch );
    ch->pcdata->extras[TOKENS]--;
    do_save( ch, "auto" );
    return;
  }
  else if ( !str_cmp( argument, "weapons3" ) )
  { for ( i = 9; i < 13; i++ )
      if ( ch->pcdata->weapons[i] < 200 )
        ch->pcdata->weapons[i] = 200;
    send_to_char( "Some of your weapon ratings have been set to 200.\n\r", ch );
    ch->pcdata->extras[TOKENS]--;
    do_save( ch, "auto" );
    return;
  }
  else if ( !str_cmp( argument, "restore" ) )
  {
    if ( ch->pcdata->extras[TIMER] > 0 )
    {
      stc( "No.\n\r", ch );
      return;
    }
    ch->pcdata->extras[TOKENS]--;
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    act( "You are restored!", ch, NULL, NULL, TO_CHAR );
    act( "A ghostly 'hallelujah!' rings out as $n is bathed in light.", ch, NULL, NULL, TO_ROOM );
    return;
  }
  else
  { send_to_char( "You can spend a token on the following things:\n\r", ch );
    send_to_char( "Experience          Gain five million exp.\n\r", ch );
    send_to_char( "Basicstances        Set all basic stances to 200.\n\r", ch );
    send_to_char( "Advancedstances     Set advanced stances to 200, reqs. basics.\n\r", ch );
    send_to_char( "\n\r", ch );
  }

  return;
}

void do_dice( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int dies, die, roll;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  { send_to_char( "Dice requires two positive interger arguments less than 100.\n\r", ch );
    return;
  }

  if ( !is_number( arg1 ) || !is_number( arg2 ) )
  { send_to_char( "Dice requires two positive integer arguments less than 100.\n\r", ch );
    return;
  }

  dies = atoi( arg1 );
  die  = atoi( arg2 );

  if ( dies <= 0 || dies > 100 || die <= 0 || die > 100 )
  { send_to_char( "Dice requires two positive integer arguments less than 100.\n\r", ch );
    return;
  }

  roll = dice( dies, die );

  sprintf( buf, "Feeling lucky, you roll %d %d-sided %s.  The total is %d!",
	dies, die, ( dies == 1 ? "die" : "dice" ), roll );
  act( buf, ch, NULL, NULL, TO_CHAR );
  sprintf( buf, "Feeling lucky, $n rolls %d %d-sided %s.  The total is %d!",
        dies, die, ( dies == 1 ? "die" : "dice" ), roll );
  act( buf, ch, NULL, NULL, TO_ROOM );

  return;
}

int assess( CHAR_DATA *ch )
{ float ratio, courage;
  int i, total = 0, diff;
  int pkills = ch->pcdata->kills[PK];
  int pdeaths = ch->pcdata->kills[PD];
  int unholy = ch->pcdata->kills[UNHOLY];

  if ( pkills == 0 )
    pkills = 1;
  if ( pdeaths == 0 )
    pdeaths = 1;

  if ( pkills >= pdeaths )
    ratio = ((float) pkills) / ((float) pdeaths);
  else
    ratio = ((float) pdeaths) / ((float) pkills);

  /* add up all recent kills */
  for ( i = 4; i <= 8; i++ )
  {
    total += ch->pcdata->kills[i];
  }

  courage = ((float)unholy) / ((float)total);
  courage = UMIN( courage * (courage+1), 1.4 );

  diff = pkills - pdeaths;
  if ( diff > 0 )
    return (int) ( diff * (1 + (ratio-1)/3.0) * courage );
  else
    return (int) (diff * (1 + (ratio-1)/3.0));
}

void do_vote( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int cv;

  if ( IS_NPC(ch) )
    return;

  if ( !voting->active )
  { send_to_char( "There isn't a vote being held right now.\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_VOTED) )
  { send_to_char( "You have already voted.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  { send_to_char( "Vote for what?\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg, "info" ) )
  {
    return;
  }
  else if ( !is_number( arg ) )
  { send_to_char( "Vote requires an integer argument.\n\r", ch );
    return;
  }

  cv = atoi( arg );
  if ( cv <= 0 || cv > voting->num )
  { sprintf( buf, "Acceptable votes are between 1 and %d.\n\r", voting->num );
    send_to_char( buf, ch );
    return;
  }

  SET_BIT(ch->pcdata->actnew,NEW_VOTED);
  voting->votes[cv]++;
  sprintf( buf, "You voted for number %d.\n\r", cv );
  send_to_char( buf, ch );
  return;
}

void do_uniques( CHAR_DATA *ch, char *argument )
{ char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found = FALSE;

  for ( obj = object_list; obj != NULL; obj = obj->next )
  {
    if ( !IS_OBJ_STAT(obj,ITEM_UNIQUE) )
      continue;
    found = TRUE;

    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
      ;

        if ( in_obj->carried_by != NULL )
        {
            sprintf( buf, "%s carried by %s.\n\r",
                obj->short_descr, PERS(in_obj->carried_by, ch) );
        }
        else
        {
            sprintf( buf, "%s in %s.\n\r",
                obj->short_descr, in_obj->in_room == NULL
                    ? "somewhere" : in_obj->in_room->name );
        }
        buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
  }
  if ( !found )
    send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

  return;
}

void do_mlist( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *master;

  if ( IS_NPC(ch) )
    return;
  if ( !ch->pcdata->mmaster || ch->pcdata->mmaster[0] == '\0' )
  { send_to_char( "You aren't in a minion.\n\r", ch );
    return;
  }

  master = ch->pcdata->mmaster;
  argument = one_argument( argument, arg );
  if ( arg[0] != '\0' && ch->level == MAX_LEVEL )
  {
    master = arg;
  }

  sprintf( buf, "The following are in %s's minion:\n\r", master );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next;
    if ( IS_NPC(vch) )
      continue;
    if ( !str_cmp( master, vch->pcdata->mmaster ) )
    {
      strcat( buf, vch->name );
      strcat( buf, "\n\r" );
    }
  }

  send_to_char( buf, ch );
  return;
}

void do_contracts( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  char buf[MAX_STRING_LENGTH];

  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
    if ( IS_NPC(vch) )
      continue;
    if ( vch->pcdata->extras2[BOUNTY] <= 0 )
      continue;

    if ( vch == ch )
      sprintf( buf, "%8d gold on YOUR head!\n\r", vch->pcdata->extras2[BOUNTY] );
    else
      sprintf( buf, "%8d gold on %s's head.\n\r", vch->pcdata->extras2[BOUNTY], vch->name );

    stc( buf, ch );
  }
  return;
}

void do_legend( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *point;
  int counter = 0, i;

  if ( IS_NPC(ch) )
    return;

  if ( !IS_SET(ch->pcdata->actnew,NEW_LEGEND) && ch->level < 20 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < 250 )
  { send_to_char( "You need 250 primal to set your Legend.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char( "Set your Legend to what?\n\r", ch );
    return;
  }

  if ( strlen( argument ) > 27 )
  {
     stc( "Too long.\n\r", ch );
     return;
  }
  point = argument;
  while ( *point != '\0' )
  {
    if ( *point == '`' && *(point+1) != '`' && *(point+1) != '\0' )
    {
      point += 2;
    }
    else if ( *point == '`' && *(point+1) == '\0' )
    {
      point++;
      counter++;
    }
    else
    {
      counter++;
      point++;
    }
  }
  if ( counter < 2 )
  {
    stc( "Legend must be at least two characters in length.\n\r", ch );
    return;
  }
  if ( counter > 9 )
  {
    stc( "Legend may not be mot be over nine characters in length.\n\r", ch );
    return;
  }
  sprintf( arg, "%s", argument );
  for ( i = counter; i < 9; i++ )
  {
    strcat( arg, " " );
  }

  sprintf( buf, "Your new legend is '%s'\n\r", arg );
  send_to_char( buf, ch );
  free_string( ch->pcdata->legend );
  ch->pcdata->legend = str_dup( arg );
  ch->pcdata->primal -= 250;
  return;
}

void do_cflag( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SET(ch->pcdata->actnew,NEW_RETIRED) && ch->level < LEVEL_IMMORTAL )
  {
    stc( "Huh?\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    stc( "Syntax: cflag <5 letter clan sigil>\n\r", ch );
    return;
  }

  colorize( buf, argument, FALSE );
  smash_tilde( buf );

  if ( strlen( buf ) > 5 )
  {
    stc( "Too long.\n\r", ch );
    return;
  }

  free_string( ch->pcdata->cflag );
  ch->pcdata->cflag = str_dup( argument );
  sprintf( buf, "Your cflag is now '%s'.\n\r", argument );
  stc( buf, ch );
  return;
}

void do_pkstat( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  PKILL_DATA *pkill;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char area_name[MAX_INPUT_LENGTH];
  char pkill_time[MAX_INPUT_LENGTH];
  time_t pkt;

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
  {
    victim = ch;
  }
  else
  {
    victim = get_char_world( ch, arg );
    if ( victim == NULL )
    {
      stc( "You can't find them.\n\r", ch );
      return;
    }
    if ( IS_NPC(victim) )
    {
      stc( "Mobiles do't have pk histories.\n\r", ch );
      return;
    }
  }

  if ( victim->pcdata->pkills == NULL )
  {
    stc( "They have no recorded kills.\n\r", ch );
    return;
  }

  for ( pkill = victim->pcdata->pkills; pkill != NULL; pkill = pkill->next )
  {
    if ( get_room_index( pkill->vnum ) == NULL )
      sprintf( area_name, "Unknown" );
    else
      sprintf( area_name, "%s", ((get_room_index(pkill->vnum))->area->name+16) );

    pkt = (time_t) pkill->time;
    sprintf( pkill_time, "%s", (char*) ctime( &pkt ) );
    pkill_time[24] = '\0'; // no nextline.

    sprintf( buf, "%s  %12s  Inflicted %-6d  Lost %-5d %s%s\n\r",
	pkill_time,
	pkill->victim,
	pkill->inf,
	pkill->lost,
	(pkill->humil ? " (humiliated) " : " "),
	(pkill->suit ? "(suit cap)" : "" ) );

    stc( buf, ch );
  }

  return;
}
     
     


