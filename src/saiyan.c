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


void do_rage( CHAR_DATA *ch, char *argument )
{
  int gain, gain2;

  if (IS_NPC(ch))
    return;

  if ( ch->class != CLASS_SAIYAN )
    return;

  if ( ch->pcdata->powers[S_POWER] >= ch->pcdata->powers[S_POWER_MAX] )
  { send_to_char( "You are already at full power!\n\r", ch );
    return;
  }

  gain = dice(ch->pcdata->body,ch->pcdata->spirit/2) + ch->pcdata->powers[S_POWER_MAX]/100;

  gain2 = isquare( gain );

  if ( ch->move <= UMIN(gain/10,1000) )
  { send_to_char( "Your body is too exhausted for you to power up.\n\r", ch );
    return;
  }

  act( "As your rage builds, your inner power increases!", ch, NULL, NULL, TO_CHAR );
  act( "$n screams with rage, and $s body ripples with power!", ch, NULL, NULL, TO_ROOM );
  ch->pcdata->powers[S_POWER] = UMIN( ch->pcdata->powers[S_POWER] + gain,
  					ch->pcdata->powers[S_POWER_MAX]);
  ch->pcdata->powers[S_SPEED] = UMIN( ch->pcdata->powers[S_SPEED] + gain2,
  					ch->pcdata->powers[S_SPEED_MAX]);
  ch->pcdata->powers[S_STRENGTH] = UMIN( ch->pcdata->powers[S_STRENGTH] + gain2,
					ch->pcdata->powers[S_STRENGTH_MAX] );
  ch->pcdata->powers[S_AEGIS] = UMIN( ch->pcdata->powers[S_AEGIS] + gain2,
  					ch->pcdata->powers[S_AEGIS_MAX]);
  ch->move -= UMIN( gain / 10, 1000 );
  WAIT_STATE( ch, 4 );

  return;
}

void do_focus( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int increase;
 
  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );

  if ( arg[0] == '\0' || arg[1] == '\0' )
  { send_to_char( "You must specify what ability you wish to focus, and how much power.\n\r", ch );
    return;
  }

  increase = is_number( arg2 ) ? atoi( arg2 ) : 0;
  if ( increase == 0 )
  { send_to_char( "How much power do you want to focus?\n\r", ch );
    return;
  }

  if ( increase < 0 )
  { send_to_char( "Uhhhh why?\n\r", ch );
    return;
  }
  if ( increase >= ch->pcdata->powers[S_POWER] )
  { send_to_char( "You don't have that much power.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg, "strength" ) )
  { ch->pcdata->powers[S_STRENGTH] = UMIN( ch->pcdata->powers[S_STRENGTH] + increase,
					ch->pcdata->powers[S_STRENGTH_MAX] );
    act( "$n's muscles bulge with strength.", ch, NULL, NULL, TO_ROOM );
  }
  else if ( !str_prefix( arg, "speed" ) )
  { ch->pcdata->powers[S_SPEED] = UMIN( ch->pcdata->powers[S_SPEED] + increase,
					ch->pcdata->powers[S_SPEED_MAX] );
    act( "$n blurs as their movements speed up.", ch, NULL, NULL, TO_ROOM );
  }
  else if ( !str_prefix( arg, "aegis" ) )
  { ch->pcdata->powers[S_AEGIS] = UMIN( ch->pcdata->powers[S_AEGIS] + increase,
					ch->pcdata->powers[S_AEGIS_MAX] );
    act( "The aegis around $n flares with renewed energy.", ch, NULL, NULL, TO_ROOM );
  }
  else
  { send_to_char( "Focus on what?\n\r", ch );
    return;
  }

  ch->pcdata->powers[S_POWER] -= increase;
  sprintf( buf, "You channel %d points of power into your %s.\n\r", increase, arg );
  send_to_char( buf, ch );
  WAIT_STATE( ch, 4 );
  return;
}

void do_technique( CHAR_DATA *ch, char *argument )
{
  int cost = 0;
  int skill;
  char buf[MAX_STRING_LENGTH];
  char *yes = "x";
  char *no  = " ";
  char name[20];

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuuh what?\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "\n\rSaiyans can learn the following techniques:\n", ch );
    send_to_char( "Kiwave      Kibomb      Kibolt      Kiblast     Kiwall\n", ch );
    send_to_char( "Kikouhou    Masenkou    Kamehameha  Solarflare  Ryuken\n", ch );
    send_to_char( "Zanzouken   Kaiouken    Flight      Kisense     Battlesense\n", ch );
    send_to_char( "Hawkeyes    Shunkanidou Kiaihou\n\r", ch );
    send_to_char( "Enter 'technique <tech>' to learn one, i.e. 'technique kibolt'\n\r", ch );
    send_to_char( "Enter 'technique learned' to see which battle skills you have mastered.\n\r", ch );
    send_to_char( "Enter 'techinque cost' to see their costs in primal.\n\r", ch );
    return;
  }

  if ( !str_cmp( argument, "kiwave" ) )
  { skill	= S_KIWAVE;
    cost	= 50;
    sprintf( name, "Ki Wave" );
  }
  else if ( !str_cmp( argument, "kibomb" ) )
  { skill	= S_KIBOMB;
    cost	= 50;
    sprintf( name, "Ki Bomb" );
  }
  else if ( !str_cmp( argument, "kibolt" ) )
  { skill	= S_KIBOLT;
    cost	= 50;
    sprintf( name, "Ki Bolt" );
  }
  else if ( !str_cmp( argument, "kiblast" ) )
  { skill	= S_KIBLAST;
    cost	= 25;
    sprintf( name, "Ki Blast" );
  }
  else if ( !str_cmp( argument, "kiwall" ) )
  { skill	= S_KIWALL;
    cost	= 75;
    sprintf( name, "Ki Wall" );
  }
  else if ( !str_cmp( argument, "kikouhou" ) )
  { skill	= S_KIKOUHOU;
    cost	= 100;
    sprintf( name, "Kikouhou" );
  }
  else if ( !str_cmp( argument, "masenkouha" ) )
  { skill	= S_MASENKOUHA;
    cost	= 125;
    sprintf( name, "Masenkou Ha" );
  }
  else if ( !str_cmp( argument, "kamehameha" ) )
  { skill	= S_KAMEHAMEHA;
    cost	= 200;
    sprintf( name, "Kame Hame Ha" );
  }
  else if ( !str_cmp( argument, "solarflare" ) )
  { skill	= S_SOLARFIST;
    cost	= 100;
    sprintf( name, "Solar Flare" );
  }
  else if ( !str_cmp( argument, "ryuken" ) )
  { skill	= S_RYUKEN;
    cost	= 150;
    sprintf( name, "Ryuken" );
  }
  else if ( !str_cmp( argument, "zanzouken" ) )
  { skill	= S_ZANZOUKEN;
    cost	= 100;
    sprintf( name, "Zanzouken" );
  }
  else if ( !str_cmp( argument, "kaiouken" ) )
  { skill	= S_KAIOUKEN;
    cost	= 200;
    sprintf( name, "Kaiouken" );
  }
  else if ( !str_cmp( argument, "flight" ) )
  { skill	= S_FLIGHT;
    cost	= 50;
    sprintf( name, "Flight" );
  }
  else if ( !str_cmp( argument, "kisense" ) )
  { skill	= S_KISENSE;
    cost	= 100;
    sprintf( name, "Ki Sense" );
  }
  else if ( !str_cmp( argument, "battlesense" ) )
  { skill	= S_BATTLESENSE;
    cost	= 75;
    sprintf( name, "Battlesense" );
  }
  else if ( !str_cmp( argument, "shunkanidou" ) )
  { skill	= S_KIMOVE;
    cost	= 125;
    sprintf( name, "Shunkan Idou" );
  }
  else if ( !str_cmp( argument, "hawkeyes" ) )
  { skill	= S_HAWKEYES;
    cost	= 50;
    sprintf( name, "Hawk Eyes" );
  }
  else if ( !str_cmp( argument, "kiaihou" ) )
  { skill	= S_KIAIHOU;
    cost	= 200;
    sprintf( name, "Kiaihou" );
  }
  else if ( !str_cmp( argument, "cost" ) )
  { send_to_char( "\n\rThese are the costs of developing Saiyan combat techniques, in primal.\n", ch );
    send_to_char( "Ki Wave      50  Ki Bomb     50  Ki Bolt     50  Ki Blast    25  Ki Wall     75\n", ch );
    send_to_char( "Kikouhou    100  Masenkou   125  Kamehameha 200  Solarflare 100  Ryuken     150\n", ch );
    send_to_char( "Zanzouken   100  Kaiouken   200  Flight      50  Ki Sense   100  Bat. Sense  75\n", ch );
    send_to_char( "ShunkanIdou 125  Hawkeyes    50  Kiaihou    200\n\r", ch );
    return;
  }
  else if ( !str_cmp( argument, "learned" ) )
  { send_to_char( "\n\r\n\rSaiyan battle techniques learned:\n", ch );
    sprintf( buf, "Ki Wave    [%s]  Ki Bomb    [%s]  Ki Bolt    [%s]  Ki Blast  [%s]  Ki Wall    [%s]\n",
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIWAVE) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIBOMB) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIBOLT) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIBLAST) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIWALL) ? yes : no) );
    send_to_char( buf, ch );
    sprintf( buf, "Kikouhou   [%s]  Masenkouha [%s]  Kamehameha [%s]  Solarflar [%s]  Ryuken     [%s]\n",
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIKOUHOU) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_MASENKOUHA) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KAMEHAMEHA) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_SOLARFIST) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_RYUKEN) ? yes : no) );
    send_to_char( buf, ch );
    sprintf( buf, "Zanzouken  [%s]  Kaiouken   [%s]  Flight     [%s]  Ki Sense  [%s]  Bat. Sense [%s]\n",
	(IS_SET(ch->pcdata->powers[S_TECH], S_ZANZOUKEN) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KAIOUKEN) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_FLIGHT) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KISENSE) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_BATTLESENSE) ? yes : no) );
    send_to_char( buf, ch );
    sprintf( buf, "Shunkan I. [%s]  Hawkeyes   [%s]  Kiaihou    [%s]\n\r",
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIMOVE) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_HAWKEYES) ? yes : no),
	(IS_SET(ch->pcdata->powers[S_TECH], S_KIAIHOU) ? yes : no) );
    send_to_char( buf, ch ); 
    return;
  }
  else
  { send_to_char( "That is not a Saiyan battle technique.\n\r", ch );
    return;
  }


  if ( ch->pcdata->primal < cost )
  { sprintf( buf, "The %s technique requires %d primal to learn.\n\r", name, cost );
    send_to_char( buf, ch );
    return;
  }

  if ( IS_SET( ch->pcdata->powers[S_TECH], skill ) )
  { sprintf( buf, "You have already mastered the %s technique.\n\r", name );
    send_to_char( buf, ch );
    return;
  }
  
  ch->pcdata->primal -= cost;
  sprintf( buf, "You have now mastered the %s technique.\n\r", name );
  send_to_char( buf, ch );
  SET_BIT( ch->pcdata->powers[S_TECH], skill );
  return;
  
}


void do_kiblast( CHAR_DATA *ch, char *argument )
{
  int dam;
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?", ch );
    return;
  }

  if ( !IS_SET( ch->pcdata->powers[S_TECH], S_KIBLAST ) )
  { send_to_char( "You don't know that technique.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[S_POWER] <= 500 )
  { send_to_char( "You need 500 points of power to use the Ki blast technique.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' && ch->fighting == NULL )
  {
    send_to_char( "Blast who?\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL && argument[0] == '\0' )
    victim = ch->fighting;
  else if ( ( victim = get_char_room( ch, argument ) )== NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }

  dam = dice( 7, ch->pcdata->spirit * 5 );
  act( "You blast $N with energy!", ch, NULL, victim, TO_CHAR );
  act( "$n blasts you with energy!", ch, NULL, victim, TO_VICT );
  act( "$n blasts $N with energy.", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dam, DAM_KIFLAME );
  ch->pcdata->powers[S_POWER] -= 500;
  WAIT_STATE( ch, 5 );
  return;
}

void do_kibolt( CHAR_DATA *ch, char *argument )
{
  int dam;
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?", ch );
    return;
  }

  if ( !IS_SET( ch->pcdata->powers[S_TECH], S_KIBOLT ) )
  { send_to_char( "You don't know that technique.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[S_POWER] <= 2000 )
  { send_to_char( "You need 2000 points of power to use the Ki bolt technique.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' && ch->fighting == NULL )
  { send_to_char( "Bolt who?\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL && argument[0] == '\0' )
    victim = ch->fighting;
  else if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }

  ch->pcdata->powers[S_POWER] -= 2000;
  dam = dice( ch->pcdata->spirit / 7, ch->pcdata->spirit * 5 );
  act( "You fire a concentrated bolt of energy at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n fires a concentrated bolt of energy at you!", ch, NULL, victim, TO_VICT );
  act( "$n fires a concentrated bolt of energy at $N.", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dam, DAM_KIFLAME );
  WAIT_STATE( ch, 8 );
  return;
}


void do_kibomb( CHAR_DATA *ch, char *argument )
{
  int dam;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KIBOMB) )
  { send_to_char( "You don't know that technique.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[S_POWER] <= 2000 )
  { send_to_char( "You need 2000 points of power to use the Ki bomb technique.\n\r", ch );
    return;
  }

  act( "You spread your arms and release an explosion of energy!", ch, NULL, NULL, TO_CHAR );
  act( "$n spreads $s arms and releases an explosion of energy!", ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  { vch_next = vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room )
      if ( vch != ch && !is_same_group( ch, vch ) )
      { dam = dice( 10, ch->pcdata->spirit * 4 );
        damage( ch, vch, dam, DAM_KIFLAME );
      }
  }

  ch->pcdata->powers[S_POWER] -= 2000;
  WAIT_STATE( ch, 8 );
  return;
}

void do_shunkanidou( CHAR_DATA *ch, char *argument )
{
  int vnum;
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KIMOVE) )
  { send_to_char( "You don't know that technique.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Your attempt to move to yourself has failed.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] <= 10000 )
  { send_to_char( "Shunkan Idou requires 10000 points of energy.\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
  {
    return;
  }
  if ( IS_SET(ch->in_room->room_flags, ROOM_HQ ) )
  { send_to_char( "You may not teleport from a headquarters.\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
  { send_to_char( "You cannot sense any such Ki.\n\r", ch );
    return;
  }
  if ( !IS_NPC(victim) )
  { send_to_char( "Their Ki is shielded.\n\r", ch );
    return;
  }
  if ( victim->level > (ch->pcdata->spirit/2) || victim->fighting != NULL
	|| victim->in_room == NULL || !IS_NPC(victim))
  { send_to_char( "You can't get a lock on their Ki.\n\r", ch );
    return;
  }
  if ( ch->pcdata->extras[TIMER] > 0 )
  { send_to_char( "Not with a fight timer.\n\r", ch );
    return;
  }

  vnum = victim->in_room->vnum;
  if ( (vnum >= 8900 && vnum <= 8999)
    || (vnum >= 5300 && vnum <= 5399)
    || (vnum >= 8400 && vnum <= 8599)
    || (vnum >= 8100 && vnum <= 8299) )
    vnum = 5148;

  ch->pcdata->powers[S_POWER] -= 10000;
  act( "You concentrate on $N's location, and will yourself there.", ch, NULL, victim, TO_CHAR );
  act( "$n vanishes into thin air!", ch, NULL, victim, TO_ROOM );
  char_from_room( ch );
  char_to_room( ch, get_room_index( vnum ) );
  act( "$n pops out of thin air!", ch, NULL, victim, TO_ROOM );
  do_look( ch, "auto" );
  WAIT_STATE( ch, 12 );
  return;
}

void do_kisense( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char areaname[MAX_STRING_LENGTH];
  int i;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KISENSE) )
  { send_to_char( "You have not learned that technique.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Sense who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
  { send_to_char( "You cannot sense their Ki.\n\r", ch );
    return;
  }
  if ( victim->in_room == NULL )
  { send_to_char( "Their Ki is oddly fuzzy, and you can't lock on to it.\n\r", ch );
    return;
  }

  if ( IS_CLASS(victim,CLASS_PATRYN) && get_runes(victim,RUNE_FIRE,TORSO) > 0
    && victim->level >= 2 && IS_SET(victim->pcdata->powers[P_BITS],P_DEFENSES) )
  { 
    if ( get_runes(victim,RUNE_FIRE,TORSO) == 1 )
      act( "Your tattoos glow with a dull red aura.", victim, NULL, NULL, TO_CHAR );
    else
      act( "Your tattoos glow dull red, as you feel $N's Ki brush against you.", victim, NULL, ch, TO_CHAR );

    act( "$n's tattoos glow with a dull red aura.", victim, NULL, NULL, TO_ROOM );
  }

  /* crummy way to get the "{ 5 15 } Larsen" part outta it */
  for ( i = 16; victim->in_room->area->name[i] != '\0'; i++ )
    areaname[i-16] = victim->in_room->area->name[i];
  areaname[i-16] = '\0';

  sprintf( buf, "\n\rYou locate %s in the vicinity of %s.\n\r", victim->name, areaname );
  send_to_char( buf, ch );
  WAIT_STATE( ch, 8 );
  return;
}

void do_kiwall( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af;
  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KIWALL ) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] <= 2500 )
  { send_to_char( "It takes 2500 points of energy to raise a wall of Ki.\n\r", ch );
    return;
  }

  act( "You raise your arms and bring up a wall of Ki around you.", ch, NULL, NULL, TO_CHAR );
  act( "$n raises a wall of blazing energy around $mself.", ch, NULL, NULL, TO_ROOM );
  ch->pcdata->powers[S_POWER] -= 2500;

  af.type      = gsn_kiwall;
  if ( is_affected( ch, gsn_kiwall ) )
  {
    affect_strip( ch, gsn_kiwall );
    af.duration	= dice(1,3);
  }
  else if ( !str_cmp( ch->name, "Piccolo" ) )
    af.duration	= dice(2,2);
  else
    af.duration = dice(2,4);
  af.modifier  = -1 * ch->pcdata->powers[S_AEGIS] / 50;
  af.location  = APPLY_AC;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  WAIT_STATE( ch, 8 );
  return;
}

void do_kiwave( CHAR_DATA *ch, char *argument )
{
  int dam;
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;

  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KIWAVE) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[S_POWER] <= 2000 )
  { send_to_char( "You need 2000 points of power to use the Ki wave technique.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' && ch->fighting == NULL )
  { 
    send_to_char( "Ki wave who?\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL && argument[0] == '\0')
    victim = ch->fighting;
  else if ( ( victim = get_char_room( ch, argument ) )== NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }

  dam = dice( 5, ch->pcdata->spirit * 7);
  act( "You launch a wave of Ki at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n launches a wave of energy at you!", ch, NULL, victim, TO_VICT );
  act( "$n launches a wave of energy at $N.", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dam, DAM_KIFLAME );
  if ( victim != NULL && !IS_NPC(victim) )
  {
    if ( IS_CLASS(victim,CLASS_FIST) && number_percent() > (victim->pcdata->body/2) )
    { if ( victim->pcdata->powers[F_KI] > 0 )
        victim->pcdata->powers[F_KI] -= dice(2,3)+1;
      if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
      act( "Your Ki wave staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "$n's wave of energy breaks your rhythm!", ch, NULL, victim, TO_VICT );
      act( "$n's wave of energy staggers $N.", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( victim->class == CLASS_SAIYAN )
    { if ( is_affected(victim,gsn_kiwall) )
      { affect_strip(victim,gsn_kiwall);
        act( "Your Ki wave staggers $N!", ch, NULL, victim, TO_CHAR );
        act( "$n's wave of energy collapes your wall of Ki!", ch, NULL, victim, TO_VICT );
        act( "$n's wave of energy staggers $N.", ch, NULL, victim, TO_NOTVICT );
      }
    }
    else if ( IS_CLASS(victim,CLASS_SORCERER) )
    {
      act( "Your Ki wave staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "$n's wave of energy disrupts your casting!", ch, NULL, victim, TO_VICT );
      act( "$n's wave of energy staggers $N.", ch, NULL, victim, TO_NOTVICT );
      lose_chant( victim );
    }
    else if ( IS_CLASS(victim,CLASS_MAZOKU) && victim->pcdata->powers[M_CTYPE] != 0 )
    {
      act( "Your Ki wave staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "Stunned, you lose control of your charge!", ch, NULL, victim, TO_VICT );
      act( "$n's wave of energy staggers $N.", ch, NULL, victim, TO_NOTVICT );
      victim->pcdata->powers[M_CTYPE] = 0;
      victim->pcdata->powers[M_CTIME] = 0;
    }

  }
  WAIT_STATE( ch, 8 );
  ch->pcdata->powers[S_POWER] -= 2000;
  return;
}

void do_masenkouha( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char vname[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_MASENKOUHA) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] <= 5000 )
  { send_to_char( "The Masenkou technique requires 5000 points of energy.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' && ch->fighting == NULL )
  { send_to_char( "Attack who?\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL && arg[0] == '\0' )
  {
    victim = ch->fighting;
    free_string( ch->pcdata->target );
    one_argument( victim->name, vname );
    ch->pcdata->target = str_dup( vname );
  }
  else if ( (victim = get_char_room(ch, arg)) != NULL )
  {
    free_string( ch->pcdata->target );
    ch->pcdata->target = str_dup( arg );
  }
  else
  {
    send_to_char( "They aren't here.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }
  
  SET_BIT(ch->pcdata->actnew,NEW_MASENKOUHA);
  do_say( ch, "Masenkou.." );
  WAIT_STATE( ch, 9 );
  ch->pcdata->powers[S_POWER] -= 5000;
  return;
}

void do_kikouhou( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  int dam;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KIKOUHOU) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] <= 5000 )
  { send_to_char( "The Kikouhou technique requires 5000 points of energy.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Attack who?\n\r", ch );
    return;
  }
  if ( (victim = get_char_area(ch, argument)) == NULL )
  { send_to_char( "They are out of range.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }
  if ( IS_NPC(victim) && victim->level > 95 )
  { send_to_char( "You are unable to target them.\n\r", ch );
    return;
  }
  if ( !IS_NPC(victim) && victim->fighting != NULL && !IS_NPC(victim->fighting) )
  {
    stc( "Don't interfere in other peoples' fights you fuckin' lamer.\n\r", ch );
    WAIT_STATE(ch,20);
    return;
  }

  ch->pcdata->powers[S_POWER] -= 5000;
  if ( ch->pcdata->extras[TIMER] < 16 )
    ch->pcdata->extras[TIMER] = 16;
  dam = dice( ch->pcdata->spirit / 10, ch->pcdata->spirit * 10 );
  do_yell( ch, "Kikouhou!!!" );
  act( "$n launches a tremendous bolt of energy at $N!", ch, NULL, victim, TO_ROOM );
  act( "A tremendous bolt of energy crashes into $n!", victim, NULL, NULL, TO_ROOM );
  act( "A tremendous bolt of energy crashes into you!", victim, NULL, NULL, TO_CHAR );
  damage( ch, victim, dam, DAM_KIFLAME );
  if ( victim != NULL && victim->fighting == ch
  && !(!IS_NPC(victim) && IS_SET(victim->act,PLR_FIREBACK)) )
    stop_fighting( victim, FALSE );
  WAIT_STATE( ch, 20 );
  return;
}

void do_kamehameha( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char vname[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KAMEHAMEHA) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] < 15000 )
  { send_to_char( "The Kame-hame-ha requires 15000 points of energy.\n\r", ch );
    return;
  }
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' && ch->fighting == NULL )
  { send_to_char( "Attack who?\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL && arg[0] == '\0' )
  {
    victim = ch->fighting;
    free_string( ch->pcdata->target );
    one_argument( victim->name, vname );
    ch->pcdata->target = str_dup( vname );
  }
  else if ( (victim = get_char_room(ch, arg)) != NULL )
  {
    free_string( ch->pcdata->target );
    ch->pcdata->target = str_dup( arg );
  }
  else
  {
    send_to_char( "They aren't here.\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }

  SET_BIT(ch->pcdata->actnew,NEW_KAME_1);
  ch->pcdata->powers[S_POWER] -= 15000;
  do_say( ch, "Kame.." );

  /* damage now done in update.c
  dam = dice( ch->pcdata->spirit*2, ch->pcdata->spirit * 5/4 );
  dam = ch->pcdata->spirit * 115 + dice( 10, ch->pcdata->spirit );
  do_yell( ch, "Kame-hame-HA!!!!" );
  act( "You focus your Ki into a tight beam and release it on $N!", ch, NULL, victim, TO_CHAR );
  act( "$n fires an incandescent bolt of energy at you!!", ch, NULL, victim, TO_VICT );
  act( "$n hurls an incandescent bolt of energy at $N!", ch, NULL, victim, TO_NOTVICT );
  damage( ch, victim, dam, DAM_KIFLAME );
  */
  WAIT_STATE( ch, 16 );
  return;
}

void do_kaiouken( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Duuuuh what?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_KAIOUKEN) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] <= 20000 )
  { send_to_char( "It costs 20000 points of energy to power up the Kaiouken attack.\n\r", ch );
    return;
  }

  do_yell( ch, "Kaiouken Attack!!" );
  act( "You initiate the Kaiouken attack and are encased in crimson flame!", ch, NULL, NULL, TO_CHAR );
  act( "`R$n bursts into crimson flame!`n", ch, NULL, NULL, TO_ROOM );
  ch->pcdata->powers[S_POWER] -= 20000;
  af.type      = gsn_kaiouken;
  af.duration  = 1;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  WAIT_STATE( ch, 12 );
  return;
}

void do_solarflare( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  int sn, lev;

  if ( IS_NPC(ch) || ch->level < 2 )
    return;
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_SOLARFIST) )
  { send_to_char( "You have not learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] < 2500 )
  { send_to_char( "You need 2500 fighting power to use the Solarflare technique.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Solarflare who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( !IS_NPC(victim) && victim->level < 2 )
  { send_to_char( "Not on mortals.  This means you, Anonbert.\n\r", ch );
    return;
  }
  sn = skill_lookup( "blind" );
  lev = ch->pcdata->spirit/2;
  act( "You call forth the suns brightness, flashing $N with its brilliance.", ch, NULL, victim, TO_CHAR );
  act( "A brilliant flash of light bursts forth from $n's forehead!", ch, NULL, victim, TO_ROOM );
  if ( ch->fighting == NULL && ch != victim && victim->position > POS_STUNNED )
    set_fighting( ch, victim );
  if ( victim->fighting == NULL && ch != victim && victim->position > POS_STUNNED )
    set_fighting( victim, ch );
  ( skill_table[sn].spell_fun) ( sn, lev, ch, victim );
  ch->pcdata->powers[S_POWER] -= 2500;
  timer_check( ch, victim );
  WAIT_STATE( ch, 8 );
  return;
}


void do_hawkeyes( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) ) 
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_HAWKEYES) )
  { send_to_char( "You don't know that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] <= 500 )
  { send_to_char( "You need 500 power to enhance your vision.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->act,PLR_TRUESIGHT) )
  { send_to_char( "You let your vision return to normal.\n\r", ch );
    REMOVE_BIT(ch->act, PLR_TRUESIGHT);
    return;
  }

  ch->pcdata->powers[S_POWER] -= 500;
  SET_BIT(ch->act,PLR_TRUESIGHT);
  send_to_char( "You concentrate, and your vision becomes crystal clear.\n\r", ch );
  return;
}

void do_flight( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_FLIGHT) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[S_POWER] < 250 )
  { send_to_char( "You need 250 points of power to lift off.\n\r", ch );
    return;
  }

  ch->pcdata->powers[S_POWER] -= 250;
  act( "Your aegis flares with energy as you lift off.", ch, NULL, NULL, TO_CHAR );
  act( "$n glows with energy as $s jumps into the air.", ch, NULL, NULL, TO_ROOM );
  ( skill_table[skill_lookup( "fly" )].spell_fun ) ( skill_lookup( "fly" ), ch->pcdata->spirit, ch, ch );
  WAIT_STATE( ch, 12 );
  return;
}

void do_ryuken( CHAR_DATA *ch, char *argument )
{
  int dam;
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_RYUKEN) )
  { send_to_char( "You haven't learned that technique.\n\r", ch );
    return;
  }
  if ( ch->pcdata->powers[S_POWER] < 10000 )
  { send_to_char( "The ryuken technique requires 10000 fighting power.\n\r", ch );
    return;
  }
  if ( ch->hit < ch->max_hit/4 )
  {
    stc( "In your weakened condition, you can't summon the strength.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
    victim = ch->fighting;
  else
  { send_to_char( "But you aren't fighting.\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim) && victim->hit < victim->max_hit/4  &&  ch->hit > ch->max_hit/2 )
  {
    stc( "No more lame run in ryuken.  Thank tander.\n\r", ch );
    return;
  }

  act( "You wind up and slam your fist into $N!", ch, NULL, victim, TO_CHAR );
  act( "$n winds up and slams $s fist into you!", ch, NULL, victim, TO_VICT );
  act( "$n winds up and slams $s fist into $N!", ch, NULL, victim, TO_NOTVICT );
  if (!IS_NPC(victim))
  { if ( victim->class == CLASS_FIST )
    { if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
        REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
      if ( victim->pcdata->powers[F_KI] > 0 )
        victim->pcdata->powers[F_KI] = 0;
    }
    else if ( victim->class == CLASS_SAIYAN && is_affected(victim,gsn_kiwall) )
      affect_strip(victim,gsn_kiwall);
    else if ( IS_CLASS(victim,CLASS_SORCERER) )
    {
      act( "Your dragon uppercut staggers $N!", ch, NULL, victim, TO_CHAR );
      act( "$n's dragon uppercut disrupts your casting!", ch, NULL, victim, TO_VICT );
      act( "$n's dragon uppercut staggers $N.", ch, NULL, victim, TO_NOTVICT );
      lose_chant(victim);
    }

  }
  dam = dice( ch->pcdata->body, ch->pcdata->body * 3 / 2 );
  damage( ch, victim, dam, F_UPPERCUT );
  if ( ch->fighting != NULL )
  { act( "An incandescent dragon curls up your arm and crashes into $N!", ch, NULL, victim, TO_CHAR );
    act( "An incancescent dragon spears through you!", ch, NULL, victim, TO_VICT );
    act( "An incandescent dragon spears through $N", ch, NULL, victim, TO_NOTVICT );
    dam = dice( ch->pcdata->spirit, ch->pcdata->spirit );
    damage( ch, victim, dam, DAM_KIFLAME );
  }
  ch->pcdata->powers[S_POWER] -= 10000;
  WAIT_STATE( ch, 42 );
  return;
}

void do_battlesense( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SAIYAN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[S_TECH],S_BATTLESENSE) )
  { send_to_char( "You do not know that technique.\n\r", ch );
    return;
  }


  if ( argument[0] == '\0' )
  { send_to_char( "Sense who?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim) && (ch->fighting == NULL || ch->position != POS_FIGHTING) )
  {
    stc( "Their Ki is shielded.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
    sprintf( buf, "%s is a mobile object.\n\r", victim->short_descr );
  else
    sprintf( buf, "%s is a %s\n\r", victim->name, class_table[victim->class].who_name );
  send_to_char( buf, ch );
  sprintf( buf, "They have %d hit points, %d mana, and %d moves.\n\r",
  		victim->hit, victim->mana, victim->move );
  send_to_char( buf, ch );
  if ( IS_CLASS(victim,CLASS_SAIYAN) )
  { sprintf( buf, "Their fighting power is at %d.\n\r", victim->pcdata->powers[S_POWER] );
    send_to_char( buf, ch );
  }

  if ( IS_CLASS(victim,CLASS_FIST) && victim->pcdata->powers[F_KI] > 0 )
  { if ( victim->pcdata->powers[F_KI] <= 9 )
      send_to_char( "They are charging their Ki.\n\r", ch );
    else if ( victim->pcdata->powers[F_KI] <= 19 )
      send_to_char( "They have focused their Ki.\n\r", ch );
    else
      send_to_char( "They are bursting with Ki.\n\r", ch );
  }

  if ( IS_CLASS(victim,CLASS_MAZOKU) )
  {
    if ( victim->pcdata->powers[M_NIHILISM] <= 25 )
      sprintf( buf, "%s's nihilistic tendencies are quiescent.\n\r", victim->name );
    else if ( victim->pcdata->powers[M_NIHILISM] <= 50 )
      sprintf( buf, "%s is deeply desturbed.. but that's normal for a Mazoku.\n\r", victim->name );
    else if ( victim->pcdata->powers[M_NIHILISM] <= 75 )
      sprintf( buf, "%s burns with self-destructive potential.\n\r", victim->name );
    else
      sprintf( buf, "%s is consumed by their hatred for all creation!\n\r", victim->name );

    stc( buf, ch );
  }
  return;
}

void do_bigbang( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  int dam;

  if ( !IS_CLASS(ch,CLASS_SAIYAN) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[S_POWER] < 250000 )
  {
    stc( "You need 250,000 fighting power to use this attack.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' && ch->fighting == NULL )
  { send_to_char( "Attack who?\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL && argument[0] == '\0' )
    victim = ch->fighting;
  else if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  ch->pcdata->powers[S_POWER] -= 250000;
  dam = ch->level * ch->pcdata->body * ch->pcdata->spirit + dice(100,100);
  do_yell( ch, "BIG BANG ATTACK!!" );
  act( "$N is engulfed in a thunderous explosion of ki-flame!", ch, NULL, victim, TO_CHAR );
  act( "You is engulfed in a thunderous explosion of ki-flame!", ch, NULL, victim, TO_VICT );
  act( "$N is engulfed in a thunderous explosion of ki-flame!", ch, NULL, victim, TO_NOTVICT );
  WAIT_STATE(victim, 24);
  damage( ch, victim, dam, DAM_KIFLAME );
  WAIT_STATE( ch, 8 );
  return;
}
