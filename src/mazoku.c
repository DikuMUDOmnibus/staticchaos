/* original by Alathon */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"


void do_develop( CHAR_DATA *ch, char *argument )
{ char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *desc;
  int cost = 0, flag = 0;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  
  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || !str_cmp( arg, "list" ) )
  { send_to_char( "Claws:     50  Spikes:   125  Blades:    150  Tentacles: 150\n\r", ch );
    send_to_char( "Third:     75  Fourth:   100  Fifth:     150  Sixth:     200\n\r", ch );
    send_to_char( "Wings:    100  Eyes:     100  Teleport:  200  Astrike:   200\n\r", ch );
    send_to_char( "Charge:   125  Blast:    150  Bolt:      150  Bomb:      150\n\r", ch );
    sprintf( buf, "Nihilism: %d  Matter: %d  Astral %d  Focus %d\n\r",
    	ch->pcdata->powers[M_NIHILISM]*1000,
    	(ch->pcdata->powers[M_MATTER]*10+500)*(ch->pcdata->powers[M_MATTER]*10+500),
        (ch->pcdata->powers[M_ASTRAL]*10+500)*(ch->pcdata->powers[M_ASTRAL]*10+500),
        (ch->pcdata->powers[M_FOCUS]*10+500)*(ch->pcdata->powers[M_FOCUS]*10+500) );
    send_to_char( buf, ch );
    return;
  }
  else if ( !str_cmp( arg, "nihilism" ) )
  { if ( ch->pcdata->powers[M_NIHILISM] <= 0 )
    { send_to_char( "But you are already at peace with oblivion!\n\r", ch );
      return;
    }
    if ( ch->pcdata->powers[M_NIHILISM]*1000 > ch->exp )
    { sprintf( buf, "You need %d experience to get a hold of yourself.\n\r",
        ch->pcdata->powers[M_NIHILISM]*1000 );
      send_to_char( buf, ch );
      return;
    }
    ch->exp -= ch->pcdata->powers[M_NIHILISM]*1000;
    ch->pcdata->powers[M_NIHILISM]--;
    send_to_char( "You are more at peace with yourself.\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg, "matter" ) )
  { if ( ch->pcdata->powers[M_MATTER] >= 100 )
    { send_to_char( "You have already mastered the manipulation of matter.\n\r", ch );
      return;
    }
    cost = (ch->pcdata->powers[M_MATTER]*10+500)*(ch->pcdata->powers[M_MATTER]*10+500);
    if ( ch->exp < cost )
    { sprintf( buf, "You need %d experience to improve your master of matter.\n\r", cost );
      send_to_char( buf, ch );
      return;
    }
    ch->exp -= cost;
    ch->pcdata->powers[M_MATTER]++;
    send_to_char( "You become more adept at manipulating matter.\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg, "astral" ) )
  { if ( ch->pcdata->powers[M_ASTRAL] >= 100 )
    { send_to_char( "You have already mastered the manipulation of astral energy.\n\r", ch );
      return;
    }
    cost = (ch->pcdata->powers[M_ASTRAL]*10+500)*(ch->pcdata->powers[M_ASTRAL]*10+500);
    if ( ch->exp < cost )
    { sprintf( buf, "You need %d experience to improve your mastery of astral energy.\n\r", cost );
      send_to_char( buf, ch );
      return;
    }
    ch->exp -= cost;
    ch->pcdata->powers[M_ASTRAL]++;
    send_to_char( "You become more adept at manipulating astral energy.\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg, "focus" ) )
  { if ( ch->pcdata->powers[M_FOCUS] >= 100 )
    { send_to_char( "You are as focused as you can get.\n\r", ch );
      return;
    }
    cost = (ch->pcdata->powers[M_FOCUS]*10+500)*(ch->pcdata->powers[M_FOCUS]*10+500);
    if ( ch->exp < cost )
    { sprintf( buf, "You need %d experience to improve your focus.\n\r", cost );
      send_to_char( buf, ch );
      return;
    }
    ch->exp -= cost;
    ch->pcdata->powers[M_FOCUS]++;
    send_to_char( "You become more focused.\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg, "claws" ) )
  { cost = 50;
    flag = M_CLAWS;
    desc = "claws";
  }
  else if ( !str_cmp( arg, "spikes" ) )
  { cost = 125;
    flag = M_SPIKES;
    desc = "spikes";
  }
  else if ( !str_cmp( arg, "blades" ) )
  { cost = 150;
    flag = M_BLADES;
    desc = "blades";
  }
  else if ( !str_cmp( arg, "tentacles" ) )
  { cost = 150;
    flag = M_TENTACLES;
    desc = "tentacles";
  }
  else if ( !str_cmp( arg, "third" ) )
  { cost = 75;
    flag = M_THIRD;
    desc = "a third arm";
  }
  else if ( !str_cmp( arg, "fourth" ) )
  { cost = 100;
    flag = M_FOURTH;
    desc = "a fourth arm";
  }
  else if ( !str_cmp( arg, "fifth" ) )
  { cost = 150;
    flag = M_FIFTH;
    desc = "a fifth arm";
  }
  else if ( !str_cmp( arg, "sixth" ) )
  { cost = 200;
    flag = M_SIXTH;
    desc = "a sixth arm";
  }
  else if ( !str_cmp( arg, "wings" ) )
  { cost = 100;
    flag = M_WINGS;
    desc = "wings";
  }
  else if ( !str_cmp( arg, "eyes" ) )
  { cost = 100;
    flag = M_EYES;
    desc = "pineal eyes";
  }
  else if ( !str_cmp( arg, "teleport" ) )
  { cost = 200;
    flag = M_TELEPORT;
    desc = "the ability to teleport";
  }
  else if ( !str_cmp( arg, "astrike" ) )
  { cost = 200;
    flag = M_ASTRIKE;
    desc = "the ability to strike from the astral plane";
  }
  else if ( !str_cmp( arg, "charge" ) )
  { cost = 125;
    flag = M_CHARGE;
    desc = "the ability to charge";
  }
  else if ( !str_cmp( arg, "blast" ) )
  { cost = 150;
    flag = M_BLAST;
    desc = "an astral blast";
  }
  else if ( !str_cmp( arg, "bolt" ) )
  { cost = 150;
    flag = M_BOLT;
    desc = "an astral bolt";
  } 
  else if ( !str_cmp( arg, "bomb" ) )
  { cost = 150;
    flag = M_BOMB;
    desc = "an astral bomb";
  }
  else
  { send_to_char( "That is not something you can develop.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->pcdata->powers[M_LEARNED],flag) )
  { send_to_char( "You have already developed that.\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < cost )
  { sprintf( buf, "You need %d primal to develop that.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  ch->pcdata->primal -= cost;
  SET_BIT(ch->pcdata->powers[M_LEARNED],flag);
  sprintf( buf, "You have developed %s.\n\r", desc );
  send_to_char( buf, ch );
  return;
}

void do_form( CHAR_DATA *ch, char *argument )
{ char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *desc;
  int flag = 0, cost = 0;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->powers[M_SET],M_HUMAN) )
  {
    stc( "You wouldn't be very human with anything other than hands.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  { send_to_char( "Form your arms into what?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "hands" ) )
  { flag = M_HANDS;
    cost = 5;
    desc = "hands";
  }
  else if ( !str_cmp( arg, "claws" ) )
  { flag = M_CLAWS;
    cost = 10;
    desc = "jagged claws";
  }
  else if ( !str_cmp( arg, "spikes" ) )
  { flag = M_SPIKES;
    cost = 12;
    desc = "cruelly hooked spikes";
  }
  else if ( !str_cmp( arg, "blades" ) )
  { flag = M_BLADES;
    cost = 15;
    desc = "curved blades";
  }
  else if ( !str_cmp( arg, "tentacles" ) )
  { flag = M_TENTACLES;
    cost = 25;
    desc = "supple tentacles";
  }
  else
  { send_to_char( "Try hands, claws, spikes, blades, or tentacles.\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[M_LEARNED],flag) && flag != M_HANDS)
  { send_to_char( "You haven't developed that technique.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[M_ESSENSE] <= cost )
  { sprintf( buf, "You need %d essense to form that.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  if ( IS_SET(ch->pcdata->powers[M_SET],M_HANDS) )
    REMOVE_BIT(ch->pcdata->powers[M_SET],M_HANDS);
  if ( IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
    REMOVE_BIT(ch->pcdata->powers[M_SET],M_CLAWS);
  if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
    REMOVE_BIT(ch->pcdata->powers[M_SET],M_SPIKES);
  if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
    REMOVE_BIT(ch->pcdata->powers[M_SET],M_BLADES);
  if ( IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
    REMOVE_BIT(ch->pcdata->powers[M_SET],M_TENTACLES);
  
  SET_BIT(ch->pcdata->powers[M_SET],flag);
  ch->pcdata->powers[M_ESSENSE] -= cost;
  act( "You form your arms into $T.", ch, NULL, desc, TO_CHAR );
  act( "$n forms $s arms into $T.", ch, NULL, desc, TO_ROOM );
  return;
}
 
void do_morph( CHAR_DATA *ch, char *argument )
{ char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  AFFECT_DATA af;
  
  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( !str_cmp( arg, "human" ) )
  { if ( IS_SET(ch->pcdata->powers[M_SET],M_HUMAN) )
    { send_to_char( "You're as human as you're gonna get.\n\r", ch );
      return;
    }
    if ( ch->pcdata->powers[M_ESSENSE] <= 50 )
    { send_to_char( "You need 50 essense to form a material body.\n\r", ch );
      return;
    }

    if ( !IS_AFFECTED(ch, AFF_INFRARED) )
    {
      af.type      = skill_lookup( "infravision" );
      af.duration  = -1;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_INFRARED;
      affect_to_char( ch, &af );
    }
    ch->pcdata->powers[M_ESSENSE] -= 50;
    if ( IS_SET(ch->pcdata->powers[M_SET],M_BATTLE) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_BATTLE);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_TRUE) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_TRUE);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_CLAWS);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_TENTACLES);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_BLADES) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_BLADES);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_SPIKES);
    if ( !IS_SET(ch->pcdata->powers[M_SET],M_HANDS) )
      SET_BIT(ch->pcdata->powers[M_SET],M_HANDS);
    SET_BIT(ch->pcdata->powers[M_SET],M_HUMAN);
    act( "You dissolve your body and reform looking vaguely human.", ch, NULL, NULL, TO_CHAR );
    act( "$n dissolves $s body and reforms looking vaguely human.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, 4 );
  }
  else if ( !str_cmp( arg, "battle" ) )
  { if ( IS_SET(ch->pcdata->powers[M_SET],M_BATTLE) )
    { send_to_char( "You have already assumed your battle form.\n\r", ch );
      return;
    }
    if ( ch->pcdata->powers[M_ESSENSE] <= 100 )
    { send_to_char( "You need 100 essense to assume this form.\n\r", ch );
      return;
    }

    if ( !IS_AFFECTED(ch, AFF_INFRARED) )
    {
      af.type      = skill_lookup( "infravision" );
      af.duration  = -1;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_INFRARED;
      affect_to_char( ch, &af );
    }
    ch->pcdata->powers[M_ESSENSE] -= 100;
    if ( IS_SET(ch->pcdata->powers[M_SET],M_HUMAN) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_HUMAN);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_TRUE) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_TRUE);
    SET_BIT(ch->pcdata->powers[M_SET],M_BATTLE);
    act( "You dissolve your body and reform in a monstrous battle hame.", ch, NULL, NULL, TO_CHAR );
    act( "$n dissolves $s body and reforms in a monstrous battle hame.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, 8 );
  }
  else if ( !str_cmp( arg, "true" ) )
  { if ( IS_SET(ch->pcdata->powers[M_SET],M_TRUE) )
    { send_to_char( "You are already in your true form.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED(ch,AFF_INFRARED) )
      affect_strip( ch, gsn_infrared );
    if ( IS_SET(ch->pcdata->powers[M_SET],M_HUMAN) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_HUMAN);
    if ( IS_SET(ch->pcdata->powers[M_SET],M_BATTLE) )
      REMOVE_BIT(ch->pcdata->powers[M_SET],M_BATTLE);
    SET_BIT(ch->pcdata->powers[M_SET],M_TRUE);
    act( "You abandon the material plane and assume your true astral form.", ch, NULL, NULL, TO_CHAR );
    act( "$n becomes diffuse and fades almost entirely from sight.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, 8 );
  }
  else
    send_to_char( "The forms you can morph into are Human, Battle, and True.\n\r", ch );

  if ( !IS_SET(ch->pcdata->powers[M_SET],M_HUMAN) )
  { 
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;
      if ( obj->wear_loc != WEAR_NONE && !IS_OBJ_STAT(obj,ITEM_ASTRAL) )
      { 
        act( "You drop $p.", ch, obj, NULL, TO_CHAR );
        act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
        unequip_char( ch, obj );
      }
    }
  }

  return;
}

void do_charge( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  int color = 0;
  char *cdesc;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );

  if ( !IS_SET(ch->pcdata->powers[M_LEARNED],M_CHARGE) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( ch->position != POS_FIGHTING )
  {
    stc( "Charge up to attack what?\n\r", ch );
    return;
  }
  if ( arg1[0] == '\0' )
  { send_to_char( "Syndax is charge <obsidian/emerald/cerulean/crimson>.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg1, "cerulean" ) )
  { color = DAM_CERULEAN;
    cdesc = "deep cerulean";
  }
  else if ( !str_prefix( arg1, "obsidian" ) )
  { color = DAM_OBSIDIAN;
    cdesc = "shadowy obsidian";
  }
  else if ( !str_prefix( arg1, "crimson" ) )
  { color = DAM_CRIMSON;
    cdesc = "flaming crimson";
  }
  else if ( !str_prefix( arg1, "emerald" ) )
  { color = DAM_EMERALD;
    cdesc = "emerald tinted";
  }
  else
  { send_to_char( "That is not an acceptable color.\n\r", ch );
    cdesc = "none";
    return;
  }

  /* removed for another syntax 
  if ( !str_prefix( arg2, "bolt" ) )
  { type = M_BOLT;
    strcat( desc, "bolt" );
  }
  else if ( !str_prefix( arg2, "blast" ) )
  { type = M_BLAST;
    strcat( desc, "blast" );
  }
  else if ( !str_prefix( arg2, "bomb" ) )
  { type = M_BOMB;
    strcat( desc, "bomb" );
  }
  else
  { send_to_char( "That is not an acceptable attack type.\n\r", ch );
    return;
  }
  */

  if ( ch->pcdata->powers[M_ESSENSE] < 250 )
  { send_to_char( "You don't have the essense to spend on such an attack!\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[M_CTYPE] != 0 )
  { send_to_char( "You dispel the previously gathered energy.\n\r", ch );
    ch->pcdata->powers[M_CTYPE] = 0;
    ch->pcdata->powers[M_CTIME] = 0;
  }

  act( "You begin charging a $T attack.", ch, NULL, cdesc, TO_CHAR );
  act( "$n glows with $T energy.", ch, NULL, cdesc, TO_ROOM );

  ch->pcdata->powers[M_CTYPE] = color;
  ch->pcdata->powers[M_CTIME] = 10;
  WAIT_STATE( ch, 15 - (ch->pcdata->powers[M_ASTRAL]/10) );
  return;
}

void do_release( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int dt, time, dam, mod;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }

  dt = ch->pcdata->powers[M_CTYPE];
  time = ch->pcdata->powers[M_CTIME];

  if ( dt <= 0 || time <= 0 )
  {
    stc( "You haven't charged up an attack yet.\n\r", ch );
    return;
  }

  if ( arg1[0] == '\0' )
  {
    stc( "Syntax: release <attack type> <target>\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
    victim = ch->fighting;
  else
  {
    if ( arg2[0] == '\0' )
    {
      stc( "Release your attack at whom?\n\r", ch );
      return;
    }
    else if ( (victim = get_char_room( ch, arg2 )) == NULL )
    {
      stc( "They aren't here.\n\r", ch );
      WAIT_STATE( ch, 4 );
      return;
    }
  }

  switch ( dt )
  {
    case DAM_CRIMSON:
      if ( IS_NPC(victim) )
        mod = 35;
      else
        mod = 10;
      break;
    case DAM_EMERALD:
      mod = 20;
      break;
    case DAM_CERULEAN:
      mod = 15;
      break;
    case DAM_OBSIDIAN:
      mod = 10;
      break;
    default:
      mod = 1;
      break;
  };

  if ( IS_SET(ch->pcdata->powers[M_SET],M_TRUE) )
    mod = mod * 7 / 4;

  if ( !str_prefix( arg1, "bolt" ) )
  {
    if ( !IS_SET(ch->pcdata->powers[M_LEARNED],M_BOLT) )
    {
      stc( "You aren't familiar with that mode of attack.\n\r", ch );
      return;
    }
    dam = dice( 15, ch->pcdata->powers[M_ASTRAL] );
    dam = dam * mod / 10;
    dam = dam * (time-5) / 5;
    act( "You release a concentrated bolt of energy at $N!", ch, NULL, victim, TO_CHAR );
    act( "$n launches a concentrated bolt of energy at you!", ch, NULL, victim, TO_VICT );
    act( "$n launches a concentrated bolt of energy at $N.", ch, NULL, victim, TO_NOTVICT );
    damage( ch, victim, dam, dt );
    WAIT_STATE(ch,4);
  }
  else if ( !str_prefix( arg1, "blast" ) )
  {
    if ( !IS_SET(ch->pcdata->powers[M_LEARNED],M_BLAST) )
    {
      stc( "You aren't familiar with that mode of attack.\n\r", ch );
      return;
    }
    dam = dice( 7, ch->pcdata->powers[M_ASTRAL] );
    dam = dam * mod / 10;
    dam = dam * (time-5) / 5;
    act( "You release a scintillating blast of energy at $N!", ch, NULL, victim, TO_CHAR );
    act( "$n directs a scintillating blast of energy at you!", ch, NULL, victim, TO_VICT );
    act( "$n directs a scintillating blast of energy at $N.", ch, NULL, victim, TO_NOTVICT );
    damage( ch, victim, dam, dt );
    if ( victim != NULL && !IS_NPC(victim) )
    { if ( IS_CLASS(victim,CLASS_FIST) && number_percent() < 50 )
      { if ( victim->pcdata->powers[F_KI] >= 4 )
          victim->pcdata->powers[F_KI] -= dice(1,4);
        if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
          REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
        act( "Your chaotic blast staggers $N!", ch, NULL, victim, TO_CHAR );
        act( "$n's chaotic blast breaks your rhythm!", ch, NULL, victim, TO_VICT );
        act( "$n's chaotic blast staggers $N.", ch, NULL, victim, TO_NOTVICT );
        WAIT_STATE(victim,4);
      }
      else if ( IS_CLASS(victim,CLASS_SAIYAN) )
      { if ( is_affected(victim,gsn_kiwall) )
        { affect_strip(victim,gsn_kiwall);
          act( "Your Ki wave staggers $N!", ch, NULL, victim, TO_CHAR );
          act( "$n's wave of energy collapes your wall of Ki!", ch, NULL, victim, TO_VICT );
          act( "$n's wave of energy staggers $N.", ch, NULL, victim, TO_NOTVICT );
        }
      }
      else if ( IS_CLASS(victim,CLASS_SORCERER) )
        lose_chant( victim );
    }   
    WAIT_STATE(ch,4);
  }
  else if ( !str_prefix( arg1, "bomb" ) )
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    if ( !IS_SET(ch->pcdata->powers[M_LEARNED],M_BOMB) )
    {
      stc( "You aren't familiar with that mode of attack.\n\r", ch );
      return;
    }
    act( "You release a tremendous explosion of energy!", ch, NULL, NULL, TO_CHAR );
    act( "$n explodes with energy!", ch, NULL, NULL, TO_ROOM );
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if ( !is_same_group(ch, vch) )
      {
        dam = dice( 11, ch->pcdata->powers[M_ASTRAL] );
        dam = dam * mod / 10;
        dam = dam * (time-5) / 5;
        damage( ch, vch, dam, dt );
      }
    }
    WAIT_STATE(ch,8);
  }
  else
  {
    stc( "Acceptable forms of attack are blast, bolt, and bomb.\n\r", ch );
    return;
  }

  ch->pcdata->powers[M_CTYPE] = 0;
  ch->pcdata->powers[M_CTIME] = 0;
  return;
}




void do_reform( CHAR_DATA *ch, char *argument )
{
  int cost;
  char buf[MAX_STRING_LENGTH];

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  cost = isquare( ch->max_hit - ch->hit ) + 50;
  if ( ch->pcdata->powers[M_ESSENSE] < cost )
  { sprintf( buf, "You need %d essense to reform yourself.\n\r", cost );
    stc( buf, ch );
    return;
  }

  act( "You repair the tears in your astral body.", ch, NULL, NULL, TO_CHAR );
  act( "The tears in $n's form begin to close.", ch, NULL, NULL, TO_ROOM );
  ch->pcdata->powers[M_ESSENSE] -= cost;
  ch->hit = UMIN( ch->hit + 100 + dice( 3, ch->pcdata->powers[M_ASTRAL] ), ch->max_hit );
  WAIT_STATE(ch,6);

  return;
}

void do_grow( CHAR_DATA *ch, char *argument )
{ char arg[MAX_INPUT_LENGTH];
  char *desc;
  int bit;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  { send_to_char( "Syntax: grow bodypart.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "third" ) )
  { bit = M_THIRD;
    desc = "a third arm";
  }
  else if ( !str_cmp( arg, "fourth" ) )
  { bit = M_FOURTH;
    desc = "a fourth arm";
  }
  else if ( !str_cmp( arg, "fifth" ) )
  { bit = M_FIFTH;
    desc = "a fifth arm";
  }
  else if ( !str_cmp( arg, "sixth" ) )
  { bit = M_SIXTH;
    desc = "a sixth arm";
  }
  else if ( !str_cmp( arg, "eye" ) )
  { bit = M_EYES;
    desc = "a pineal eye";
  }
  else if ( !str_cmp( arg, "wings" ) )
  { bit = M_WINGS;
    desc = "wings";
  }
  else
  { send_to_char( "You can grow: third, fourth, fifth, sixth, eye, wings.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->pcdata->powers[M_SET],bit) )
  { send_to_char( "You can't grow two of those.\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[M_LEARNED],bit) )
  {
    stc( "You haven't developed that ability.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[M_ESSENSE] <= 50000 )
  { send_to_char( "You need 50000 essense to grow a body part.\n\r", ch );
    return;
  }

  ch->pcdata->powers[M_ESSENSE] -= 50000;
  SET_BIT(ch->pcdata->powers[M_SET],bit);
  act( "You grow $T.", ch, NULL, desc, TO_CHAR );
  act( "$n grows $T.", ch, NULL, desc, TO_ROOM );
  WAIT_STATE(ch,12);
  return;
}

void do_astrike( CHAR_DATA *ch, char *argument )
{  
  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[M_LEARNED],M_ASTRIKE) )
  {
    stc( "YOu haven't developed that ability.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[M_ESSENSE] <= 250 )
  { send_to_char( "You need 250 essense to perform an astral strike.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[M_EGO] <= 1 )
  { send_to_char( "You don't need to do that to win!\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
  { send_to_char( "You've already initiated an astral strike!\n\r", ch );
    return;
  }
  if ( is_affected(ch,gsn_blindness) )
  {
    stc( "You need to be able to see your target.\n\r", ch );
    return;
  }
  if ( ch->position > POS_FIGHTING || ch->fighting == NULL )
  {
    stc( "You must have a target to astrike.\n\r", ch );
    return;
  }

  ch->pcdata->powers[M_ESSENSE] -= 250;
  SET_BIT(ch->pcdata->powers[M_SET],M_ASTRIKE);
  WAIT_STATE(ch,8);
  act( "You begin to phase in and out of the material plane..", ch, NULL, NULL, TO_CHAR );
  act( "$n begins to phase in and out of existance..", ch, NULL, NULL, TO_ROOM );
  return;
}

void do_instantiate( CHAR_DATA *ch, char *argument )
{ char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int item;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[M_ESSENSE] <= 10000 )
  { send_to_char( "You need 10000 spare essense to instantiate an astral item.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
  { send_to_char( "You can instantiate the following:\n\r", ch );
    send_to_char( "Ring, amulet, shirt, cap, pants, boots, gloves,\n\r", ch );
    send_to_char( "sleeves, cloak, belt, bracer, shield, sceptre.\n\r", ch );
    return;
  }
  else if ( !str_cmp( arg, "ring" ) )
    item = OBJ_VNUM_ASTRAL_RING;
  else if ( !str_cmp( arg, "amulet" ) )
    item = OBJ_VNUM_ASTRAL_AMULET;
  else if ( !str_cmp( arg, "shirt" ) )
    item = OBJ_VNUM_ASTRAL_SHIRT;
  else if ( !str_cmp( arg, "cap" ) )
    item = OBJ_VNUM_ASTRAL_CAP;
  else if ( !str_cmp( arg, "pants" ) )
    item = OBJ_VNUM_ASTRAL_PANTS;
  else if ( !str_cmp( arg, "boots" ) )
    item = OBJ_VNUM_ASTRAL_BOOTS;
  else if ( !str_cmp( arg, "gloves" ) )
    item = OBJ_VNUM_ASTRAL_GLOVES;
  else if ( !str_cmp( arg, "sleeves" ) )
    item = OBJ_VNUM_ASTRAL_SLEEVES;
  else if ( !str_cmp( arg, "boots" ) )
    item = OBJ_VNUM_ASTRAL_BOOTS;
  else if ( !str_cmp( arg, "cloak" ) )
    item = OBJ_VNUM_ASTRAL_CLOAK;
  else if ( !str_cmp( arg, "belt" ) )
    item = OBJ_VNUM_ASTRAL_BELT;
  else if ( !str_cmp( arg, "bracer" ) )
    item = OBJ_VNUM_ASTRAL_BRACER;
  else if ( !str_cmp( arg, "shield" ) )
    item = OBJ_VNUM_ASTRAL_SHIELD;
  else if ( !str_cmp( arg, "sceptre" ) )
    item = OBJ_VNUM_ASTRAL_SCEPTRE;
  else
  { send_to_char( "You can instantiate the following:\n\r", ch );
    send_to_char( "Ring, amulet, shirt, cap, pants, boots, gloves,\n\r", ch );
    send_to_char( "sleeves, boots, belt, bracer, shield, sceptre.\n\r", ch );
    return;
  }
  
  ch->pcdata->powers[M_ESSENSE] -= 10000;
  obj = create_object( get_obj_index( item ), 0 ); 
  obj->value[0] = 15;  // set AC to max
  obj_to_char( obj, ch );
  act( "Your pour your essence into $p.", ch, obj, NULL, TO_CHAR );
  act( "$p fades into being in $n's hands.", ch, obj, NULL, TO_ROOM );
  return;
}

void do_imbue( CHAR_DATA *ch, char *argument )
{ OBJ_DATA *obj;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int hitdam = 0, cost;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  { send_to_char( "Imbue what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
  { send_to_char( "You aren't carrying that.\n\r", ch );
    return;
  }

  if ( !IS_OBJ_STAT(obj,ITEM_ASTRAL) )
  { send_to_char( "Only astral items can be imbued.\n\r", ch );
    return;
  }

  for ( paf = obj->affected; paf != NULL; paf = paf_next )
  { paf_next = paf->next;
    if ( paf->location == APPLY_HITROLL )
      hitdam += paf->modifier;
  }
  
  if ( hitdam >= 10 )
  { send_to_char( "That item is as powerful as it can become.\n\r", ch );
    return;
  }

  cost = 8000 + (hitdam * 8000);
  if ( ch->pcdata->powers[M_ESSENSE] <= cost )
  { sprintf( buf, "You will need %d essense to imbue that.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  hitdam++;
  ch->pcdata->powers[M_ESSENSE] -= cost;
  act( "You channel a portion of your essense into $p.", ch, obj, NULL, TO_CHAR );
  act( "$n grasps $p in an almost intimate fashion..", ch, obj, NULL, TO_ROOM );


  for ( paf = obj->affected; paf != NULL; paf = paf_next )
  { paf_next = paf->next;
    if ( paf->location == APPLY_HITROLL )
      obj_affect_remove( obj, paf );
    else if ( paf->location == APPLY_DAMROLL )
      obj_affect_remove( obj, paf );
  }

  if ( affect_free == NULL )
    paf             = alloc_perm( sizeof(*paf), PERM_AFF );
  else
  { paf             = affect_free;
    affect_free     = affect_free->next;
  }

  paf->type           = 0;
  paf->duration       = -1;
  paf->location       = APPLY_HITROLL;
  paf->modifier       = hitdam;
  paf->bitvector      = 0;
  paf->next           = obj->affected;
  obj->affected       = paf;

  if ( affect_free == NULL )
    paf             = alloc_perm( sizeof(*paf), PERM_AFF);
  else
  { paf             = affect_free;
    affect_free     = affect_free->next;
  }

  paf->type           = 0;
  paf->duration       = -1;
  paf->location       = APPLY_DAMROLL;
  paf->modifier       = hitdam;
  paf->bitvector      = 0;
  paf->next           = obj->affected;
  obj->affected       = paf;

  return;
}

void do_teleport( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[M_LEARNED],M_TELEPORT) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[M_SET],M_TRUE) )
  {
    stc( "You must be in your true form to teleport.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[M_ESSENSE] < 500 )
  {
    stc( "You need 500 essense to teleport.\n\r", ch );
    return;
  }
  if ( ch->hit <= 500 )
  {
    stc( "You're too weak to teleport.\n\r", ch );
    return;
  }
  if ( ch->mana <= 500 )
  {
    stc( "You lack the energy to teleport.\n\r", ch );
    return;
  }
  if ( ch->move <= 500 )
  {
    stc( "You are too tired to teleport.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[TIMER] > 0 )
  {
    stc( "Why would you want to RUN from a fight?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->powers[M_SET],M_ASTRIKE) )
  {
    stc( "You have to be fully in the astral plane to teleport.\n\r", ch );
    return;
  }
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    stc( "Teleport to who?\n\r", ch );
    return;
  }
  if ( (victim = get_char_area(ch,arg)) == NULL )
  {
    stc( "You cannot sense them nearby.\n\r", ch );
    return;
  }
  if ( !IS_NPC(victim) || victim->level > 95 )
  {
    stc( "You can't pinpoint their location.\n\r", ch );
    return;
  }
  if ( victim->level > ch->pcdata->powers[M_FOCUS] - 5 )
  {
    stc( "You lack the skill to pinpoint their location.\n\r", ch );
    return;
  }

  act( "You will yourself along the astral, concentrating on $N.", ch, NULL, victim, TO_CHAR );
  act( "$n fades from view..", ch, NULL, NULL, TO_ROOM );
  char_from_room( ch );
  char_to_room( ch, victim->in_room );
  act( "$n fades into existances.", ch, NULL, NULL, TO_ROOM );
  WAIT_STATE(ch,10);
  ch->pcdata->powers[M_ESSENSE] -= 500;
  ch->hit -= 500;
  ch->mana -= 500;
  ch->move -= 500;
  return;
}

void do_rake( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  int dam, i = 0 , attacks = 2;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[M_SET],M_CLAWS) )
  {
    stc( "You have no claws with which to rake.", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->powers[M_SET],M_THIRD) )
    attacks++;
  if ( IS_SET(ch->pcdata->powers[M_SET],M_FOURTH) )
    attacks++;
  if ( IS_SET(ch->pcdata->powers[M_SET],M_FIFTH) )
    attacks++;
  if ( IS_SET(ch->pcdata->powers[M_SET],M_SIXTH) )
    attacks++;

  if ( (victim = ch->fighting) == NULL )
  {
    stc( "Your claws shred the air.\n\r", ch );
    return;
  }

  act( "You rake your claws across $N!", ch, NULL, victim, TO_CHAR );
  act( "$n rakes $s claws across you!", ch, NULL, victim, TO_VICT );
  act( "$n rakes $s claws across $N.", ch, NULL, victim, TO_NOTVICT );
  do
  {
    dam = dice( 5, ch->pcdata->weapons[5] );
    if ( IS_NPC(victim) )
      dam += dam/2;
    damage( ch, victim, dam, TYPE_HIT+5 );
    i++;
  }
  while ( victim && victim->position > POS_MORTAL && i < attacks && number_percent() < 83 );
  WAIT_STATE( ch, 6 );
  return;
}

void do_gouge( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  int duration;

  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[M_SET],M_SPIKES) )
  {
    stc( "You have no spikes with which to gouge.\n\r", ch );
    return;
  }
  if ( (victim = ch->fighting) == NULL )
  {
    stc( "You jam your spikes into the wind.\n\r", ch );
    return;
  }

  act( "You jam your spikes into $N's eyes!", ch, NULL, victim, TO_CHAR );
  act( "$n jams $s spikes into your eyes! Ouch!", ch, NULL, victim, TO_VICT );
  act( "$n jams $s spikes into $N's eyes.", ch, NULL, victim, TO_NOTVICT );
  if ( (!IS_NPC(victim) && victim->pcdata->body-50 > number_percent())
  ||   (IS_NPC(victim) && victim->level > number_percent() + 10) )
  {
    WAIT_STATE( ch, 8 );
    return;
  }

  duration = ch->pcdata->weapons[11]/6 + 5;
  if ( IS_NPC(victim) && victim->level >= 96 )
    duration = dice(1,5);
  af.type      = skill_lookup( "blindness" );
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;
  af.duration  = duration;
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );
  act( "$N is blinded!", ch, NULL, victim, TO_CHAR );
  stc( "You are blinded!", victim );
  WAIT_STATE( ch, 8 );
  return;
}

void do_lash( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

  if ( IS_SUIT(ch) && ch->pcdata->suit[SUIT_READY] == MS_HEAT_ROD )
  {
    int combat, power, fight, dam;
    if ( ch->position < POS_FIGHTING )
    {
      stc( "You're too relaxed.\n\r", ch );
      return;
    }
    if ( ch->pcdata->suit[SUIT_PLASMA] < 1 )
    {
      stc( "You do not have enough plasma.\n\r", ch );
      return;
    }
    if ( (victim = ch->fighting) == NULL )
    {
      stc( "You aren't fighting anybody.\n\r", ch );
      return;
    }
    combat = ch->pcdata->suit[SUIT_COMBAT];
    power = suit_table[ch->pcdata->suit[SUIT_NUMBER]].power;
    fight = suit_table[ch->pcdata->suit[SUIT_NUMBER]].fight;
    dam = (power + fight) * 10 + dice(2,combat);

    damage( ch, victim, dam, DAM_HEATROD );
    if ( victim && !IS_NPC(victim) )
    {
      if ( stanced(victim) > 0 )
        unstance( victim );
    }
    WAIT_STATE(ch,5);
    return;
  }


  if ( ch->position < POS_FIGHTING )
  {
    stc( "You're too relaxed.\n\r", ch );
    return;
  }
  if ( !IS_CLASS(ch,CLASS_MAZOKU) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[M_SET],M_TENTACLES) )
  {
    stc( "You have no tentacles with which to lash.\n\r", ch );
    return;
  }
  if ( (victim = ch->fighting) == NULL )
  {
    stc( "You lash the air.\n\r", ch );
    return;
  }

  act( "You lash $N with your tentacles!", ch, NULL, victim, TO_CHAR );
  act( "$n lashes you with $s tentacles!", ch, NULL, victim, TO_VICT );
  act( "$n lashes $N with $s tentacles.", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dice(5,100), TYPE_HIT+4 );
  if ( victim && victim->position >= POS_FIGHTING && !IS_NPC(victim) )
  {
    if ( number_percent() < ch->pcdata->body - 20 )
      unstance( victim );
  }
  WAIT_STATE(ch,12);
  return;
}




