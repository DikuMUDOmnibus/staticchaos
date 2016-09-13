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


void do_induct( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int clan, cost;

  if ( IS_NPC(ch) )
    return;

  clan = ch->pcdata->clan[CLAN];

  if ( argument[0] == '\0' )
  { send_to_char( "Induct who?\n\r", ch );
    return;
  }
  if ( clan <= 0 )
  { send_to_char( "You aren't even in a clan!\n\r", ch );
    return;
  }
  if ( ch->pcdata->clan[CLAN_RANK] > 2 )
  { send_to_char( "You don't have enough rank to initiate new members.\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "You have to buy guards, not induct them.\n\r", ch );
    return;
  }
  if ( victim->level < 2 )
  { send_to_char( "They aren't even an avatar!\n\r", ch );
    return;
  }
  if ( victim->pcdata->clan[CLAN] != 0 )
  { send_to_char( "They're already in another clan.\n\r", ch );
    return;
  }
  cost = 100 + victim->pcdata->extras2[EXCLANS] * 100;
  if ( ch->pcdata->primal < cost )
  {
    sprintf( buf, "You will need %d primal to induct %s.", cost, victim->name );
    stc( buf, ch );
    return;
  }

  sprintf( buf, "You offer %s membership in %s.\n\r", victim->name, clan_table[clan].name );
  send_to_char( buf, ch );
  sprintf( buf, "%s offers you membership in %s.\n\r", ch->name, clan_table[clan].name );
  send_to_char( buf, victim );
  sprintf( buf, "Type 'accept %s' to accept this offer.\n\r", ch->name );
  send_to_char( buf, victim );
  victim->pcdata->clan[CLAN_TEMP] = clan;
  return;
}

void do_clantalk( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MAX_STRING_LENGTH];
  int clan;

  if ( IS_NPC(ch) )
    return;

  if ( argument[0] == '\0' )
  { send_to_char( "Say what?\n\r", ch );
    return;
  }

  clan = ch->pcdata->clan[CLAN];

  if ( clan <= 0 )
  { send_to_char( "You aren't in a clan.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
  {
    stc( "Better concentrate on the matter at hand.\n\r", ch );
    return;
  }
  sprintf( buf, "You clantalk '`C%s`n'\n\r", argument );
  send_to_char( buf, ch );
  sprintf( buf, "%s clantalks '`C%s`n'\n\r", ch->name, argument );
  for ( vch = char_list; vch != NULL; vch = vch_next )
  { vch_next        = vch->next;
    if ( IS_NPC(vch) || vch->in_room == NULL || vch == ch )
      continue;
    if ( vch->pcdata->clan[CLAN] == clan || vch->level >= 20 )
      send_to_char( buf, vch );
  }
  return;
}

void do_accept( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  CHAR_DATA *leader;
  int clan, cost;

  if ( IS_NPC(ch) )
    return;
  cost = 100 + ch->pcdata->extras2[EXCLANS] * 100;
  if ( ch->pcdata->primal < cost )
  {
    sprintf( buf, "You will need %d primal to join a clan.\n\r", cost );
    stc( buf, ch );
    return;
  }
  if ( ch->pcdata->clan[CLAN] > 0 )
  { send_to_char( "You're already in another clan.\n\r", ch );
    return;
  }
  if ( ch->pcdata->clan[CLAN_TEMP] <= 0 )
  { send_to_char( "You haven't been offered induction to a clan.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "Accept whose offer?\n\r", ch );
    return;
  }
  if ( ( leader = get_char_room( ch, argument ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(leader) )
  { send_to_char( "Uhh... that's a mob.\n\r", ch );
    return;
  }
  if ( leader->pcdata->clan[CLAN] != ch->pcdata->clan[CLAN_TEMP] )
  { send_to_char( "They didn't make that offer.\n\r", ch );
    return;
  }
  if ( leader->pcdata->clan[CLAN_RANK] > 2 )
  { send_to_char( "They don't have the authority to induct you.\n\r", ch );
    return;
  }
  if ( leader->pcdata->primal < cost  )
  { send_to_char( "They don't have enough primal to induct you.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= cost;
  leader->pcdata->primal -= cost;
  clan = ch->pcdata->clan[CLAN_TEMP];
  ch->pcdata->clan[CLAN] = clan;
  ch->pcdata->clan[CLAN_TEMP] = 0;
  ch->pcdata->clan[CLAN_RANK] = 5;
  act( "You face $N, and place your palm against $S.", ch, NULL, leader, TO_CHAR );
  act( "$n faces you, and places $s palm against yours.", ch, NULL, leader, TO_VICT );
  act( "$n faces $N, placing $s palm against $S.", ch, NULL, leader, TO_NOTVICT );
  act( "Your hand sizzles as the sigil of the $T clan is burned into it!", ch, NULL, clan_table[clan].name, TO_CHAR );
  act( "$n's hand sizzles as the sigil of the $T clan is burned into it!", ch, NULL, clan_table[clan].name, TO_ROOM );
  sprintf( buf, "%s has been inducted into clan %s.", ch->name, clan_table[clan].name );
  do_info( ch, buf );
  do_save( ch, "auto" );
  return;
}

void do_banish( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int clan;

  if ( IS_NPC(ch) )
    return;

  clan = ch->pcdata->clan[CLAN];

  if ( argument[0] == '\0' )
  { send_to_char( "Banish who?\n\r", ch );
    return;
  }
  else if ( !str_cmp( argument, "self" ) )
  { if ( clan <= 0 )
    { send_to_char( "You aren't even in a clan.\n\r", ch );
      return;
    }
    if ( ch->pcdata->clan[CLAN_RANK] == 1 )
    { send_to_char( "You're a leader, buttfucker.  No quitting.\n\r", ch );
      return;
    }
    if ( ch->pcdata->primal < 300 )
    { send_to_char( "It costs 300 primal to banish yourself.\n\r", ch );
      return;
    }

    ch->pcdata->primal -= 300;
    send_to_char( "Agony shoots up your arm as the sigil on your hand shatters!\n\r", ch );
    sprintf( buf, "%s has quit clan %s.\n\r", ch->name, clan_table[clan].name );
    do_info( ch, buf );
    ch->pcdata->clan[CLAN] = 0;
    ch->pcdata->clan[CLAN_RANK] = 0;
    ch->pcdata->extras2[EXCLANS]++;
    ch->pcdata->extras[HOME] = 806;
    do_save( ch, "auto" );
    return;
  }
  else
  { if ( clan <= 0 || ch->pcdata->clan[CLAN_RANK] > 2 )
    { send_to_char( "You don't have the authority to do that.\n\r", ch ); 
      return;
    }
    if ( ch->pcdata->primal < 100 )
    { send_to_char( "It'll cost you 100 primal to banish them.\n\r", ch ); 
      return;
    }
    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    { send_to_char( "They aren't here.\n\r", ch );
      return;
    }
    if ( IS_NPC(victim) )
    { send_to_char( "They're a mob, dipshit.\n\r", ch );
      return;
    }
    if ( victim->pcdata->clan[CLAN] != ch->pcdata->clan[CLAN] )
    { send_to_char( "They're not in your clan, dipshit.\n\r", ch );
      return;
    }
    if ( victim->pcdata->clan[CLAN_RANK] <= ch->pcdata->clan[CLAN_RANK] )
    { send_to_char( "Hahaha.. yah right.\n\r", ch );
      return;
    }

    ch->pcdata->primal -= 100;
    act( "You point at $N accusingly.", ch, NULL, victim, TO_CHAR );
    act( "$n points at you accusingly!", ch, NULL, victim, TO_VICT );
    act( "$n points at $N accusingly.", ch, NULL, victim, TO_NOTVICT );
    do_yell( ch, "Begone!!" );
    send_to_char( "Agony shoots up your arm as the sigil on your hand shatters!\n\r",victim); 
    sprintf( buf, "%s has been banished from clan %s.\n\r", victim->name, clan_table[clan].name );
    do_info( ch, buf );
    victim->pcdata->clan[CLAN] = 0;
    victim->pcdata->clan[CLAN_RANK] = 0;
    victim->pcdata->extras2[EXCLANS]++;
    victim->pcdata->extras[HOME] = 806;
    do_save( victim, "auto" );
  }

  return;
}

void do_clanrank( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int clan, rank;

  if ( IS_NPC(ch) ) 
    return;
  
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  clan = ch->pcdata->clan[CLAN];

  if ( clan <= 0 )
  { send_to_char( "You aren't in a clan.\n\r", ch );
    return;
  }
  if ( ch->pcdata->clan[CLAN_RANK] > 2 )
  { send_to_char( "You don't have that authority.\n\r", ch );
    return;
  }
  if ( arg1[0] == '\0' )
  { send_to_char( "Clanrank who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  { send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_NPC(victim) )
  { send_to_char( "Uhhh.. they're a mob.\n\r", ch );
    return;
  }
  if ( victim->pcdata->clan[CLAN] != ch->pcdata->clan[CLAN] )
  { send_to_char( "They don't fall under your jurisdiction.\n\r", ch );
    return;
  }
  if ( victim->pcdata->clan[CLAN_RANK] <= ch->pcdata->clan[CLAN_RANK] )
  { send_to_char( "Only Alathon can do that.\n\r", ch );
    return;
  }
  if ( arg2[0] == '\0' || !is_number( arg2 ) )
  { send_to_char( "Syntax: clanrank <target> <new rank>\n\r", ch );
    return;
  }
  rank = atoi( arg2 );
  if ( rank < 2 || rank > 5 )
  { send_to_char( "Acceptable ranks are integers between 2 to 5.\n\r", ch );
    return;
  }
  if ( rank == 2 && ch->pcdata->clan[CLAN_RANK] == 2 )
  { send_to_char( "You don't have the authority.\n\r", ch );
    return;
  }

  sprintf( buf, "You advance %s to the rank of %s.\n\r", victim->name, clan_table[clan].rank[rank] );
  send_to_char( buf, ch );
  sprintf( buf, "You have been raised to the rank of %s!\n\r", clan_table[clan].rank[rank] );
  send_to_char( buf, victim );
  victim->pcdata->clan[CLAN_RANK] = rank;
  do_save( victim, "auto" );
  return;
}

void do_clanboards( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i, j;


  sprintf( buf, "\n\r`d-----------------------------------" );
  for ( i = 1; i <= MAX_CLANS; i++ )
  { sprintf( buf2, "-------" );
    strcat( buf, buf2 );
  }
  sprintf( buf2, "\n\r`nClan      `d|   `WPks   Pds `d| `WUnclanned  " );
  strcat( buf, buf2 );
  for ( i = 1; i <= MAX_CLANS; i++ )
  { sprintf( buf2, "%s  ", clan_table[i].title );
    strcat( buf, buf2 );
  }
  sprintf( buf2, "\n\r`d-----------------------------------" );
  strcat( buf, buf2 );
  for ( i = 1; i <= MAX_CLANS; i++ )
  { sprintf( buf2, "-------" );
    strcat( buf, buf2 );
  }
  sprintf( buf2, "`n\n\r" );
  strcat( buf, buf2 );

  for ( i = 0; i <= MAX_CLANS; i++ )
  { sprintf( buf2, "%-10s`d|`n ", clan_table[i].name );
    strcat( buf, buf2 );
    sprintf( buf2, "%5d %5d `d|`n  %5d    ", clan_table[i].pkills,
    	clan_table[i].pdeaths, clan_table[i].kills[0] );
    strcat( buf, buf2 );
    for ( j = 1; j <= MAX_CLANS; j++ )
    { sprintf( buf2, "%4d   ", clan_table[i].kills[j] );
      strcat( buf, buf2 );
    }
    sprintf( buf2, "\n\r" );
    strcat( buf, buf2 );
  }

  sprintf( buf2, "`d-----------------------------------" );
  strcat( buf, buf2 );
  for ( i = 1; i <= MAX_CLANS; i++ )
  { sprintf( buf2, "-------" );
    strcat( buf, buf2 );
  }
  sprintf( buf2, "`n\n\r" );
  strcat( buf, buf2 );
  send_to_char( buf, ch );
  return;
}

void do_clandeposit( CHAR_DATA *ch, char *argument )
{ CHAR_DATA *banker;
  char buf[MAX_STRING_LENGTH];
  char log_buf[MAX_STRING_LENGTH];
  int gold;

  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->clan[CLAN] <= 0 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  for ( banker = ch->in_room->people; banker != NULL; banker = banker->next_in_room )
  { if ( IS_NPC(banker) && IS_SET(banker->act, ACT_TRAIN) )
      break;
  }
  if ( banker == NULL )
  { send_to_char( "Find a banker first.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "Deposit how much?\n\r", ch ); 
    return;
  }
  if ( !is_number( argument ) )
  { send_to_char( "Deposit a number of gold, not gibberish.\n\r", ch );
    return;
  }
  gold = atoi( argument );
  if ( gold < 1 || gold > 999999 )
  { send_to_char( "Deposit between 1 and 999999 gold.\n\r", ch );
    return;
  }
  if ( gold > ch->gold )
  { send_to_char( "You don't have that much.\n\r", ch );
    return;
  }
  ch->gold -= gold;
  clan_table[ch->pcdata->clan[CLAN]].gold += gold;
  sprintf( buf, "You deposit %d gold.  Your clan now has %d gold saved.\n\r",
    gold, clan_table[ch->pcdata->clan[CLAN]].gold );
  send_to_char( buf, ch );
  sprintf( log_buf, "%s deposited %d, total of %d.", ch->name, gold,
    clan_table[ch->pcdata->clan[CLAN]].gold );
  log_string( log_buf );
  return;
}

void do_clanbalance( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  int i;

  if ( IS_NPC(ch) )
    return;

  if ( ch->level < MAX_LEVEL )
  { if ( ch->pcdata->clan[CLAN] <= 0 )
    { send_to_char( "But you're not in a clan.\n\r", ch );
      return;
    }
    sprintf( buf, "Clan %s has %d gold.\n\r", clan_table[ch->pcdata->clan[CLAN]].name,
      clan_table[ch->pcdata->clan[CLAN]].gold );
    send_to_char( buf, ch );
  }
  else
  { for ( i = 1; i <= MAX_CLANS; i++ )
    { sprintf( buf, "Clan %9s has %d gold.\n\r", clan_table[i].name, clan_table[i].gold );
      send_to_char( buf, ch );
    }
  }
  return;
}

void do_banis( CHAR_DATA *ch, char *argument )
{
  stc( "If you want to banish somebody, spell it out in full.\n\r", ch );
  return;
}

void do_diplomacy( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i, j;

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' || str_cmp( arg1, "set" ) )
  {
    sprintf( buf, "\n\r`d-----------" );
    for ( i = 1; i <= MAX_CLANS; i++ )
    { sprintf( buf2, "--------" );
      strcat( buf, buf2 );
    }
    sprintf( buf2, "\n\r`nClan      `d| " );
    strcat( buf, buf2 );
    for ( i = 1; i <= MAX_CLANS; i++ )
    { sprintf( buf2, " %s  ", clan_table[i].title );
      strcat( buf, buf2 );
    }
    sprintf( buf2, "\n\r`d-----------" );
    strcat( buf, buf2 );
    for ( i = 1; i <= MAX_CLANS; i++ )
    { sprintf( buf2, "--------" );
      strcat( buf, buf2 );
    }
    sprintf( buf2, "`n\n\r" );
    strcat( buf, buf2 );
    for ( i = 1; i <= MAX_CLANS; i++ )
    { sprintf( buf2, "%-10s`d|`n ", clan_table[i].name );
      strcat( buf, buf2 );
      for ( j = 1; j <= MAX_CLANS; j++ )
      { 
        if ( i != j && clan_table[j].active )
          sprintf( buf2, "%s ", dip_table[clan_table[i].diplomacy[j]] );
        else
          sprintf( buf2, "  n/a   " );
        strcat( buf, buf2 );
      }
      sprintf( buf2, "\n\r" );
      strcat( buf, buf2 );
    }

  
    sprintf( buf2, "`d-----------" );
    strcat( buf, buf2 );
    for ( i = 1; i <= MAX_CLANS; i++ )
    { sprintf( buf2, "--------" );
      strcat( buf, buf2 );
    }
    sprintf( buf2, "`n\n\r" );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    return;
  }
  else if ( !str_cmp( arg1, "set" ) )
  {
    if ( IS_NPC(ch) )
      return;
    if ( ch->pcdata->clan[CLAN] < 1 || ch->pcdata->clan[CLAN] > MAX_CLANS )
    {
      stc( "You aren't even in a clan.\n\r", ch );
      return;
    }
    if ( ch->pcdata->clan[CLAN_RANK] > 1 )
    {
      stc( "Only your leader can set policy.\n\r", ch );
      return;
    }

    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg2[0] == '\0' || arg3[0] == '\0' )
    {
      stc( "Syntax: diplomacy set <clanname> <allied/amity/neutral/hostile/war>\n\r", ch );
      return;
    }

    for ( i = 1; i <= MAX_CLANS && str_cmp( arg2, clan_table[i].name ); i++ )
      ;
    // no body of for loop.

    if ( i > MAX_CLANS )
    {
      stc( "There is no such clan.\n\r", ch );
      return;
    }

    if ( !str_prefix( arg3, "allied" ) )
      j = CLAN_ALLIED;
    else if ( !str_prefix( arg3, "amity" ) )
      j = CLAN_AMITY;
    else if ( !str_prefix( arg3, "neutral" ) )
      j = CLAN_NEUTRAL;
    else if ( !str_prefix( arg3, "hostile" ) )
      j = CLAN_HOSTILE;
    else if ( !str_prefix( arg3, "war" ) )
      j = CLAN_WAR;
    else
    {
      stc( "Syntax: diplomacy set <clanname> <allied/amity/neutral/hostile/war>\n\r", ch );
      return;
    }

    clan_table[ch->pcdata->clan[CLAN]].diplomacy[i] = j;
    stc( "Clan diplomacy table updated.\n\r", ch );
    return;
  }



  return;
}
