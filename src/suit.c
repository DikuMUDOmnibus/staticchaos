
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

void	fire_miniguns	args( ( CHAR_DATA *ch, char *argument ) );
void	fire_buster	args( ( CHAR_DATA *ch, int dam, int dir ) );
void	fire_flamethrower args( (CHAR_DATA *ch ) );
bool	shields_check	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );

void do_mount( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int i, suit;

  if ( IS_NPC(ch) )
    return;
  if ( ch->fighting != NULL )
  { send_to_char( "You can't do that in the middle of combat!\n\r", ch );
    return;
  }
  if ( IS_SUIT( ch ) )
  { send_to_char( "You're already in a mobile suit.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Mount what?\n\r", ch );
    return;
  }
  if ( ( obj = get_obj_here( ch, argument ) ) == NULL )
  { send_to_char( "That suit isn't here.\n\r", ch );
    return;
  }
  if ( obj->item_type != ITEM_SUIT )
  { send_to_char( "You can only mount mobile suits.\n\r", ch );
    return;
  }
  suit = obj->weight;
  for ( i = 10; i <= 13; i++ )
  { if ( ch->pcdata->suit[i] < suit_table[suit].requisite )
    { send_to_char( "You are not a strong enough pilot to use that suit.\n\r", ch );
      return;
    }
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_HANGER) )
  {
    if ( ch->pcdata->clan[CLAN] != ch->in_room->clan )
    {
      stc( "Planet defensors activate, preventing you from entering.\n\r", ch );
      return;
    }
  }

  act( "You mount $p and close the hatch.", ch, obj, NULL, TO_CHAR );
  act( "$n mounts $p and closes the hatch.", ch, obj, NULL, TO_ROOM );
  for ( i = 0; i <= 3; i++ )
    ch->pcdata->suit[i] = obj->value[i];
  ch->pcdata->suit[SUIT_NUMBER] = suit;
  for ( i = 0; i <= 3; i++ )
    ch->pcdata->suit[i+5] = obj->extra[i];
  ch->pcdata->suit[SUIT_MISSILES] = obj->extra[4];
  extract_obj( obj );

  return;
}


void do_leave( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int i, suit, count = 0;

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You're not in a mobile suit!\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL )
  { send_to_char( "You can't do that in the middle of a battle!\n\r", ch );
    return;
  }

  for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
  {
    if ( obj->item_type == ITEM_SUIT )
      count++;
  }
  if ( count >= 4 && IS_SET(ch->in_room->room_flags, ROOM_HANGER) )
  {
    stc( "The hanger is already full!", ch );
    return;
  }

  /* suits start at vnum 60 in limbo */
  suit = ch->pcdata->suit[SUIT_NUMBER] + 59;
  obj = create_object( get_obj_index( suit ), 0 );
  for ( i = 0; i <= 3; i++ )
    obj->value[i] = ch->pcdata->suit[i];
  for ( i = 0; i <= 3; i++ )
    obj->extra[i] = ch->pcdata->suit[i+5];
  obj->extra[4] = ch->pcdata->suit[SUIT_MISSILES];
  /* clear out ch's suit values */
  for ( i = 0; i <= 9; i++ )
    ch->pcdata->suit[i] = 0;
  ch->pcdata->suit[SUIT_MISSILES] = 0;
  obj_to_room( obj, ch->in_room );
  act( "You open the hatch and leave $p", ch, obj, NULL, TO_CHAR );
  act( "$n opens the hatch and leaves $p", ch, obj, NULL, TO_ROOM );
  WAIT_STATE(ch,20);
  return;
}

void do_fly( CHAR_DATA *ch, char *argument )
{ int dir = DIR_NORTH, wait;
  ROOM_INDEX_DATA *location;
  int mod = 0;
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "But you're not in a suit.\n\r", ch );
    return;
  }
  if ( ch->position < POS_FIGHTING )
  {
    stc( "You're too relaxed.\n\r", ch );
    return;
  }
  if ( IS_TIED(ch) )
  {
    stc( "Your suit has been immobilized!\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    stc( "Fly where?\n\r", ch );
    return;
  }
  if ( !str_prefix( arg, "north" ) )
    dir = DIR_NORTH;
  else if ( !str_prefix( arg, "east" ) )
    dir = DIR_EAST;
  else if ( !str_prefix( arg, "south" ) )
    dir = DIR_SOUTH;
  else if ( !str_prefix( arg, "west" ) )
    dir = DIR_WEST;
  else if ( !str_prefix( arg, "up" ) )
    dir = DIR_UP;
  else if ( !str_prefix( arg, "down" ) )
    dir = DIR_DOWN;
  else if ( !str_prefix( arg, "home" ) )
  { if ( ( location = get_room_index( ch->pcdata->extras[6] ) ) == NULL )
    { send_to_char( "You are completely lost.\n\r", ch );
      return;
    }
    if ( ch->pcdata->extras[TIMER] > 16 )
    { send_to_char( "The gods do not heed the prayers of cowards.\n\r", ch );
      return;
    }
    if ( IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
    {
      stc( "You're already in the home.\n\r", ch );
      return;
    }
    if ( ch->in_room == location )
      return;
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
    {
        send_to_char( "God has forsaken you.\n\r", ch );
        return;
    }

    if ( ch->fighting != NULL )
    {
      send_to_char( "Duhhhhhh what? don't you mean FLEE?\n\r", ch );
      return;
    }

    ch->move -= 50;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
  }
  else
  { send_to_char( "You can't fly that way!\n\r", ch );
    return;
  }

  if ( ch->in_room->exit[dir] == NULL
  ||   ch->in_room->exit[dir]->to_room == NULL )
  {
    send_to_char( "Alas, you cannot fly that way.\n\r", ch );
    return;
  }

  if ( ch->pcdata->suit[SUIT_FUEL] <= 0 )
  {
    stc( "But you're out of fuel!\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL )
  {
    CHAR_DATA *victim = ch->fighting;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    char buf[MAX_STRING_LENGTH];
    int attackers = 0;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    { vch_next = vch->next_in_room;
      if ( vch->fighting == ch )
      attackers++;
    }

    /* mod is for attackers Over the required first */
    attackers = UMAX(0,attackers - 1);

    if ( IS_NPC(ch->fighting) )
      attackers++;

    /* Karma disabled
    if ( !IS_NPC(ch) )
      mod += ch->pcdata->extras[PUSSY];
    */
 
    if ( number_percent() > (40 + mod + attackers*20) )
    { act( "You fail!", ch, NULL, victim, TO_CHAR );
      act( "$n attempts to fly away.", ch, NULL, victim, TO_VICT );
      act( "$n attempts to fly away.", ch, NULL, victim,TO_NOTVICT);
      WAIT_STATE(ch,8);
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
    stop_fighting( ch, TRUE ); 
  }

  wait = 4 - ((suit_table[ch->pcdata->suit[SUIT_NUMBER]].speed - 95)/15);
  wait = UMAX( wait, 1 );
  move_char( ch, dir );
  WAIT_STATE(ch,wait);
  return;
}


void do_status( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char *con;
  int suit, cond, i, j;

  if ( IS_NPC(ch) )
    return;
  if ( ch->level < 20 )
    victim = ch;
  else if ( argument[0] == '\0' )
    victim = ch;
  else if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim ) )
  { send_to_char( "they're a mob.\n\r", ch );
    return;
  }
  suit = victim->pcdata->suit[SUIT_NUMBER];
  cond = 0; j = 1;
  for ( i = 0; i < 12; i++ )
  { if ( IS_SET(victim->pcdata->suit[SUIT_COND],j) )
      cond++;
    j *= 2;
  }
  if ( cond == 0 )
    con = "Perfect";
  else if ( cond < 3 )
    con = "Good";
  else if ( cond < 6 )
    con = "Damaged";
  else if ( cond < 9 )
    con = "Seriously Damaged";
  else if ( cond < 12 )
    con = "Critically Damaged";
  else
    con = "Seriously fucked up";
  send_to_char( "`d------------------------------------------------------------`n\n\r", ch);
  sprintf( buf, "        Mobile Suit: %s    Condition: %s\n\r\n\r", suit_table[suit].name, con);
  send_to_char( buf, ch  );
  sprintf( buf, "                Missiles:  %4d  Armor:    %4d  Fuel: %4d\n\r", 
  	victim->pcdata->suit[14], victim->pcdata->suit[2], victim->pcdata->suit[3] );
  send_to_char( buf, ch );
  sprintf( buf, "Bullets:  %4d  Shells:    %4d  Plasma:   %4d  Beam: %4d\n\r\n\r",
 	victim->pcdata->suit[5],        victim->pcdata->suit[6],
  	victim->pcdata->suit[7],        victim->pcdata->suit[8] );
  send_to_char( buf, ch );
  sprintf( buf, "Weapon:  %5d  Combat:   %5d  Reflex:  %5d  Aim: %5d\n\r",
 	victim->pcdata->suit[10],        victim->pcdata->suit[11],
  	victim->pcdata->suit[12],        victim->pcdata->suit[13] );
  send_to_char( buf, ch );
  send_to_char( "`d------------------------------------------------------------`n\n\r", ch );

  return;
}


void do_load( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int max;

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You must be in a mobile suit to load munitions.\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL )
  { send_to_char( "You can't do that in the middle of a fight.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Load up what?\n\r", ch );
    return;
  }
  if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
  { send_to_char( "You aren't carrying that.\n\r", ch );
    return;
  }
  if ( obj->item_type != ITEM_MUNITION )
  { send_to_char( "You can't load that into a mobile suit.\n\r", ch );
    return;
  }

  if ( obj->value[0] != 3 && obj->value[0] != 5 && obj->value[0] != 6 &&
       obj->value[0] != 7 && obj->value[0] != 8 && obj->value[0] != 14 )
  { send_to_char( "don't load that, it'll fuck up your suit.\n\r", ch );
    return;
  }
  max = 1000;
  if ( obj->value[0] == SUIT_MISSILES )
  {
    switch( ch->pcdata->suit[SUIT_NUMBER] )
    {
      case 2:	max = 8;	break;
      case 11:	max = 64;	break;
      case 12:	max = 2;	break;
      case 15:	max = 64;	break;
      default:	max = 0;	break;
    }
  }
  if ( ch->pcdata->suit[obj->value[0]] + obj->value[1] > max )
  { send_to_char( "You can't load up any more.\n\r", ch );
    return;
  }
  ch->pcdata->suit[obj->value[0]] += obj->value[1];

  act( "You load $p into your mobile suit.", ch, obj, NULL, TO_CHAR );
  act( "$n loads $p into $s mobile suit.", ch, obj, NULL, TO_ROOM );
  if ( obj->value[0] == SUIT_MISSILES )
  {
    WAIT_STATE(ch,8);
  }
  extract_obj( obj );
  return;
}

void do_ready( CHAR_DATA *ch, char *argument )
{ int weap;
  char *buf;

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You can only do that in a mobile suit.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Ready what?\n\r", ch );
    return;
  }
  if ( !str_prefix( argument, "machine gun" ) )
  { weap = MS_MACHINE_GUN;
    buf = "105mm machine gun";
  }
  else if ( !str_prefix( argument, "beam rifle" ) )
  { weap = MS_BEAM_RIFLE;
    buf = "beam rifle";
  }
  else if ( !str_prefix( argument, "dobergun" ) )
  { weap = MS_DOBERGUN;
    buf = "dobergun";
  }
  else if ( !str_prefix( argument, "beam cannon" ) )
  { weap = MS_BEAM_CANNON;
    buf = "beam cannon";
  }
  else if ( !str_prefix( argument, "laser cannon" ) )
  { weap = MS_LASER_CANNON;
    buf = "laser cannon";
  }
  else if ( !str_prefix( argument, "heavy beam cannon" ) )
  { weap = MS_HEAVY_BEAM_CANNON;
    buf = "heavy beam cannon";
  }
  else if ( !str_prefix( argument, "vulcan" ) )
  { weap = MS_VULCAN;
    buf = "vulcan cannons";
  }
  else if ( !str_prefix( argument, "beam sabre" ) )
  { weap = MS_BEAM_SABRE;
    buf = "beam sabre";
  }
  else if ( !str_prefix( argument, "beam sword" ) )
  { weap = MS_BEAM_SWORD;
    buf = "beam sword";
  }
  else if ( !str_prefix( argument, "crasher" ) )
  { weap = MS_CRASHER;
    buf = "crasher";
  }
  else if ( !str_prefix( argument, "buster rifle" ) )
  { weap = MS_BUSTER_RIFLE;
    buf = "buster rifle";
  }
  else if ( !str_prefix( argument, "twin buster rifle" ) )
  { weap = MS_TWIN_BUSTER_RIFLE;
    buf = "twin buster rifle";
  }
  else if ( !str_prefix( argument, "beam scythe" ) )
  { weap = MS_BEAM_SCYTHE;
    buf = "beam scythe";
  }
  else if ( !str_prefix( argument, "gatling gun" ) )
  { weap = MS_GATLING_GUN;
    buf = "gatling gun";
  }
  else if ( !str_prefix( argument, "miniguns" ) )
  { weap = MS_MINIGUNS;
    buf = "miniguns";
  }
  else if ( !str_prefix( argument, "heat shotels" ) )
  { weap = MS_HEAT_SHOTELS;
    buf = "heat shotels";
  }
  else if ( !str_prefix( argument, "beam glaive" ) )
  { weap = MS_BEAM_GLAIVE;
    buf = "beam glaive";
  }
  else if ( !str_prefix( argument, "dragon fang" ) )
  { weap = MS_DRAGON_FANG;
    buf = "dragon fang";
  }
  else if ( !str_prefix( argument, "heat rod" ) )
  { weap = MS_HEAT_ROD;
    buf = "heat rod";
  }
  else if ( !str_prefix( argument, "flamethrower" ) )
  {
    weap = MS_FLAMETHROWER;
    buf = "flamethrower";
  }

  else
  { send_to_char( "That's not a known mobile suit weapon.\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->suit[SUIT_EQ],weap) )
  { send_to_char( "Your mobile suit is not equipped with one of those.\n\r", ch );
    return;
  }

  ch->pcdata->suit[SUIT_READY] = weap;
  act( "You ready your $T.", ch, NULL, buf, TO_CHAR );
  act( "$n readies $s $T.", ch, NULL, buf, TO_ROOM );
  return;
}

void do_fire( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  CHAR_DATA *victim = NULL;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  extern char * const dir_name[];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char *buf;
  int dam, dam1, wait, ammo, i, j, k, type;
  int aim, weapon, attacks, count, cost, dir = 0, range = 0;
  bool hitpc = FALSE;
  bool targeted = FALSE;

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You can't do that without a mobile suit.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_READY] == 0 )
  { send_to_char( "You don't have a weapon readied.", ch );
    return;
  }
  cost = 1;
  range = 0;
  attacks = 1;
  aim = ch->pcdata->suit[SUIT_AIM];
  weapon = suit_table[ch->pcdata->suit[SUIT_NUMBER]].weapon;

  if ( ch->pcdata->suit[SUIT_READY] == MS_FLAMETHROWER )
  {
    fire_flamethrower( ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_READY] == MS_MINIGUNS )
  {
    fire_miniguns( ch, argument );
    return;
  }

  switch( ch->pcdata->suit[SUIT_READY] )
  { case MS_MACHINE_GUN:
 	dam = dice( 1, 50 + aim / 5 ) + (weapon-90)*5;
 	buf = "machine gun";
 	wait = 6;
 	ammo = SUIT_BULLETS;
 	range = 1;
 	type = DAM_BULLETS;
 	attacks = 5;
 	break;
    case MS_BEAM_RIFLE:
    	dam = 150 + dice( 8, aim / 5 ) + (weapon-90)*10;
    	buf = "beam rifle";
    	wait = 6;
    	ammo = SUIT_BEAM;
    	cost = 1;
    	range = 1;
    	type = DAM_BEAMRIFLE;
    	break;
    case MS_BEAM_CANNON:
    	dam = 650 + dice( 5, aim / 3 ) + (weapon-90)*10;
    	buf = "beam cannon";
    	wait = 6;
    	ammo = SUIT_BEAM;
    	cost = 3;
    	range = 2;
    	type = DAM_BEAMRIFLE;
    	break;
    case MS_LASER_CANNON:
    	dam = 1500 + dice( 8, aim / 2 ) + (weapon-90)*10;
    	buf = "laser cannon";
    	wait = 8;
    	ammo = SUIT_PLASMA;
    	cost = 1;
    	range = 1;
    	type = DAM_BEAMRIFLE;
    	break;
    case MS_HEAVY_BEAM_CANNON:
    	dam = 1500 + dice( 3, aim ) + (weapon-90)*50;
    	buf = "heavy beam cannon";
    	wait = 12;
    	ammo = SUIT_BEAM;
    	cost = 10;
    	range = 10;
    	type = DAM_BEAMRIFLE;
    	break;
    case MS_VULCAN:
    	dam = dice( 2, 50 + aim / 5 ) + (weapon-90)*5;
    	buf = "vulcan cannon";
    	wait = 4;
    	ammo = SUIT_BULLETS;
    	cost = 1;
    	range = 1;
    	type = DAM_BULLETS;
    	attacks = 8;
    	break;
    case MS_DOBERGUN:
    	dam = dice( 3, aim ) + (weapon-90)*10;
    	buf = "dobergun";
    	wait = 16-((suit_table[ch->pcdata->suit[SUIT_NUMBER]].speed-80)/10);
    	ammo = SUIT_SHELLS;
    	cost = 1;
    	range = 0;
    	type = DAM_SHELLS;
    	break;
    case MS_BUSTER_RIFLE:
    	dam = aim + dice(3,aim) + (weapon-90)*10;
    	buf = "buster rifle";
    	wait = 12;
    	ammo = SUIT_BEAM;
    	cost = 200;
    	range = 3;
    	type = DAM_BEAMRIFLE;
    	break;
    case MS_TWIN_BUSTER_RIFLE:
    	dam = aim + dice(6,aim) + (weapon-90)*20;
    	buf = "twin buster rifle";
    	wait = 8;
    	ammo = SUIT_BEAM;
    	cost = 200;
    	range = 10;
    	type = DAM_BEAMRIFLE;
    	break;
    case MS_GATLING_GUN:
        dam = dice( 2, 50 + aim/3 ) + (weapon-90)*5;
        buf = "gatling gun";
        wait = 6;
        ammo = SUIT_BULLETS;
        cost = 1;
        range = 2;
        type = DAM_BULLETS;
        attacks = 10;
        break;
    case MS_MINIGUNS:
        dam = dice( 2, 50 + aim/4 ) + (weapon-90)*5;
        buf = "miniguns";
        wait = 8;
        ammo = SUIT_BULLETS;
        cost = 1;
        range = 0;
        type = DAM_BULLETS;
	break;
    default:
    	send_to_char( "You can't fire that!\n\r", ch );
    	buf = "fuck this";
    	wait = 12;
    	ammo = SUIT_BULLETS;
    	range = 0;
    	type = TYPE_HIT;
    	return;
  }

  if ( ch->fighting != NULL )
  {
    victim = ch->fighting;
    if ( !IS_NPC(victim) && victim->level < 2 )
    { send_to_char( "Not on mortals.\n\r", ch );
      return;
    }
    targeted = TRUE;
  }
  else if ( argument[0] == '\0' )
  {
    send_to_char( "Fire at whom?\n\r", ch );
    WAIT_STATE( ch, 4 );
    return;
  }
  else if ( !str_cmp( argument, "north" ) )
    dir = DIR_NORTH;
  else if ( !str_cmp( argument, "east" ) )
    dir = DIR_EAST;
  else if ( !str_cmp( argument, "south" ) )
    dir = DIR_SOUTH;
  else if ( !str_cmp( argument, "west" ) )
    dir = DIR_WEST;
  else if ( !str_cmp( argument, "up" ) )
    dir = DIR_UP;
  else if ( !str_cmp( argument, "down" ) )
    dir = DIR_DOWN;
  else if ( (victim = get_char_room(ch,argument)) == NULL )
  { send_to_char( "Fire at whom?\n\r", ch );
    return;
  }
  else
    targeted = TRUE;


  if ( !targeted && range <= 0 )
  { send_to_char( "You may only fire that at nearby targets.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[ammo] <= cost )
  { send_to_char( "You don't have enough ammunition to do that!\n\r", ch );
    return;
  }

  /* break for special attacks for gundams */
  if ((ch->pcdata->suit[SUIT_READY] == MS_BUSTER_RIFLE
    || ch->pcdata->suit[SUIT_READY] == MS_TWIN_BUSTER_RIFLE)
    && !targeted )
  { fire_buster( ch, dam, dir );
    return;
  }

  if ( targeted )
  { sprintf( buf1, "You fire your %s at $N!", buf );
    sprintf( buf2, "$n fires $s %s at you!", buf );
    sprintf( buf3, "$n fires $s %s at $N.", buf );
    act( buf1, ch, NULL, victim, TO_CHAR );
    act( buf2, ch, NULL, victim, TO_VICT );
    act( buf3, ch, NULL, victim, TO_NOTVICT );
  }
  else
  { sprintf( buf1, "You fire your %s to the %s!", buf, dir_name[dir] );
    sprintf( buf2, "$n fires $s %s to the %s!", buf, dir_name[dir] ); 
    act( buf1, ch, NULL, NULL, TO_CHAR );
    act( buf2, ch, NULL, NULL, TO_ROOM );
  }

  dam1 = dam;
  if ( targeted )
  {

    if ( !IS_NPC(victim) )
    {
      hitpc = TRUE;
      if ( victim->level < 2 )
      {
        stc( "Firing on mortals isn't nice.\n\r", ch );
        return;
      }
    }

    for ( i = 0; i < attacks && victim && ch->pcdata->suit[ammo] >= cost; i++ )
    {
      if ( attacks > 1 )
        dam1 = dice( 5, dam ) / 5;

      if ( !IS_NPC(victim) && number_percent() < (victim->pcdata->body-60)
        && !IS_SUIT(victim) && type == DAM_BULLETS )
      {
        if ( !IS_SET( victim->act, PLR_BRIEF ) )
          act( "You dodge $n's bullets.",  ch, NULL, victim, TO_VICT    );
        if ( !IS_SET( ch->act, PLR_BRIEF ) )
          act( "$N dodges your bullets.", ch, NULL, victim, TO_CHAR    );
      }
      else if ( !shields_check( ch, victim, type ) )
      {
        if ( IS_NPC(victim)
          && ch->in_room->clan != ch->pcdata->clan[CLAN]
          && dice(1, 1+aim + isquare(aim)) >= aim
          && aim < 2500
          && ch->in_room == victim->in_room
          && aim < ch->pcdata->extras2[EVAL]*45 )
        {
          ch->pcdata->suit[SUIT_AIM]++;
        }
        damage( ch, victim, dam1, type );
      }

      ch->pcdata->suit[ammo] -= cost;
    }
  }
  else
  { in_room = ch->in_room;
    if ( ( pexit = in_room->exit[dir] ) == NULL
      || ( pexit->to_room ) == NULL )
    { send_to_char( ".. and manage to hit the wall.\n\r", ch );
      return;
    }
    for ( i = 0; i < range; i++ )
    { /* check for a room being in the direction */
      if ( ( pexit = in_room->exit[dir] ) == NULL
        || ( to_room = pexit->to_room ) == NULL )  
      {
        continue;
      }
      /* no looping through loop rooms */
      if ( in_room == to_room )
      {
        continue;
      }
      /* find out how many people thre are in the room */
      count = 0;
      for ( vch = to_room->people; vch != NULL; vch = vch_next )
      { vch_next = vch->next_in_room;
        count++;
      }

      /* now pick a victim */
      if ( count > 0 )
      count = dice(1,count);
      vch = to_room->people;
      dam1 = dam;
      if ( count > 0 )
      { for ( j = 1; j < count; j++ )
          vch = vch->next_in_room;
        for ( k = 0; k < attacks && vch && ch->pcdata->suit[ammo] >= cost; k++ )
        { /* randomize damage for multiple attacks */
          if ( attacks > 1 )
            dam1 = dice( 5, dam ) / 5;
          if ( ch->pcdata->suit[ammo] >= cost && !shields_check( ch, vch, type ) ) 
          { 
            if ( !IS_NPC(vch) )
              hitpc = TRUE;

            if ( !IS_NPC(vch) && number_percent() < (vch->pcdata->body-60)
              && !IS_SUIT(vch) && type == DAM_BULLETS )
            {
              if ( !IS_SET( vch->act, PLR_BRIEF ) )
                act( "You dodge $n's bullets.", ch, NULL, vch, TO_VICT);
              if ( !IS_SET( ch->act, PLR_BRIEF ) )
                act( "$N dodges your bullets.", ch, NULL, vch, TO_CHAR );
            }
            else
              damage( ch, vch, dam1, type );

            if ( vch != NULL && vch->fighting == ch
            && !(!IS_NPC(vch) && IS_SET(vch->act,PLR_FIREBACK)) )
              stop_fighting( vch, FALSE );

            ch->pcdata->suit[ammo] -= cost;
          }
        } 
      }
      /* set the stage for 2nd and 3rd room attacks */
      in_room = to_room;
    }
  }



  WAIT_STATE(ch,wait);
  return;
}

void do_shell( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  extern char * const dir_name[];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  int range, aim, i, dam, dir;
  bool hitpc = FALSE;

  if ( !IS_SUIT(ch) )
  { send_to_char( "You can't do that without a mobile suit!\n\r", ch );
    return;
  }
  aim = ch->pcdata->suit[SUIT_AIM];
  if ( ch->fighting != NULL )
  { send_to_char( "You can't shell while in combat.\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->suit[SUIT_READY],MS_DOBERGUN) )
  { send_to_char( "You need a Dobergun to shell.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_SHELLS] < 1 )
  { send_to_char( "You have no shells to fire!\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Syntax is shell <direction> <number of rooms>.\n\r", ch );
    return;
  }
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' || arg2[0] == '\0' )
 { send_to_char( "Syntax is shell <direction> <number of rooms>.\n\r", ch );
    return;
  }
  if ( !is_number( arg2 ) )
  { send_to_char( "The distance must be an integer between 1 and 50.\n\r", ch );
    return;
  }
  range = atoi( arg2 );
  if ( range <= 0 || range > 50 )
  { send_to_char( "Acceptable ranges are between 1 and 50.\n\r", ch );
    return;
  }
  if ( !str_prefix( arg1, "north" ) )
    dir = DIR_NORTH;
  else if ( !str_prefix( arg1, "east" ) )
    dir = DIR_EAST;
  else if ( !str_prefix( arg1, "south" ) )
    dir = DIR_SOUTH;
  else if ( !str_prefix( arg1, "west" ) )
    dir = DIR_WEST;
  else if ( !str_prefix( arg1, "up" ) )
    dir = DIR_UP;
  else if ( !str_prefix( arg1, "down" ) )
    dir = DIR_DOWN;  
  else
  { send_to_char( "That isn't a direction.\n\r", ch );
    return;
  }
  act( "You fire a shell $T!", ch, NULL, dir_name[dir], TO_CHAR );
  act( "$n fires a shell $T!", ch, NULL, dir_name[dir], TO_ROOM );
  in_room = ch->in_room;
  if ( ( pexit = in_room->exit[dir] ) == NULL
    || ( pexit->to_room ) == NULL )
  { send_to_char( "You fire against the wall.\n\r", ch );
    return;
  } 
  to_room = in_room;
  for ( i = 0; i < range; i++ )
  { if ( ( pexit = in_room->exit[dir] ) == NULL
      || ( to_room = pexit->to_room ) == NULL )  
    { send_to_char( "Your shell smashes into a wall.\n\r", ch );
      return;
    }
    in_room = to_room;
  }
  if ( to_room->people == NULL )
  { send_to_char( "Your shell explodes harmlessly.\n\r", ch );
    WAIT_STATE( ch, 20-((suit_table[ch->pcdata->suit[SUIT_NUMBER]].speed-90)/5) );
    return;
  }
  else
  { act( "A shell arcs in and explodes!", in_room->people, NULL, NULL, TO_ROOM );
    for ( vch = in_room->people; vch != NULL; vch = vch_next )
    { 
      vch_next = vch->next_in_room;

      if ( !can_see(ch,vch) )
        continue;

      dam = dice(3,aim) + (suit_table[ch->pcdata->suit[SUIT_NUMBER]].weapon - 90)*10;
      if ( !IS_NPC(vch) )
        hitpc = TRUE;

      if ( !shields_check( ch, vch, DAM_SHELLS ) )
      { 
        damage( ch, vch, dam, DAM_SHELLS );
        if ( vch != NULL && vch->fighting == ch )
          stop_fighting( vch, FALSE );
        if ( ch->fighting == vch )
          stop_fighting( ch, FALSE );
      }
    }
  }
  
  ch->pcdata->suit[SUIT_SHELLS]--;
  WAIT_STATE( ch, 20-((suit_table[ch->pcdata->suit[SUIT_NUMBER]].speed-90)/5) );  
  return;
}

void do_punch( CHAR_DATA *ch, char *argument )
{ int dam, combat, suit;
  CHAR_DATA *victim;
  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You can't do that without a mobile suit.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
    victim = ch->fighting;
  else if ( argument[0] == '\0' )
  { send_to_char( "Punch who?\n\r", ch );
    return;
  }
  else if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  combat = ch->pcdata->suit[SUIT_COMBAT];
  suit = ch->pcdata->suit[SUIT_NUMBER];
  dam = isquare( combat ) * dice(5,10) + (suit_table[suit].fight - 90)*10;
  dam += (suit_table[suit].power - 90) * 10;

  if ( IS_NPC(victim)
    && ch->in_room->clan != ch->pcdata->clan[CLAN]
    && dice(1,1+combat+isquare(combat)*2) >= combat
    && combat < 2500
    && combat < ch->pcdata->extras2[EVAL]*45 )
  {
      ch->pcdata->suit[SUIT_COMBAT]++;
  }
  damage( ch, victim, dam, TYPE_HIT );


  if ( dice(1,1+combat+isquare(combat)*2) >= combat )
    if ( combat < 2500 && victim && victim->position > POS_STUNNED
      && combat < ch->pcdata->extras2[EVAL]*45 )
      ch->pcdata->suit[SUIT_COMBAT]++;
  WAIT_STATE( ch, 5 );
  return;
}

void do_slash( CHAR_DATA *ch, char *argument )
{ int dam, weapon, suit, fuel, cost;
  CHAR_DATA *victim;
  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You can't do that without a mobile suit.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_READY] != MS_BEAM_SABRE
    && ch->pcdata->suit[SUIT_READY] != MS_BEAM_SWORD
    && ch->pcdata->suit[SUIT_READY] != MS_CRASHER
    && ch->pcdata->suit[SUIT_READY] != MS_BEAM_SCYTHE
    && ch->pcdata->suit[SUIT_READY] != MS_HEAT_SHOTELS
    && ch->pcdata->suit[SUIT_READY] != MS_BEAM_GLAIVE )
  { send_to_char( "You need a melee weapon to slash with.\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL )
    victim = ch->fighting;
  else if ( argument[0] == '\0' )
  { send_to_char( "Slash who?\n\r", ch );
    return;
  }
  else if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  weapon = ch->pcdata->suit[SUIT_WEAPON];
  suit = ch->pcdata->suit[SUIT_NUMBER];
  cost = 1;
  dam = isquare( weapon ) * dice(5,10);
  dam += (suit_table[suit].fight-90) * 10;
  dam += (suit_table[suit].power - 90) * 10;
  fuel = SUIT_BEAM;
  switch( ch->pcdata->suit[SUIT_READY] )
  { case MS_CRASHER:
    case MS_BEAM_SABRE:		dam *= 2;	break;
    case MS_BEAM_SWORD:	dam *= 4; cost=10;	break;
    case MS_BEAM_SCYTHE:
    case MS_BEAM_GLAIVE:	dam *= 3;	break;
    case MS_HEAT_SHOTELS: fuel = SUIT_PLASMA;	break;
    default:					break;
  }
  if ( ch->pcdata->suit[fuel] < cost )
  { send_to_char( "You need at least one unit of ammo.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_READY] == MS_HEAT_SHOTELS && IS_SUIT(victim) )
    dam *= 3;

  if ( IS_NPC(victim)
    && ch->in_room->clan != ch->pcdata->clan[CLAN]
    && dice(1,+weapon+isquare(weapon)*2) >= weapon
    && weapon < 2500
    && weapon < ch->pcdata->extras2[EVAL]*45 )
  {
      ch->pcdata->suit[SUIT_WEAPON]++;
  }

  damage( ch, victim, dam, DAM_BEAMSABRE );
  ch->pcdata->suit[fuel] -= cost; 
  if ( ch->pcdata->suit[SUIT_READY] == MS_HEAT_SHOTELS
    && victim && ch->pcdata->suit[fuel] > 0 )
  {
    dam = dam/2 + dice(1,dam);
    damage( ch, victim, dam, DAM_BEAMSABRE );
    ch->pcdata->suit[fuel]--;
  }

  WAIT_STATE( ch, 12 - (suit_table[ch->pcdata->suit[SUIT_NUMBER]].speed-100)/10 );
  return;
}

void do_install( CHAR_DATA *ch, char *argument )
{ int item, suit, i, j;
  bool match = FALSE;
  OBJ_DATA *obj;

  if ( IS_NPC(ch) )
    return;
  if ( !IS_SUIT(ch) )
  { send_to_char( "You can only install equipment in a mobile suit.\n\r", ch );
    return;
  }
  if ( ch->fighting != NULL )
  { send_to_char( "You can't do that in combat!\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Install what?\n\r", ch );
    return;
  }
  if  ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
  { send_to_char( "You don't have that.\n\r", ch );
    return;
  }
  if ( obj->item_type != ITEM_ACCESSORY )
  { send_to_char( "You can't install that in a mobile suit.\n\r", ch );
    return;
  }
  suit = ch->pcdata->suit[SUIT_NUMBER];
  item = obj->value[0];
  if ( item == 0 )
  { if ( ch->pcdata->suit[SUIT_COND] == 0
      && ch->pcdata->suit[SUIT_ARMOR] >= (suit_table[suit].armor * 15) )
    { send_to_char( "Your mobile suit needs no repairs.\n\r", ch );
      return;
    }
    ch->pcdata->suit[SUIT_ARMOR] = suit_table[suit].armor * 15;
    j = 1;
    for ( i = 0; i < 12 && !match; i++ )
    { if ( IS_SET(ch->pcdata->suit[SUIT_COND],j) )
      { REMOVE_BIT(ch->pcdata->suit[SUIT_COND],j);
        match = TRUE;
      }
      j *= 2;
    }
    act( "You patch up your $T with a field repair kit.",
    	ch, NULL, suit_table[suit].name, TO_CHAR );
    act( "$n patches up $s $T with a field repair kit.",
    	ch, NULL, suit_table[suit].name, TO_ROOM );
    extract_obj( obj );
    WAIT_STATE( ch, 20 );
    return;
  }
  if ( !IS_SET( suit_table[suit].eq, item ) )
  { send_to_char( "Your mobile suit is not equipped to use this.\n\r", ch );
    return;
  }
  if ( IS_SET( ch->pcdata->suit[SUIT_EQ], item ) )
  { send_to_char( "your mobile suit already has one of those.\n\r", ch );
    return;
  }

  SET_BIT(ch->pcdata->suit[SUIT_EQ],item);
  act( "You install $p in your mobile suit.", ch, obj, NULL, TO_CHAR );
  act( "$n installs $p in $s mobile suit.", ch, obj, NULL, TO_ROOM );
  extract_obj( obj );
  return;
}

void fire_miniguns( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char arg[MAX_INPUT_LENGTH];
  int aim, bullets, i;

  if ( IS_NPC(ch) || !IS_SUIT(ch) )
    return;

  if ( ch->pcdata->suit[SUIT_READY] != MS_MINIGUNS )
  {
    stc( "You don't have Miniguns readied.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_BULLETS] < 1 )
  {
    stc( "You're out of ammo!\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( ch->fighting != NULL )
    victim = ch->fighting;
  else
  {
    if ( arg[0] == '\0' )
    {
      stc( "Fire at who?\n\r", ch );
      return;
    }
    if ( (victim = get_char_room(ch,arg)) == NULL )
    {
      stc( "They aren't here.\n\r", ch );
      WAIT_STATE(ch,4);
      return;
    }
  }

  bullets = ch->pcdata->suit[SUIT_BULLETS];
  aim = ch->pcdata->suit[SUIT_AIM];

  act( "You open up with a burst of minigun fire.", ch, NULL, NULL, TO_CHAR );
  act( "$n opens up on the room with a burst of minigun fire!", ch, NULL, NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL && bullets > 0; vch = vch_next )
  {
    vch_next = vch->next_in_room;

    if ( !IS_NPC(vch) && vch->level < 2 )
      continue;
    if ( !is_same_group(victim,vch) && !(IS_NPC(vch) && IS_NPC(victim))  )
      continue;

    for ( i = 0; i < 6 && vch != NULL && bullets > 0; i++ )
    {
      bullets--;
      ch->pcdata->suit[SUIT_BULLETS]--;
      damage( ch, vch, dice(3,50+aim/5), DAM_BULLETS );
    }
  }

  WAIT_STATE(ch,10);
  return;
}

void fire_buster( CHAR_DATA *ch, int dam, int dir )
{ ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i, cost, range, wait;
  extern char * const dir_name[];
  bool hitpc = FALSE;

  if ( ch->pcdata->suit[SUIT_READY] == MS_BUSTER_RIFLE )
  { cost = 200;
    range = 3;
    wait = 14;
  }
  else
  { cost = 250;
    range = 10;
    wait = 8;
  }

  if ( ch->pcdata->suit[SUIT_BEAM] < cost )
  { send_to_char( "You don't have enough beam energy.\n\r", ch );
    return;
  }

  in_room = ch->in_room;
  if ( ( pexit = in_room->exit[dir] ) == NULL
    || ( pexit->to_room ) == NULL )
  { send_to_char( ".. and manage to hit the wall.\n\r", ch );
    return;
  }
  else
  { sprintf( buf1, "You fire your buster rifle to the %s!", dir_name[dir] );
    sprintf( buf2, "$n fires $s buster rifle to the %s!", dir_name[dir] );
    act( buf1, ch, NULL, NULL, TO_CHAR );
    act( buf2, ch, NULL, NULL, TO_ROOM );
  }
  for ( i = 0; i < range; i++ )
  { /* check for a room being in the direction */
    if ( ( pexit = in_room->exit[dir] ) == NULL
      || ( to_room = pexit->to_room ) == NULL )
    { continue;
    }
    /* No looping fire */
    if ( to_room == in_room )
    {
      continue;
    }
    for ( vch = to_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if ( vch != NULL  )
      {
        if ( !can_see(ch,vch) )
          continue;
        if ( !IS_NPC(vch) )
          hitpc = TRUE;
        damage( ch, vch, dam, DAM_BEAMRIFLE );
        if ( vch != NULL && vch->fighting == ch
          && !(!IS_NPC(vch) && IS_SET(vch->act,PLR_FIREBACK)) )
          stop_fighting( vch, FALSE );
      }
    }
    /* set the stage for 2nd and 3rd room attacks */
    in_room = to_room;
  }

  ch->pcdata->suit[SUIT_BEAM] -= cost;
  WAIT_STATE(ch,wait);
  return;
}

bool shields_check( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
  if ( IS_NPC(victim) )
    return FALSE;
  if ( dt != DAM_BULLETS && dt != DAM_SHELLS )
    return FALSE;

  if ( IS_CLASS(victim,CLASS_SORCERER) && !IS_SUIT(victim) )
    if ( IS_AFFECTED(victim,AFF_WINDY_SHIELD) )
      if ( number_percent() < victim->pcdata->powers[SCHOOL_WIND] )
      { if ( !IS_SET( ch->act, PLR_BRIEF ) )
          act( "$N's windy shield deflects your bullets.", ch, NULL, victim, TO_CHAR );
        if ( !IS_SET( victim->act, PLR_BRIEF ) )
          act( "Your windy shield deflects $n's bullets!", ch, NULL, victim, TO_VICT );
        return TRUE;
      }

  if ( IS_SUIT(victim) && IS_SET(victim->pcdata->suit[SUIT_EQ],MS_SHIELDS) )
    if ( victim->pcdata->suit[SUIT_PLASMA] > 0 )
      if ( number_percent() < 75 )
      { victim->pcdata->suit[SUIT_PLASMA]--;
        if ( !IS_SET( ch->act, PLR_BRIEF ) )
          act( "$N's planet defensors deflect your bullets.", ch, NULL, victim, TO_CHAR );
        if ( !IS_SET( victim->act, PLR_BRIEF ) )
          act( "Your planet defensors deflect $n's bullets!", ch, NULL, victim, TO_VICT );
        return TRUE;
      }

  return FALSE;
}

bool is_cloaked( CHAR_DATA *ch, CHAR_DATA *victim )
{ 
  OBJ_DATA *obj;
  int i, j, cond;

  /* Mriswith cloak */
  if ( !IS_SUIT(ch)
    && ((obj = get_eq_char( ch, WEAR_ABOUT )) != NULL)
    && obj->pIndexData->vnum == OBJ_VNUM_MRISWITH
    && ch->fighting == NULL
    && ch->position == POS_STANDING
    && ((IS_NPC(victim) && victim->level < 90) || ch == victim) )
  {
    return TRUE;
  }


  /* Deathscythe cloaker */
  if ( !IS_SUIT(ch) )
    return FALSE;
  if ( !IS_SET(ch->pcdata->suit[SUIT_EQ],MS_CLOAKER) )
    return FALSE;
  if ( ch->fighting != NULL )
    return FALSE;
  if ( IS_NPC(victim) && victim->level >= 100 )
    return FALSE;

  cond = 0; j = 1;
  for ( i = 0; i < 12; i++ )
  { if ( IS_SET(ch->pcdata->suit[SUIT_COND],j) )
     cond++;
    j *= 2;
  }
  if ( cond >= 6 ) // Serious condition
    return FALSE;

  return TRUE;
}

void do_wreck( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char *buf;
  OBJ_DATA *suit;
  int i, j, part;
  bool match = FALSE;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  { send_to_char( "Wreck what?\n\r", ch );
    return;
  }
  if ( ( suit = get_obj_list( ch, arg, ch->in_room->contents ) ) == NULL )
  { send_to_char( "That isn't here.\n\r", ch );
    return;
  }
  if ( suit->item_type != ITEM_SUIT )
  { send_to_char( "You can't wreck that.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_HANGER) )
  {
    if ( ch->pcdata->clan[CLAN] != ch->in_room->clan )
    {
      stc( "Planet defensors activate and absorb your attack.\n\r", ch );
      return;
    }
  }
  if ( IS_NPC(ch) )
  { act( "You give $p a good hard kick!", ch, suit, NULL, TO_CHAR );
    act( "$n gives $p a good hard kick!", ch, suit, NULL, TO_ROOM );
  }
  else
  { switch( ch->class )
    { case CLASS_SAIYAN:
        act( "You blast $p with Ki flame!", ch, suit, NULL, TO_CHAR );
        act( "$n blasts $p with Ki flame.", ch, suit, NULL, TO_ROOM );
        break;
      case CLASS_PATRYN:
        act( "You short circuit $p with a bolt of lightning!", ch, suit, NULL, TO_CHAR );
        act( "$n short circuits $p with a bolt of lightning.", ch, suit, NULL, TO_ROOM );
        break;
      case CLASS_FIST:
        act( "You slam your fist into $p, crushing it!", ch, suit, NULL, TO_CHAR );
        act( "$n slam your fist into $p, crushing it.", ch, suit, NULL, TO_ROOM );
        break;
      case CLASS_SORCERER:
        act( "You drop a fireball on $p!", ch, suit, NULL, TO_CHAR );
        act( "$n drops a fireball on $p.", ch, suit, NULL, TO_ROOM );
        break;
      case CLASS_MAZOKU:
        act( "You engulf $p in demonic energy!", ch, suit, NULL, TO_CHAR );
        act( "$n engulfs $p in demonic energy!", ch, suit, NULL, TO_ROOM );
        break;
      default:
        act( "You give $p a good hard kick!", ch, suit, NULL, TO_CHAR );
        act( "$n gives $p a good hard kick!", ch, suit, NULL, TO_ROOM );
        break;
    };
  }

  part = 1;
  if ( suit->value[SUIT_COND] < 4095 )
  { while ( !match )
    { i = dice(1,12) - 1;
      part = 1;
      for ( j = 0; j < i; j++ )
        part *= 2;
      if ( !IS_SET(suit->value[SUIT_COND],part) )
        match = TRUE;
    }
    SET_BIT(suit->value[SUIT_COND],part);
  }

  switch( part )
  { case 1:     buf = "torso has been damaged";         break;
    case 2:     buf = "guidance package has been damaged";      break;
    case 4:     buf = "head has been damaged";          break;
    case 8:     buf = "camera has been damaged";        break;
    case 16:    buf = "left leg has been damaged";      break;
    case 32:    buf = "right hand has been damaged";    break;
    case 64:    buf = "right leg has been damaged";     break;
    case 128:   buf = "left hand has been damaged";     break;
    case 256:   buf = "left arm has been damaged";      break;
    case 512:   buf = "radar system has been damaged";  break;
    case 1024:  buf = "right arm has been damaged";     break;
    case 2048:  buf = "propulsion system has been damaged";     break;
    default:    buf = "This is a bug";			break;
  };

  act( "The $T.", ch, NULL, buf, TO_CHAR );
  act( "The $T.", ch, NULL, buf, TO_ROOM );

  if ( suit->value[SUIT_COND] >= 4095 )
  { act( "$p collapses into a pile of scraps!", ch, suit, NULL, TO_CHAR );
    act( "$p collapses into a pile of scraps!", ch, suit, NULL, TO_ROOM );
    extract_obj( suit );
  }

  WAIT_STATE(ch,8);
  return;
}

void fire_flamethrower( CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam, combat;

  if ( IS_NPC(ch) || !IS_SUIT(ch) )
    return;

  if ( ch->pcdata->suit[SUIT_PLASMA] < 10 )
  {
    stc( "You need at least 10 units of plasma.\n\r", ch );
    return;
  }

  combat = ch->pcdata->suit[SUIT_COMBAT];
  ch->pcdata->suit[SUIT_PLASMA] -= 10;

  act( "You bathe your enemies in flame!", ch, NULL, NULL, TO_CHAR );
  act( "$n looses a huge gout of flame-like plasma!", ch, NULL, NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;
    if ( is_same_group( ch, vch ) )
      continue;
    dam = number_range( 3 * combat, 4 * combat );
    damage( ch, vch, dam, DAM_FIRE );
  }

  WAIT_STATE(ch,12);
  return;
}

void do_salvo( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *in_room;
  EXIT_DATA *pexit;
  extern char * const dir_name[];
  CHAR_DATA *victim = NULL;
  char arg[MAX_INPUT_LENGTH];
  int i, j, aim, weapon, wait, missiles, dam, rdam, dir;

  if ( ch->position < POS_FIGHTING )
  {
    stc( "You're too relaxed.\n\r", ch );
    return;
  }
  if ( ch->position == POS_FIGHTING )
  {
    stc( "At this range? Are you suicidal?\n\r", ch );
    return;
  }
  if ( !IS_SUIT(ch) )
  {
    stc( "Go behind a tree, will ya?\n\r", ch );
    return;
  }
  if ( !IS_SET(ch->pcdata->suit[SUIT_EQ],MS_MISSILES) )
  {
    stc( "Your suit is not equipped with missiles.\n\r", ch );
    return;
  }
  if ( ch->pcdata->suit[SUIT_MISSILES] < 1 )
  {
    stc( "You have no missiles left.\n\r", ch );
    return;
  }
  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    stc( "Fire your missiles in which direction?\n\r", ch );
    return;
  }
  if ( !str_prefix( arg, "north" ) )
    dir = DIR_NORTH;
  else if ( !str_prefix( arg, "east" ) )
    dir = DIR_EAST;
  else if ( !str_prefix( arg, "south" ) )
    dir = DIR_SOUTH;
  else if ( !str_prefix( arg, "west" ) )
    dir = DIR_WEST;
  else if ( !str_prefix( arg, "up" ) )
    dir = DIR_UP;
  else if ( !str_prefix( arg, "down" ) )
    dir = DIR_DOWN;
  else
  {
    stc( "ERROR: That direction does not parse.\n\r", ch );
    return;
  }

  act( "You launch missiles to the $T.", ch, NULL, dir_name[dir], TO_CHAR );
  act( "$n launches missiles to the $T.", ch, NULL, dir_name[dir], TO_ROOM );

  weapon = suit_table[ch->pcdata->suit[SUIT_NUMBER]].weapon;
  aim = ch->pcdata->suit[SUIT_AIM];
  switch( ch->pcdata->suit[SUIT_NUMBER] )
  {
    case 2: /* Aries */
      missiles = 4;
      wait = 4;
      dam = weapon + dice(3,aim/10);
      break;
    case 11: /* Heavyarms */
      missiles = 16;
      wait = 16;
      dam = weapon*5 + dice(10,aim/10);
      break;
    case 12: /* Sandrock */
      missiles = 2;
      wait = 4;
      dam = weapon*20 + dice(20,aim/10);
      break;
    case 15: /* M1A1 Tank */
      missiles = 8;
      wait = 2;
      dam = weapon*10 + dice(10,aim/5);
      break;
    default:
      return;
      break;
  }

  missiles = UMIN( ch->pcdata->suit[SUIT_MISSILES], missiles );
  ch->pcdata->suit[SUIT_MISSILES] -= missiles;
  in_room = ch->in_room;

  for ( ; ; )
  {
    if ( (pexit = in_room->exit[dir]) == NULL ||
         (in_room = pexit->to_room) == NULL )
    {
      stc( "The missiles explode against a wall.\n\r", ch );
      WAIT_STATE(ch,wait);
      return;
    }

    if ( in_room->people == NULL )
      continue;

    for ( i = 0; i < missiles && in_room->people != NULL; i++ )
    {
      rdam = dice(10,dam) / 5;
      j = 0;
      while ( victim != NULL )
      {
        j++;
        victim = victim->next_in_room;
      }
      j = dice(1,j);
      victim = in_room->people;
      for ( ; j > 1 && victim->next_in_room != NULL; j-- )
        victim  = victim->next_in_room;

      damage( ch, victim, rdam, DAM_MISSILES );
      if ( ch->fighting == victim )
        stop_fighting( ch, FALSE );
      if ( victim != NULL && victim->fighting == ch
        && !(!IS_NPC(victim) && IS_SET(victim->act,PLR_FIREBACK)) )
        stop_fighting( victim, FALSE );
    }
    break;
  }

  WAIT_STATE(ch,wait);
  return;
}
