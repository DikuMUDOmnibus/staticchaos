/* Original by Alathon and Larsen */
#if defined (macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "merc.h"

void gain_ki( CHAR_DATA *ch, int ki );


void do_combo( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  char arg[MAX_INPUT_LENGTH];
  int dam;

  argument = one_argument( argument, arg );

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "You combine vinegar and baking soda, and watch it bubble.\n\r", ch );
    return;
  }

  if ( ch->fighting == NULL )
  { send_to_char( "But you aren't fighting anybody.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_KI] < 10 )
  { send_to_char( "Your Ki is not excited enough.\n\r", ch );
    return;
  }

  victim = ch->fighting;

  if ( ch->move <= ch->pcdata->powers[F_KI]*(100-(ch->pcdata->powers[F_DISC]*8)) )
  { send_to_char( "You are too exhausted to execute this combo.\n\r", ch );
    ch->pcdata->powers[F_KI] = 0;
    WAIT_STATE( ch, 8 );
    return;
  }

  ch->move -= ch->pcdata->powers[F_KI]*(100-(ch->pcdata->powers[F_DISC]*8));

  switch ( ch->pcdata->powers[F_KI] )
  {
    case 10:
      act( "You execute the Quivering Palm on $N.", ch, NULL, victim, TO_CHAR );
      act( "$n executes the Quivering Palm on you.", ch, NULL, victim, TO_VICT );
      act( "$n executes the Quivering Palm on $N.", ch, NULL, victim, TO_NOTVICT );
      if ( !IS_CLASS(victim,CLASS_FIST) )
      {
        act( "$N twitches violently.", ch, NULL, victim, TO_CHAR );
        act( "You twitch violently.", ch, NULL, victim, TO_VICT );
        act( "$N twitches violently.", ch, NULL, victim, TO_NOTVICT );
        victim->mana = UMAX( 1, victim->mana - (1500 + dice(5,100)) );

        if ( IS_CLASS(victim,CLASS_SAIYAN) )
        { 
          ch->pcdata->powers[F_KI] = victim->pcdata->powers[S_POWER] / 1000;
          victim->pcdata->powers[S_POWER] = UMAX( 1, victim->pcdata->powers[S_POWER] - dice(10,500));

          /* Quivering palm can drain a charged ki attack into healing */
          if ( IS_SET(victim->pcdata->actnew,NEW_KAME_1) ||
               IS_SET(victim->pcdata->actnew,NEW_KAME_2) ||
               IS_SET(victim->pcdata->actnew,NEW_MASENKOUHA) )
          {
            act( "You absorb $N's Ki charge.", ch, NULL, victim, TO_CHAR );
            act( "$n glows for a moment as $e absorbs your Ki charge!", ch, NULL, victim, TO_VICT );
            act( "$n glows for a moment as $e absorbs $N's Ki charge.", ch, NULL, victim, TO_NOTVICT );

            if ( IS_SET(victim->pcdata->actnew,NEW_KAME_1) )
            {
              REMOVE_BIT(victim->pcdata->actnew,NEW_KAME_1);
              dam = victim->pcdata->spirit * 10 + dice(10,victim->pcdata->spirit);
            }
            else if ( IS_SET(victim->pcdata->actnew,NEW_KAME_2) )
            {
              REMOVE_BIT(victim->pcdata->actnew,NEW_KAME_2);
              dam = victim->pcdata->spirit * 10 + dice(10,victim->pcdata->spirit);
            }
            else if ( IS_SET(victim->pcdata->actnew,NEW_MASENKOUHA) )
            {
              REMOVE_BIT(victim->pcdata->actnew,NEW_MASENKOUHA);
              dam = victim->pcdata->spirit * 5 + dice(8,victim->pcdata->spirit);
            }
            else
              dam = 1;

            ch->hit = UMIN( ch->max_hit, ch->hit + dam );
          }


        }
        else
          ch->pcdata->powers[F_KI] = 0;
      }
      else if ( victim->pcdata->powers[F_KI] <= 0 )
      { act( ".. but nothing happens.", ch, NULL, NULL, TO_CHAR );
        act( ".. but nothing happens.", ch, NULL, NULL, TO_ROOM );
        ch->pcdata->powers[F_KI] = 0;
      }
      else
      { act( ".. and absorb their strength!", ch, NULL, NULL, TO_CHAR );
        act( ".. your Ki has been stolen!", ch, NULL, victim, TO_VICT );
        act( ".. golden energy flows from $N to $n.", ch, NULL, victim, TO_NOTVICT );
        ch->pcdata->powers[F_KI] = 9 + victim->pcdata->powers[F_KI]/2;
        victim->pcdata->powers[F_KI] /= 2;
      }
      WAIT_STATE(ch,4);
      return;
      break;
    case 11: 
      send_to_char( "You stomp on your opponents toes, breaking them.\n\r", ch );
      dam = dice( 10, ch->pcdata->body );
      damage( ch, victim, dam, F_STOMP );
      break;
    case 12:
      send_to_char( "You execute the Di-Amon Mega Rotation Death.\n\r", ch );
      dam = dice( 1, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_SHINKICK );
      damage( ch, victim, dam, F_SHINKICK );
      break;
    case 13:
      do_yell( ch, "Hadoken!" );
      act( "You launch a blazing ball of Ki at $N!", ch, NULL, victim, TO_CHAR );
      act( "$n flings a ball of blazing energy at you!", ch, NULL, victim, TO_VICT );
      act( "$n flings a ball of blazing energy at $N.", ch, NULL, victim, TO_NOTVICT);
      dam = dice( (ch->pcdata->body / 2) , (ch->pcdata->weapons[0] / 3) );
      damage( ch, victim, dam, DAM_KIFLAME );
      break;
    case 14:
      act( "You move your arms in a figure-eight pattern to defend yourself.",
           ch, NULL, NULL, TO_CHAR );
      act( "$n's arms swirl in a figure-eight pattern.", ch, NULL, victim, TO_ROOM );
      if ( !IS_SET(ch->pcdata->actnew,NEW_FIGUREEIGHT ) )
        SET_BIT(ch->pcdata->actnew,NEW_FIGUREEIGHT );
      break;
    case 15:
      do_say( ch, "Boot to the head." );
      dam = dice( (ch->pcdata->body/2), ch->pcdata->weapons[0] * 2 / 5  );
      damage( ch, victim, dam, F_JUMPKICK );
      break;
    case 16:
      act( "You attempt to bodyslam $N, and fall flat on your face!", ch, NULL, victim, TO_CHAR );
      act( "You dodge $ns bodyslam, and $e falls on $s face!", ch, NULL, victim, TO_VICT);
      act( "$n fails to bodyslam $N and falls on $s face!", ch, NULL, victim, TO_NOTVICT );
      unstance( ch );
      break;
    case 17:
      act( "You slam into $N, knocking them off balance!", ch, NULL, victim, TO_CHAR );
      act( "$n slams into you, and you lose your balance!", ch, NULL, victim, TO_VICT );
      act( " $n slams into $N, and $E loses $S balance.", ch, NULL, victim, TO_NOTVICT );
      unstance( victim );
      break;
    case 18:
      act( "You slam into $N, and you both go down in a tangle!", ch,NULL, victim, TO_CHAR );
      act( "$n crashes into you, and you both fall down.", ch, NULL, victim, TO_VICT );
      act( "$n and $N crash to the ground, arms and legs akimbo!", ch, NULL, victim, TO_NOTVICT );
      unstance( ch );
      unstance( victim );
      break;
    case 19:
      act( "You pummel $N with blows as you execute the Fists of Fury.", ch, NULL, victim, TO_CHAR );
      act( "$n pummels you with a rapid series of blows!", ch, NULL, victim, TO_VICT );
      act( "$ns fists slam into $N in a flurry of blows.", ch, NULL, victim, TO_NOTVICT );
      dam = dice( 6, ch->pcdata->weapons[0] / 4 );
      damage( ch, victim, dam, F_JAB );
      dam = dice( 6, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_JAB );
      dam = dice( 6, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_JAB );
      dam = dice( 15, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_UPPERCUT );
      break;
   case 20:
      if ( IS_NPC(victim) )
      { act( "You palm thrust $N, but nothing happens.", ch, NULL, victim, TO_CHAR );
        act( "$n palm thrusts $N, but nothing happens.", ch, NULL, victim, TO_ROOM );
      }
      else if ( is_affected(victim,gsn_kiwall)
      	     || IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT)
      	     || is_affected(victim,gsn_defense) 
             || IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
      { act( "You palm thrust $N, and their defenses shatter!", ch, NULL, victim, TO_CHAR );
        act( "$n shatters your defenses with a palm thrust!", ch, NULL, victim, TO_VICT );
        act( "$n shatters $N's defenses with a palm thrust.", ch, NULL, victim,TO_NOTVICT);
	if ( is_affected(victim,gsn_kiwall) )
	  affect_strip(victim,gsn_kiwall);
	if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	  REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
	if ( is_affected(victim,gsn_defense) )
	  affect_strip(victim,gsn_defense);
        if ( IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
          REMOVE_BIT(victim->pcdata->actnew,NEW_VAN_REHL);
        dam = dice( ch->pcdata->body/3, ch->pcdata->weapons[0] );
        if ( IS_CLASS(victim,CLASS_FIST) )
          dam *= 3;
        damage( ch, victim, dam, F_PALMTHRUST );
      }
      else
      { act( "You palm thrust $N, but nothing happens.", ch, NULL, victim, TO_CHAR );
        act( "$n palm thrusts you, but nothing happens.", ch, NULL, victim, TO_VICT );
        act( "$n palm thrusts $N, but nothing happens.", ch, NULL, victim, TO_NOTVICT );
      }
      break;
   case 21:
      act( "You attempt to intimidate $N with a show of skill.", ch, NULL, victim, TO_CHAR );
      act( "$ns body blurs as $e executes a stunning attack routine!", ch, NULL, victim, TO_VICT );
      act( "$n executes a stunning attach routine against $N.", ch, NULL, victim, TO_NOTVICT );
      dam = dice( 3, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_SHINKICK );
      dam = dice( 5, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_JAB );
      dam = dice( 8, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_SPINKICK );
      dam = dice( 10, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_KNEE );
      dam = dice( 12, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_ELBOW );
      dam = dice( 14, ch->pcdata->weapons[0] / 3 );
      damage( ch, victim, dam, F_UPPERCUT );
      break;
    case 22:
      act( "You hesitate, and lose the rhythm of combat.", ch, NULL, victim, TO_CHAR );
      act( "$n looks uncertain for a moment, and they fall out of sync.", ch, NULL, victim, TO_VICT );
      act( "$ns hesitation breaks their combat rhythm.", ch, NULL, victim, TO_NOTVICT );
      WAIT_STATE( ch, 8 );
      break;
   case 23:
      act( "You reverse your Ki, and focus the negative energy on $N.", ch, NULL, victim, TO_CHAR );
      act( "Black energy arcs out from $n, streaming into you!", ch, NULL, victim, TO_VICT );
      act( "$n executes the Touch of Death on $N.", ch, NULL, victim, TO_NOTVICT );
      dam = UMAX( victim->hit * 2/5, 2500 );
      damage( ch, victim, dam, F_DEATHTOUCH );
      break;
   case 24:
      act( "Your attempt to deathtouch $N rebounds!", ch, NULL, victim, TO_CHAR );
      act( "$N smokes as black energy arcs about $m.", ch, NULL, victim, TO_ROOM );
      dam = UMAX( ch->hit / 4, 2500 );
      damage( ch, ch, dam, F_DEATHTOUCH );
      break;
   case 25:
      act( "You attempt to execute the Maiden Masher, but whiff and are out of position!", ch, NULL, victim, TO_CHAR );
      act( "$n launches a powerful attack at you, but whiffs and stumbles!", ch, NULL, victim, TO_VICT );
      act( "$n launches a powerful attack at $N, but whiffs and stumbles!", ch, NULL, victim, TO_NOTVICT );
      SET_BIT(ch->pcdata->actnew,NEW_NOBLOCK);
      WAIT_STATE(ch,8);
      break;
   case 26:
      act( "Blazing with Ki, you execute the vicious Maiden Masher attack!", ch, NULL, victim, TO_CHAR );
      act( "$n tears into you with the Maiden Masher!", ch, NULL, victim, TO_VICT );
      act( "$n tears into $N with the Maiden Masher!", ch, NULL, victim, TO_NOTVICT );
      dam = dice( 12, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_SPINKICK );
      dam = dice( 5, ch->pcdata->spirit );
      damage( ch, victim, dam, DAM_KIFLAME );
      dam = dice( 14, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_SPINKICK );
      dam = dice( 7, ch->pcdata->spirit );
      damage( ch, victim, dam, DAM_KIFLAME );
      dam = dice( 17, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_SPINKICK );
      dam = dice( 8, ch->pcdata->spirit );
      damage( ch, victim, dam, DAM_KIFLAME );
      dam = dice( 19, ch->pcdata->weapons[0] / 2 );
      damage( ch, victim, dam, F_UPPERCUT );
      dam = dice( 20, ch->pcdata->spirit );
      damage( ch, victim, dam, DAM_KIFLAME );
      break;
    case 27:
      do_yell( ch, "Gadouken!" );
      act( "You hurl a pink puffball of Ki at $N..", ch, NULL, victim, TO_CHAR );
      act( "$n hurls a pink puffball at you..", ch, NULL, victim, TO_VICT );
      act( "$n hurls a pink puffball at $N..", ch, NULL, victim, TO_NOTVICT);
      dam = dice( 1, ch->hitroll * ch->pcdata->body / 3 );
      damage( ch, victim, dam, DAM_KIFLAME );
      break;
    case 28:
      do_yell( ch, "Shinkyuu Hadoken!" );
      act( "You launch a brilliant missile of Ki at $N!", ch, NULL, victim, TO_CHAR );
      act( "$n flings a brilliant mass of energy at you!", ch, NULL, victim, TO_VICT );
      act( "$n flings a brilliant mass of energy at $N.", ch, NULL, victim, TO_NOTVICT);
      dam = dice( 22, (ch->pcdata->body / 2) + (ch->pcdata->weapons[0] / 4) );
      damage( ch, victim, dam, DAM_KIFLAME );
      dam = dice( 22, (ch->pcdata->body) + (ch->pcdata->weapons[0] / 4) );
      damage( ch, victim, dam, DAM_KIFLAME );
      dam = dice( 22, (ch->pcdata->body) + (ch->pcdata->weapons[0] / 2) );
      damage( ch, victim, dam, DAM_KIFLAME );
      break;
    case 29:
      affect_strip(ch,gsn_golden_blaze);
      affect_strip(ch,gsn_judicators_ire);
      affect_strip(ch,gsn_dark_blaze);
      af.location  = APPLY_AC;
      af.modifier  = ch->pcdata->spirit / -10;
      af.bitvector = 0;

      if ( arg[0] != '\0' && !str_prefix( arg, "holy" ) )
      {
        act( "You focus your Ki into flames around your fists.", ch, NULL, NULL, TO_CHAR );
        act( "$n's fists are encased in golden flames!", ch, NULL, NULL, TO_ROOM );
        af.type = gsn_golden_blaze;
        af.duration = ch->pcdata->spirit*10;
      }
      else if ( arg[0] != '\0' && !str_prefix( arg, "dark" ) )
      {
        act( "Your fists blaze with dark power!", ch, NULL, NULL, TO_CHAR );
        act( "$n's fists blaze with dark power!", ch, NULL, NULL, TO_ROOM );
        af.type = gsn_dark_blaze;
        af.duration = ch->pcdata->spirit*10;
      }
      else
      {
        act( "Your fists vibrate with the force of your Ki.", ch, NULL, NULL, TO_CHAR );
        act( "$n's fists are encased in shimmering blue light.", ch, NULL, NULL, TO_ROOM );
        af.type = gsn_judicators_ire;
        af.duration = ch->pcdata->spirit*10;
        af.modifier = ch->pcdata->spirit / -2;
      }
      affect_to_char( ch, &af );
      break;

    case 30:
      if ( ch->level >= 20 )
      {
        do_yell( ch, "Instant Hell Murder." );
        act( "Darkness surrounds you as you murder $N.", ch, NULL, victim, TO_CHAR );
        act( "Darkness engulfs you, and there is nothing but pain.", ch, NULL, victim, TO_VICT );
        act( "$n and $N are engulfed in darkness.. a scream issues forth.", ch, NULL, victim, TO_NOTVICT );
        while ( victim && victim->position > POS_STUNNED )
        {
          switch( dice(1,4) )
          {
            case 1:
	      damage( ch, victim, dice(10,500), F_JAB );
	      break;
	    case 2:
	      damage( ch, victim, dice(12,500), F_ELBOW );
	      break;
	    case 3:
	      damage( ch, victim, dice(14,500), F_KNEE );
	      break;
	    case 4:
	      damage( ch, victim, dice(16,500), F_UPPERCUT );
	      break;
	  }
	}
      }
      break;

    default:
      break;

  }

  WAIT_STATE( ch, 4 );
  ch->pcdata->powers[F_KI] = 0;
  return;
}


void do_shinkick( CHAR_DATA *ch, char *argument )
{
  int dam, num;
  int ki = ch->pcdata->powers[F_MASTER] % 10;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "You kick yourself in the shin.  OUCH!!\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < F_SHINKICK )
  { send_to_char( "You have not yet mastered that attack.\n\r", ch );
    return;
  }

  if (ch->fighting == NULL )
  { send_to_char( "But you are not in combat!\n\r", ch );
    return;
  }

  dam = dice( 10-ki, ch->pcdata->weapons[0] / 4 );

  damage( ch, ch->fighting, dam, F_SHINKICK );
  num = number_percent();
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
    num += ch->pcdata->powers[F_KI];
  if ( num < 15 )
    ki += 1;
  else if ( num < 30 )
    ki -= 1;
  gain_ki( ch, ki );
  WAIT_STATE( ch, 8 );
  return;
}

void do_jab( CHAR_DATA *ch, char *argument )
{
  int dam, num;
  int ki = (ch->pcdata->powers[F_MASTER]/10) % 10;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "You attempt to jab yourself, and fail.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < F_JAB )
  { send_to_char( "You have not yet mastered that attack.\n\r", ch );
    return;
  }

  if (ch->fighting == NULL )
  { send_to_char( "But you are not in combat!\n\r", ch );
    return;
  }

  dam = dice( 10-ki, ch->pcdata->weapons[0] / 4 );

  damage( ch, ch->fighting, dam, F_JAB );
  num = number_percent();
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
    num += ch->pcdata->powers[F_KI];
  if ( num < 15 )
    ki += 1;
  else if ( num < 30 )
    ki -= 1;
  gain_ki( ch, ki );
  WAIT_STATE( ch, 8 );
  return;

}

void do_spinkick( CHAR_DATA *ch, char *argument )
{
  int dam, num;
  int ki = (ch->pcdata->powers[F_MASTER]/100) % 10;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "You look just like Jackie Chan....NOT.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < F_SPINKICK )
  { send_to_char( "You have not yet mastered that attack.\n\r", ch );
    return;
  }

  if (ch->fighting == NULL )
  { send_to_char( "But you are not in combat!\n\r", ch );
    return;
  }

  dam = dice( 10-ki, ch->pcdata->weapons[0] / 4 );

  damage( ch, ch->fighting, dam, F_SPINKICK );
  num = number_percent();
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
    num += ch->pcdata->powers[F_KI];
  if ( num < 15 )
    ki += 1;
  else if ( num < 30 )
    ki -= 1;
  gain_ki( ch, ki );
  WAIT_STATE( ch, 8 );
  return;
}

void do_knee( CHAR_DATA *ch, char *argument )
{
  int dam, num;
  int ki = (ch->pcdata->powers[F_MASTER]/1000) % 10;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "You knee yourself in the groin.. smooth move!\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < F_KNEE )
  { send_to_char( "You have not yet mastered that attack.\n\r", ch );
    return;
  }

  if (ch->fighting == NULL )
  { send_to_char( "But you are not in combat!\n\r", ch );
    return;
  }

  dam = dice( 10-ki, ch->pcdata->weapons[0] / 4 );

  damage( ch, ch->fighting, dam, F_KNEE );
  num = number_percent();
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
    num += ch->pcdata->powers[F_KI];
  if ( num < 15 )
    ki += 1;
  else if ( num < 30 )
    ki -= 1;
  gain_ki( ch, ki );
  WAIT_STATE( ch, 8 );
  return;

}

void do_elbow( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  int dam, num;
  int ki = (ch->pcdata->powers[F_MASTER]/10000) % 10;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "You stick your elbows out and cluck like a chicken.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < F_ELBOW )
  { send_to_char( "You have not yet mastered that attack.\n\r", ch );
    return;
  }

  if (ch->fighting == NULL )
  { if ( argument[0] == '\0' )
    { send_to_char( "Elbow who?\n\r", ch );
      return;
    }
    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      WAIT_STATE( ch, 4 );
      return;
    }
  }
  else
    victim = ch->fighting;

  if ( !IS_NPC(victim) && victim->level < 2 )
  {
    stc( "Not on mortals.\n\r", ch );
    return;
  }
  dam = dice( 10-ki, ch->pcdata->weapons[0] / 4 );

  damage( ch, victim, dam, F_ELBOW );
  num = number_percent();
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
    num += ch->pcdata->powers[F_KI];
  if ( num < 15 )
    ki += 1;
  else if ( num < 30 )
    ki -= 1;
  gain_ki( ch, ki );
  WAIT_STATE( ch, 8 );
  return;

}

void do_uppercut( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  int dam, num;
  int ki = ch->pcdata->powers[F_MASTER]/100000;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "Your fist connects with your jaw, and you see stars.\n\r", ch );
    damage( ch, ch, number_range(2,5), F_UPPERCUT );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < F_UPPERCUT )
  { send_to_char( "You have not yet mastered that attack.\n\r", ch );
    return;
  }

  if (ch->fighting == NULL )
  { if ( argument[0] == '\0' )
    { send_to_char( "Uppercut who?\n\r", ch );
      return;
    }
    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      WAIT_STATE( ch, 4 );
      return;
    }
  }
  else 
    victim = ch->fighting;

  if ( !IS_NPC(victim) && victim->level < 2 )
  {
    stc( "Not on mortals.\n\r", ch );
    return;
  }
  dam = dice( 10-ki, ch->pcdata->weapons[0] / 4 );

  damage( ch, victim, dam, F_UPPERCUT );
  num = number_percent();
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
    num += ch->pcdata->powers[F_KI];
  if ( num < 15 )
    ki += 1;
  else if ( num < 30 )
    ki -= 1;
  gain_ki( ch, ki );
  WAIT_STATE( ch, 8 );
  return;
}

void gain_ki( CHAR_DATA *ch, int ki )
{
  int oldki;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
    return;

  if ( ch->pcdata->powers[F_KI] == 0 )
  { send_to_char( "Your body glows dimly with Ki.\n\r", ch );
    act( "$n begins to glow dimly.", ch, NULL, NULL, TO_ROOM );
  }

  if ( ki > ch->pcdata->powers[F_KI_MAX] )
    return;

  if ( ch->pcdata->powers[F_KI] + ki > ch->pcdata->powers[F_KI_MAX] )
  { send_to_char( "Your ki exceeds your training, and you lose control of it.\n\r", ch );
    ch->pcdata->powers[F_KI] = 0;
  }

  oldki = ch->pcdata->powers[F_KI];
  ch->pcdata->powers[F_KI] += ki;

  if ( ch->pcdata->powers[F_KI] > 30 )
  { send_to_char( "The energy around you flares violently and collapses.  You overextended.\n\r", ch );
    ch->pcdata->powers[F_KI] = 0;
    return;
  }
  else if ( ch->pcdata->powers[F_KI] >= 20 && oldki < 20 )
  { send_to_char( "Your Ki flame burns with intense energy!\n\r", ch );
    act( "$n glows like a christmas tree!.", ch, NULL, NULL, TO_ROOM );
  }
  else if ( ch->pcdata->powers[F_KI] >= 10 && oldki < 10 )
  { send_to_char( "The Ki surrounding you burns brightly.\n\r", ch );
    act( "The energy surounding $n burns brightly.", ch, NULL, NULL, TO_ROOM );
  }

  return;
}

void do_master( CHAR_DATA *ch, char *argument )
{
  int ability = 0, move1, move2, i, j;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "Best leave mastery of the martial arts to the REAL experts.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' || !str_cmp( arg1, "list" ) )
  {
    stc( "Attacks | shinkick | jab | spinkick | knee | elbow | uppercut\n\r", ch );
    stc( "--------+----------+-----+----------+------+-------+----------\n\r", ch );
    sprintf( buf, "Learned |   [%c]    | [%c] |   [%c]    | [%c]  |  [%c]  |   [%c]\n\r",
    	(ch->pcdata->powers[F_LEARNED] >= F_SHINKICK ? 'x' : ' '),
    	(ch->pcdata->powers[F_LEARNED] >= F_JAB ? 'x' : ' '),
    	(ch->pcdata->powers[F_LEARNED] >= F_SPINKICK ? 'x' : ' '),
    	(ch->pcdata->powers[F_LEARNED] >= F_KNEE ? 'x' : ' '),
    	(ch->pcdata->powers[F_LEARNED] >= F_ELBOW ? 'x' : ' '),
    	(ch->pcdata->powers[F_LEARNED] >= F_UPPERCUT ? 'x' : ' ') );
    stc( buf, ch );
    sprintf( buf, "Ki Gain |   (%d)    | (%d) |   (%d)    | (%d)  |  (%d)  |   (%d)\n\r",
    	ch->pcdata->powers[F_MASTER] % 10,
    	(ch->pcdata->powers[F_MASTER]/10)%10,
    	(ch->pcdata->powers[F_MASTER]/100)%10,
    	(ch->pcdata->powers[F_MASTER]/1000)%10,
    	(ch->pcdata->powers[F_MASTER]/10000)%10,
    	(ch->pcdata->powers[F_MASTER]/100000) );
    stc( buf, ch );
    sprintf( buf, "\n\rMaximum Ki: %d\n\r", ch->pcdata->powers[F_KI_MAX] );
    stc( buf, ch );
    return;
  }


  if ( !str_cmp( arg1, "shinkick" ) )
    ability = F_SHINKICK;
  else if ( !str_cmp( arg1, "jab" ) )
    ability = F_JAB;
  else if ( !str_cmp( arg1, "spinkick" ) )
    ability = F_SPINKICK;
  else if ( !str_cmp( arg1, "knee" ) )
    ability = F_KNEE;
  else if ( !str_cmp( arg1, "elbow" ) )
    ability = F_ELBOW;
  else if ( !str_cmp( arg1, "uppercut" ) )
    ability = F_UPPERCUT;
  else if ( !str_cmp( arg1, "ki" ) )
  { if ( ch->pcdata->powers[F_KI_MAX] >= 30 )
    { send_to_char( "You have developed your Ki to its maximum potential already.\n\r", ch );
      return;
    }


    if ( ch->exp < 500000 )
    {
      stc( "You need 500000 exp to increase your maximum Ki.\n\r", ch );
      return;
    }
    ch->pcdata->powers[F_KI_MAX]++;
    if ( ch->pcdata->powers[F_KI_MAX] == 1 )
      send_to_char( "You have learned the fundamentals of holding your Ki.\n\r", ch );
    else if ( ch->pcdata->powers[F_KI_MAX] <= 9 )
      send_to_char( "You learn more about holding your Ki.\n\r", ch );
    else if ( ch->pcdata->powers[F_KI_MAX] == 10 )
      send_to_char( "You have learned the fundamentals of channelling your Ki.\n\r", ch );
    else if ( ch->pcdata->powers[F_KI_MAX] <= 19 )
      send_to_char( "You learn more about channelling your Ki.\n\r", ch );
    else if ( ch->pcdata->powers[F_KI_MAX] == 20 )
      send_to_char( "You have learned the fundamentals of focusing your Ki.\n\r", ch );
    else if ( ch->pcdata->powers[F_KI_MAX] <= 29 )
      send_to_char( "You learn more about focusing your Ki.\n\r", ch );
    else
      send_to_char( "You have mastered the use of your Ki as a Fist.\n\r", ch );
    ch->exp -= 500000;
    return;
  }
  else if ( !str_cmp( arg1, "swap" ) )
  {
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    if ( arg2[0] == '\0' || arg3[0] == '\0' )
    {
      stc( "Syntax: master swap move1 move2\n\r", ch );
      return;
    }
    if ( !str_cmp( arg2, "shinkick" ) )
      move1 = F_SHINKICK-1300;
    else if ( !str_cmp( arg2, "jab" ) )
      move1 = F_JAB-1300;
    else if ( !str_cmp( arg2, "spinkick" ) )
      move1 = F_SPINKICK-1300;
    else if ( !str_cmp( arg2, "knee" ) )
      move1 = F_KNEE-1300;
    else if ( !str_cmp( arg2, "elbow" ) )
      move1 = F_ELBOW-1300;
    else if ( !str_cmp( arg2, "uppercut" ) )
      move1 = F_UPPERCUT-1300;
    else
    {
      stc( "Unacceptable move.\n\r", ch );
      return;
    }
 
    if ( !str_cmp( arg3, "shinkick" ) )
      move2 = F_SHINKICK-1300;
    else if ( !str_cmp( arg3, "jab" ) )
      move2 = F_JAB-1300;
    else if ( !str_cmp( arg3, "spinkick" ) )
      move2 = F_SPINKICK-1300;
    else if ( !str_cmp( arg3, "knee" ) )
      move2 = F_KNEE-1300;
    else if ( !str_cmp( arg3, "elbow" ) )
      move2 = F_ELBOW-1300;
    else if ( !str_cmp( arg3, "uppercut" ) )
      move2 = F_UPPERCUT-1300;
    else 
    {
      stc( "Unacceptable move.\n\r", ch );
      return;
    }

    if ( move1 == move2 )
    {
      stc( "You can't swap the same move with itself.\n\r", ch );
      return;
    }

    if ( ch->pcdata->primal < 150 )
    {
      stc( "You require 150 primal to swap master moves.\n\r", ch );
      return;
    }

    i = (ch->pcdata->powers[F_MASTER] / (int)pow(10,move1)) % 10;
    j = (ch->pcdata->powers[F_MASTER]/ (int)pow(10,move2)) % 10;
    ch->pcdata->powers[F_MASTER] -= i * pow(10,move1);
    ch->pcdata->powers[F_MASTER] -= j * pow(10,move2);
    ch->pcdata->powers[F_MASTER] += i * pow(10,move2);
    ch->pcdata->powers[F_MASTER] += j * pow(10,move1);
    ch->pcdata->primal -= 150;
    stc( "Done.\n\r", ch );
    return;
  }
  else
  { send_to_char( "Try 'master list' for a list of things to master.\n\r", ch );
    return;
  }

  if ( ch->exp <= 500000 )
  {
    send_to_char( "You do not have enough experience.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[F_LEARNED] >= ability )
  { send_to_char( "You have already mastered that.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_LEARNED] < (ability-1)  && ability != F_SHINKICK )
  { send_to_char( "You must master a simpler technique first.\n\r", ch );
    return;
  }

  send_to_char( "You have mastered a new combat ability!\n\r", ch );
  ch->exp -= 500000;
  ch->pcdata->powers[F_LEARNED] = ability;
  return;
}

void do_bodytrain( CHAR_DATA *ch, char *argument )
{
  int cost, abil;
  char buf[MAX_STRING_LENGTH];
  
  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_FIST )
  { send_to_char( "You don't know the secrets of Fist body training.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "You must specify which body part to train:\n\rTorso, hands, arms, legs.\n\r", ch );
    return;
  }

  if ( !str_cmp( argument, "torso" ) )
  { cost = 1500 * (ch->pcdata->powers[F_TORSO]+1) * isquare(ch->pcdata->powers[F_TORSO]+1);
    abil = F_TORSO;
  }
  else if ( !str_cmp( argument, "hands" ) )
  { cost = 1500 * (ch->pcdata->powers[F_HANDS]+1) * isquare(ch->pcdata->powers[F_HANDS]+1);
    abil = F_HANDS;
  }
  else if ( !str_cmp( argument, "arms" ) )
  { cost = 1500 * (ch->pcdata->powers[F_ARMS]+1) * isquare(ch->pcdata->powers[F_HANDS]+1);
    abil = F_ARMS;
  }
  else if ( !str_cmp( argument, "legs" ) )
  { cost = 1500 * (ch->pcdata->powers[F_LEGS]+1) * isquare(ch->pcdata->powers[F_HANDS]+1);
    abil = F_LEGS;
  }
  else
  { send_to_char( "You may train your torso, hands, arms, or legs.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[abil] >= 100 )
  { send_to_char( "You have already attained peak physical condition in that part.\n\r", ch );
    return;
  }
  if ( cost > ch->exp )
  { sprintf( buf, "You need %d experience to train your body.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  send_to_char( "You train and train, and gradually become buffer.\n\r", ch );
  ch->exp -= cost;
  ch->pcdata->powers[abil]++;
  return;
}

void do_discipline( CHAR_DATA *ch, char *argument )
{
  int cost;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) ) 
    return;
  if ( ch->class != CLASS_FIST )
  { send_to_char( "You are not disciplined enough.\n\r", ch );
    return;
  }

  cost = 500000 * ( 1 + ch->pcdata->powers[F_DISC] );
  if ( ch->exp < cost )
  { sprintf( buf, "You need %d experience to achieve the next level of discipline.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  if ( ch->pcdata->powers[F_DISC] >= 10 )
  { send_to_char( "You have already mastered all your disciplines.\n\r", ch );
    return;
  }

  ch->exp -= cost;
  ch->pcdata->powers[F_DISC]++;
  sprintf( buf, "You achieve rank %d of Fist disciplines.\n\r", ch->pcdata->powers[F_DISC] );
  send_to_char( buf, ch );
  return;
}

void do_eyesight( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_FIST )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_DISC] < 1 )
  { send_to_char( "You are not sufficiently disciplined to use this.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->act, PLR_TRUESIGHT) )
  { send_to_char( "You relax your eyes.\n\r", ch );
    REMOVE_BIT(ch->act, PLR_TRUESIGHT);
  }
  else
  { send_to_char( "You concentrate on enhancing your vision.\n\r", ch );
    SET_BIT(ch->act,PLR_TRUESIGHT);
  }

  return;
}

void do_levitate( CHAR_DATA *ch, char *argument )
{
  int sn, lev;
  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_FIST )
  { send_to_char( "Hu?\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[F_DISC] < 4 )
  { send_to_char( "You are not disciplined enough.\n\r", ch );
    return;
  }
  if ( ch->move < 1000 )
  { send_to_char( "You need 1000 move to levitate yourself.\n\r", ch );
    return;
  }
  sn = skill_lookup( "fly" );
  lev = ch->pcdata->spirit / 3;
  send_to_char( "You focus your Ki and gently lift yourself from the ground.\n\r", ch );
  ch->move -= 1000;
  ( skill_table[sn].spell_fun ) ( sn, lev, ch, ch );
  return;
}

void do_roundhouse( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_FIST )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_DISC] < 5 )
  { send_to_char( "You are not disciplined enough.\n\r", ch );
    return;
  }
  if ( ch->move < 150 )
  { send_to_char( "You are too tired.\n\r", ch );
    return;
  }

  act( "You spin into a powerful roundhouse kick.", ch, NULL, NULL, TO_CHAR );
  act( "$n spins into a powerful roundhouse kick.", ch, NULL, NULL, TO_ROOM );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( !is_same_group( ch, vch ) && can_see( ch, vch ) )
    { dam = ch->pcdata->body + dice( 15, ch->pcdata->powers[F_LEGS]*2 );
      damage( ch, vch, dam, F_SPINKICK );
    }
  }

  ch->move -= 150;
  WAIT_STATE( ch, 12 );
  return;
}

void do_innerfire( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_FIST )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[F_DISC] <  7)
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_KI] < 1 )
  { send_to_char( "Using your inner fire requires Ki points.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->pcdata->actnew,NEW_INNERFIRE) )
  { send_to_char( "You are already doing that!\n\r", ch );
    return;
  }

  if ( dice(1,15) > ch->pcdata->powers[F_KI] )
  { send_to_char( "You attempt to focus your Ki, but lose your concentration.\n\r", ch );
    ch->pcdata->powers[F_KI] = 0;
    return;
  }

  SET_BIT(ch->pcdata->actnew,NEW_INNERFIRE);
  ch->pcdata->powers[F_KI] = 0;
  send_to_char( "You turn your focus inward, attempting to burn away any impurities.\n\r", ch );
  return;
}

void do_phoenixaura( CHAR_DATA *ch, char *argument )
{ if ( !IS_CLASS(ch,CLASS_FIST) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[F_DISC] < 8 )
  { send_to_char( "You aren't disciplined enough.\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_PHOENIX_AURA) )
  { act( "You relax your will, and allow the Phoenix Aura to collapse.", ch, NULL, NULL, TO_CHAR );
    act( "The aura of Ki Flame around $n flickers and dies.", ch, NULL, NULL, TO_ROOM );
    REMOVE_BIT(ch->pcdata->actnew,NEW_PHOENIX_AURA);
    ch->hitroll -= 15;
    ch->damroll -= 15;
  }
  else
  {
    if ( ch->move < 500 )
    { 
      send_to_char( "You are too exhausted to summon forth the Phoenix Aura.\n\r", ch );
      return;
    }
    act( "You center your mind, and expand your aura.", ch, NULL, NULL, TO_CHAR );
    act( "$n glows with spiritual flame.", ch, NULL, NULL, TO_CHAR );
    SET_BIT(ch->pcdata->actnew,NEW_PHOENIX_AURA);
    ch->hitroll += 15;
    ch->damroll += 15;
    ch->move -= 500;
  }
  return;
}

void do_dim_mak( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int dam, kicost, movecost;

  if ( !IS_CLASS(ch,CLASS_FIST) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_DISC] < 9 )
  {
    stc( "You are not disciplined enough.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[F_KI] > 0 )
  {
    kicost = URANGE( 0, ch->pcdata->powers[F_KI], 20 );
    movecost = 1000 - kicost*50;
  }
  else
  {
    kicost = 0;
    movecost = 1000;
  }

  if ( ch->move < movecost )
  {
    stc( "Your are too exhausted to use the Dim-Mak.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    stc( "Poison who?\n\r", ch );
    return;
  }

  if ( (victim = get_char_room(ch,arg)) == NULL )
  {
    stc( "They aren't here.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }

  dam = dice( 10, 200+isquare(ch->pcdata->kills[PK]) );
  if ( IS_CLASS(victim,CLASS_MAZOKU) )
  {
    if ( IS_SET(victim->pcdata->powers[M_SET],M_TRUE) )
      dam *= 2;
    if ( IS_SET(victim->pcdata->powers[M_SET],M_ASTRIKE) )
      dam *= 5;
  }

  act( "You strike at $N's throat, dark Ki coiling around your fingers!", ch, NULL, victim, TO_CHAR );
  act( "$n strikes at your throat, dark energy coiling around $s fingers!", ch, NULL, victim, TO_VICT );
  act( "$n strikes at $N's throat, dark energy coiling around $s fingers.", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dam, F_DEATHTOUCH );
  if ( victim != NULL )
  {
    if ( IS_CLASS(victim,CLASS_MAZOKU) )
    {
      if ( IS_SET(victim->pcdata->powers[M_SET],M_ASTRIKE) )
      {
        REMOVE_BIT(victim->pcdata->powers[M_SET],M_ASTRIKE);
        act( "$N writhes in agony!", ch, NULL, victim, TO_CHAR );
        act( "Your very being is wracked by the poison!", ch, NULL, victim, TO_VICT );
        act( "$N writhes in agony.", ch, NULL, victim, TO_NOTVICT );
      }
    }
    else if ( IS_CLASS(victim,CLASS_SORCERER) && number_percent() < 60 )
    {
      act( "$N stares blankly, their mystickal energy dispelled.", ch, NULL, victim, TO_CHAR );
      act( "Your guts twist as your energy is drained!", ch, NULL, victim, TO_VICT );
      lose_chant( victim );
    }
  }
  ch->move -= movecost;
  ch->pcdata->powers[F_KI] -= kicost;
  WAIT_STATE(ch,10);
  return;
}
