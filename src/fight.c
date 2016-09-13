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
#include <string.h>
#include <time.h>
#include "merc.h"

INF_DATA	*	inf_free;


/*
 * Local functions.
 */
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_block	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_shields	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch, bool pdox ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, bool pdox ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	trip		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	calc_attacks	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_quest	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	suit_damage	args( ( CHAR_DATA *victim, int dam ) );
void	essense_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int gain, int dt ) );
/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    CHAR_DATA *rch;
    CHAR_DATA *rch_next;

    for ( ch = char_list; ch != NULL; ch = ch->next )
      if ( IS_CLASS(ch,CLASS_SAIYAN) && IS_SET(ch->pcdata->actnew,NEW_KIAIHOU) )
        REMOVE_BIT(ch->pcdata->actnew,NEW_KIAIHOU);

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	/* new location, before combat round */
	mprog_fight_trigger( ch, victim );

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room 
	  && victim->position > POS_STUNNED )
	{
	  if ( !IS_SUIT(ch) )
	  {
	      multi_hit( ch, victim, TYPE_UNDEFINED );
	  }
	}
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

        mprog_hitprcnt_trigger( ch, victim );
        /* original location
        mprog_fight_trigger( ch, victim );
        */

	/*
	 * Fun for the whole family!
	 */
	for ( rch = ch->in_room->people; rch != NULL; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

	    if ( IS_AWAKE(rch) && rch->fighting == NULL )
	    {
		/*
		 * everyody auto-assist others in their group.
		 */
		if ( is_same_group(ch,rch) &&
		     (IS_NPC(rch) || (!IS_TIED(rch) && !IS_SUIT(rch))) )
		{
		  if ( !IS_NPC(rch) )
		    stancecheck( rch, victim );

		  multi_hit( rch, victim, TYPE_UNDEFINED );
		  continue;
		}

		/*
		 * NPC's assist NPC's of same type or 12.5% chance regardless.
		 */
		if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM) )
		{
		    if ( rch->pIndexData == ch->pIndexData )
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			target = NULL;
			number = 0;
			for ( vch = ch->in_room->people; vch; vch = vch->next )
			{
			    if ( can_see( rch, vch )
			    &&   is_same_group( vch, victim )
			    &&   number_range( 0, number ) == 0 )
			    {
				target = vch;
				number++;
			    }
			}

			if ( target != NULL )
			  multi_hit( rch, target, TYPE_UNDEFINED );
		    }
		}
	    }
	}
    }

    return;
}



/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance, attacks, i, cn;
    bool isSame = TRUE;

    if ( victim == NULL )
      return;
    if ( victim->in_room == NULL )
      return;


    if ( IS_CLASS(ch,CLASS_SORCERER) && ch->pcdata->powers[SORC_PREP] > 0 )
    {
      cn = 1; // crummy safety measure
      for ( i = MAX_CHANT; i > 0; i-- )
      {
        if ( chant_table[i].school == ch->pcdata->powers[SORC_PREP] &&
             chant_table[i].rank <= ch->pcdata->powers[ch->pcdata->powers[SORC_PREP]] &&
             chant_table[i].prep == TRUE )
        {
          cn = i;
          break;
        }
      }

      dt = 1400 + cn;
      if ( ch->mana >= chant_table[cn].cost / 2 )
      {
        ch->mana -= chant_table[cn].cost / 2;
      }
      else
        return;
    }

    if ( !IS_NPC(ch) && IS_CLASS(victim,CLASS_SAIYAN) && victim->level >= 2 )
    {
      if ( IS_SET(victim->pcdata->actnew,NEW_KIAIHOU) )
      { i = victim->pcdata->powers[S_AEGIS_MAX] - victim->pcdata->powers[S_AEGIS];
        if ( victim->pcdata->powers[S_POWER] <= i*2 )
          act( "You're too tired to refocus your aegis!", victim, NULL, NULL, TO_CHAR );
        else
        { act( "You refocus your aegis!", victim, NULL, NULL, TO_CHAR );
          act( "$n's aegis flares with renewed energy.", victim, NULL, NULL, TO_ROOM );
          victim->pcdata->powers[S_POWER] -= i*2;
          victim->pcdata->powers[S_AEGIS] = victim->pcdata->powers[S_AEGIS_MAX];
        }
      }
      else if ( IS_SET(victim->pcdata->powers[S_TECH],S_KIAIHOU) )
        SET_BIT(victim->pcdata->actnew,NEW_KIAIHOU);
    }


    if ( IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
    { 
      if ( IS_NPC(victim) && ch->pcdata->powers[M_EGO] <= 1 )
      {
        stc( "You lack the will to execute an astrike.\n\r", ch );
        REMOVE_BIT(ch->pcdata->powers[M_SET],M_ASTRIKE);
      }
      else
      {
        if ( IS_NPC(victim) )
          ch->pcdata->powers[M_EGO] -= 1;
        act( "You materialize.. right behind $N!", ch, NULL, victim, TO_CHAR );
        act( "$n phases into existance.. right behind you!", ch, NULL, victim, TO_VICT );
        act( "$n phases into existance.. right behind $N!", ch, NULL, victim, TO_NOTVICT );
      }
    }

    one_hit( ch, victim, dt );
    if ( ch->fighting != victim || dt == gsn_backstab )
	return;

    attacks = calc_attacks( ch, victim );
    for ( i = 0; i < attacks && isSame; i++ )
    { one_hit( ch, victim, dt );
      if ( ch->fighting != victim || victim->position <= POS_STUNNED )
        isSame = FALSE;
    }

    chance = 30;
    if ( !IS_NPC(ch) )
      chance = 50 + ( ch->pcdata->body / 2 );
    if ( isSame && number_percent( ) < chance )
    {
	one_hit( ch, victim, dt );
	if ( ch->fighting != victim || victim->position <= POS_STUNNED )
          isSame = FALSE;
    }

    chance = 20;
    if ( !IS_NPC(ch) )
      chance = 10 + ( ch->pcdata->body / 2 );
    if ( isSame && number_percent( ) < chance )
    {
	one_hit( ch, victim, dt );
	if ( ch->fighting != victim || victim->position <= POS_STUNNED )
    	  isSame = FALSE;
    }

    if ( IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_ASTRIKE);

    return;
}



/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int dam, diceroll, chance, kai, roll, i, weapon, cdam, vdam;
    int stance = 0;
    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_WIELD );
    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	  dt += wield->value[3];
    }

    /* Mazoku damage is based off of their arm forms */
    if ( IS_CLASS(ch,CLASS_MAZOKU) && wield == NULL && dt >= TYPE_HIT )
    { dt = TYPE_HIT;
      /* magic numbers are bad.. oh well */
      if ( IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
        dt += 5;
      else if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
        dt += 2;
      else if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
        dt += 1;
      else if ( IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
        dt += 4;
    }


    /*
     * Fuck to-hit-armor-class-0 versus armor bullshit.
     */
    if ( IS_NPC(ch) )
    {
	chance = URANGE( 30, (ch->level * 3/2), 95);
    }
    else
    {
	if ( dt < TYPE_HIT )
	  chance = dice(10,5)+50;
	else if ( dt > TYPE_HIT+13 )
	  chance = 100;
	else
	  chance = URANGE( 30, ch->pcdata->weapons[dt-1000] , 95);
    }

    /* homersexual
    if ( !can_see( ch, victim ) )
	chance -= chance / 4;
    */

    /*
     * The moment of excitement!
     */
    diceroll = number_percent();

    if ( diceroll > chance )
    {
	/* Miss. */
	damage( ch, victim, 0, dt );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) )
    {
	dam = number_range( ch->level / 2, ch->level * 3 );
	if ( ch->level >= 25 ) dam += dam;
	if ( ch->level >= 50 ) dam += dam;
	if ( ch->level >= 75 ) dam += dam;
	if ( ch->level > 95 ) dam += dam * ( ch->level - 94 );

	if ( wield != NULL )
	    dam += dam / 2;
    }
    else
    {
	if ( IS_CLASS(ch,CLASS_SORCERER) && ch->pcdata->powers[SORC_PREP] > 0 )
	{
	  i = ch->pcdata->powers[ch->pcdata->powers[SORC_PREP]];
	  dam = number_range(i*(UMIN(ch->pcdata->powers[SORC_MYSTIC],i)/15),i*15);
	}
	else if ( wield != NULL )
	{ if ( dt < TYPE_HIT )
	    dam = number_range(ch->pcdata->weapons[0]/8+1,ch->pcdata->weapons[0]/5+5);
	  else
	    dam = number_range( (ch->pcdata->weapons[dt-1000]/8)+(wield->value[1]*2),
				(ch->pcdata->weapons[dt-1000]/5)+(wield->value[2]*2) );
        }
        /* for mazoku, essentially */
        else if ( IS_CLASS(ch,CLASS_MAZOKU) && dt >= TYPE_HIT )
        { dam = number_range( ch->pcdata->weapons[dt-TYPE_HIT] / 4,
                              ch->pcdata->weapons[dt-TYPE_HIT] / 3 );
	}
	/* unarmed combat */
	else
	{
	  weapon = ch->pcdata->weapons[0];
	  if ( ch->level < 2 )
	    weapon = UMIN( 200, weapon );
	  if ( ch->class == CLASS_FIST )
	    dam = number_range(weapon/8+1,weapon/3+5);
	  else if ( ch->class == CLASS_SAIYAN && ch->pcdata->powers[S_AEGIS] > 0 )
	    dam = number_range((weapon/8)+20,(weapon/5)+45);
	  else if ( ch->class == CLASS_SORCERER && IS_AFFECTED(ch,AFF_VISFARANK) )
	  { i = UMAX( 0, (ch->pcdata->powers[SCHOOL_WHITE]-20)*2 );
	    dam = number_range((weapon/2)+i,(weapon*2)+40+i);
	  }
	  else
	    dam = number_range(weapon/10+1,weapon/5+5);
	}
    }

    if ( !IS_NPC(ch) )
    {
      int max = 200;
      if ( ch->class == CLASS_FIST && dt == TYPE_HIT && ch->pcdata->powers[F_DISC] >= 2 )
        max = 500;
      chance = number_percent();
      if ( ch->pcdata->weapons[dt-1000] < max )
        if ( chance < UMAX( 5, 100 - ch->pcdata->weapons[dt-1000] ) )
	  ch->pcdata->weapons[dt-1000]++;
      stance = stanced(ch);
      max = 200;
      if ( ch->class == CLASS_FIST && ch->pcdata->powers[F_DISC] >= 3 )
        max = 300;
      if ( stance > 0 && stance <= 10 )
      { roll = number_percent();
	if ( ch->pcdata->stances[stance] < max )
	  if ( roll > ch->pcdata->stances[stance] || roll > 95 )
	    ch->pcdata->stances[stance]++;
      }
    }

    /*
     * Bonuses.
     */

    if ( !IS_NPC(ch) )
      cdam = UMIN( ch->damroll, ch->pcdata->extras2[EVAL]*3 );
    else
      cdam = ch->damroll;
    if ( !IS_NPC(victim) )
      vdam = UMIN( victim->damroll, victim->pcdata->extras2[EVAL]*3 );
    else
      vdam = victim->damroll;

    /* dam += dam * isquare(UMAX( 0, cdam - vdam )) / 33; */
    dam += dam * URANGE(0,cdam-vdam, 40) / 100;

    if ( IS_CLASS(ch,CLASS_SAIYAN) && ch->level >= 2 )
    { if ( ch->pcdata->powers[S_STRENGTH] > 0 )
	dam += ( dam * ch->pcdata->powers[S_STRENGTH] / 175 );
      if ( is_affected(ch,gsn_kaiouken) )
        dam += dam * ch->pcdata->powers[S_STRENGTH_MAX] / 1000;
    }
    
    if ( IS_CLASS(ch,CLASS_FIST) && ch->level >= 2 )
      if ( ch->pcdata->powers[F_HANDS] > 0 )
	dam += dam * ch->pcdata->powers[F_HANDS] / 33; 

    if ( IS_CLASS(ch,CLASS_PATRYN) && ch->level >= 2 ) 
      dam += dam * (50 + ch->pcdata->powers[P_FIRE] / 2) * (get_runes(ch,RUNE_FIRE,LEFTARM) + get_runes(ch,RUNE_FIRE,RIGHTARM)) / 100;


    if ( IS_CLASS(ch,CLASS_MAZOKU) && ch->level >= 2 )
    {
      if ( IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
        dam += dam * 3 * (50+ch->pcdata->powers[M_MATTER]+ch->pcdata->powers[M_FOCUS]) / 120;
      else if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
        dam += dam * 3 * (ch->pcdata->powers[M_MATTER]+ch->pcdata->powers[M_FOCUS]) / 150;
      else if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
        dam += dam * 4 * (ch->pcdata->powers[M_MATTER]+ch->pcdata->powers[M_FOCUS]) / 120;
      else if ( IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
        dam += dam * 3 * (ch->pcdata->powers[M_MATTER]+ch->pcdata->powers[M_FOCUS]) / 170;
      else
        dam += dam * 3 * ch->pcdata->powers[M_MATTER] / 100;
      if ( IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
      {
        if ( IS_NPC(victim) )
          dam += dice( 3, dam/2 );
        else
          dam += dice( 3, dam ) / 3;
      }
      if ( IS_SUIT(victim) )
        dam /= 2;
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
    if ( dt == gsn_backstab )
    {
      dam *= dice( 2, 4 );
      if ( IS_CLASS(victim,CLASS_FIST) && !IS_CLASS(ch,CLASS_FIST) )
        dam *= 3;

      else if ( IS_CLASS(victim,CLASS_MAZOKU) && !IS_CLASS(ch,CLASS_FIST) )
        dam *= 2;
    }

    if ( !IS_NPC(ch) ) 
    { stance = stanced( ch );
      if ( stance )
      {
        if ( IS_CLASS(ch,CLASS_SORCERER) && ch->pcdata->powers[SORC_PREP] > 0 )
          dam += dam * ch->pcdata->stances[stance] * stance_table[stance].damage[0] / 5000;
        else
	  dam += dam * ch->pcdata->stances[stance] * stance_table[stance].damage[0] / 2000 ;
      }
    }
    if ( !IS_NPC(victim) )
    { stance = stanced( victim );	
      if ( stance > 0 && stance <= 10 )
	dam -= dam * victim->pcdata->stances[stance] * stance_table[stance].redux / 3000;
    }

    /* AC damage reduction for pcs */
    if ( !IS_NPC(victim) )
    { if ( victim->armor < 0 )
        dam -= dam * -1 * victim->armor / 1000;
    }

    if ( !IS_NPC(ch) && ch->class == CLASS_SAIYAN )
      if ( is_affected(ch,gsn_kaiouken) )
      { kai = dice( 10, 20 );
	if ( ch->move < kai )
	{ affect_strip(ch,gsn_kaiouken);
	  act( "Exhausted, you cease using the Kaiouken attack.", ch, NULL, NULL, TO_CHAR );
	  act( "The crimson flame encasing $n flickers and vanishes.", ch, NULL, NULL, TO_ROOM );
	}
	else ch->move -= kai;
      }


    if ( dam <= 0 )
	dam = 1;

    damage( ch, victim, dam, dt );

    /* Lightning Ball runeweave check */
    if ( IS_CLASS(ch,CLASS_MAZOKU) && IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE)
      && victim != NULL && !IS_NPC(victim) && victim->level >= 2
      && IS_SET(victim->pcdata->actnew,NEW_BALL_LIGHTNING) )
    {
      damage( victim, ch, dice(18,65), DAM_SHOCKSHIELD );
    }

    tail_chain( );
    return;
}



/*
 * Inflict damage from a hit.
 */
int damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    sh_int mod;
    char log_buf[MAX_STRING_LENGTH];
    int i;

    if ( victim->position == POS_DEAD )
	return -1;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 30000 && !IS_IMMORTAL(ch) )
    {
	bug( "Damage: %d: more than 30000 points!", dam );
	dam = 30000;
    }

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return -1;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return -1;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );

	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	{
	    affect_strip( ch, gsn_invis );
	    affect_strip( ch, gsn_mass_invis );
	    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	    act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
	}
	if ( is_affected(ch,gsn_sneak) )
	  affect_strip( ch, gsn_sneak );

	/*
	 * Damage modifiers.
	 */
	if ( IS_AFFECTED(victim, AFF_SANCTUARY) && !IS_SUIT(victim) )
	  dam /= 2;
	if ( IS_AFFECTED(victim,AFF_STEELY) )
	  dam /= 2;


	if ( dam < 0 )
	    dam = 0;

	/* No lowbies fucking with maxed char fights */
	if ( !IS_NPC(ch) &&
	     (!IS_NPC(victim) || victim->level >= 96) &&
	     ch->pcdata->extras2[EVAL] < 50 )
	{
	  dam = dam * (1 + ch->pcdata->extras2[EVAL]/10) / 6;
	}


	/*
	 * Check for disarm, trip, parry, and dodge.
	 */
	    /*
	    if ( IS_NPC(ch) && number_percent( ) < ch->level / 10 )
		disarm( ch, victim );
	    if ( IS_NPC(ch) && number_percent( ) < ch->level / 2 )
		trip( ch, victim );
        */

	  /* Mystic melee can be dodged  - removed
	  if ( (dt >= TYPE_HIT && dt < TYPE_HIT+15) ||
	       (dt >= 1400 && dt < 1480) )
	  {
	    if ( check_dodge( ch, victim ) )
	      return -1;
	  }
	  */

	  if ( dt >= TYPE_HIT && dt < TYPE_HIT+15 )
	  { 
	    if ( check_parry( ch, victim ) )
		return -1;
	    if ( check_dodge( ch, victim ) )
	        return -1;
	    if ( check_block( ch, victim ) )
		return -1;
  	    if ( check_shields( ch, victim ) )
	        return -1;
	  }

	  if ( dt >= F_SHINKICK && dt <= F_JUMPKICK )
	  {
	    if ( is_affected(victim,gsn_defense) )
	    {
	      act( "Your defense barrier blocks $n's blow.", ch, NULL, victim, TO_VICT );
	      act( "$N's defense barrier blocks your blow.", ch, NULL, victim, TO_CHAR );
	      return TRUE;
	    }
	    if ( !IS_NPC(ch) && stanced(ch) > 0 )
	    {
	      if ( stance_table[stanced(ch)].redux > 2 )
	        dam -= dam * stance_table[stanced(ch)].redux / 20;
	    }

	  }

        /* special damage mods */
        if ( dt == DAM_SHELLS || dt == DAM_BEAMSABRE || dt == DAM_BEAMRIFLE
          || dt == DAM_BEAMSWORD || dt == DAM_BULLETS)
        { if ( IS_NPC(victim) && victim->level < 97 )
            dam*= 2;
          else if ( IS_NPC(victim) && victim->level >= 97 )
            dam /= 3;
        }

        if ( dt == DAM_KIFLAME )
	{ if ( IS_NPC(victim) )
	    dam *= 2;
	  else if ( victim->class == CLASS_SAIYAN && victim->pcdata->powers[S_AEGIS] > 0 )
	      dam -= dam * victim->pcdata->powers[S_AEGIS] / 2100;
          else if ( is_affected(victim,gsn_balus_wall) )
          { 
            dam = 0;
            act( "Your attack is sucked into a flourescent wall.", ch, NULL, victim,TO_CHAR);
            act( "Your Balus Wall absorbs $n's attack and vanishes!",ch,NULL,victim,TO_VICT );
            act( "$n's attack is absorbed by a flourescent wall.",ch,NULL,victim,TO_NOTVICT);
            affect_strip(victim,gsn_balus_wall);
          }
	}

	/* can turtle against melee component of fist combos */
	if ( dt >= 1020 && dt <= 1027 )
	{ if ( !IS_NPC(victim) && stanced(victim) > 0 )
	    dam -= dam * victim->pcdata->stances[stanced(victim)] *
	                 stance_table[stanced(victim)].redux / 5000;
	}


	if ( IS_SUIT(victim) )
	{ if ( dt == DAM_LIGHTNING )
	    dam *= 2;
	  dam -= dam * (suit_table[victim->pcdata->suit[SUIT_NUMBER]].armor - 60) / 140;
	}
             /* damage reduction from toughness */
        else if (!IS_NPC(victim) && victim->level >= 2)
        {
          if ( IS_CLASS(victim,CLASS_SAIYAN) )
          {
            /* Resist to mystickal damage */
            if ( dt >= 1400 && dt < 1480 )
              dam -= dam * victim->pcdata->powers[S_AEGIS] / 3000;

            if ( victim->pcdata->powers[S_AEGIS] > 0 )
            {
              dam -= dam * ( victim->pcdata->powers[S_AEGIS] + number_range(1,300) ) / 1500;

	      if ( dt != DAM_MISSILES || dt != DAM_FIRE || dt != DAM_LIGHTNING )
	      {
	        victim->pcdata->powers[S_AEGIS] =
	          UMAX( victim->pcdata->powers[S_AEGIS_MAX]/2,
	          victim->pcdata->powers[S_AEGIS] - isquare(dam) ); 
	      }
	    }

            if ( is_affected(victim,gsn_kiwall) )
            { 
              if ( dt == F_DEATHTOUCH )
                dam /= 2;
              else if ( dt != DAM_CERULEAN )
                dam /= 3;
            }

            if ( dt == DAM_OBSIDIAN )
            {
              victim->pcdata->powers[S_STRENGTH] = UMAX(
                victim->pcdata->powers[S_STRENGTH] - dam/5,
                victim->pcdata->powers[S_STRENGTH_MAX]/2 );
              victim->pcdata->powers[S_SPEED] = UMAX(
                victim->pcdata->powers[S_SPEED] - dam/5,
                victim->pcdata->powers[S_SPEED_MAX]/2 );
              victim->pcdata->powers[S_AEGIS] = UMAX(
                victim->pcdata->powers[S_AEGIS] - dam/5,
                victim->pcdata->powers[S_AEGIS_MAX]/2 );
            }

          }
          if ( victim->class == CLASS_FIST )
	  { if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	      dam /= 3;
	    dam -= dam * ((number_range(1,15))+(25+victim->pcdata->powers[F_TORSO] / 2)) / 100;

	    /* Kaiouken effect on Ki */
	    if ( IS_CLASS(ch,CLASS_SAIYAN) && is_affected(ch,gsn_kaiouken) )
	    {
	      if ( victim->pcdata->powers[F_KI] > 0 &&
	          !IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	        victim->pcdata->powers[F_KI]--;
	    }
	  }

	  if ( victim->class == CLASS_PATRYN )
	  { 
	    if ( dt == DAM_KIFLAME || dt == DAM_CRIMSON )
	    {
	      if ( get_runes(victim,RUNE_FIRE,TORSO) > 0 )
	        mod = get_runes(victim,RUNE_EARTH,TORSO) +
	              (get_runes(victim,RUNE_FIRE,TORSO)*3/2);
	      else 
	        mod = UMAX( 0, get_runes(victim,RUNE_EARTH,TORSO)/2 );
	    }
	    else if ( dt == DAM_LIGHTNING || dt == F_DEATHTOUCH )
	    {
	      mod = get_runes(victim,RUNE_EARTH,TORSO);
	      if ( get_runes(victim,RUNE_AIR,TORSO) <= 0 )
	        mod /= 2;
	    }
	    else if ( dt == DAM_FIRE )
	    {
	      mod = get_runes(victim,RUNE_EARTH,TORSO);
	      if ( get_runes(victim,RUNE_FIRE,TORSO) <= 0 )
	        mod /= 2;
	    }
	    else
	      mod = get_runes(victim,RUNE_EARTH,TORSO);
	    dam -= ( dam * mod * (4+victim->pcdata->powers[P_EARTH]/20) ) /
	           ( mod * 10 + ( 35 - (mod * 2) ) + dice(1,20) );
	    switch ( dt )
	    {
	      default:
	        break;
	      case DAM_SHOCKWAVE:
	        if ( is_affected( victim, gsn_wind_ward ) )
	        {
	          dam -= 500;
	          dec_duration( victim, gsn_wind_ward, 500 );
	        }
	        break;
	      case DAM_EMERALD:
	        if ( is_affected( victim, gsn_earth_ward ) )
	        {
	          dam -= 200;
	          dec_duration( victim, gsn_earth_ward, 200 );
	        }
	        break;
	      case F_SHINKICK:
	      case F_JAB:
	      case F_SPINKICK:
	      case F_KNEE:
	      case F_ELBOW:
	      case F_UPPERCUT:
                if ( is_affected( victim, gsn_earth_ward ) )
                {
                  dam -= 15;
                  dec_duration( victim, gsn_earth_ward, 60 );
                }
	      	break;
	      /* all melee attack types */
	      case 1000:	case 1001:	case 1002:
	      case 1003:	case 1004:	case 1005:
	      case 1006:	case 1007:	case 1008:
	      case 1009:	case 1010:	case 1011:
	      case 1012:
	        if ( is_affected( victim, gsn_earth_ward ) )
	        {
	          dam -= 15;
	          dec_duration( victim, gsn_earth_ward, 15 );
	        }
	        break;
	      case DAM_KIFLAME:
	      case DAM_CRIMSON:
	        if ( is_affected( victim, gsn_flame_ward ) )
	        {
	          dam -= 400;
	          dec_duration( victim, gsn_flame_ward, 400 );
	        }
	        break;
	      case DAM_CERULEAN:
	        if ( is_affected( victim, gsn_water_ward ) )
	        {
	          dam -= 400;
	          dec_duration( victim, gsn_water_ward, 400 );
	        }
	        break;
	      case F_DEATHTOUCH:
	      case DAM_OBSIDIAN:
	        if ( is_affected( victim, gsn_negative_ward ) )
	        {
	          dam -= 200;
	          dec_duration( victim, gsn_negative_ward, 200 );
	        }
	        break;
	    };

	  }

	  if ( IS_CLASS(victim,CLASS_SORCERER) )
	  {

	    if ( IS_CLASS(ch,CLASS_FIST) )
	    {
	      if ( IS_AFFECTED(ch,AFF_CHAOS_STRING) )
	      {
	        if ( (dt >= TYPE_HIT && dt < TYPE_HIT+15 )
	          || (dt >= F_SHINKICK && dt <= F_JUMPKICK) )
	        {
                  dam /= 2;
                }
              }
            }

	    if ( IS_AFFECTED(victim,AFF_HOLY_RESIST) )
	    {
	      if ( dt == DAM_FIRE || dt == DAM_LIGHTNING || dt == DAM_NEGATIVE )
	        dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]) / 150;
	      if ( dt == DAM_OBSIDIAN || dt == DAM_CRIMSON || dt == DAM_EMERALD )
	        dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]) / 100;
	      if ( dt == DAM_CERULEAN )
	        dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]) / 150;
	      if ( dt == F_DEATHTOUCH )
	        dam -= dam / 2;
	    }

            if ( is_affected(victim,gsn_mos_varim) )
            { if ( dt == DAM_KIFLAME || dt == DAM_FIRE )
                dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]-20) / 35;
              /* resistance to mobile suit attacks */
              if ( dt == DAM_BEAMRIFLE || dt == DAM_BEAMSABRE )
              { dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]-25) / 50;
                if ( dam <= 0 )
                  dam = 1;
              }
            }

	    if ( IS_AFFECTED(victim,AFF_VAS_GLUUDO) )
	    { if  ( dt == DAM_KIFLAME || (dt >= 130 && dt < 140) 
	         || dt == DAM_SHOCKWAVE )
	        dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]-20) / 35;
	      if ( dt == DAM_BEAMRIFLE || dt == DAM_BEAMSABRE || dt == DAM_BEAMSWORD )
	        dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE] - 10) / 60;
	      if ( dt == DAM_SHOCKSHIELD )
	        dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]-20) / 60;
	    }
	  }

	  if ( IS_CLASS(victim,CLASS_MAZOKU) )
	  { 
	    if ( dt == DAM_KIFLAME )
	    {
	      dam += dam / 3;
	      dam -= dam * victim->pcdata->powers[M_NIHILISM] / 400;
	    }
	    if ( IS_SET(victim->pcdata->powers[M_SET],M_HUMAN) )
	      dam -= dam * victim->pcdata->powers[M_MATTER] / 130;
	    else if ( IS_SET(victim->pcdata->powers[M_SET],M_BATTLE) )
	      dam -= dam * (victim->pcdata->powers[M_MATTER] + 15) / 135;
	    else if ( IS_SET(victim->pcdata->powers[M_SET],M_TRUE) )
	      dam -= dam * victim->pcdata->powers[M_ASTRAL] / 140;
	  }


	}
        /* NPC's get damage reduction by level */
        else
        {
          dam -= dam * victim->level / 200;
          if ( victim->level > 95 )
            dam -= dam * (victim->level - 95) * 10 / 100;

          /* No shelling down boss mobs */
          if ( IS_SET(victim->act,ACT_NO_RANGED) &&
                ch->in_room != victim->in_room )
            dam = 0;
        }

    /*	dam_message( ch, victim, dam, dt ); */
    }

    if ( dam < 0 )
      dam = 0;

    timer_check( ch, victim );
    dam_message( ch, victim, dam, dt );
    essense_gain( ch, victim, dam, dt );

    if ( victim->position >= POS_STUNNED )
      add_inf( ch, victim, dam );

    if ( IS_SUIT(victim) && dt != DAM_EXPLOSION )
      return suit_damage( victim, dam );

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    /* Mazoku nihilism gain */
    if ( IS_CLASS(victim,CLASS_MAZOKU) && dam > victim->max_hit/10 && victim != ch )
    {
      if ( victim->pcdata->powers[M_NIHILISM] < 100 )
        victim->pcdata->powers[M_NIHILISM]++;
    }
    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	if ( IS_CLASS(victim,CLASS_SAIYAN) )
	{
	  if ( IS_SET(victim->pcdata->actnew,NEW_KAME_1) )
	    REMOVE_BIT(victim->pcdata->actnew,NEW_KAME_1);
	  if ( IS_SET(victim->pcdata->actnew,NEW_KAME_2) )
	    REMOVE_BIT(victim->pcdata->actnew,NEW_KAME_2);
	  if ( IS_SET(victim->pcdata->actnew,NEW_MASENKOUHA) )
	    REMOVE_BIT(victim->pcdata->actnew,NEW_MASENKOUHA);
	}
	else if ( IS_CLASS(victim,CLASS_SORCERER) )
	{
	  while ( victim->pcdata->chant != NULL )
	    lose_chant( victim );
	}
	break;

	if ( IS_NPC(ch) && !IS_NPC(victim) && IS_SET(ch->act,ACT_FINISH) )
	  do_finish( ch, victim->name );

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );
    
    if ( !IS_NPC(victim) && victim->position < POS_STUNNED )
    {
	stop_fighting( ch, FALSE );
	stop_fighting( victim, TRUE );
	if ( !IS_NPC(ch) )
	{
	  sprintf( log_buf, "%s morted by %s at %d from %d.",
		victim->name, ch->name, victim->in_room->vnum, ch->in_room->vnum );
	  log_string( log_buf );
	}

	if ( IS_NPC(ch) && victim->position == POS_MORTAL && IS_SET(ch->act,ACT_FINISH) )
	  do_finish( ch, victim->name );
    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	if ( !IS_NPC(ch) )
	{ ch->pcdata->kills[MK]++;
	  clan_table[ch->pcdata->clan[CLAN]].mobkills++;
	}
	if ( !IS_NPC(victim) )
	{ victim->pcdata->kills[MD]++;
	  clan_table[victim->pcdata->clan[CLAN]].mobdeaths++;
	}

	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     * taken out
	    if ( victim->exp > 1000 * victim->level )
		gain_exp( victim, (1000 * victim->level - victim->exp)/2 );
	     */
	}

	check_quest( ch, victim );
	if ( IS_NPC(victim) && victim->level >= 50 )
	  rand_obj( victim );
	raw_kill( victim, FALSE );

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) )
		do_get( ch, "all corpse" );
	    else
		do_look( ch, "in corpse" );

	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
		do_sacrifice( ch, "corpse" );
	}

	return dam;
    }

    if ( victim == ch )
	return -1;

    /*
     * Take care of link dead people.
     * Fucking abusable
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return;
	}
    } fuckin' pansies */

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 )
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
	&&   victim->hit < victim->max_hit / 2 )
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait == 0 )
      send_to_char( "You're a fucking PANSY ASS BITCH!!", victim );

    /* Clumsy positioning necessary because of parry/dodge */
    if ( dam > 0 && dt == TYPE_HIT && ch && victim && IS_CLASS(ch,CLASS_FIST) && IS_NPC(victim) )
    {
      if ( is_affected(ch,gsn_golden_blaze) )
        damage( ch, victim, 400 + number_percent(), DAM_KIFLAME );
      if ( is_affected(ch,gsn_judicators_ire) && victim )
        damage( ch, victim, 400 + number_percent(), DAM_SHOCKWAVE );
      if ( is_affected(ch,gsn_dark_blaze) && number_percent() < 50 && victim ) 
      {
        i = victim->hit;
        damage( ch, victim, 200 + number_percent(), DAM_NEGATIVE );
        if ( victim && victim->position > POS_STUNNED )
        {
          i -= victim->hit;
          ch->hit = UMIN(ch->max_hit,ch->hit+i);
          ch->move = UMIN(ch->max_move,ch->move+i);
        }
      }
    }

    tail_chain( );
    return dam;
}

/*
 * Damage function for sorcerer chants
 */
void chant_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
  int mod, rank, school;

    if ( victim->position == POS_DEAD )
	return;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 30000 )
    {
	bug( "Damage: %d: more than 30000 points!", dam );
	dam = 30000;
    }

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );

	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	{
	    affect_strip( ch, gsn_invis );
	    affect_strip( ch, gsn_mass_invis );
	    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	    act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
	}

	/* no attacking avatars, bitch */
	if ( !IS_NPC(victim) && victim->level < 2 )
	{ send_to_char( "Don't fuck with mortals.\n\r", ch );
	  return;
	}

        /* I am no longer willing to tolerate this pussy trash. */
        if ( !str_cmp( ch->name, "Karth" ) )
          dam -= dam / 15;

        rank = chant_table[dt].rank;
        school = chant_table[dt].school;

	/*
	 * Check for curses being removed.
	 */
	if ( school == SCHOOL_WIND )
	{
	  if ( is_affected(ch,gsn_wind_curse) )
	    affect_strip(ch,gsn_wind_curse);
	  if ( is_affected(ch,gsn_earth_curse) && is_affected(ch,gsn_flame_curse) )
	  {
	    act( "Your spell fizzles and dies!", ch, NULL, NULL, TO_CHAR );
	    act( "$n's spell fizzles and dies.", ch, NULL, NULL, TO_ROOM );
	    return;
	  }
	}
        if ( school == SCHOOL_EARTH )
        {
          if ( is_affected(ch,gsn_earth_curse) )
            affect_strip(ch,gsn_earth_curse);
          if ( is_affected(ch,gsn_wind_curse) && is_affected(ch,gsn_water_curse) )
          {
            act( "Your spell fizzles and dies!", ch, NULL, NULL, TO_CHAR );
            act( "$n's spell fizzles and dies.", ch, NULL, NULL, TO_ROOM );
            return;
          }
        }
        if ( school == SCHOOL_FIRE )
        {
          if ( is_affected(ch,gsn_flame_curse) )
            affect_strip(ch,gsn_flame_curse);
          if ( is_affected(ch,gsn_water_curse) && is_affected(ch,gsn_earth_curse) )
          {
            act( "Your spell fizzles and dies!", ch, NULL, NULL, TO_CHAR );
            act( "$n's spell fizzles and dies.", ch, NULL, NULL, TO_ROOM );
            return;
          }
        }
        if ( school == SCHOOL_WATER )
        {
          if ( is_affected(ch,gsn_water_curse) )
            affect_strip(ch,gsn_water_curse);
          if ( is_affected(ch,gsn_flame_curse) && is_affected(ch,gsn_wind_curse) )
          {
            act( "Your spell fizzles and dies!", ch, NULL, NULL, TO_CHAR );
            act( "$n's spell fizzles and dies.", ch, NULL, NULL, TO_ROOM );
            return;
          }
        }
 	if ( school == SCHOOL_BLACK )
 	{
 	  if ( is_affected(ch,gsn_flame_curse) && is_affected(ch,gsn_water_curse) )
 	  {
 	    act( "Your spell fizzles and dies!", ch, NULL, NULL, TO_CHAR );
 	    act( "$n's spell fizzles and dies.", ch, NULL, NULL, TO_ROOM );
 	    return;
 	  }
 	}
        if ( school == SCHOOL_ASTRAL )
        {
          if ( is_affected(ch,gsn_earth_curse) && is_affected(ch,gsn_wind_curse) )
          {
            act( "Your spell fizzles and dies!", ch, NULL, NULL, TO_CHAR );
            act( "$n's spell fizzles and dies.", ch, NULL, NULL, TO_ROOM );
            return;
          }
        }

	/*
	 * Damage modifiers.
	 */
	if ( IS_AFFECTED(victim, AFF_SANCTUARY) && !IS_SUIT(victim) )
	    dam /= 2;



        /* No lowbies fucking with maxed char fights */
        if ( !IS_NPC(ch) &&
             (!IS_NPC(victim) || victim->level >= 96) &&
             ch->pcdata->extras2[EVAL] < 50 )
        {
          dam = dam * (1 + ch->pcdata->extras2[EVAL]/10) / 6;
        }


        /* damage reduction from toughness */
	if ( IS_AFFECTED(victim,AFF_STEELY) )
 	  dam = dam / 5;

	if ( IS_NPC(victim) )
        { dam -= dam * victim->level / (300 + rank*2);
          if ( victim->level > 95 )
            dam -= dam * (victim->level - 95) * 10 / 100;
        }
        else if (IS_SUIT(victim) )
	{ if ( school == SCHOOL_WIND )
	    dam *= 2;
	  dam -= dam * (suit_table[victim->pcdata->suit[SUIT_NUMBER]].armor - 60) / 160;
	}
	else
        {
          if ( victim->class == CLASS_SAIYAN )
          { if ( is_affected(victim,gsn_kiwall) )
              dam /= 2;
            if ( victim->pcdata->powers[S_AEGIS] > 0 )
            { dam -= (dam * victim->pcdata->powers[S_AEGIS]+dice(1,300)) / (1200+rank*4);
              victim->pcdata->powers[S_AEGIS] = UMAX( victim->pcdata->powers[S_AEGIS_MAX] / 2, victim->pcdata->powers[S_AEGIS] - isquare(dam) );
            }
          }
          else if ( victim->class == CLASS_FIST )
          { if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
              dam /= 2;
            if ( victim->pcdata->powers[F_TORSO] > 0 )
              dam -= dam * ( dice(1,10) + victim->pcdata->powers[F_TORSO] ) / 127;
          }
          else if ( IS_CLASS(victim,CLASS_PATRYN) )
          { 

            if ( get_runes(victim,RUNE_FIRE,TORSO) > 0 )
              mod = get_runes(victim,RUNE_EARTH,TORSO);
            else
              mod = get_runes(victim,RUNE_EARTH,TORSO)/2;
            dam -= (dam * mod * victim->pcdata->powers[P_AIR]) /
                   ((mod+4) * 100+(rank*5));

            switch( school )
            {
              case SCHOOL_WIND:
                if ( is_affected( victim, gsn_water_ward ) )
                {
                  dam = UMAX( dam - 100, 1 );
                  dec_duration( victim, gsn_water_ward, 100 );
                }
                if ((mod = get_runes(victim,RUNE_WATER,TORSO)) > 0 )
                {
                  if ( is_affected( victim, gsn_water_ward ) )
                    dam += dam * mod / 6;
                  else
                    dam += dam * mod / 3;
                }
                break;
              case SCHOOL_EARTH:
                if ( is_affected( victim, gsn_earth_ward ) )
                {
                  dam = UMAX( dam - 100, 1 );
                  dec_duration( victim, gsn_earth_ward, 100 );
                }
                break;
              case SCHOOL_FIRE:
                if ( is_affected( victim, gsn_flame_ward ) )
                {
                  dam = UMAX( dam - 100, 1 );
                  dec_duration( victim, gsn_flame_ward, 100 );
                }
                break;
              case SCHOOL_WATER:
                if ( is_affected( victim, gsn_water_ward ) )
                {
                  dam = UMAX( dam - 100, 1 );
                  dec_duration( victim, gsn_water_ward, 100 );
                }
                break;
	      case SCHOOL_ASTRAL:
	        if ( is_affected( victim, gsn_spirit_ward ) )
	        {
                  dam = UMAX( dam - 100, 1 );
	          dec_duration( victim, gsn_spirit_ward, 100 );
	        }
	        break;
	      case 2:
	        if ( is_affected( victim, gsn_negative_ward ) )
	        {
                  dam = UMAX( dam - 100, 1 );
	          dec_duration( victim, gsn_negative_ward, 100 );
	        }
	        break;
	      default:
	        break;
	    };


          }

          if ( victim->class == CLASS_SORCERER )
          {
            if ( IS_AFFECTED(victim,AFF_HOLY_RESIST) )
              dam -= dam * 2 * (victim->pcdata->powers[SCHOOL_WHITE]-20) / 100;

            if ( is_affected(victim,gsn_mos_varim) && chant_table[dt].school == SCHOOL_FIRE)
              dam -= dam * (victim->pcdata->powers[SCHOOL_WHITE]-20) / 35;

            if ( is_affected(victim,gsn_balus_wall) && school == SCHOOL_FIRE )
            {
              dam = 0;
              act( "Your attack is sucked into a flourescent wall.", ch, NULL, victim, TO_CHAR );
              act( "Your Balus Wall absorbs $n's attack and vanishes!",ch,NULL,victim,TO_VICT );
              act( "$n's attack is absorbed by a flourescent wall.",ch,NULL,victim,TO_NOTVICT);
              affect_strip(victim,gsn_balus_wall);
            }
          }

          if ( IS_CLASS(victim,CLASS_MAZOKU) )
          {
            if ( school == SCHOOL_WHITE )
              dam += dam / 2;
            if ( IS_SET(victim->pcdata->powers[M_SET],M_HUMAN) )
              dam -= dam * (victim->pcdata->powers[M_MATTER]+12) / 135;
            else if ( IS_SET(victim->pcdata->powers[M_SET],M_BATTLE) )
              dam -= dam * victim->pcdata->powers[M_MATTER] / 135;
            else
              dam -= dam * victim->pcdata->powers[M_ASTRAL] / 150;

            if ( victim->pcdata->powers[M_NIHILISM] > 0 )
              dam -= dam * victim->pcdata->powers[M_NIHILISM] / 400;

            if ( victim->pcdata->powers[M_CTIME] >= 10 )
              victim->pcdata->powers[M_CTIME] -= dam/250;
          }

        }     

    }

    if ( dam < 0 )
      dam = 0;

    timer_check( ch, victim );
    add_inf( ch, victim, dam );
    dam_message( ch, victim, dam, (dt+1400) );

    if ( IS_SUIT(victim) )
    { suit_damage( victim, dam );
      return;
    }
    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    if ( IS_CLASS(victim,CLASS_MAZOKU) && dam > victim->max_hit/10 )
    {
      if ( victim->pcdata->powers[M_NIHILISM] < 100 )
        victim->pcdata->powers[M_NIHILISM]++;
    }

    /* End combat on morting or killing */
    if ( victim->position <= POS_STUNNED )
      stop_fighting(ch,FALSE);

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	if ( IS_CLASS(victim,CLASS_SAIYAN) )
	{
	  if ( IS_SET(victim->pcdata->actnew,NEW_KAME_1) )
	    REMOVE_BIT(victim->pcdata->actnew,NEW_KAME_1);
	  if ( IS_SET(victim->pcdata->actnew,NEW_KAME_2) )
	    REMOVE_BIT(victim->pcdata->actnew,NEW_KAME_2);
	  if ( IS_SET(victim->pcdata->actnew,NEW_MASENKOUHA) )
	    REMOVE_BIT(victim->pcdata->actnew,NEW_MASENKOUHA);
	}
        else if ( IS_CLASS(victim,CLASS_SORCERER) )
        {
          while ( victim->pcdata->chant != NULL )
            lose_chant( victim );
        }
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /* 
    if ( IS_NPC(victim) || !morted )
      stop_fighting( ch, FALSE );
    */

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	if ( !IS_NPC(ch) )
	{ ch->pcdata->kills[MK]++;
	  clan_table[ch->pcdata->clan[CLAN]].mobkills++;
	}
	if ( !IS_NPC(victim) )
	{ victim->pcdata->kills[MD]++;
	  clan_table[victim->pcdata->clan[CLAN]].mobdeaths++;
	}

	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	}

	check_quest( ch, victim );
	raw_kill( victim, FALSE );

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) )
		do_get( ch, "all corpse" );
	    else
		do_look( ch, "in corpse" );

	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
		do_sacrifice( ch, "corpse" );
	}

	return;
    }

    if ( victim == ch )
	return;

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 )
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
	&&   victim->hit < victim->max_hit / 4 )
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    /* no pussy assed wimpy
    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait == 0 )
	do_flee( victim, "" );
	*/

    tail_chain( );
    return;
}



bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;

    if ( IS_SET( victim->act, PLR_KILLER ) )
        return FALSE;

    if ( ch->level < 2 )
    {
	send_to_char( "Only avatars can kill players.\n\r", ch );
	return TRUE;
    }

    if ( victim->level < 2 )
    {
	send_to_char( "But they're not an avatar!", ch );
	return TRUE;
    }

    return FALSE;
}




/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *chw;
    int stance, chdt, suit, reflex;
    int chance = 0;
    int chstance = 0;
    int runes = 0;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chw = get_eq_char( ch, WEAR_WIELD );
    if ( chw == NULL )
      chdt = 0;
    else chdt = chw->value[3];

    chstance = stanced( ch );

    if ( IS_NPC(victim) )
    {
	chance += victim->level * 3 / 2;
        if ( IS_AFFECTED(ch,AFF_VISFARANK) )
          return FALSE;
    }
    else if ( IS_SUIT(victim) )
    { if ( !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_BUCKLER) 	&&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_HEATER)	&&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_BEAM_SABRE) &&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_CRASHER)    &&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_BEAM_SCYTHE)&&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_BEAM_GLAIVE)&&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_HEAT_SHOTELS)&&
    	   !IS_SET(victim->pcdata->suit[SUIT_EQ],MS_BEAM_SWORD) )
        return FALSE;
      suit = victim->pcdata->suit[SUIT_NUMBER];
      reflex = victim->pcdata->suit[SUIT_REFLEX];
      chance += ((suit_table[suit].power -80)*2/3) + isquare(reflex);
      if ( IS_SET(victim->pcdata->suit[SUIT_EQ],MS_BUCKLER) ||
           IS_SET(victim->pcdata->suit[SUIT_EQ],MS_CRASHER) )
        chance += 15;
      else if ( IS_SET(victim->pcdata->suit[SUIT_EQ],MS_HEATER) )
        chance += 30;
      if ( dice(1, 1+reflex+isquare(reflex)) > reflex )
        if ( reflex < 2500 && reflex < (victim->pcdata->extras2[EVAL]*45) )
          victim->pcdata->suit[SUIT_REFLEX]++;
    }
    else
    {
	stance = stanced( victim );
	if ( stance )
	  chance += ( victim->pcdata->stances[stance] / 40 ) *
		      ( stance_table[stance].parry[chstance] );
	if ( get_eq_char( victim, WEAR_WIELD ) == NULL &&
	     ( !(IS_CLASS(victim,CLASS_SAIYAN) && victim->pcdata->powers[S_AEGIS] >= 250)
	     && !IS_CLASS(victim,CLASS_MAZOKU) ) )
	    return FALSE;
	chance += victim->pcdata->weapons[chdt] / 5;

	if ( victim->class == CLASS_PATRYN )
	{
	  runes = get_runes(victim,RUNE_EARTH,LEFTARM)+get_runes(victim,RUNE_EARTH,RIGHTARM);
	  chance += runes * 8;
	  chance += victim->pcdata->powers[P_EARTH] * 2 / 5;
	}
        if ( victim->class == CLASS_FIST )
          chance /= 2;
        if ( IS_CLASS(victim,CLASS_MAZOKU) )
        { if ( IS_SET(victim->pcdata->powers[M_SET],M_THIRD) )
            chance += 5;
          if ( IS_SET(victim->pcdata->powers[M_SET],M_FOURTH) )
            chance += 5;
          if ( IS_SET(victim->pcdata->powers[M_SET],M_FIFTH) )
            chance += 5;
          if ( IS_SET(victim->pcdata->powers[M_SET],M_SIXTH) )
            chance += 5;
          else if ( IS_SET(victim->pcdata->powers[M_SET],M_HUMAN) )
            chance += 20;
          if ( IS_SET(victim->pcdata->powers[M_SET],M_BLADES) )
            chance += 20;
          else if ( IS_SET(victim->pcdata->powers[M_SET],M_TENTACLES) )
            chance += 30;
          else if ( IS_SET(victim->pcdata->powers[M_SET],M_SPIKES) )
            chance += 15;
          else if ( IS_SET(victim->pcdata->powers[M_SET],M_CLAWS) )
            chance += 10;
          chance -= victim->pcdata->powers[M_NIHILISM]/2;
          chance += UMIN(100,victim->pcdata->powers[M_EGO]) / 2;
        }
    }

    if ( IS_NPC(ch) )
    {
	chance -= victim->level * 3 / 2;
    }
    else
    {	
	chance -= ch->pcdata->weapons[chdt] / 5;
	if ( ch->class == CLASS_SAIYAN )
	{ chance -= isquare( ch->pcdata->powers[S_SPEED] );
	  if ( is_affected(ch,gsn_kaiouken) )
	    return FALSE;
	}
	if ( ch->class == CLASS_PATRYN )
	{ runes = get_runes(ch,RUNE_EARTH,LEFTLEG) + get_runes(ch,RUNE_EARTH,RIGHTLEG);
    	  chance -= runes * UMIN(13,21-runes);
    	}
    	if ( IS_CLASS(ch,CLASS_MAZOKU) )
    	{ chance -= ch->pcdata->powers[M_NIHILISM]/3;
    	  if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
    	    chance -= ch->pcdata->powers[M_FOCUS]/2+20;
    	  if ( IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
    	    chance -= ch->pcdata->powers[M_FOCUS]/3;
    	  if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
    	    chance -= ch->pcdata->powers[M_FOCUS]/3;
    	  if ( IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
    	    chance /= 2;
    	}

    	if ( IS_WIELDING(ch,LAGUNA_BLADE) )
    	  return FALSE;
    	if ( IS_SUIT(ch) )
	  return FALSE;
    }
    
    chance = URANGE( 20, chance, 80 );
    if ( IS_NPC(ch) && ch->level > 95 )
      chance -= (ch->level - 95) * 5;
    if ( number_percent( ) >= chance )
	return FALSE;

    if ( !IS_SET( victim->act, PLR_BRIEF ) && !IS_NPC(victim) )
    act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    if ( !IS_SET( ch->act, PLR_BRIEF ) && !IS_NPC(ch) )
    act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance = 0;
    int suit, reflex, runes;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
    {
      chance  += victim->level * 3 / 2;
      if ( IS_AFFECTED(ch,AFF_VISFARANK) )
        chance /= 2;
    }
    else if ( IS_SUIT(victim) )
    { suit = victim->pcdata->suit[SUIT_NUMBER];
      reflex = victim->pcdata->suit[SUIT_REFLEX];
      chance += (suit_table[suit].speed - 80) + isquare(reflex);
      if ( dice(1, 1+reflex+isquare(reflex)) > reflex )
        if ( reflex < 2500 && reflex < (victim->pcdata->extras2[EVAL]*45) )
          victim->pcdata->suit[SUIT_REFLEX]++;
    }
    else
    {
        if ( ch->level >= 2 )
          chance += victim->pcdata->weapons[0] / 5;
        else
          chance += UMIN( victim->pcdata->weapons[0], 200 ) / 5;
	if ( victim->class == CLASS_SAIYAN )
	{
	  chance += victim->pcdata->powers[S_SPEED] / 7;
	  if ( is_affected(victim,gsn_kaiouken) )
	    chance *= 2;
	}
	if ( victim->class == CLASS_PATRYN )
	{ runes = get_runes(victim,RUNE_WATER,LEFTLEG);
	  runes += get_runes(victim,RUNE_WATER,RIGHTLEG);
	  if ( runes >= 8 )
	    chance += 90;
	  else
	    chance += runes*12;
	  chance += UMAX(runes-8,1) * 5;
	}
	if ( victim->class == CLASS_SORCERER )
	  if ( IS_AFFECTED(victim,AFF_RAYWING) )
	    chance += (victim->pcdata->powers[SCHOOL_WIND]-7)*2;
        if ( IS_CLASS(victim,CLASS_MAZOKU) )
        { if ( IS_SET(victim->pcdata->powers[M_SET],M_BATTLE) )
            chance += victim->pcdata->powers[M_MATTER]*3/4;
          if ( IS_SET(victim->pcdata->powers[M_SET],M_HUMAN) )
            chance += victim->pcdata->powers[M_MATTER]/2;
        }
    }

    if ( IS_NPC(ch) )
	chance -= ch->level * 3 / 2;
    else
    {
    	chance -= ch->pcdata->weapons[0] / 5;
	if ( ch->class == CLASS_SAIYAN )
	{ if ( is_affected(ch,gsn_kaiouken) )
	    chance -= ch->pcdata->powers[S_SPEED] / 10;
	  else
	    chance -= ch->pcdata->powers[S_SPEED] / 15;
	}

	if ( ch->class == CLASS_PATRYN )
	{ runes = get_runes(ch,RUNE_EARTH,LEFTLEG);
	  runes += get_runes(ch,RUNE_EARTH,RIGHTLEG);
          if ( runes <= 9 )
            chance -= runes * 9;
          else
            chance -= runes * 7;
	}

        if ( IS_CLASS(ch,CLASS_MAZOKU) )
        { chance -= ch->pcdata->powers[M_NIHILISM]/3;
          if ( IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
            chance -= ch->pcdata->powers[M_FOCUS]/2;
          if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
            chance -= ch->pcdata->powers[M_FOCUS]/2+20;
          if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
            chance -= ch->pcdata->powers[M_FOCUS]/3;
          if ( IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
            chance -= (ch->pcdata->powers[M_FOCUS]+ch->pcdata->powers[M_MATTER])/3;
          if ( IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
          {
            if ( IS_CLASS(victim,CLASS_MAZOKU) &&
                 IS_SET(victim->pcdata->powers[M_SET],M_TRUE) )
            {
              chance = 80;
            }
            else
              chance -= chance/4;
          }
        }


    	if ( IS_WIELDING(ch,LAGUNA_BLADE) )
    	  chance /= 2;
        if ( IS_SUIT(ch) )
    	  return FALSE;
    }

    chance = URANGE( 20, chance, 80 );
    if ( IS_NPC(ch) && ch->level > 95 )
      chance -= (ch->level - 95 ) * 5;
    if ( number_percent( ) >= chance )
        return FALSE;

    if ( !IS_SET( victim->act, PLR_BRIEF ) && !IS_NPC(victim) )
    act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    if ( !IS_SET( ch->act, PLR_BRIEF ) && !IS_NPC(ch) )
    act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    return TRUE;
}

/*
 * Monk defensive skill
 */
bool check_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int chance = 0;
  if ( IS_NPC(victim) )
    return FALSE;
  if ( victim->class != CLASS_FIST )
    return FALSE;
  if ( victim->level < 2 )
    return FALSE;

  if ( victim->position < POS_STUNNED )
    return FALSE;

  if ( IS_SUIT(victim) )
    return FALSE;

  if ( get_eq_char( victim, WEAR_WIELD ) != NULL )
    return FALSE;

  chance += (victim->pcdata->powers[F_ARMS] / 2) + (victim->pcdata->weapons[0] / 10);

  if ( IS_NPC(ch) )
    chance -= ch->level / 2;
  else
  { if ( ch->class == CLASS_SAIYAN )
    { chance -= ch->pcdata->powers[S_SPEED] / 40;
      if ( is_affected(ch,gsn_kaiouken) )
	return FALSE;
    }
    if ( ch->class == CLASS_FIST )
      chance -= ch->pcdata->powers[F_ARMS] * 3 / 8;
    if ( ch->class == CLASS_PATRYN )
      chance -= 8 * (get_runes(ch,RUNE_EARTH,LEFTLEG)+get_runes(ch,RUNE_EARTH,RIGHTLEG));
    if ( IS_CLASS(ch,CLASS_MAZOKU) )
    { if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
        chance -= ch->pcdata->powers[M_MATTER]*2/3;
      if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) ||
           IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
        chance -= ch->pcdata->powers[M_MATTER]/2;
      if ( IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
        chance /= 2;
    }
    if( IS_WIELDING(ch,LAGUNA_BLADE) )
      chance /= 3;
    if ( IS_SUIT(ch) )
      return FALSE;
  }
  
  if (IS_SET(victim->pcdata->actnew,NEW_NOBLOCK) )
    return FALSE;

  chance = URANGE( 25, chance, 85 );
  if ( IS_NPC(ch) && ch->level > 95 )
    chance -= (ch->level - 95) * 5;
  if ( number_percent() >= chance )
    return FALSE;

  if ( !IS_SET( victim->act, PLR_BRIEF ) && !IS_NPC(victim) )
    act( "You block $n's attack.", ch, NULL, victim, TO_VICT );
  if ( !IS_SET( ch->act, PLR_BRIEF ) && !IS_NPC(ch) )
    act( "$N blocks your attack.", ch, NULL, victim, TO_CHAR );
  return TRUE;
}

bool check_shields( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int num;

  if ( IS_AFFECTED(victim,AFF_STEELY) && number_percent() > 20 )
  {
    if ( !IS_SET(ch->act,PLR_BRIEF) )
      act( "Your blow skitters off $N's incredible battle aura.", ch, NULL, victim, TO_CHAR );
    if ( !IS_SET(victim->act,PLR_BRIEF) )
      act( "$n's blow skitters off your battle aura.", ch, NULL, victim, TO_VICT );
    return TRUE;
  }
  if ( IS_NPC(victim) )
    return FALSE;

  if ( IS_SUIT(victim) )
  { if ( IS_SET(victim->pcdata->suit[SUIT_EQ],MS_SHIELDS) )
    { if ( number_percent() < 50 && victim->pcdata->suit[SUIT_PLASMA] > 0 )
      { victim->pcdata->suit[SUIT_PLASMA]--;
        if ( !IS_SET(ch->act,PLR_BRIEF) )
          act( "$N's planet shields block your attack.", ch, NULL, victim, TO_CHAR );
        if ( !IS_SET(victim->act,PLR_BRIEF) )
          act( "Your planet shields block $n's attack.", ch, NULL, victim, TO_VICT );
        return TRUE;
      }
    }

    return FALSE;
  }

  if ( IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
  { if ( number_percent() < 50 )
    { if ( !IS_SET(ch->act,PLR_BRIEF) )
        act( "Your attack slides off an icy barrier.", ch, NULL, victim, TO_CHAR );
      if ( !IS_SET(victim->act,PLR_BRIEF) )
        act( "$n's attack slides off your icy barrier.", ch, NULL, victim, TO_VICT );
    }
  }

  if ( IS_AFFECTED(victim,AFF_WINDY_SHIELD) )
  {
    num = 30 + (victim->pcdata->powers[SCHOOL_WIND]-40);
    if ( IS_CLASS(ch,CLASS_SAIYAN) && is_affected(ch,gsn_kaiouken) )
      num -= num / 4;
    if ( number_percent() < num )
    { act( "Your windy shield deflects $n's attack.", ch, NULL, victim, TO_VICT    );
      act( "$N's windy shield deflects your attack.", ch, NULL, victim, TO_CHAR    );
      return TRUE;
    }
  }

  if ( is_affected(victim,gsn_defense) )
  {
    if ( IS_NPC(ch) && ch->level >= 95 )
    {
      act( "Your defense barrier shatters!", ch, NULL, victim, TO_VICT );
      act( "$N's defense barrier shatters!", ch, NULL, victim, TO_CHAR );
      affect_strip(victim,gsn_defense);
      return FALSE;
    }
    act( "Your defense barrier blocks $n's blow.", ch, NULL, victim, TO_VICT );
    act( "$N's defense barrier blocks your blow.", ch, NULL, victim, TO_CHAR );
    return TRUE;
  }
  return FALSE;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) || (victim->hit <= -10 && victim->level < 2) )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit < -10 )
	victim->hit = -10;

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, bool pdox )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 2, 4 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 50, 80 );
	corpse->pIndexData->value[0] = 0;
    }

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	/*
	 * If item is claimed, or is non-unique on a pdox cap,
	 * item stays with the char, unequipped.  Otherwise,
	 * send item to corpse.
	 */
	if ( (!IS_NPC(ch) && is_name( obj->owner, ch->name ))
	  || (pdox && !IS_OBJ_STAT(obj,ITEM_UNIQUE)) )
	{
	  if ( obj->wear_loc != -1 )
	    unequip_char( ch, obj );
	}
	else
	{
	  obj_from_char( obj );
	  if ( IS_SET(obj->extra_flags,ITEM_INVENTORY) )
	    extract_obj( obj );
	  else
	    obj_to_obj( obj, corpse );
	}

    }

    obj_to_room( corpse, ch->in_room );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    switch ( number_bits( 4 ) )
    {
    default: msg  = "You hear $n's death cry.";				break;
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: msg  = "$n splatters blood on your armor.";		break;
    case  2: msg  = "You smell $n's sphincter releasing in death.";
	     vnum = OBJ_VNUM_FINAL_TURD;				break;
    case  3: msg  = "$n's severed head plops on the ground.";
	     vnum = OBJ_VNUM_SEVERED_HEAD;				break;
    case  4: msg  = "$n's heart is torn from $s chest.";
	     vnum = OBJ_VNUM_TORN_HEART;				break;
    case  5: msg  = "$n's arm is sliced from $s dead body.";
	     vnum = OBJ_VNUM_SLICED_ARM;				break;
    case  6: msg  = "$n's leg is sliced from $s dead body.";
	     vnum = OBJ_VNUM_SLICED_LEG;				break;
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->to_room != NULL
	&&   pexit->to_room != was_in_room )
	{
	    ch->in_room = pexit->to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *victim, bool pdox )
{
    stop_fighting( victim, TRUE );
    mprog_death_trigger( victim );
    make_corpse( victim, pdox );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= 0;
    victim->armor	= 100;
    victim->position	= POS_RESTING;
    /* not sure whats up with this 
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    */
    victim->hit		= 1;
    victim->mana	= 1;
    victim->move	= 1;
    if ( !IS_NPC(victim) && victim->class == CLASS_SAIYAN )
    { victim->pcdata->powers[S_POWER] = 5;
      victim->pcdata->powers[S_SPEED] = victim->pcdata->powers[S_SPEED_MAX]/2;
      victim->pcdata->powers[S_STRENGTH] = victim->pcdata->powers[S_STRENGTH_MAX]/2;
      victim->pcdata->powers[S_AEGIS] = victim->pcdata->powers[S_AEGIS_MAX]/2;
    }
    if ( IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_DRAGON_SLAVE) )
      REMOVE_BIT(victim->pcdata->actnew,NEW_DRAGON_SLAVE);

    save_char_obj( victim );
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;

    /*
     * Monsters don't get kill xp's.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( IS_NPC(ch) || !IS_NPC(victim) || victim == ch )
	return;
    
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {

	if ( !is_same_group( gch, ch ) )
	    continue;

	/* unnecessary in levelless system
	if ( gch->level - lch->level >= 6 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -6 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}
	*/

	xp = xp_compute( gch, victim ) * 4 / (3+members);

	if ( ch == gch )
	{ 
	  xp += xp * UMAX( 0, (1200 - ch->played) ) / 1200;
	  if ( IS_CLASS(gch,CLASS_MAZOKU) )
	    if ( victim->level > gch->pcdata->extras2[EVAL]/2 )
	    {
	      act( "You bask in $N's dying agony.", gch, NULL, victim, TO_CHAR );
	      gch->pcdata->powers[M_EGO] = UMIN( 200, gch->pcdata->powers[M_EGO]+1 );
	    }
	}

	if ( ch->in_room != victim->in_room )
	  xp /= 10;
	else if ( IS_SUIT(ch) )
	  xp /= 2;
	sprintf( buf, "You receive %d experience points.\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

    }

    return;
}



/*
 * Compute xp for a kill.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int xp;

    xp    = 1000 + victim->level * 1000 + victim->max_hit;
    if ( victim->level > 50 ) xp += victim->level * 250;
    if ( victim->level > 75 ) xp += victim->level * 500;
    if ( victim->level > 90 ) xp += victim->level * 1000;
    if ( victim->level > 95 ) xp += ( victim->level - 95 ) * 200000;

    /*
     * Adjust for popularity of target:
     *   -1/8 for each target over  'par' (down to -100%)
     *   +1/8 for each target under 'par' (  up to + 25%)
     * I don't really like this, so it's coming out
    level  = URANGE( 0, victim->level, MAX_LEVEL - 1 );
    number = UMAX( 1, kill_table[level].number );
    extra  = victim->pIndexData->killed - kill_table[level].killed / number;
    xp    -= xp * URANGE( -2, extra, 8 ) / 8;
     */

    xp     = number_range( xp * 3 / 4, xp * 5 / 4 );
    if ( !IS_NPC(gch) && gch->pcdata->clan[CLAN] > 0 )
    { if ( (IS_SET(victim->act,ACT_VALHERU) && gch->pcdata->clan[CLAN] == 1)
        || (IS_SET(victim->act,ACT_CITHDEUX) && gch->pcdata->clan[CLAN] == 2)
        || (IS_SET(victim->act,ACT_SYNDICATE) && gch->pcdata->clan[CLAN] == 3)
        || (IS_SET(victim->act,ACT_BROTHERHOOD) && gch->pcdata->clan[CLAN] == 4)
        || (IS_SET(victim->act,ACT_EXODUS) && gch->pcdata->clan[CLAN] == 5)
        || (IS_SET(victim->act,ACT_MERCENARY) && gch->pcdata->clan[CLAN] == 6) )
        xp = 0;
    }

    if ( !IS_NPC(gch) && gch->pcdata->extras2[EVAL] > 35 && victim->level < 50 )
        xp /= 5;

    /* sanity check */
    if ( xp > 3000000 )
      xp = 1;

    xp     = UMAX( 0, xp );

    return xp;
}



void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    static char * const attack_a[] =
    {
	"punch",
	"slice",  "stab",  "slash", "whip", "claw",
	"blast",  "pound", "crush", "grep", "bite",
	"pierce", "blow"
    };

    static char * const attack_b[] =
    {
	"punches",	"slices",	"stabs",	"slashes",
	"whips",	"claws",	"blasts",	"pounds",
	"crushes",	"greps",	"bites",	"pierces",
	"blows"
    };

    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    const char *vp;
    const char *attack;
    const char *attack2;
    char cdam[10]; char vdam[10];
    char punct;

    /* april fools joke
    if ( dt == F_JUMPKICK )
      dam *= 5;
    */
    /* check for numeric damage being shown */
    sprintf( cdam, " " );
    sprintf( vdam, " " );
    if ( !IS_NPC(ch) )
      if ( dam != 0 )
        if ( ch->pcdata->extras[LEGEND] >= 1 )
	  sprintf( cdam, "(%d)", dam );
    if ( !IS_NPC(victim) )
      if ( dam != 0 )
	if ( victim->pcdata->extras[LEGEND] >= 1 )
	  sprintf( vdam, "[%d]", dam );


	 if ( dam ==    0 ) { vp = "and miss";			}
    else if ( dam <=    5 ) { vp = "`bbarely`n";		}
    else if ( dam <=   20 ) { vp = "`bslightly`n";		}
    else if ( dam <=   35 ) { vp = "`bmoderately`n";		}
    else if ( dam <=   50 ) { vp = "`bfairly well`n";		}
    else if ( dam <=   75 ) { vp = "`gsomewhat hard`n";		}
    else if ( dam <=  100 ) { vp = "`ghard`n";			}
    else if ( dam <=  125 ) { vp = "`gvery hard`n";		}
    else if ( dam <=  150 ) { vp = "`gamazingly hard`n";	}
    else if ( dam <=  200 ) { vp = "`ginsanely hard`n";		}
    else if ( dam <=  250 ) { vp = "`Yskillfully`n";		}
    else if ( dam <=  350 ) { vp = "`Yvery skillfully`n";	}
    else if ( dam <=  500 ) { vp = "`Yamazingly skillfully`n";	}
    else if ( dam <=  750 ) { vp = "`rpowerfully`n";		}
    else if ( dam <= 1000 ) { vp = "`rvery powerfully`n";	}
    else if ( dam <= 1500 ) { vp = "`ramazingly powerfully`n";	}
    else if ( dam <= 2000 ) { vp = "`Rwith AWESOME force`n";	}
    else if ( dam <= 2500 ) { vp = "`Rwith GODLIKE force`n";	}
    else if ( dam <= 3500 ) { vp = "`Rwith UNHOLY force`n";	}
    else if ( dam <= 5000 ) { vp = "`WLIKE A BITCH`n!";		}
    else if ( dam <= 10000) { vp = "`WMACK TRUCK HARD`n!";	}
    else if ( dam <= 15000) { vp = "`WWITH FURIOUS ANGER`n!!";  }
    else if ( dam <= 30000) { vp = "`WLIKE A `RRED HEADED STEPCHILD`n!!";}
    else		    { vp = "`dWR`nAT`WHF`nUL`dLY`n";	}

    punct   = (dam <= 500) ? '.' : '!';

    /*
     * Buf4 added to show damage to room when ranged attacks
     * are used, so they are reversed in damage message so the
     * act function may be used.
     */
    switch( dt )
    {
    case TYPE_HIT:
	sprintf( buf1, "$n punches $N %s%c",  vp, punct );
	sprintf( buf2, "You punch $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n punches you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N punches $n %s%c", vp, punct );
	break;

    case TYPE_LSLASH:
        sprintf( buf1, "$n leaps and strikes down at $N %s%c",  vp, punct );
        sprintf( buf2, "You leap and strike down at $N %s%c %s", vp, punct, cdam );
        sprintf( buf3, "$n leaps and strikes down at you %s%c %s", vp, punct, vdam );
        sprintf( buf4, "$N leaps and strikes down at $n %s%c", vp, punct );
        break;

    case TYPE_USTROKE:
        sprintf( buf1, "$n's upward stroke cuts $N %s%c",  vp, punct );
        sprintf( buf2, "Your upward stroke cuts $N %s%c %s", vp, punct, cdam );
        sprintf( buf3, "$n's upward stroke cuts you %s%c %s", vp, punct, vdam );
        sprintf( buf4, "$N's upward stroke cuts $n %s%c", vp, punct );
        break;

    case F_PALMTHRUST:
    	sprintf( buf1, "The explosion blasts $N %s%c", vp, punct );
    	sprintf( buf2, "The explosion blasts $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "The explosion blasts you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "The explosion blasts $n %s%c", vp, punct );
    	break;
    case F_SHINKICK:
	sprintf( buf1, "$n kicks $N in the shins %s%c", vp, punct );
	sprintf( buf2, "You kick $N in the shins %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n kicks you in the shin %s%c That smarts! %s", vp, punct, vdam );
	sprintf( buf4, "$N kicks $n in the shins %s%c", vp, punct );
	break;
    case F_JAB:
	sprintf( buf1, "$n lands a quick jab on $N %s%c", vp, punct );	
	sprintf( buf2, "You land a quick jab on $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n lands a quick jab on you %s %c %s", vp, punct, vdam );
	sprintf( buf4, "$N lands a quick jab on $n %s%c", vp, punct );
	break;
    case F_SPINKICK:
	sprintf( buf1, "$n spins and kicks $N %s%c", vp, punct );
	sprintf( buf2, "You spin and kick $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n spins quickly and kicks you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N spins and kicks $n %s%c", vp, punct );
	break;
    case F_KNEE:
	sprintf( buf1, "$n drives $s knee into $Ns gut %s%c", vp, punct );
	sprintf( buf2, "You drive your knee into $Ns gut %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n drives $s knee into your gut %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N drives $s knee into $ns gut %s%c", vp, punct );
	break;
    case F_ELBOW:
	sprintf( buf1, "$n jams $s elbow into $Ns face %s%c", vp, punct );
	sprintf( buf2, "You jam your elbow into $Ns face %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n rams $s elbow into your face %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N jams $s elbow into $ns face %s%c", vp, punct );
	break;
    case F_UPPERCUT:
	sprintf( buf1, "$n slams $s fist into $Ns jaw %s%c", vp, punct );
	sprintf( buf2, "You slam your fist into $Ns jaw %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$ns fist connects with your jaw %s%c You see stars! %s", vp, punct, vdam );
	sprintf( buf4, "$N slams $s fist into $ns jaw %s%c", vp, punct );
	break;
    case F_STOMP:
	sprintf( buf1, "$n toestomps $N %s%c", vp, punct );
	sprintf( buf2, "You toestomp $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n toestomps you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N toestomps $n %s%c", vp, punct );
	break;
    case F_JUMPKICK:
	sprintf( buf1, "$n boots $N in the head %s%c", vp, punct );
	sprintf( buf2, "You boot $N in the head %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n boots you in the head %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N boots $n in the head %s%c", vp, punct );
	break;
    case F_DEATHTOUCH:
	sprintf( buf1, "$ns death touch strikes $N %s%c", vp, punct );
	sprintf( buf2, "Your death touch strikes $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$ns death touch strikes you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$Ns death touch strikes $n %s%c", vp, punct );
	break;
    case DAM_KIFLAME:
	sprintf( buf1, "$ns burst of Ki flame burns $N %s%c", vp, punct );
	sprintf( buf2, "Your burst of Ki flame burns $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$ns burst of Ki flame burns you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$Ns burst of Ki flame burns $n %s%c", vp, punct );
	break;
    case DAM_SHOCKSHIELD:
	sprintf( buf1, "`Y$n's defenses flare to life, shocking $N%c`n", punct );
	sprintf( buf2, "`YYour defenses flare to life, shocking $N%c `n%s", punct, cdam );
	sprintf( buf3, "`Y$n's defenses flare to life, shocking you%c `n%s", punct, vdam );
	sprintf( buf4, "`Y$N's defenses flare to life, shocking $n%c`n", punct );
	break;
    case DAM_LIGHTNING:
    	sprintf( buf1, "$n's lightning bolt shocks $N %s%c", vp, punct );
    	sprintf( buf2, "Your lightning bolt shocks $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's lightning bolt shocks $N %s%c %s", vp, punct, cdam );
    	sprintf( buf4, "$N's lightning bolt shocks $n %s%c", vp, punct );
    	break;
    case DAM_FIRE:
	sprintf( buf1, "$n's firey blast burns $N %s%c", vp, punct );
	sprintf( buf2, "Your firey blast burns $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n's firey blast burns you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N's firey blast burns $n %s%c", vp, punct );
	break;
    case DAM_NEGATIVE:
    	sprintf( buf1, "$n drains $N %s%c", vp, punct );
    	sprintf( buf2, "You drain $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n drains you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N drains $n %s%c", vp, punct );
    	break;
    case DAM_BULLETS:
    	sprintf( buf1, "$n's bullets pierce $N %s%c", vp, punct );
    	sprintf( buf2, "Your bullets pierce $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's bullets pierce you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's bullets pierce $n %s%c", vp, punct );
    	break;
    case DAM_BEAMRIFLE:
    	sprintf( buf1, "$n's beams burn $N %s%c", vp, punct );
    	sprintf( buf2, "Your beams burn $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's beams burn you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's beams burn $n %s%c", vp, punct );
    	break;
    case DAM_BEAMSABRE:
    	sprintf( buf1, "$n's beam blade sears $N %s%c", vp, punct );
    	sprintf( buf2, "Your beam blade sears $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's beam blade sears you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's beam blade sears $n %s%c", vp, punct );
    	break;
    case DAM_BEAMSWORD:
	sprintf( buf1, "$n's beamsword incinerates $N %s%c", vp, punct );
	sprintf( buf2, "Your beamsword incinerates $N %s%c %s", vp, punct, cdam );
	sprintf( buf3, "$n's beamsword incinerates you %s%c %s", vp, punct, vdam );
	sprintf( buf4, "$N's beamsword incinerates $n %s%c", vp, punct );
	break;
    case DAM_SHELLS:
    	sprintf( buf1, "$n's shells explode on $N %s%c", vp, punct );
    	sprintf( buf2, "Your shells explode on $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's shells explode on you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's shells explode on $n %s%c", vp, punct );
    	break;
    case DAM_HEATROD:
    	sprintf( buf1, "$'n heat rod rips $N %s%c", vp, punct );
    	sprintf( buf2, "Your heat rod rips $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's heat rod rips you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's heat rod rips $n %s%c", vp, punct );
    	break;
    case DAM_MISSILES:
        sprintf( buf1, "$n's missiles explode on $N %s%c", vp, punct );
        sprintf( buf2, "Your missiles explode on $N %s%c %s", vp, punct, cdam );
        sprintf( buf3, "$n's missiles explode on you %s%c %s", vp, punct, vdam );
        sprintf( buf4, "$N's missiles explode on $n %s%c", vp, punct );
        break;
    case DAM_SHOCKWAVE:
        sprintf( buf1, "$n's shockwave attack blasts $N %s%c", vp, punct );
        sprintf( buf2, "Your shockwave attack blasts $N %s%c %s", vp, punct, cdam );
        sprintf( buf3, "$n's shockwave attack blasts you %s%c %s", vp, punct, vdam );
        sprintf( buf4, "$N's shockwave attack blasts $n %s%c", vp, punct );
        break;
    case DAM_EXPLOSION:
    	sprintf( buf1, "$N is hurt by the explosion." );
    	sprintf( buf2, "You are hurt by the explosion! %s", cdam );
    	sprintf( buf3, "$N is hurt by the explosion! %s", vdam );
    	sprintf( buf4, "$N is hurt by the explosion!" );
    	break;
    case DAM_FLAME_BREATH:
    	sprintf( buf1, "$N's skin smolders." );
    	sprintf( buf2, "Your skin smolders! %s", cdam );
    	sprintf( buf3, "$N's skin smolders! %s", vdam );
    	sprintf( buf4, "$N's skins molders." );
    	break;
    case DAM_CRIMSON:
    	sprintf( buf1, "$n's crimson flames scorch $N %s%c", vp, punct );
    	sprintf( buf2, "Your crimson flames scorch $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's crimson flames scorch you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's crimson flames scorch $n %s%c", vp, punct );
    	break;
    case DAM_EMERALD:
    	sprintf( buf1, "$n's emerald rays sear $N %s%c", vp, punct );
    	sprintf( buf2, "Your emerald rays sear $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's emerald rays sear you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's emerald rays sear $n %s%c", vp, punct );
    	break;
    case DAM_CERULEAN:
    	sprintf( buf1, "$n's cerulean storm tears at $N %s%c", vp, punct );
    	sprintf( buf2, "Your cerulean storm tears at $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's cerulean storm tears at you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's cerulean storm tears at $n %s%c", vp, punct );
    	break;
    case DAM_OBSIDIAN:
    	sprintf( buf1, "$n's demonic energies wither $N %s%c", vp, punct );
    	sprintf( buf2, "Your demonic energies wither $N %s%c %s", vp, punct, cdam );
    	sprintf( buf3, "$n's demonic energies wither you %s%c %s", vp, punct, vdam );
    	sprintf( buf4, "$N's demonic energies wither $n %s%c", vp, punct );
    	break;

    default:
	if ( dt  >= 1400 && dt <= 1480 )
	{ sprintf(buf1, "$n's %s strikes $N %s%c", chant_table[dt-1400].name, vp, punct );
	  sprintf(buf2,"Your %s strikes $N %s%c %s",chant_table[dt-1400].name,vp,punct,cdam );
	  sprintf(buf3,"$n's %s strikes you %s%c %s",chant_table[dt-1400].name,vp,punct,vdam);
	  sprintf(buf4,"$N's %s strikes $n %s%c", chant_table[dt-1400].name, vp, punct );
	  break;
	}
	else if ( dt >= 0 && dt < MAX_SKILL )
	{   attack	= skill_table[dt].noun_damage;
	    attack2	= skill_table[dt].noun_damage;
	}
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + sizeof(attack_a)/sizeof(attack_a[0]) )
	{ attack	= attack_a[dt - TYPE_HIT];
	  attack2	= attack_b[dt-TYPE_HIT];
	}
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_a[0];
	    attack2 = attack_b[0];
	}
	sprintf( buf1, "$n %s $N %s%c",  attack2, vp, punct );
	sprintf( buf2, "You %s $N %s%c %s",  attack, vp, punct, cdam );
	sprintf( buf3, "$n %s you %s%c %s", attack2, vp, punct, vdam );
	sprintf( buf4, "$N %s $n %s%c", attack2, vp, punct );
	break;
    }

    act( buf1, ch, NULL, victim, TO_NOTVICT );
    act( buf2, ch, NULL, victim, TO_CHAR );
    act( buf3, ch, NULL, victim, TO_VICT );
    /* show ranged attacks to victims room */
    if ( ch->in_room != victim->in_room )
      act( buf4, victim, NULL, ch, TO_NOTVICT );

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL && number_bits( 1 ) == 0 )
	return;

    act( "$n DISARMS you!", ch, NULL, victim, TO_VICT    );
    act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n DISARMS $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_NPC(victim) )
	obj_to_char( obj, victim );
    else
	obj_to_room( obj, victim->in_room );

    return;
}



/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{

    if ( victim->wait == 0 )
    {
	act( "$n trips you and you go down!", ch, NULL, victim, TO_VICT    );
	act( "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR    );
	act( "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );

	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
	victim->position = POS_RESTING;
    }

    return;
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	WAIT_STATE( ch, 4 );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You attempt to double your efforts, but fail!\n\r", ch );
	return;
    }

    stancecheck( ch, victim );
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    do_shout( victim, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_NPC(ch) )
      return;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
      if ( ch->fighting == NULL )
      { send_to_char( "Backstab whom?\n\r", ch );
        return;
      }
      else
        victim = ch->fighting;
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;


    if ( IS_NPC(victim) || !IS_SET(victim->pcdata->actnew,NEW_FLEEING)  )
    {
	send_to_char( "Their back isn't turned.\n\r", ch );
	return;
    }

    stancecheck( ch, victim );

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_backstab] )
	multi_hit( ch, victim, gsn_backstab );
    else
	damage( ch, victim, 0, gsn_backstab );

    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    char buf[MAX_STRING_LENGTH];
    int attackers = 0, attempt, mod = 0;

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = dice(1,6)-1;
	if ( ( pexit = was_in->exit[door] ) == 0
	    ||   pexit->to_room == NULL
	    ||   IS_SET(pexit->exit_info, EX_CLOSED)
	    || ( !IS_NPC(ch) && ch->in_room->area != pexit->to_room->area
	        &&   pexit->to_room->clan > 0 
	        &&   pexit->to_room->clan != ch->pcdata->clan[CLAN] )
	    || ( IS_NPC(ch)
		&& ( IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
		    || ( IS_SET(ch->act, ACT_STAY_AREA)
			&& pexit->to_room->area != ch->in_room->area ) ) ) )
	    continue;

	if ( IS_NPC(ch) && IS_CLASS(victim,CLASS_FIST) && victim->pcdata->powers[F_DISC] >= 6 )
	  if ( victim->pcdata->powers[F_DISC] >= 6 && number_percent() > 50 )
	  { act( "You attempt to flee, but $N tackles you!", ch, NULL, victim, TO_CHAR );
	    act( "$n attempts to flee and you tackle $m.", ch, NULL, victim, TO_VICT );
	    act( "$n attempts to flee and $N tackles $m.", ch, NULL, victim, TO_NOTVICT );
	    WAIT_STATE( ch, 8 );
	    return;
	  }

	if ( IS_AFFECTED(ch,AFF_NO_FLEE) && IS_NPC(ch) )
	{ act( "You attempt to flee but are restrained!", ch, NULL, victim, TO_CHAR );
	  act( "$n attempts to flee but is restrained!", ch, NULL, victim, TO_VICT );
	  act( "$n attempts to flee from $N but is restrained.", ch,NULL,victim,TO_NOTVICT);
	  return;
	}

	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{ vch_next = vch->next_in_room;
	  if ( vch->fighting == ch )
	    attackers++;
	}
	attackers = UMAX(0,attackers-1);
	if ( IS_NPC(ch->fighting) )
	  attackers++;

	if ( IS_SUIT(ch->fighting) )
	  mod = 20;

	/* Karma disabled
	if ( !IS_NPC(ch) )
	  mod += ch->pcdata->extras[PUSSY];
	*/

	if ( number_percent() > (15 + mod + attackers*40 ) )
	{ act( "You fail!", ch, NULL, victim, TO_CHAR );
	  act( "$n turns $s back in an attempt to flee.", ch, NULL, victim, TO_VICT );
	  act( "$n turns $s back on $N in an attempt to flee.", ch, NULL, victim,TO_NOTVICT);
	  WAIT_STATE(ch,12);
	  if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->actnew,NEW_FLEEING) )
	    SET_BIT(ch->pcdata->actnew,NEW_FLEEING );
	  return;
	}

	if ( !IS_NPC(ch) && !IS_NPC(victim) )
	{ switch( attackers )
	  { case 0:
	    sprintf( buf, "%s is fleeing in terror from %s.", ch->name, victim->name );
	    break;
	    case 1:
	    sprintf( buf, "%s is on the run from %s's group.", ch->name, victim->name );
	    break;
	    case 2:
	    sprintf( buf, "%s is running for their life from %s's posse.", ch->name, victim->name );
	    break;
	    default:
	    sprintf( buf, "%s has successfully evaded %s's gangbang.", ch->name, victim->name );
	    break;
	  }
	  do_info( ch, buf );
	}
	move_char( ch, door );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!  You lose 25 exps.\n\r", ch );
	    gain_exp( ch, -25 );
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "You failed!  You lose 10 exps.\n\r", ch );
    gain_exp( ch, -10 );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *fch = NULL;
    bool match = FALSE;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_rescue].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the heroic acts to warriors.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }


    for ( vch = ch->in_room->people; vch != NULL && !match; vch = vch->next_in_room )
    { if ( vch != ch && vch != victim && vch->fighting == victim )
      { match = TRUE;
        fch = vch;
      }
    }
    if ( !match || fch == NULL )
    { send_to_char( "They don't need to be rescued.\n\r", ch );
      return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_rescue] )
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );

    if ( fch != ch )
      fch->fighting = ch;
    if ( ch != fch )
      ch->fighting = fch;
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) )
      dam = dice( 10, ch->level );
    else if ( ch->class == CLASS_FIST && ch->level >= 2 )
      dam = number_range( ch->pcdata->weapons[0] * 2, ch->pcdata->weapons[0] * 4 + ch->pcdata->powers[F_LEGS] * 15 );
    else if ( ch->class == CLASS_FIST )
      dam = number_range( UMIN(ch->pcdata->weapons[0], 200), UMIN( ch->pcdata->weapons[0], 200 )*5 );
    else
      dam = number_range( ch->pcdata->weapons[0], ch->pcdata->weapons[0] * 5 );

    if ( !IS_NPC(victim) || victim->level > 95 )
      dam /= 15;

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_kick] )
	damage( ch, victim, dam, gsn_kick );
    else
	damage( ch, victim, 0, gsn_kick );

    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent, defense;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_disarm].skill_level[ch->class] )
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
	defense = 50 + ch->pcdata->body;
    else
	defense = victim->level;
    WAIT_STATE( ch, skill_table[gsn_disarm].beats );
    percent = number_percent( ) + defense - ch->level;
    if ( IS_NPC(ch) || percent < ch->pcdata->learned[gsn_disarm] * 2 / 3 )
	disarm( ch, victim );
    else
	send_to_char( "You failed.\n\r", ch );
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= ch->level )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim, FALSE );
    return;
}


int stanced( CHAR_DATA *ch )
{
  if ( IS_NPC(ch) )
    return 0;
 
  return ch->pcdata->stances[MAX_STANCES+1];
}

void stancecheck( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int autostance;

  if (!IS_NPC(ch))
  { autostance = ch->pcdata->stances[0];
    if ( autostance > MAX_STANCES ) autostance = 0;
    if ( autostance > 0 && ch->pcdata->stances[MAX_STANCES+1] == 0 )
      do_stance( ch, stance_table[autostance].name );
  }

  if (!IS_NPC(victim))
  { autostance = victim->pcdata->stances[0];
    if ( autostance > MAX_STANCES ) autostance = 0;
    if ( autostance > 0 && victim->pcdata->stances[MAX_STANCES+1] == 0 )
      do_stance( victim, stance_table[autostance].name );
  }

  return;
}

int calc_attacks( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int atk, stance, chit, vhit;

  if (IS_NPC(ch))
  { atk = 1;
    atk += ch->level / 25;
    if ( ch->level > 95 )
      atk += ( ch->level - 95 );
    if ( IS_AFFECTED(ch,AFF_CHAOS_STRING) )
      atk = UMAX( 1, atk * 2 / 3 );
    return atk;
  }

  atk = 1;
  stance = stanced( ch );
  if ( stance > 0 )
    atk += stance_table[stance].attacks;

  if ( !IS_NPC(ch) && !IS_NPC(victim) )
  {
    chit = UMIN( ch->hitroll, ch->pcdata->extras2[EVAL]*3 );
    vhit = UMIN( victim->hitroll, victim->pcdata->extras2[EVAL]*3 );
    if ( !IS_CLASS(ch,CLASS_SORCERER) || ch->pcdata->powers[SORC_PREP] <= 0 )
      atk += URANGE( 0, (chit - vhit)/10, 3 );
  }

  if ( ch->class == CLASS_SAIYAN )
  { if ( ch->pcdata->powers[S_SPEED] > 50 )
      atk++;
    if ( ch->pcdata->powers[S_SPEED] >= 150 )
      atk += (ch->pcdata->powers[S_SPEED] / 150 );
    if ( IS_SET(ch->pcdata->powers[S_TECH],S_ZANZOUKEN) )
      atk += 2;
  }
  else if ( ch->class == CLASS_PATRYN )
  { atk += get_runes(ch,RUNE_AIR,LEFTARM) + get_runes(ch,RUNE_AIR,RIGHTARM);
    // atk += ( get_runes(ch,RUNE_AIR,LEFTLEG) + get_runes(ch,RUNE_AIR,RIGHTLEG) ) / 2;
  }
  else if ( IS_CLASS(ch,CLASS_MAZOKU) )
  { if ( IS_SET(ch->pcdata->powers[M_SET],M_BATTLE) )
    { if ( IS_SET(ch->pcdata->powers[M_SET],M_THIRD) )
        atk++;
      if ( IS_SET(ch->pcdata->powers[M_SET],M_FOURTH) )
        atk++;
      if ( IS_SET(ch->pcdata->powers[M_SET],M_FIFTH) )
        atk++;
      if ( IS_SET(ch->pcdata->powers[M_SET],M_SIXTH) )
        atk++;
    }
    if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
      atk++;
    if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
      atk += 2;
    if ( IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
      atk += 7;
  }

  if ( IS_AFFECTED(victim,AFF_RAYWING) )
    atk = UMAX( 1, atk * 2 / 3 );

  return atk;
}

void do_whirl( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam, weapon, cost, mod, suit;
  int type = DAM_BEAMSABRE;
  int ammo = SUIT_BEAM;

  /* whirling for suits only */
  if ( IS_SUIT(ch) )
  { weapon = ch->pcdata->suit[SUIT_WEAPON];
    suit = ch->pcdata->suit[SUIT_NUMBER];
    switch( ch->pcdata->suit[SUIT_READY] )
    { case MS_BEAM_SABRE:
      case MS_CRASHER:
        cost = 1;
        mod = 2;
        break;
      case MS_HEAT_SHOTELS:
        cost = 1;
        mod = 1;
        ammo = SUIT_PLASMA;
        break;
      case MS_BEAM_SCYTHE:
      case MS_BEAM_GLAIVE:
        cost = 1;
        mod = 3;
        break;
      case MS_BEAM_SWORD:
        cost = 10;
        mod = 4;
        break;
      default:
        send_to_char( "You can't whirl without a melee weapon.\n\r", ch );
        return;
        break;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    { vch_next = vch->next;
      if ( vch == ch || vch->in_room == NULL )
        continue;
      if ( vch->in_room == ch->in_room )
      { dam = isquare( weapon ) * dice(5,5) + (suit_table[suit].fight-90)*30;
	dam *= mod;
	if ( IS_SUIT(vch) && ch->pcdata->suit[SUIT_READY] == MS_HEAT_SHOTELS )
	  dam *= 3;
	if ( ch->pcdata->suit[ammo] < cost )
	  send_to_char( "You are out of ammunition.\n\r", ch );
	else
	{
	  damage( ch, vch, dam, type );
	  ch->pcdata->suit[ammo] -= cost;
	}
      }
    }
    WAIT_STATE(ch, 16-((suit_table[suit].speed-80)/10) );
    return;
  }

  /* return to normal whirl function */
  if ( ch->position < POS_FIGHTING || IS_TIED(ch) )
  { send_to_char( "Nah... You feel too relaxed...\n\r", ch );
    return;
  }
  for ( vch = char_list; vch != NULL; vch = vch_next )
  { vch_next = vch->next;
    if ( vch == ch || vch->in_room == NULL )
      continue;

    /* whirls by mortals skip avatars, no free autostance */
    if ( !IS_NPC(ch) && !IS_NPC(vch) && ch->level < 2 )
      continue;
    if ( vch->in_room == ch->in_room )
    { stancecheck( ch, vch );
      one_hit( ch, vch, gsn_whirl );
    }
  }
  WAIT_STATE( ch, 24 );

  return;
}


void check_quest( CHAR_DATA *ch, CHAR_DATA *victim )
{
  if ( IS_NPC(ch) || !IS_NPC(victim) )
    return;

  if ( ch->pcdata->extras2[QUEST_TYPE] != QUEST_MOB )
    return;

  if ( ch->pcdata->extras2[QUEST_INFO] == victim->pIndexData->vnum )
  { send_to_char( "`WYou have completed your quest, return to your quest master!`n\n\r",ch);
    ch->pcdata->extras2[QUEST_TYPE] = QUEST_FINISHED;
    ch->pcdata->extras2[QUEST_INFO] = victim->level;
  }

  return;
}

int suit_damage( CHAR_DATA *victim, int dam )
{ int suit, part, i, j;
  bool match = FALSE;
  char *buf;
  char buf2[MAX_STRING_LENGTH];

  if ( IS_NPC(victim) )
    return -1;
  if ( !IS_SUIT(victim) )
    return -1;

  
  suit = victim->pcdata->suit[SUIT_NUMBER];

  victim->pcdata->suit[SUIT_ARMOR] -= dam;
  if ( victim->pcdata->suit[SUIT_ARMOR] > 0 )
    return dam;
  victim->pcdata->suit[SUIT_ARMOR] = suit_table[suit].armor * 15;
  
  part = 1;
  if ( victim->pcdata->suit[SUIT_COND] < 4095 )
  { while ( !match )
    { i = dice(1,12) - 1;
      part = 1;
      for ( j = 0; j < i; j++ )
        part *= 2;
      if ( !IS_SET(victim->pcdata->suit[SUIT_COND],part) )
        match = TRUE;
    }
    SET_BIT(victim->pcdata->suit[SUIT_COND],part);
  }

  switch( part )
  { case 1:	buf = "torso has been damaged";		break;
    case 2:	buf = "guidance package has been damaged";	break;
    case 4:	buf = "head has been damaged";		break;
    case 8:	buf = "camera has been damaged";	break;
    case 16:	buf = "left leg has been damaged";	break;
    case 32:	buf = "right hand has been damaged";	break;
    case 64:	buf = "right leg has been damaged";	break;
    case 128:	buf = "left hand has been damaged";	break;
    case 256:	buf = "left arm has been damaged";	break;
    case 512:	buf = "radar system has been damaged";	break;
    case 1024:	buf = "right arm has been damaged";	break;
    case 2048:	buf = "propulsion system has been damaged";	break;
    default:	buf = "This is a bug";
    		sprintf( buf2, "part %d  cond %d", part, victim->pcdata->suit[SUIT_COND]);
    		send_to_char( buf2, victim );
    		break;
  };

  act( ".. Your $T.", victim, NULL, buf, TO_CHAR );
  act( ".. $n's $T.", victim, NULL, buf, TO_ROOM );

  if ( victim->pcdata->suit[SUIT_COND] >= 4095 )
  { act( "Your mobile suit explodes, throwing you from the cockpit!", victim, NULL, NULL, TO_CHAR );
    act( "$n's mobile suit explodes!", victim, NULL, NULL, TO_ROOM );
    for ( i = 0; i < 10; i++ )
      victim->pcdata->suit[i] = 0;
    victim->hit = dice( 1, UMAX(1,victim->hit)/5 );
  }
  else
    damage( victim, victim, dice(5,400), DAM_EXPLOSION );

  return dam;
}

void essense_gain( CHAR_DATA *ch, CHAR_DATA *victim, int gain, int dt )
{
  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
    return;

  if ( ch == victim )
  {
    ch->pcdata->powers[M_NIHILISM] = UMIN( 100, ch->pcdata->powers[M_NIHILISM]+1 );
    return;
  }

  if ( dt != DAM_OBSIDIAN )
    gain /= 10;

  gain -= gain * ch->pcdata->powers[M_ESSENSE] / 100000;

  if ( (ch->pcdata->powers[M_ESSENSE] + gain) < 100000 )
    ch->pcdata->powers[M_ESSENSE] += gain;

  return;
}

void do_retarget( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if ( ch->fighting == NULL )
  {
    stc( "Why bother?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( (victim = get_char_room( ch, arg )) == NULL )
  {
    stc( "They aren't here.\n\r", ch );
    return;
  }

  act( "You focus your attacks on $N.", ch, NULL, victim, TO_CHAR );
  act( "$n focuses $s attacks on you!", ch, NULL, victim, TO_VICT );
  act( "$n focuses $s attacks on $N.", ch, NULL, victim, TO_NOTVICT );
  ch->fighting = victim;

  return;
}


void do_sap( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  char arg[MAX_INPUT_LENGTH];

  if ( (obj = get_eq_char(ch,WEAR_HOLD)) == NULL )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( obj->pIndexData->vnum != 35 )
  {
    stc( "Huh?\n\r", ch );
    return;
  }

  if ( !IS_NPC(ch) && ch->pcdata->extras2[EVAL] < 10 )
  {
    stc( "You're too young to play with one of those.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    stc( "Sap who?\n\r", ch );
    return;
  }
  if ( (victim = get_char_room(ch,arg)) == NULL )
  {
    stc( "They aren't here.\n\r", ch );
    return;
  }
  if ( victim->position <= POS_SLEEPING )
  {
    stc( "Why bother?\n\r", ch );
    return;
  }
  if ( victim->hit < victim->max_hit )
  {
    act( "$N is hurt and suspicious.", ch, NULL, victim, TO_CHAR );
    act( "You avoid a vicious attack with $p by $n!", ch, obj, victim, TO_VICT );
    return;
  }

  af.type      = skill_lookup( "sleep" );
  af.duration  = 15 + dice(1,5);
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SLEEP;

  act( "You slam $p against $N's temple!", ch, obj, victim, TO_CHAR );
  act( "$N slumps to the ground.", ch, NULL, victim, TO_CHAR );
  act( "Unnnnnngh....   zzzzzzzz.......", ch, NULL, victim, TO_VICT );
  act( "$N slumps to the ground.", ch, NULL, victim, TO_NOTVICT );

  victim->position = POS_SLEEPING;
  affect_to_char( victim, &af );

  if ( !IS_NPC(ch) && ch->pcdata->extras[TIMER] < 15 )
    ch->pcdata->extras[TIMER] = 15;

  WAIT_STATE( ch, 12 );
  return;
}

void add_inf( CHAR_DATA *ch, CHAR_DATA *victim, int dam )
{
  INF_DATA *inf = NULL;

  if ( ch == NULL || IS_NPC(ch) || victim == NULL || dam <= 0 )
    return;


  for ( inf = ch->pcdata->inf; inf != NULL; inf = inf->next )
  {
    if ( !str_cmp( inf->victim, victim->name ) )
      break;
  }

  if ( inf == NULL )
  {
    inf = new_inf();
    inf->pc = !IS_NPC(victim);
    inf->victim = str_dup( victim->name );
    inf->next = ch->pcdata->inf;
    ch->pcdata->inf = inf;
  }

  inf->damage += dam;
  inf->time = current_time;

  return;
}


/*
 * Remove all inf records of damage from a character
 */
void clear_inf( CHAR_DATA *ch )
{
  INF_DATA *inf;
  INF_DATA *inf_last;

  if ( ch == NULL || IS_NPC(ch) || ch->pcdata->inf == NULL )
    return;

  inf_last = ch->pcdata->inf;
  for ( inf = ch->pcdata->inf; inf != NULL; inf = inf->next )
  {
    free_string( inf->victim );
    inf_last = inf;
  }

  inf_last->next = inf_free;
  inf_free = ch->pcdata->inf;
  ch->pcdata->inf = NULL;

  return;
}

void inf_update()
{
  CHAR_DATA *vch;
  INF_DATA *inf;
  INF_DATA *inf_next;
  INF_DATA *inf_prev;

  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
    if ( IS_NPC(vch) )
      continue;
    if ( vch->pcdata->inf == NULL )
      continue;

    inf_prev = NULL;
    for ( inf = vch->pcdata->inf; inf != NULL; inf = inf_next )
    {
      inf_next = inf->next;

      if ( inf->time + 30 < current_time ) /* infs expire after 30 seconds */
      {
        free_string( inf->victim );

        if ( inf == vch->pcdata->inf )
          vch->pcdata->inf = inf_next;
        else
          inf_prev->next = inf_next;

        inf->next = inf_free;
        inf_free = inf;
      }
      else
        inf_prev = inf;
    }
  }

  return;
}

int get_inf_dam( CHAR_DATA *ch, char *name )
{
  INF_DATA *inf;

  if ( ch == NULL || IS_NPC(ch) || ch->pcdata->inf == NULL )
    return 0;

  inf = ch->pcdata->inf;

  while ( inf != NULL && str_cmp( inf->victim, name ) )
    inf = inf->next;

  if ( inf == NULL )
    return 0;

  return inf->damage;
}


