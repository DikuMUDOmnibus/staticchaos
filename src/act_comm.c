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


/*
 * Local functions.
 */
void	talk_channel	args( ( CHAR_DATA *ch, char *argument,
			    int channel, const char *verb ) );
void	check_activation args( ( CHAR_DATA *ch, char *argument ) );


/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
    char buf[MAX_STRING_LENGTH];
    char cbuf[10];
    DESCRIPTOR_DATA *d;
    int position;

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "%s what?\n\r", verb );
	buf[0] = UPPER(buf[0]);
	return;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) 
      && channel != CHANNEL_INFO && channel != CHANNEL_FATALITY)
    {
	sprintf( buf, "You can't %s.\n\r", verb );
	send_to_char( buf, ch );
	return;
    }

    if ( !IS_NPC(ch) && channel != CHANNEL_INFO && channel != CHANNEL_FATALITY )
    {
      if ( ch->pcdata->extras2[GLOBALS] >= 0 )
        ch->pcdata->extras2[GLOBALS]++;
      else
        ch->pcdata->extras2[GLOBALS] -= 20;

      if ( ch->pcdata->extras2[GLOBALS] >= 10 )
        ch->pcdata->extras2[GLOBALS] = -150;

      if ( ch->pcdata->extras2[GLOBALS] < 0 )
      {
        stc( "Silence, spammer.\n\r", ch );
        ch->pcdata->extras[TIMER] = 300;
        return;
      }
    }

    REMOVE_BIT(ch->deaf, channel);

    switch ( channel )
    {
      case CHANNEL_CHAT:	sprintf( cbuf, "`G" );	break;
      case CHANNEL_MUSIC:	sprintf( cbuf, "`Y" );	break;
      case CHANNEL_QUESTION:	sprintf( cbuf, "`y" );	break;
      case CHANNEL_YELL:	sprintf( cbuf, "`r" );	break;
      default:			sprintf( cbuf, "`n" );	break;
    }

    switch ( channel )
    {
    default:
	sprintf( buf, "You %s '%s%s`n'\n\r", verb, cbuf, argument );
	send_to_char( buf, ch );
	sprintf( buf, "$n %ss '%s$t`n'", verb, cbuf );
	break;

    case CHANNEL_IMMTALK:
	sprintf( buf, "$n: `W$t`n" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act( buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;

    case CHANNEL_INFO:
    case CHANNEL_FATALITY:
	sprintf( buf, "%s %s\n\r", verb, argument );
	send_to_char( buf, ch );
	sprintf( buf, "%s %s", verb, argument );
	break;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	&&   vch != ch
	&&  !IS_SET(och->deaf, channel) )
	{
	    if ( channel == CHANNEL_IMMTALK
	    && !IS_HERO(och)
	    && !( !IS_NPC(och) && IS_SET(och->act,PLR_IMMTALK) ))
		continue;
	    if ( channel == CHANNEL_YELL
	    &&   vch->in_room->area != ch->in_room->area )
		continue;

	    position		= vch->position;
	    if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
		vch->position	= POS_STANDING;
	    if ( !IS_NPC(ch) && IS_SET(ch->pcdata->actnew,NEW_TARD) )
	    {
	      switch( dice(1,5) )
	      { 
	        case 1:
	          act( buf, ch, "Pikachu.", vch, TO_VICT );
	          break;
	        case 2:
	          act( buf, ch, "Chuuuuu...", vch, TO_VICT );
	          break;
	        case 3:
	          act( buf, ch, "Pika.", vch, TO_VICT );
	          break;
	        case 4:
	          act( buf, ch, "Pika Pi!", vch, TO_VICT );
	          break;
	        case 5:
	          act( buf, ch, "Pika Pika!", vch, TO_VICT );
	          break;
	        default:
	          act( buf, ch, "Pikachuu.... pika?", vch, TO_VICT );
	          break;
	      };
	    }
	    else
	      act( buf, ch, argument, vch, TO_VICT );
	    vch->position	= position;
	}
    }

    return;
}



/* removed for new version 
void do_auction( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_AUCTION, "auction" );
    return;
}
*/



void do_chat( CHAR_DATA *ch, char *argument )
{
    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
    {
      stc( "Better concentrate on the matter at hand.\n\r", ch );
      return;
    }
    if ( IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
    }
    talk_channel( ch, argument, CHANNEL_CHAT, "chat" );
    return;
}



/*
 * Alander's new channels.
 */
void do_music( CHAR_DATA *ch, char *argument )
{
    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
    {
      stc( "Better concentrate on the matter at hand.\n\r", ch );
      return;
    }
    if ( IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
    }
    talk_channel( ch, argument, CHANNEL_MUSIC, "music" );
    return;
}



void do_question( CHAR_DATA *ch, char *argument )
{
    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
    {
      stc( "Better concentrate on the matter at hand.\n\r", ch );
      return;
    }
    if ( IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
    }
    talk_channel( ch, argument, CHANNEL_QUESTION, "question" );
    return;
}



void do_answer( CHAR_DATA *ch, char *argument )
{
    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
    {
      stc( "Better concentrate on the matter at hand.\n\r", ch );
      return;
    }
    if ( IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
    }
    talk_channel( ch, argument, CHANNEL_QUESTION, "answer" );
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_SHOUT, "shout" );
    WAIT_STATE( ch, 12 );
    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
     }
    talk_channel( ch, argument, CHANNEL_YELL, "yell" );
    check_activation( ch, argument );
    return;
}



void do_immtalk( CHAR_DATA *ch, char *argument )
{
    if ( ch->level < 15 && !IS_NPC(ch) && !IS_SET(ch->act,PLR_IMMTALK) )
    {
      stc( "Huh?\n\r", ch );
      return;
    }
    talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
    return;
}

void do_info( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_INFO, "`YInfo ->`n" );
    return;
}

void do_fatality( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_FATALITY, "`RFatality ->`n" );
    return;
}


void do_say( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }
    if ( !IS_NPC(ch) && IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
    }

    act( "$n says '`m$T`n'", ch, NULL, argument, TO_ROOM );
    act( "You say '`m$T`n'", ch, NULL, argument, TO_CHAR );
    mprog_speech_trigger(argument,ch);
    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int position;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }
    if ( !IS_NPC(ch) && IS_SET(ch->act,PLR_WIZINVIS) && ch->level < MAX_LEVEL )
    {
      stc( "You may not use public channels while wizinvis.\n\r", ch );
      return;
    }

    if ( !IS_NPC(ch) )
    {
      if ( ch->pcdata->extras2[GLOBALS] >= 0 )
        ch->pcdata->extras2[GLOBALS]++;
      else
        ch->pcdata->extras2[GLOBALS] -= 20;
      if ( ch->pcdata->extras2[GLOBALS] >= 10 )
        ch->pcdata->extras2[GLOBALS] = -150;
      if ( ch->pcdata->extras2[GLOBALS] < 0 )
      {
        stc( "Silence, spammer.\n\r", ch );
        return;
      }
    }

    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
    {
      stc( "Better concentrate on the matter at hand.\n\r", ch );
      return;
    }
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) && (victim->position < POS_MORTAL) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_SET(victim->deaf,CHANNEL_TELL) && !IS_IMMORTAL(ch) )
    { send_to_char( "They aren't listening.\n\r", ch );
      return;
    }

    act( "You tell $N '`M$t`n'", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    act( "$n tells you '`M$t`n'", ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int position;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }
    if ( !IS_NPC(ch) )
    {
      if ( ch->pcdata->extras2[GLOBALS] >= 0 )
        ch->pcdata->extras2[GLOBALS]++;
      else
        ch->pcdata->extras2[GLOBALS] -= 20;
      if ( ch->pcdata->extras2[GLOBALS] >= 10 )
        ch->pcdata->extras2[GLOBALS] = -150;
      if ( ch->pcdata->extras2[GLOBALS] < 0 )
      {
        stc( "Silence, spammer.\n\r", ch );
        return;
      }
    }


    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
    {
      stc( "Better concentrate on the matter at hand.\n\r", ch );
      return;
    }
    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) && (victim->position < POS_MORTAL) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    act( "You tell $N '`M$t`n'", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    act( "$n tells you '`M$t`n'", ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    for ( plast = argument; *plast != '\0'; plast++ )
	;

    strcpy( buf, argument );
    if ( isalpha(plast[-1]) )
	strcat( buf, "." );

    act( "`B$n $T`n", ch, NULL, buf, TO_ROOM );
    act( "`B$n $T`n", ch, NULL, buf, TO_CHAR );
    return;
}



/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god Mota gives you a staff.",
	    "The great god Mota gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0],
	ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1],
	ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}



void do_idea( CHAR_DATA *ch, char *argument )
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    int helpnum;

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
    {
        send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
        return;
    }

    if ( IS_SUIT(ch) && ch->pcdata->suit[SUIT_NUMBER] > 4 )
    {
      stc( "You may not quit out in a Gundam.\n\r", ch );
      return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->extras[TIMER] > 0 )
    { send_to_char( "Not with a fight timer.\n\r", ch );
      return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_HQ) )
    { send_to_char( "You may not quit out in a headquarters.\n\r", ch );
      return;
    }
    if ( IS_SET(ch->in_room->room_flags,ROOM_PRIVATE) )
    {
      stc( "You may not quit out in private room.\n\r", ch );
      return;
    }

    /* no longer necessary */
    if ( current_time < ch->pcdata->extras2[UNIQUE_TIMER] )
    {
      sprintf( buf, "Stripping %s of uniques on logout.", ch->name );
      log_string( buf );
      for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( IS_OBJ_STAT(obj,ITEM_UNIQUE) )
        {
          obj_from_char( obj );
          obj_to_room( obj, get_rand_room() );
        }
      }
    }

    if ( ch->level < LEVEL_HERO )
    {
      for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( IS_OBJ_STAT(obj,ITEM_UNIQUE) )
        {
          obj_from_char( obj );
          obj_to_room( obj, get_rand_room() );
        }
      }
    }

    if ( ch->pcdata->in_progress )
      free_note (ch->pcdata->in_progress );

    sprintf( buf, "%s %s", ch->name, ch->pcdata->bamfout );
    do_info( ch, buf );

    send_to_char( "\n\r", ch );
    helpnum = dice(1,MAX_LOGOUTS);
    sprintf( buf, "EXITMESSAGE%d%d", helpnum/10, helpnum%10 );
    do_help( ch, buf );

    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_fin( ch );
    save_char_obj( ch );
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( ch->level < 1 )
    {
	send_to_char( "You must be at least first level to save.\n\r", ch );
	return;
    }

    save_char_obj( ch );
    save_char_fin( ch );
    if ( str_cmp( argument, "auto" ) )
      send_to_char( "Ok.\n\r", ch );
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!",
	    ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( !IS_NPC(victim) && victim != ch && IS_SET(victim->act,PLR_NOFOLLOW) )
    {
      stc( "They are not interested in your company.\n\r", ch );
      return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower( ch );
	return;
    }
    
    /* more stupid level shit 
    if ( ( ch->level - victim->level < -5 || ch->level - victim->level >  5 )
    &&   !IS_HERO(ch) )
    {
	send_to_char( "You are not of the right caliber to follow.\n\r", ch );
	return;
    }
    */

    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
  
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{

    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) )
	act( "$n stops following you.",
	    ch, NULL, ch->master, TO_VICT );
    act( "You stop following $N.",
	ch, NULL, ch->master, TO_CHAR );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
	stop_follower( ch );

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    act( "$n orders you to '$t'.", ch, argument, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
	send_to_char( "Ok.\n\r", ch );
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"%-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
      for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
      {
        if ( victim->master != ch )
          continue;
        if ( is_same_group( victim, ch ) )
          continue;

        victim->leader = ch;
        act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
        act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
        act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
      }
      return;
    }


    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.", ch, NULL, victim, TO_VICT   );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR  );
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->act, PLR_NO_TELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "%s tells the group '`B%s`n'.\n\r", ch->name, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	{
	    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
	    {
	      if ( ch->in_room == gch->in_room )
	        send_to_char( buf, gch );
	    }
	    else
	      send_to_char( buf, gch );
	}

    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}


void do_ansi( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;

  if ( ch->pcdata->ansi )
  {
    ch->pcdata->ansi = 0;
    send_to_char( "Ansi color is now off.\n\r", ch );
  }
  else
  {
    ch->pcdata->ansi = 1;
    send_to_char( "Ansi color is now on.\n\r", ch );
  }
  return;
}

void check_activation( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *wield;
  CHAR_DATA *victim;
  int dam;

  if ( IS_NPC(ch) )
    return;

  if ( IS_SUIT(ch) )
    return;

  /* now, check for special phrases for uni's */
  if ( !str_cmp( argument, "Demon Sword!" ) )
  { if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
      return;
    if ( wield->pIndexData->vnum != 30 )
      return;
    if ( ch->mana < 500 || ch->move < 500 )
    { send_to_char( "You are too exhausted to use the Murasame's powers.\n\r", ch );
      return;
    }
    if ( ( victim = ch->fighting ) == NULL )
    { send_to_char( "Demon Sword who?\n\r", ch );
      return;
    }
    dam = dice( ch->pcdata->will *3/4, ch->pcdata->spirit );
    act( "A wave of energy erupts from the Murasame, racing toward $N!", ch, NULL, victim, TO_CHAR );
    act( "A wave of energy erupts from the Murasame, racing toward $N!", ch, NULL, victim, TO_ROOM );
    damage( ch, victim, dam, DAM_SHOCKWAVE );
    ch->mana -= 500;
    ch->move -= 500;
    WAIT_STATE( ch, 12 );
    return;
  }
  else if ( !str_cmp( argument, "Demon Wave Motion Sword!" ) )
  { if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
      return;
    if ( wield->pIndexData->vnum != 30 )
      return;
    if ( ch->mana < 500 || ch->move < 500 )
    { send_to_char( "You are too exhausted to use the Murasame's powers.\n\r", ch );
      return;
    }
    if ( ( victim = ch->fighting ) == NULL )
    { send_to_char( "You aren't fighting anyone\n\r", ch );
      return;
    }
    dam = dice( ch->pcdata->will/2, ch->pcdata->spirit );
    act( "A wave of energy erupts from the Murasame, racing toward $N!", ch, NULL, victim, TO_CHAR );
    act( "A wave of energy erupts from the Murasame, racing toward $N!", ch, NULL, victim, TO_ROOM );
    damage( ch, victim, dam, DAM_SHOCKWAVE );
    if ( victim != NULL && !IS_NPC(victim) )
    { if ( number_percent() > (victim->pcdata->will/2) )
      { if ( victim->class == CLASS_FIST )
        { victim->pcdata->powers[F_KI] = UMAX( 0, victim->pcdata->powers[F_KI] - dice( 1, 4 )
  );
          if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
            REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
          WAIT_STATE( victim, 4 );
          act( "Your shockwave staggers $N!", ch, NULL, victim, TO_CHAR );
          act( "$n's shockwave breaks your rhythm!", ch, NULL, victim, TO_VICT );
          act( "$n's shockwave staggers $N.", ch, NULL, victim, TO_NOTVICT );
        }
        else if ( victim->class == CLASS_SAIYAN )
        { if ( is_affected(victim,gsn_kiwall) )
          { affect_strip(victim,gsn_kiwall);
            act( "Your shockwave staggers $N!", ch, NULL, victim, TO_CHAR );
            act( "$n's shockwave shatters your wall of Ki!", ch, NULL, victim, TO_VICT );
            act( "$n's shockwave staggers $N.", ch, NULL, victim, TO_NOTVICT );
          }
        }
      }
    }
    ch->mana -= 500;
    ch->move -= 500;
    WAIT_STATE( ch, 12 );
    return;
  }


  else if ( !str_cmp( argument, "Uranus Planet Power Makeup!" ) )
  { if ( ( wield = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
      return;
    if ( wield->pIndexData->vnum != 31 )
      return;
    if ( ch->mana < 1000 )
    { send_to_char( "You are too exhausted to use the locket's powers.\n\r", ch );
      return;
    }

    act( "Energy swirls around you, in the form of sparklies.", ch, NULL, NULL, TO_CHAR );
    act( "Energy swirls around $n, in the form of sparklies.", ch, NULL, NULL, TO_ROOM );
    act( "Your clothes vanish as you begin spinning rapidly.", ch, NULL, NULL, TO_CHAR );
    act( "$n's clothes vanish as $e begins spinning rapidly.", ch, NULL, NULL, TO_ROOM );
    act( "Sparklies coalesce into a Sailor Fuku and suit!", ch, NULL, NULL, TO_CHAR );
    act( "Sparklies coalesce into a Sailor Fuku and suit!", ch, NULL, NULL, TO_ROOM );
    act( "You power-pose with your pen held high!", ch, NULL, NULL, TO_CHAR );
    act( "$n power-poses with $s pen held high!", ch, NULL, NULL, TO_ROOM );
    act( "You say '`mMy name is Sailor Uranus!`n'", ch, NULL, NULL, TO_CHAR );
    act( "$n says '`mMy name is Sailor Uranus!`n'", ch, NULL, NULL, TO_ROOM );
    act( "You say '`mIn the name of Love and Justice, I will punish you!`n'", ch, NULL, NULL, TO_CHAR );
    act( "$n says '`mIn the name of Love and Justice, I will punish you!`n'", ch, NULL, NULL, TO_ROOM );
    if ( !is_affected( ch, skill_lookup( "henshin" ) ) )
    { AFFECT_DATA af;
      af.type      = skill_lookup( "henshin" );
      af.duration  = 300;
      af.bitvector = AFF_HENSHIN;
      af.location  = APPLY_HITROLL;
      af.modifier  = 5;
      affect_to_char( ch, &af );
      af.location  = APPLY_DAMROLL;
      affect_to_char( ch, &af );
      if ( ch->sex != SEX_FEMALE )
      { af.location	= APPLY_SEX;
        af.bitvector	= skill_lookup( "change sex" );
        af.modifier	= SEX_FEMALE;
        send_to_char( ".. you feel different.\n\r", ch );
      }
    }
    WAIT_STATE(ch,16);
    return;
  }

  return;
}

void do_mtalk( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( !ch->pcdata->mname || !str_cmp( ch->pcdata->mname, "(null)" ) )
  { send_to_char( "You aren't in a minion.\n\r", ch );
    return;
  }
  if ( !ch->pcdata->mmaster || ch->pcdata->mmaster == '\0' )
  { send_to_char( "You aren't in a minion.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "Say what?\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
  {
    stc( "Better concentrate on the matter at hand.\n\r", ch );
    return;
  }
  sprintf( buf, "[%s] %s: `c%s`n\n\r", ch->pcdata->mname, ch->name, argument );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  { vch_next        = vch->next;
    if ( IS_NPC(vch) )
      continue;
    if ( vch->in_room == NULL )
      continue;
    if ( !str_cmp( ch->pcdata->mmaster, vch->pcdata->mmaster ) || vch->level >= 20 )
      send_to_char( buf, vch );
  }

  return;
}

void do_mname( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  if ( IS_NPC(ch) )
    return;
  if ( str_cmp( ch->name, ch->pcdata->mmaster ) )
  { send_to_char( "But you are not the master of this minion.\n\r", ch );
    return;
  }
  if ( strlen( argument ) > 30 )
  { send_to_char( "Name too long.\n\r", ch );
    return;
  }
  smash_tilde( argument );
  ch->pcdata->mname = str_dup( argument );
  sprintf( buf, "Your minion is now named %s.\n\r", argument );
  send_to_char( buf, ch );
  return;
}

void do_mjoin( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  if ( !str_cmp( argument, "self" ) )
  { send_to_char( "You are now the master of your own minion.\n\r", ch );
    free_string( ch->pcdata->mmaster );
    free_string( ch->pcdata->mname );
    ch->pcdata->mmaster = str_dup( ch->name );
    ch->pcdata->mname = str_dup( "none" );
    return;
  }
  if ( str_cmp( ch->pcdata->mmaster, ch->name ) )
  { send_to_char( "But you aren't the master of a minion.\n\r", ch );
    return;
  }
  if ( !ch->pcdata->mname || ch->pcdata->mname == '\0' )
  { send_to_char( "You should probably name your minion first.\n\r", ch );
    return;
  }
  if ( (victim = get_char_world( ch, argument ) ) == NULL )
  { send_to_char( "Join who?\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Are you so desperate for members that you'll take in mobs?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_TARD) )
  {
    stc( "Tards can't run minions, go play with a box of chocolates.\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < 50 )
  {
    stc( "50 primal is required to join a member.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= 50;
  free_string( victim->pcdata->temp );
  victim->pcdata->temp = str_dup( ch->name );
  sprintf( buf, "%s has offered you a place in the %s minion.\n\r", ch->name, ch->pcdata->mname );
  send_to_char( buf, victim );
  sprintf( buf, "Type 'maccept %s' to join.\n\r", ch->name );
  send_to_char( buf, victim );
  sprintf( buf, "You offer %s a place in the %s minion.\n\r", victim->name, ch->pcdata->mname);
  send_to_char( buf, ch );
  return;
}

void do_maccept( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *master;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  { send_to_char( "Accept whose minion offer?\n\r", ch );
    return;
  }
  if ( str_cmp( ch->pcdata->temp, arg ) )
  { send_to_char( "But they haven't offered you a place in a minion.\n\r", ch );
    return;
  }
  if ( !ch->pcdata->temp || ch->pcdata->temp == '\0' )
  { send_to_char( "But you haven't been offered membership to a minion.\n\r", ch );
    return;
  }
  if ( ( master = get_char_world( ch, ch->pcdata->temp ) ) == NULL )
  { send_to_char( "Too late, they left the game.\n\r", ch );
    free_string( ch->pcdata->temp );
    ch->pcdata->temp = str_dup( "" );
    return;
  }
  if ( str_cmp( master->name, master->pcdata->mmaster ) )
  { send_to_char( "So sorry, they no longer lead a minion.\n\r", ch );
    free_string( ch->pcdata->temp );
    ch->pcdata->temp = str_dup( "" );
    return;
  }

  sprintf( buf, "%s has accepted your offere and joined %s.\n\r", ch->name, master->pcdata->mname );
  send_to_char( buf, master );
  sprintf( buf, "You inform %s that you accept their offer to join %s.\n\r", master->name, master->pcdata->mname );
  send_to_char( buf, ch );
  ch->pcdata->mmaster = str_dup( master->name );
  ch->pcdata->mname = str_dup( master->pcdata->mname );
  return;
}

void do_mbanish( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( str_cmp( ch->name, ch->pcdata->mmaster ) )
  { send_to_char( "But you are not the master of this minion.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Banish who?\n\r", ch );
    return;
  }
  if ( (victim = get_char_world( ch, argument ) ) == NULL )
  { send_to_char( "Banish who?\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Uhh.. that's a mob.\n\r", ch );
    return;
  }
  if ( victim->pcdata->mmaster == NULL || str_cmp( ch->name, victim->pcdata->mmaster ) )
  { send_to_char( "You are not their master.\n\r", ch );
    return;
  }

  sprintf( buf, "%s has been banished.\n\r", victim->name );
  send_to_char( buf, ch );
  sprintf( buf, "%s has banished you from %s.\n\r", ch->name, ch->pcdata->mname );
  send_to_char( buf, victim );
  free_string( victim->pcdata->mname );
  free_string( victim->pcdata->mmaster );
  victim->pcdata->mname = str_dup( "none" );
  victim->pcdata->mmaster = str_dup( victim->name );
  do_save( victim, "auto" );
  return;
}

/* FILE: act_comm.c */

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"`cAuction ->`n %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->deaf,CHANNEL_AUCTION) )
            act (buf, original, NULL, NULL, TO_CHAR);

    }
}

void do_suicide( CHAR_DATA *ch, char *argument )
{
  char strsave[MAX_INPUT_LENGTH];
  char strfin[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );
  sprintf( name, "%s", ch->name );

  if ( IS_NPC(ch) )
  {
    stc( "Just die, that'll work.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[TIMER] > 0 )
  {
    stc( "Somebody else has dibs on your life.\n\r", ch );
    return;
  }
  if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
  {
    send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
    return;
  }
  if ( IS_SET(ch->in_room->room_flags, ROOM_HQ) )
  { send_to_char( "You may not kill yourself in a headquarters.\n\r", ch );
    return;
  } 
  if ( ch->pcdata->extras2[EVAL] >= 30 )
  {
    stc( "Your will to live is too strong to overcome.\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->actnew,NEW_SUICIDE) )
  {
    stc( "Enter suicide <password> to do the deed.\n\r", ch );
    stc( "Enter suicide followed by anything else to call it off.\n\r", ch );
    stc( "Remember.. suicide is final, no take backs, just oblivion.\n\r", ch );
    WAIT_STATE( ch, 20 );
    SET_BIT(ch->pcdata->actnew,NEW_SUICIDE);
    sprintf( buf, "%s is planning to commit suicide.", ch->name );
    log_string( buf );
    return;
  }

  if ( str_cmp( arg, ch->pcdata->pwd ) )
  {
    stc( "You decide to spare your own life.\n\r", ch );
    REMOVE_BIT(ch->pcdata->actnew,NEW_SUICIDE);
    sprintf( buf, "%s cancelled suicide, argument '%s'.\n\r", ch->name, arg );
    log_string( buf );
    return;
  }


  sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ), "/", capitalize( name ) ); 
  sprintf( strfin, "%s%s", "../finger/", capitalize( name ) );
  sprintf( buf, "%s has committed suicide.", ch->name );
  log_string( buf );
  stc( "Goodbye, cruel world.\n\r\n\r", ch );
  stc( ".\n\r..\n\r.....\n\r................\n\r\n\r", ch );
  do_quit( ch, "auto" );

  sprintf( buf, "rm -f %s", strsave );
  system( buf );
  sprintf( buf, "rm -f %s", strfin );
  system( buf );

  return;
}

void wiznet( char *argument )
{
  CHAR_DATA *vch;
  char timebuf[30];
  char buf[MAX_STRING_LENGTH];

  sprintf( timebuf, "%s", ctime( &current_time ) );
  timebuf[19] = '\0';

  sprintf( buf, "`c[%s] %s`n\n\r", timebuf+11, argument );
  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
    if ( IS_NPC(vch) )
      continue;
    if ( vch->level < MAX_LEVEL )
      continue;
    if ( IS_SET(vch->deaf,CHANNEL_WIZNET) )
      continue;
    if ( vch->level == MAX_LEVEL )
      stc( buf, vch );
  }
  return;
}

