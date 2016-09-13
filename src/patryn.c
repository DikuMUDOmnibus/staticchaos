/* Original by Alathon */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

void	add_rune	args( ( CHAR_DATA *ch, int type, int part ) );
void	remove_rune	args( ( CHAR_DATA *ch, int type, int part ) );
void	runecast	args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *arg ) );

const	int	runemax[5] = { 15, 5, 5, 8, 8 };

struct cost_type
{
  int spell;
  int cost;
  int target;
  int wait;
};

const	struct	cost_type	cost_table	[]	=
{
  {	65,	300,	TAR_CHAR_OFFENSIVE,	9	},
  {	66,	300,	TAR_CHAR_OFFENSIVE,	9	},
  {	68,	300,	TAR_CHAR_OFFENSIVE,	9	},
  {	72,	300,	TAR_CHAR_OFFENSIVE,	9	},
  {	80,	750,	TAR_CHAR_DEFENSIVE,	10	},
  {	96,	400,	TAR_CHAR_OFFENSIVE,	10	},
  {	129,	500,	TAR_CHAR_OFFENSIVE,	10	},
  {	132,	500,	TAR_CHAR_OFFENSIVE,	10	},
  {	264,	250,	TAR_CHAR_DEFENSIVE,	10	},
  {	257,	750,	TAR_CHAR_DEFENSIVE,	10	},
  {	272,	100,	TAR_CHAR_DEFENSIVE,	10	},
  {	513,	750,	TAR_CHAR_OFFENSIVE,	9	},
  {	516,	400,	TAR_IGNORE,		10	},
  {	528,	600,	TAR_CHAR_OFFENSIVE,	10	},
  {	544,	500,	TAR_CHAR_OFFENSIVE,	10	},
  {	1025,	200,	TAR_CHAR_SELF,		10	},
  {	1026,	250,	TAR_CHAR_SELF,		10	},
  {	1028,	750,	TAR_CHAR_SELF,		8	},
  {	1032,	100,	TAR_CHAR_SELF,		10	},
  {	1056,	250,	TAR_CHAR_DEFENSIVE,	10	},
  {	2049,	350,	TAR_CHAR_DEFENSIVE,	10	},
  {	2052,	350,	TAR_CHAR_DEFENSIVE,	10	},
  {	2064,	400,	TAR_CHAR_SELF,		10	},
  {	2080,	100,	TAR_CHAR_OFFENSIVE,	10	},
  {	4097,	100,	TAR_CHAR_DEFENSIVE,	10	},
  {	4128,	400,	TAR_CHAR_OFFENSIVE,	8	},
  {	16385,	750,	TAR_CHAR_SELF,		8	},
  {     16386,  750,    TAR_CHAR_SELF,		8	},
  {     16388,  750,    TAR_CHAR_SELF,          8	},
  {     16392,  750,    TAR_CHAR_SELF,          8	},
  {     16400,  750,    TAR_CHAR_SELF,	        8	},
  {     16416,  750,    TAR_CHAR_SELF,          8	}
};


void do_learn( CHAR_DATA *ch, char *argument )
{
  int type = 0;
  int cost = 0;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "\n\rYou simply don't understand rune magic.\n\r", ch );
    return;
  }
  cost = UMIN( 200, 50 + ch->pcdata->powers[P_LEARNED_NUM] * 10 );

  if ( argument[0] == '\0' )
  { send_to_char( "\n\rYou can learn the following runes:\n\r", ch );
    send_to_char( "Primary: earth air fire water energy negative\n\r", ch );
    send_to_char( "Secondary: life death creation destruction protection\n\r", ch );
    send_to_char( "           transformation movement abjuration\n\r", ch );
    return;
  }
  else if ( !str_cmp( argument, "air" ) )
    type = RUNE_AIR;
  else if ( !str_cmp( argument, "earth" ) )
    type = RUNE_EARTH;
  else if ( !str_cmp( argument, "fire" ) )
    type = RUNE_FIRE;
  else if ( !str_cmp( argument, "water" ) )
    type = RUNE_WATER;
  else if ( !str_cmp( argument, "energy" ) )
    type = RUNE_ENERGY;
  else if ( !str_cmp( argument, "negative" ) )
    type = RUNE_NEGATIVE;
  else if ( !str_cmp( argument, "life" ) )
    type = RUNE_LIFE;
  else if ( !str_cmp( argument, "death" ) )
    type = RUNE_DEATH;
  else if ( !str_cmp( argument, "creation" ) )
    type = RUNE_CREATION;
  else if ( !str_cmp( argument, "destruction" ) )
    type = RUNE_DESTRUCTION;
  else if ( !str_cmp( argument, "protection" ) )
    type = RUNE_PROTECTION;
  else if ( !str_cmp( argument, "transformation" ) )
    type = RUNE_TRANSFORMATION;
  else if ( !str_cmp( argument, "movement" ) )
    type = RUNE_MOVEMENT;
  else if ( !str_cmp( argument, "abjuration" ) )
    type = RUNE_ABJURATION;
  else if ( !str_cmp( argument, "cost" ) )
  { sprintf( buf, "\n\rIt will cost you %d primal to learn another rune.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }
  else if ( !str_cmp( argument, "learned" ) )
  {
    return;
  }
  else
  { send_to_char( "Thats not a rune.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->pcdata->powers[P_LEARNED],type) )
  { send_to_char( "But you already know that rune!\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < cost )
  { send_to_char( "You don't have enough primal to learn another rune.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= cost;
  SET_BIT(ch->pcdata->powers[P_LEARNED],type);
  ch->pcdata->powers[P_LEARNED_NUM]++;
  send_to_char( "You have learned how to use a new rune!\n\r", ch );
  return;
}

void do_runeweave( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char def1[MAX_STRING_LENGTH];
  char def2[MAX_STRING_LENGTH];
  CHAR_DATA *victim = NULL;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;
  int rune1, rune2, spell, slot, i, cost, dam=0, target = TAR_CHAR_SELF;
  bool match = FALSE;


  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN || ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  /* arguments out of order for kludgy targeting */
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg1 );

  if ( arg2[0] == '\0' || arg3[0] == '\0' )
  { send_to_char( "You must specify two runes to weave.\n\r", ch );
    return;
  }


  if ( !str_prefix( arg2, "air" ) )
    rune1 = RUNE_AIR;
  else if ( !str_prefix( arg2, "earth" ) )
    rune1 = RUNE_EARTH;
  else if ( !str_prefix( arg2, "fire" ) )
    rune1 = RUNE_FIRE;
  else if ( !str_prefix( arg2, "water" ) )
    rune1 = RUNE_WATER;
  else if ( !str_prefix( arg2, "energy" ) )
    rune1 = RUNE_ENERGY;
  else if ( !str_prefix( arg2, "negative" ) )
    rune1 = RUNE_NEGATIVE;
  else
  { sprintf( buf, "You know of no rune of %s.\n\r", arg2 );
    send_to_char( buf, ch );
    return;
  }

  if ( !str_prefix( arg3, "life" ) )
    rune2 = RUNE_LIFE;
  else if ( !str_prefix( arg3, "death" ) )
    rune2 = RUNE_DEATH;
  else if ( !str_prefix( arg3, "creation" ) )
    rune2 = RUNE_CREATION;
  else if ( !str_prefix( arg3, "destruction" ) )
    rune2 = RUNE_DESTRUCTION;
  else if ( !str_prefix( arg3, "protection" ) )
    rune2 = RUNE_PROTECTION;
  else if ( !str_prefix( arg3, "transformation" ) )
    rune2 = RUNE_TRANSFORMATION;
  else if ( !str_prefix( arg3, "movement" ) )
    rune2 = RUNE_MOVEMENT;
  else if ( !str_prefix( arg3, "abjuration" ) )
    rune2 = RUNE_ABJURATION;
  else
  { sprintf( buf, "You know of no rune of %s.\n\r", arg3 );
    send_to_char( buf, ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[P_LEARNED],rune1) || !IS_SET(ch->pcdata->powers[P_LEARNED],rune2) )
  { send_to_char( "You have not learned the uses of that rune.\n\r", ch );
    return;
  }

  spell = rune1 + rune2;
  slot = 0; cost = 30000;
  while ( slot < MAX_RUNESPELLS && !match )
  {
    if ( cost_table[slot].spell == spell )
    {
      match = TRUE;
      cost = cost_table[slot].cost;
      target = cost_table[slot].target;
    }
    else
     slot++;
  }

  if ( !match )
  { send_to_char( "There is no such spell..yet.\n\r", ch );
    return;
  }
  if ( ch->mana < cost )
  { send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  if ( cost_table[slot].target == TAR_CHAR_SELF || cost_table[slot].target == TAR_IGNORE )
    victim = ch;
  else if ( cost_table[slot].target == TAR_CHAR_OFFENSIVE )
  { if ( arg1[0] == '\0' )
    { if ( ch->fighting != NULL )
        victim = ch->fighting;
      else
      { send_to_char( "Weave at who?\n\r", ch );
        return;
      }
    }
    else if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      WAIT_STATE( ch, 4 );
      return;
    }
    timer_check( ch, victim );
  }
  else
  { if ( arg1[0] == '\0' )
      victim = ch;
    else if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    { send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  }

  if ( !IS_NPC(victim) && victim->level < 2 )
  { send_to_char( "No runeweaving on mortals.\n\r", ch );
    return;
  }

  sprintf( def1, "You move your hands in mystick patterns, uttering runes under your breath." );
  sprintf( def2, "$n moves $s hands in a complex pattern, while muttering broken phrases." );

  switch( spell )
  {
    case RUNE_AIR + RUNE_LIFE:
    	if ( is_affected(victim,gsn_wind_curse) )
    	{
    	  stc( "They are already so cursed.\n\r", ch );
    	  break;
    	}
    	act( "A pale blue mist settles around $N.", ch, NULL, victim, TO_CHAR );
    	act( "A pale blue mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_wind_curse;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    case RUNE_EARTH + RUNE_LIFE:
        if ( is_affected(victim,gsn_earth_curse) )
        {
          stc( "They are already so cursed.\n\r", ch );
          break;
        }
        act( "A dull yellow mist settles around $N.", ch, NULL, victim, TO_CHAR );
        act( "A dull yellow mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_earth_curse;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind *2/3; 
        af.bitvector = 0 *2/3;;
        affect_to_char( victim, &af );
        break;
    case RUNE_FIRE + RUNE_LIFE:
        if ( is_affected(victim,gsn_flame_curse) )
        {
          stc( "They are already so cursed.\n\r", ch );
          break;
        }
        act( "A blood red mist settles around $N.", ch, NULL, victim, TO_CHAR );
        act( "A blood red mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_flame_curse;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    case RUNE_WATER + RUNE_LIFE:
        if ( is_affected(victim,gsn_water_curse) )
        {
          stc( "They are already so cursed.\n\r", ch );
          break;
        }
        act( "A murky mist settles around $N.", ch, NULL, victim, TO_CHAR );
        act( "A murky mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_water_curse;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    /* ENERGY + LIFE */
    case 80:
	act( def1, ch, NULL, victim, TO_CHAR );
	act( def2, ch, NULL, victim, TO_ROOM );
	for ( i = 0; i < ch->pcdata->powers[P_ENERGY]; i += 33 )
	  runecast( ch, victim, "heal" );
	break;
    /* NEGATIVE + LIFE */
    case 96:
    	if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
    	{
    	  if ( victim == ch )
    	  {
    	    stc( "You failed.\n\r", ch );
    	    return;
    	  }
    	  act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_CHAR );
    	  act( "Dark energy streaks toward and is absorbed by you!", ch, NULL, victim, TO_VICT );
    	  act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_NOTVICT );
	  victim->hit += dice( ch->pcdata->mind/8, ch->pcdata->powers[P_NEGATIVE] );
	  victim->hit = UMIN( victim->hit, victim->max_hit );
    	}
    	else
    	{
    	  act( "Dark energy streaks from you to $N, and rebounds!", ch, NULL, victim,TO_CHAR);
    	  act( "Dark energy streaks from $n to you, and rebounds to $m!", ch, NULL, victim, TO_VICT );
    	  act( "Dark energy streaks from $n to $N, then rebounds.", ch, NULL, victim, TO_NOTVICT );
    	  dam = dice( ch->pcdata->mind/4, ch->pcdata->powers[P_NEGATIVE] );
    	  i = victim->hit;
    	  damage( ch, victim, dam, DAM_NEGATIVE );
    	  if ( victim != NULL && victim->hit > 0 )
    	    ch->hit = UMIN( ch->max_hit, ch->hit + (i - victim->hit) );
	}
    	break;

    /* AIR + DEATH */
    case 129:
    	act( "A bolt of lightning leaps from your fingers toward $N!", ch, NULL, victim, TO_CHAR );
    	act( "A bolt of lightning leaps from $n's fingers toward you!", ch, NULL, victim, TO_VICT );
    	act( "A bolt of lightning leaps from $n's fingers toward $N!", ch, NULL, victim, TO_NOTVICT );
        if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_AIR_BLOCK) )
        { act( "The lightning bolt rebounds!", ch, NULL, NULL, TO_CHAR );
          act( "The lightning bolt rebounds!", ch, NULL, NULL, TO_ROOM );
          dam = dice( ch->pcdata->mind / 4, ch->pcdata->powers[P_AIR] );
	  damage( ch, ch, dam, DAM_LIGHTNING );
          break;
        }
        else
        {
	  dam = dice( ch->pcdata->mind / 2, ch->pcdata->powers[P_AIR] );
	  damage( ch, victim, dam, DAM_LIGHTNING );
	}
	break;
    /* FIRE + DEATH */
    case 132:
	act( "A lance of flame streaks from your fingers toward $N!", ch, NULL, victim, TO_CHAR );
	act( "A lance of flame streaks from $n's fingers toward you!", ch, NULL, victim, TO_VICT );
	act( "A lance of flame streaks from $n's fingers toward $N!", ch, NULL, victim, TO_NOTVICT );
	if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_FIRE_BLOCK) )
	{ act( "The flame lance rebounds!", ch, NULL, NULL, TO_CHAR );
	  act( "The flame lance rebounds!", ch, NULL, NULL, TO_ROOM );
	  dam = dice( ch->pcdata->mind / 4, ch->pcdata->powers[P_AIR] );
	  damage( ch, ch, dam, DAM_FIRE );
	  break;
	}
	else
	{ dam = dice( ch->pcdata->mind / 2, ch->pcdata->powers[P_FIRE] );
          if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
          { REMOVE_BIT(victim->pcdata->actnew,NEW_VAN_REHL);
            act( "Their icy barrier melts!", ch, NULL, victim, TO_CHAR );
            act( "Your icy barrier melts!", ch, NULL, victim, TO_VICT );
          }
	  damage( ch, victim, dam, DAM_FIRE );
	}
	break;
    /* AIR + CREATION */
    case 257:
    	if ( IS_NPC(victim) )
    	{
    	  stc( "Nothing happens.\n\r", ch );
    	  return;
    	}
    	act( "Ball lightning crackles around $N.", ch, NULL, victim, TO_CHAR );
    	act( "Ball lightning crackles around $N.", ch, NULL, victim, TO_ROOM );
    	if ( !IS_SET(victim->pcdata->actnew,NEW_BALL_LIGHTNING) )
    	  SET_BIT(victim->pcdata->actnew,NEW_BALL_LIGHTNING);
    	break;
    /* WATER + CREATION */
    case 264:
        act( def1, ch, NULL, victim, TO_CHAR );
        act( def2, ch, NULL, victim, TO_ROOM );
        if ( is_affected(victim,skill_lookup("flame breath")) )
        {
          affect_strip(victim,skill_lookup("flame breath"));
          act( "The flames are doused.", ch, NULL, victim, TO_CHAR );
          act( "Your skin abruptly stops burning.", ch, NULL, victim, TO_VICT );
          act( "The flames burning $N are doused.", ch, NULL, victim, TO_ROOM );
        }
        break;
    /* ENERGY + CREATION */
    case 272:
	act( def1, ch, NULL, victim, TO_CHAR );
	act( def2, ch, NULL, victim, TO_ROOM );
	if ( ch->pcdata->powers[P_ENERGY] < 25 )
	  runecast( ch, victim, "cure light" );
	else if ( ch->pcdata->powers[P_ENERGY] < 50 )
	  runecast( ch, victim, "cure serious" );
	else if ( ch->pcdata->powers[P_ENERGY] < 75 )
	{
	  runecast( ch, victim, "cure critical" );
	  runecast( ch, victim, "refresh" );
	}
	else
	{
	  runecast( ch, victim, "heal" );
	  runecast( ch, victim, "refresh" );
	  runecast( ch, victim, "cure poison" );
	}
	break;
    /* AIR + DESTRUCTION */
    case 513:
    	act( "You launch a gust of wind into $N.", ch, NULL, victim, TO_CHAR );
    	act( "$n's gust of wind slams into you!", ch, NULL, victim, TO_VICT );
    	act( "$n's gust of wind slams into $N.", ch, NULL, victim, TO_NOTVICT );
    	if ( !IS_NPC(victim) )
    	{ if ( victim->class == CLASS_FIST )
    	  { if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	      REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
	    if ( victim->pcdata->powers[F_KI] > 0 )
	      victim->pcdata->powers[F_KI]--;
	  }
	  else if ( IS_CLASS(victim,CLASS_SORCERER ) )
	    lose_chant( victim );

	  if ( is_affected(victim,gsn_kiwall) )
	    affect_strip(victim,gsn_kiwall);

        }
        break;
    /* FIRE + DESTRUCTION */
    case 516:
	act( "Fire sprays forth from your hands, engulfing the room in flame!", ch, NULL, victim, TO_CHAR );
	act( "The room is is engulfed in flame!", ch, NULL, victim, TO_ROOM );
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{ vch_next = vch->next;
	  if ( vch->in_room == NULL )
	    continue;
	  if ( vch->in_room == ch->in_room )
	  { if ( vch != ch && !is_same_group( ch, vch ) )
	    { if ( !IS_NPC(vch) && IS_SET(vch->pcdata->actnew,NEW_FIRE_BLOCK) )
	      { act( "The flames flow around $N.", ch, NULL, vch, TO_CHAR );
	        act( "The flames flow around you.", ch, NULL, vch, TO_VICT );
	        act( "The flames flow around $N.", ch, NULL, vch, TO_ROOM );
	      }
	      else
	      { dam = dice( ch->pcdata->mind / 4, ch->pcdata->powers[P_FIRE] );
	        if ( !IS_NPC(vch) && IS_SET(vch->pcdata->actnew,NEW_VAN_REHL) )
	        { REMOVE_BIT(vch->pcdata->actnew,NEW_VAN_REHL);
	          act( "Their icy barrier melts!", ch, NULL, vch, TO_CHAR );
	          act( "Your icy barrier melts!", ch, NULL, vch, TO_VICT );
	        }
	        damage( ch, vch, dam, DAM_FIRE );
	      }
	    }
	  }
	}
	break;
    /* ENERGY + DESTRUCTION */
    case 528:
	runecast( ch, victim, "dispel magic" );
	if ( victim != ch && victim->fighting == NULL )
	  victim->fighting = ch;
	break;
    /* NEGATIVE + DESTRUCTION */
    case 544:
    	act( "You launch a pearlescent ball of light at $N.", ch, NULL, victim, TO_CHAR );
    	act( "$n launches a pearlescent ball of light at you!", ch, NULL, victim, TO_VICT );
    	act( "$n launches a pearlescent ball of light at $N.", ch, NULL, victim, TO_NOTVICT );
    	if ( !IS_CLASS(victim,CLASS_MAZOKU) )
    	  break;
    	if ( victim->pcdata->powers[M_CTIME] <= 0 )
    	  break;
    	act( ".. the dark energies surrounding $N are dispelled!", ch, NULL, victim, TO_CHAR );
    	act( ".. you lose control of your dark energies!", ch, NULL, victim, TO_VICT );
    	act( ".. the dark energies surrounding $N are dispelled!", ch, NULL, victim, TO_NOTVICT );
    	victim->pcdata->powers[M_CTIME] = 0;
    	victim->pcdata->powers[M_CTYPE] = 0;
	break;
    /* AIR + PROTECTION */
    case 1025:
	act( def1, ch, NULL, victim, TO_CHAR );
	act( def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, victim, "shield" );
	break;
    /* EARTH + PROTECTION */
    case 1026:
	act( def1, ch, NULL, victim, TO_CHAR );
	act( def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, victim, "stone skin" );
	break;
    /* FIRE + PROTECTION */
    case 1028:
        if ( is_affected( ch, gsn_balus_wall ) )
        affect_strip( ch, gsn_balus_wall );
      
        af.type      = gsn_balus_wall;
        af.duration  = 30;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = 0;
        affect_to_char( ch, &af );
	act( "A reddish shield shimmers into existance in front of you.", ch, NULL, NULL, TO_CHAR );
	act( "A reddish shield shimmers into existance in front of $n.", ch, NULL, NULL, TO_ROOM );
	break;
    /* WATER + PROTECTION */
    case 1032:
	act( def1, ch, NULL, victim, TO_CHAR );
	act( def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, victim, "armor" );
	break;
    case 1056:
    /* NEGATIVE + PROTECTION */
    	if ( IS_NPC(victim) )
    	{ 
    	  stc( "All your base are belong to us.\n\r", ch );
    	  return;
    	}
    	act( "A black helix swirls up around you.", victim, NULL, NULL, TO_CHAR );
    	act( "A black helix swirls up around $n.", victim, NULL, NULL, TO_ROOM );
    	if ( !IS_SET(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
    	  SET_BIT(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK);
    	break;
    /* AIR + TRANSFORMATION */
    case 2049:
    	if ( IS_NPC(victim) )
    	{ act( "You failed.", ch, NULL, NULL, TO_CHAR );
    	  break;
    	}
    	act( "Inverted runes of air swirl up around you.", victim, NULL, NULL, TO_CHAR );
    	act( "Oddly disfigured runes swirl up around $n.", victim, NULL, NULL, TO_ROOM );
    	if ( IS_SET(victim->pcdata->actnew,NEW_FIRE_BLOCK) )
    	{ REMOVE_BIT(victim->pcdata->actnew,NEW_FIRE_BLOCK);
    	  act( "The inverted fire runes crumble.", ch, NULL, NULL, TO_CHAR );
    	}
    	SET_BIT(victim->pcdata->actnew,NEW_AIR_BLOCK);
    	break;
    case 2052:
        if ( IS_NPC(victim) )
        { act( "You failed.", ch, NULL, NULL, TO_CHAR );
          break;
        }
        act( "Inverted runes of fire ignite all around you.", victim, NULL, NULL, TO_CHAR );
        act( "Oddly disfigured runes ignite all around $n.", victim, NULL, NULL, TO_ROOM );
        if ( IS_SET(victim->pcdata->actnew,NEW_AIR_BLOCK) )
        { REMOVE_BIT(victim->pcdata->actnew,NEW_AIR_BLOCK);
          act( "The inverted air runes fade away.", ch, NULL, NULL, TO_CHAR );
        }
        SET_BIT(victim->pcdata->actnew,NEW_FIRE_BLOCK);
        break;

    /* ENERGY + TRANSFORMATION */
    case 2064:
        if ( IS_SET(ch->act, PLR_TRUESIGHT) )
        { send_to_char( "Your vision returns to normal.\n\r", ch );
          REMOVE_BIT(ch->act,PLR_TRUESIGHT);
        }
        else
        { send_to_char( "Your vision increases beyond mortal standards.\n\r", ch );
          SET_BIT(ch->act,PLR_TRUESIGHT);
        }
        break;
    /* NEGATIVE + TRANSFORMATION */
    case 2080:
        if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
        {
          if ( victim == ch )
          {
            stc( "You failed.\n\r", ch );
            return;
          }
          act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_CHAR );
          act( "Dark energy streaks toward and is absorbed by you!", ch, NULL, victim, TO_VICT );
          act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_NOTVICT
);
          victim->mana += dice( ch->pcdata->mind/4, ch->pcdata->powers[P_NEGATIVE] );
          victim->mana = UMIN( victim->mana, victim->max_mana );
        }
        else
        {
          act( "Dark energy streaks from you to $N, and rebounds!", ch, NULL, victim,TO_CHAR);
          act( "Dark energy streaks from $n to you, and rebounds to $m!", ch, NULL, victim, TO_VICT );
          act( "Dark energy streaks from $n to $N, then rebounds.", ch, NULL, victim, TO_NOTVICT );
          dam = dice( ch->pcdata->mind/4, ch->pcdata->powers[P_NEGATIVE] );
          if ( IS_AFFECTED(victim,AFF_HOLY_RESIST) )
            dam -= dam * number_percent() / 100;
          if ( victim->mana > dam )
            victim->mana -= dam;
          else
          { dam = victim->mana - 1;
            victim->mana = 1;
          }
          ch->mana = UMIN( ch->max_mana, ch->mana + dam/2 );
        }
        break;
    case RUNE_AIR + RUNE_MOVEMENT:
	act( def1, ch, NULL, victim, TO_CHAR );
	act( def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, ch, "fly" );
	break;
    case RUNE_NEGATIVE + RUNE_MOVEMENT:
    	act( "A black haze surrounds you, sapping your strength!", ch, NULL, victim, TO_VICT );
    	act( "A black haze descends upon $N", ch, NULL, victim, TO_CHAR );
    	act( "A black haze descends upon $N", ch, NULL, victim, TO_ROOM );
    	dam = dice( 10, 15 );
    	victim->move = UMAX( victim->move - dam, 1 );
    	ch->move = UMIN( ch->move + dam, ch->max_move );
    	if ( IS_CLASS(victim, CLASS_SAIYAN) )
    	{
    	  victim->pcdata->powers[S_STRENGTH] = UMAX(
    	    victim->pcdata->powers[S_STRENGTH] - dam,
    	    victim->pcdata->powers[S_STRENGTH_MAX]/2 );
    	  victim->pcdata->powers[S_SPEED] = UMAX(
    	    victim->pcdata->powers[S_SPEED] - dam,
    	    victim->pcdata->powers[S_SPEED_MAX]/2 );
    	  victim->pcdata->powers[S_AEGIS] = UMAX(
    	    victim->pcdata->powers[S_AEGIS] - dam,
    	    victim->pcdata->powers[S_AEGIS_MAX]/2 );
    	}
    	break;
    case RUNE_AIR + RUNE_ABJURATION:
   	act( "A dome of azure energy encases you.", ch, NULL, victim, TO_CHAR );
   	act( "A dome of azure energy encases $n.", ch, NULL, victim, TO_ROOM );
	if ( is_affected( ch, gsn_earth_ward ) )
	  affect_strip( ch, gsn_earth_ward );
	if ( is_affected( ch, gsn_wind_ward ) )
	  affect_strip( ch, gsn_wind_ward ) ;
	if ( is_affected( ch, gsn_flame_ward ) )
	  affect_strip( ch, gsn_flame_ward );
	if ( is_affected( ch, gsn_water_ward ) )
	  affect_strip( ch, gsn_water_ward );
	if ( is_affected( ch, gsn_spirit_ward ) )
	  affect_strip( ch, gsn_spirit_ward );
	if ( is_affected( ch, gsn_negative_ward ) )
	  affect_strip( ch, gsn_negative_ward );
	af.type      = gsn_wind_ward;
   	af.location  = 0;
   	af.modifier  = 0;
   	af.duration  = ch->pcdata->mind*15;
   	af.bitvector = 0;
   	affect_to_char( ch, &af );
   	break;
    case RUNE_EARTH + RUNE_ABJURATION:
        act( "A dome of verdant energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of verdant energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_earth_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    case RUNE_FIRE + RUNE_ABJURATION:
        act( "A dome of crimson energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of crimson energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_flame_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15; 
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    case RUNE_WATER + RUNE_ABJURATION:
        act( "A dome of sea green energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of sea green energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_water_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    case RUNE_ENERGY + RUNE_ABJURATION:
        act( "A dome of pale white energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of pale white energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_spirit_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    case RUNE_NEGATIVE + RUNE_ABJURATION:
        act( "A dome of obsidian energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of obsidian energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_negative_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        break;
    default:
	send_to_char( "Nothing happens.\n\r", ch );
	return;
  }

  if ( victim != NULL && target == TAR_CHAR_OFFENSIVE && victim != ch
    && victim->position >= POS_STUNNED )
  { if ( ch->fighting == NULL )
      ch->fighting = victim;
    if ( victim->fighting == NULL )
      victim->fighting = ch;
  }
  WAIT_STATE( ch, cost_table[slot].wait );
  ch->mana -= cost;
  return;
}

void do_tattoo( CHAR_DATA *ch, char *argument )
{
  int cost, mcost, location, type;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "You know nothing of the ancient art of tattooing.\n\r", ch );
    return;
  }
  if ( ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  { send_to_char( "You must specify both a location, and a rune to tattoo.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "torso" ) )
    location = TORSO;
  else if ( !str_cmp( arg1, "leftarm" ) )
    location = LEFTARM;
  else if ( !str_cmp( arg1, "rightarm" ) )
    location = RIGHTARM;
  else if ( !str_cmp( arg1, "leftleg" ) )
    location = LEFTLEG;
  else if ( !str_cmp( arg1, "rightleg" ) )
    location = RIGHTLEG;
  else
  { send_to_char( "\n\rYou can tattoo runes on the following bodyparts:\n\r", ch );
    send_to_char( "Torso, leftarm, rightarm, leftleg, rightleg.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "earth" ) )
    type = RUNE_EARTH;
  else if ( !str_cmp( arg2, "air" ) )
    type = RUNE_AIR;
  else if ( !str_cmp( arg2, "fire" ) )
    type = RUNE_FIRE;
  else if ( !str_cmp( arg2, "water" ) )
    type = RUNE_WATER;
  else
  { send_to_char( "\n\rYou can tattoo the following runes onto your body:\n\r", ch );
    send_to_char( "Earth, air, fire, water.\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[P_LEARNED],type) )
  { send_to_char( "\n\rBut you don't know that rune!\n\r", ch );
    return;
  }

  cost = 10 + get_runes( ch, RUNE_ALL, location ) * 10;
  if ( ch->pcdata->primal < cost )
  { send_to_char( "You don't have enough primal.\n\r", ch );
    return;
  }
  mcost = cost * 25;
  if ( ch->mana < mcost )
  { send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  if ( get_runes( ch, RUNE_ALL, location ) >= runemax[location] )
  { send_to_char( "There is no more space for another rune there.\n\r", ch );
    return;
  }

  add_rune( ch, type, location );
  ch->pcdata->primal -= cost;
  ch->mana -= mcost;
  sprintf( buf, "\n\rYou tattoo a rune of %s onto your %s.\n\r", arg2, arg1 );
  send_to_char( buf, ch );
  WAIT_STATE( ch, 12 );
  return;
}

int get_runes( CHAR_DATA *ch, int type, int part )
{
  int i;
  int num = 0;

  if ( IS_NPC(ch) )
    return 0;
  if ( ch->class != CLASS_PATRYN )
    return 0;
  if ( part > 4 || part < 0 )
    return 0;

  if ( type != RUNE_ALL )
  {
    for ( i = 0; i < runemax[part]; i++ )
      if ( ch->pcdata->runes[part][i] == type )
	num++;
  }
  else
  {
    for ( i = 0; i < runemax[part]; i++ )
      if ( ch->pcdata->runes[part][i] != 0 )
	num++;
  }

  return URANGE( 0, num, 15 );
}


void add_rune( CHAR_DATA *ch, int type, int part )
{
  int i = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
    return;

  if ( get_runes( ch, RUNE_ALL, part ) >= runemax[part] )
    return;
  while ( ch->pcdata->runes[part][i] != 0 && i < runemax[part] - 1 )
    i++;
  ch->pcdata->runes[part][i] = type;
  return;
}

void remove_rune( CHAR_DATA *ch, int type, int part )
{
  int i = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
    return;

  if ( get_runes( ch, type, part ) == 0 )
    return;

  while ( ch->pcdata->runes[part][i] != type )
    i++;

  ch->pcdata->runes[part][i] = 0;
  return;
}

void do_runestats( CHAR_DATA *ch, char *argument )
{
  int part, rune;
  char *message[5][15];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "What do you think you are, a Patryn?\n\r", ch );
    return;
  }

  for ( part = 0; part < 5; part++ )
    for ( rune = 0; rune < runemax[part]; rune++ )
    { if ( ch->pcdata->runes[part][rune] == RUNE_AIR )
	message[part][rune] = "air";
      else if ( ch->pcdata->runes[part][rune] == RUNE_EARTH )
	message[part][rune] = "earth";
      else if ( ch->pcdata->runes[part][rune] == RUNE_FIRE )
	message[part][rune] = "fire";
      else if ( ch->pcdata->runes[part][rune] == RUNE_WATER )
	message[part][rune] = "water";
      else if ( ch->pcdata->runes[part][rune] == RUNE_NONE )
	message[part][rune] = "none";
      else
	message[part][rune] = "?err?";
    }

  send_to_char( "\n\rYou have the following runes on you:\n\r", ch );

  sprintf( buf, "Torso: %s %s %s %s %s %s %s %s\n\r",
	message[0][0],message[0][1],message[0][2],message[0][3],
	message[0][4],message[0][5],message[0][6],message[0][7] );
  send_to_char( buf, ch );
  sprintf( buf, "       %s %s %s %s %s %s %s\n\r",
	message[0][8],message[0][9],message[0][10],message[0][11],
	message[0][12],message[0][13],message[0][14] );
  send_to_char( buf, ch );
  sprintf( buf, "Left Arm: %s %s %s %s %s\n\r",
	message[1][0],message[1][1],message[1][2],message[1][3],message[1][4] );
  send_to_char( buf, ch );
  sprintf( buf, "Right Arm: %s %s %s %s %s\n\r",
	message[2][0],message[2][1],message[2][2],message[2][3],message[2][4] );
  send_to_char( buf, ch );
  sprintf( buf, "Left Leg: %s %s %s %s %s %s %s %s\n\r",
	message[3][0],message[3][1],message[3][2],message[3][3],
	message[3][4],message[3][5],message[3][6],message[3][7] );
  send_to_char( buf, ch );
  sprintf( buf, "Right Leg: %s %s %s %s %s %s %s %s\n\r\n\r",
	message[4][0],message[4][1],message[4][2],message[4][3],
	message[4][4],message[4][5],message[4][6],message[4][7] );
  send_to_char( buf, ch );

  return;

}

void do_erase( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int rune, part;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  { send_to_char( "You must specify a body part and a rune to erase.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "torso" ) )
    part = TORSO;
  else if ( !str_cmp( arg1, "leftarm" ) )
    part = LEFTARM;
  else if ( !str_cmp( arg1, "rightarm" ) )
    part = RIGHTARM;
  else if ( !str_cmp( arg1, "leftleg" ) )
    part = LEFTLEG;
  else if ( !str_cmp( arg1, "rightleg" ) )
    part = RIGHTLEG;
  else
  { send_to_char( "That is not an acceptable body part.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "earth" ) )
    rune = RUNE_EARTH;
  else if ( !str_cmp( arg2, "air" ) )
    rune = RUNE_AIR;
  else if ( !str_cmp( arg2, "fire" ) )
    rune = RUNE_FIRE;
  else if ( !str_cmp( arg2, "water" ) )
    rune = RUNE_WATER;
  else
  { send_to_char( "That is not an acceptable rune.\n\r", ch );
    return;
  }

  if ( get_runes(ch,rune,part) == 0 )
  { send_to_char( "But you have none of those runes there!\n\r", ch );
    return;
  }

  if ( ch->pcdata->primal < 25 || ch->mana < 1000 )
  { send_to_char( "It costs 25 primal and 1000 mana to erase a rune.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= 25;
  ch->mana -= 1000;
  remove_rune(ch,rune,part);
  sprintf( buf, "You painstakingly remove the rune of %s from your %s.\n\r", arg2, arg1 );
  send_to_char( buf, ch );
  return;
}

void runecast( CHAR_DATA *ch, CHAR_DATA *victim, char *arg )
{
  int sn = skill_lookup( arg );
  int lev = ch->pcdata->will;

  (*skill_table[sn].spell_fun) ( sn, lev, ch, victim );

  if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE && ch != victim )
  { CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    for ( vch = char_list; vch != NULL; vch = vch_next )
    { vch_next = vch->next_in_room;
      if ( victim == vch && victim->fighting == NULL )
      { multi_hit( victim, ch, TYPE_UNDEFINED );
	break;
      }
    }
  }

  return;
}

void do_runetrain( CHAR_DATA *ch, char *argument )
{
  int rune, cost;
  char buf[MAX_STRING_LENGTH];
  char * const runenames[] = { "motherfucker", "shitface", "air", "earth", "fire", "water", "energy", "negative" };

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN || ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  
  if ( argument[0] == '\0' )
  { send_to_char( "Your understanding of the following runes can be trained:\n\r", ch );
    sprintf( buf, "Air    %5d  Earth  %5d  Fire     %5d\n\r",
	150 * (ch->pcdata->powers[P_AIR]+1) * (ch->pcdata->powers[P_AIR]+1),
	150 * (ch->pcdata->powers[P_EARTH]+1) * (ch->pcdata->powers[P_EARTH]+1),
	150 * (ch->pcdata->powers[P_FIRE]+1) * (ch->pcdata->powers[P_FIRE]+1) );
    send_to_char( buf, ch );
    sprintf( buf, "Water  %5d  Energy %5d  Negative %5d\n\r", 
   	150 * (ch->pcdata->powers[P_WATER]+1) * (ch->pcdata->powers[P_WATER]+1),
   	150 * (ch->pcdata->powers[P_ENERGY]+1) * (ch->pcdata->powers[P_ENERGY]+1),
   	150 * (ch->pcdata->powers[P_NEGATIVE]+1) * (ch->pcdata->powers[P_NEGATIVE]+1) );
    send_to_char( buf, ch );
    return;
  }

  if ( !str_cmp( argument, "air" ) )
    rune = P_AIR;
  else if ( !str_cmp( argument, "earth" ) )
    rune = P_EARTH;
  else if ( !str_cmp( argument, "fire" ) )
    rune = P_FIRE;
  else if ( !str_cmp( argument, "water" ) )
    rune = P_WATER;
  else if ( !str_cmp( argument, "energy" ) )
    rune = P_ENERGY;
  else if ( !str_cmp( argument, "negative" ) )
    rune = P_NEGATIVE;
  else
  { send_to_char( "You can't runetrain that.\n\r", ch );
    return;
  }

  cost = 150 * (ch->pcdata->powers[rune]+1) * ( ch->pcdata->powers[rune]+1);
  if ( ch->pcdata->powers[rune] >= 100 )
  { send_to_char( "You already understand all the uses of that rune.\n\r", ch );
    return;
  }
  if ( ch->exp < cost )
  { sprintf( buf, "You need %d experience to train that rune.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  ch->exp -= cost;
  ch->pcdata->powers[rune]++;
  sprintf( buf, "Your understanding of the rune of %s improves.\n\r", runenames[rune] );
  send_to_char( buf, ch );
  return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
  if ( !IS_CLASS( ch, CLASS_PATRYN ) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( IS_SUIT(ch) )
  { send_to_char( "Not in a mobile suit.\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_CIRCLE) )
  { send_to_char( "Your circle is already closed!\n\r", ch );
    return;
  }
  if ( ch->position < POS_RESTING || ch->position > POS_RESTING )
  { send_to_char( "You must be at rest to close the circle of your being.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "You close the circle of your being.\n\r", ch );
    SET_BIT(ch->pcdata->actnew,NEW_CIRCLE);
    return;
  }
  else
    send_to_char( "Sorry, multi-person circling is still in the works.\n\r", ch );

  return;
}

void do_defenses( CHAR_DATA *ch, char *argument )
{ 
  char arg[MAX_INPUT_LENGTH];
  argument = one_argument( argument, arg );
  
  if ( !IS_CLASS(ch,CLASS_PATRYN) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
  {
    if ( IS_SET(ch->pcdata->powers[P_BITS],P_DEFENSES) )
      send_to_char( "Your defenses are raised.\n\r", ch );
    else
      send_to_char( "Your defenses are lowered.\n\r", ch );
  }
  else if ( !str_prefix( arg, "raise" ) )
  {
    if ( IS_SET(ch->pcdata->powers[P_BITS],P_DEFENSES) )
      send_to_char( "Your defenses are already raised.\n\r", ch );
    else
    { SET_BIT(ch->pcdata->powers[P_BITS],P_DEFENSES);
      send_to_char( "You raise your defenses.\n\r", ch );
    }
  }
  else if ( !str_prefix( arg, "lower" ) )
  {
    if ( !IS_SET(ch->pcdata->powers[P_BITS],P_DEFENSES) )
      send_to_char( "Your defenses are already lowered.\n\r", ch );
    else
    { REMOVE_BIT(ch->pcdata->powers[P_BITS],P_DEFENSES);
      send_to_char( "You lower your defenses.\n\r", ch );
    }
  }
  else
   send_to_char( "Acceptable arguments are raise, lower, or nothing.\n\r", ch );

  return;
}

