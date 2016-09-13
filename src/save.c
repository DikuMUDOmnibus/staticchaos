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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif

#if !defined(macintosh) && !defined(MSDOS)
//void    system          args( ( char *string ) );
#endif

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	write_finger	args( ( CHAR_DATA *ch,	FILE *fp ) );
void	fwrite_finger	args( ( CHAR_DATA *ch,	FILE *fp ) );
void	fwrite_corpse	args( ( OBJ_DATA *obj, FILE *fp, int iNest ) );
void	fread_corpse	args( ( FILE *fp ) );
void	fwrite_pkill	args( ( PKILL_DATA *pkill, FILE *fp ) );


/* Courtesy of Yaz of 4th Realm */
char *initial( const char *str )
{
    static char strint[ MAX_INPUT_LENGTH ];

    strint[0] = LOWER( str[ 0 ] );
    return strint;

}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char tempsave[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) || ch->level < 1 )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    ch->save_time = current_time;
    fclose( fpReserve );

    /* player files parsed directories by Yaz 4th Realm */
    /* This is the file which the written file will be copied to */
#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    "/", capitalize( ch->name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
#endif

    /* file to actually write to */
    sprintf( tempsave, "%s%s%s", PLAYER_TEMP_DIR, "/", capitalize( ch->name ) );

    if ( ( fp = fopen( tempsave, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( tempsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    /* file successfully written, now copy */
    sprintf( buf, "cp -f %s %s", tempsave, strsave );
    system( buf );
    return;
}

void save_char_fin( CHAR_DATA *ch )
{ char finfile[MAX_INPUT_LENGTH];
  FILE *fp;

  if ( IS_NPC(ch) || ch->level < 1 )
    return;

  if ( ch->desc != NULL && ch->desc->original != NULL )
    ch = ch->desc->original;

  fclose( fpReserve );
  sprintf( finfile, "%s%s", "../finger/", capitalize( ch->name ) );

  if ( ( fp = fopen( finfile, "w" ) ) == NULL )
  { bug( "Save_char_finger: fopen", 0 );
    perror( finfile );
  }
  else
  { fwrite_finger( ch, fp );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

/*
 * Write the finger file.
 */
void write_finger( CHAR_DATA *ch, FILE *fp )
{ char timebuf[50];
  char *point;
  char *sex;

  /* create a time string that doesn't include a next line */
  sprintf( timebuf, (char *) ctime( &current_time) );
  point = timebuf;
  while( *point != '\0' )
  { if ( *point == '\n' || *point == '\r' )
      *point = ' ';
    point++;
  }

  switch ( ch->sex )
  { case 1: sex = "He"; break;
    case 2: sex = "She"; break;
    default: sex = "It"; break;
  }


  fprintf( fp, "%s~\n",	ch->name			);
  fprintf( fp, "%s~\n",	ch->pcdata->title		);
  fprintf( fp, "%s~\n", sex				);
  fprintf( fp, "%s~\n", class_table[ch->class].who_name );
  fprintf( fp, "%s~\n",	timebuf				);
  fprintf( fp, "%d %d %d %d\n",
  	ch->pcdata->kills[PK],	ch->pcdata->kills[PD],
  	ch->pcdata->kills[MK],	ch->pcdata->kills[MD] );
  fprintf( fp, "%d %d %d\n", ch->level, ch->pcdata->extras[STATUS], ch->pcdata->extras[LEGEND] );
  fprintf( fp, "%d %d\n", ch->pcdata->clan[CLAN], ch->pcdata->clan[CLAN_RANK] );
  return;
}

void fwrite_finger( CHAR_DATA *ch, FILE *fp )
{
  char timebuf[50];
  char create[50];
  char *sex;
  int i, courage, total = 0;

  /* create a time string that doesn't include a next line */
  sprintf( timebuf, (char *) ctime( &current_time) );
  timebuf[strlen(timebuf)-1] = '\0';

  if ( strlen(ch->pcdata->created) < 20 )
    sprintf( create, "in the distant past." );
  else
    sprintf( create, ch->pcdata->created );

  switch ( ch->sex )
  { case 1: sex = "He"; break;
    case 2: sex = "She"; break;
    default: sex = "It"; break;
  }

  for ( i = 4; i <= 8; i++ )
    total += ch->pcdata->kills[i];

  if ( total < 10 )
    courage = -1;
  else
    courage = 100 * ch->pcdata->kills[8] / UMAX( 1, total );

  fprintf( fp, "Name     %s~\n", ch->name                        );
  fprintf( fp, "Title    %s~\n", ch->pcdata->title               );
  fprintf( fp, "Sex      %s~\n", sex                             );
  fprintf( fp, "Class    %s~\n", class_table[ch->class].who_name );
  fprintf( fp, "Laston   %s~\n", timebuf                         );
  fprintf( fp, "Created  %s~\n", create				 );
  fprintf( fp, "Clan     %d %d\n",
  	ch->pcdata->clan[CLAN],	ch->pcdata->clan[CLAN_RANK]	 );
  fprintf( fp, "Level    %d\n", ch->level			 );

  fprintf( fp, "PkPdMkMd %d %d %d %d\n",
        ch->pcdata->kills[PK],     ch->pcdata->kills[PD],
        ch->pcdata->kills[MK],     ch->pcdata->kills[MD] );
  fprintf( fp, "Ostat    %d\n", ch->pcdata->extras[STATUS]	 );
  fprintf( fp, "Status   %d\n", ch->pcdata->extras2[EVAL]	 );
  fprintf( fp, "Courage  %d\n",	courage				 );
  return;
}


/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, i;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"		);

    fprintf( fp, "Name         %s~\n",	ch->name		);
    fprintf( fp, "ShortDescr   %s~\n",	ch->short_descr		);
    fprintf( fp, "LongDescr    %s~\n",	ch->long_descr		);
    fprintf( fp, "Description  %s~\n",	ch->description		);
    fprintf( fp, "Prompt       %s~\n",	ch->prompt		);
    fprintf( fp, "Version      %d\n",	ch->pcdata->version	);
    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Class        %d\n",	ch->class		);
    fprintf( fp, "Race         %d\n",	ch->race		);
    fprintf( fp, "Level        %d\n",	ch->level		);
    fprintf( fp, "Language     %d\n",	ch->language		);
    fprintf( fp, "Trust        %d\n",	ch->trust		);
    fprintf( fp, "Wizbit       %d\n",	ch->wizbit		);
    fprintf( fp, "Security     %d\n",	ch->pcdata->security	);
    fprintf( fp, "Played       %d\n",	ch->played		);
    fprintf( fp, "Note         %d\n",   (int)ch->last_note      );
    fprintf( fp, "Room         %d\n",
	(  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	&& ch->was_in_room != NULL )
	    ? ch->was_in_room->vnum
	    : ch->in_room->vnum );

    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    fprintf( fp, "Gold         %d\n",	ch->gold		);
    fprintf( fp, "Exp          %d\n",	ch->exp			);
    fprintf( fp, "Act          %d\n",   ch->act			);
    fprintf( fp, "Totalexp     %d\n",	ch->totalexp		);
    fprintf( fp, "Newact       %d\n",   ch->pcdata->actnew	);
    fprintf( fp, "AffectedBy   %d\n",	ch->affected_by		);
    /* Bug fix from Alander */
    fprintf( fp, "Position     %d\n",
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "SavingThrow  %d\n",	ch->saving_throw	);
    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);
    fprintf( fp, "Armor        %d\n",	ch->armor		);
    fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    fprintf( fp, "Deaf         %d\n",	ch->deaf		);

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum         %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
	fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
	fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
	fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
	fprintf( fp, "Mname        %s~\n",	ch->pcdata->mname	);
	fprintf( fp, "Mmaster      %s~\n",	ch->pcdata->mmaster	);
	fprintf( fp, "Humiliate    %s~\n",	ch->pcdata->humiliate	);
	fprintf( fp, "Created      %s~\n",	ch->pcdata->created	);
	fprintf( fp, "Ignore       %s~\n",	ch->pcdata->ignore	);
	fprintf( fp, "Legend       %s~\n",	ch->pcdata->legend	);
	fprintf( fp, "Cflag        %s~\n",	ch->pcdata->cflag	);
	fprintf( fp, "Weapons      %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->weapons[0],	ch->pcdata->weapons[1],
		ch->pcdata->weapons[2],	ch->pcdata->weapons[3],
		ch->pcdata->weapons[4],	ch->pcdata->weapons[5],
		ch->pcdata->weapons[6],	ch->pcdata->weapons[7],
		ch->pcdata->weapons[8],	ch->pcdata->weapons[9],
		ch->pcdata->weapons[10],ch->pcdata->weapons[11],
		ch->pcdata->weapons[12] );
	fprintf( fp, "Stances      %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->stances[0], ch->pcdata->stances[1],
		ch->pcdata->stances[2], ch->pcdata->stances[3],
		ch->pcdata->stances[4], ch->pcdata->stances[5],
		ch->pcdata->stances[6], ch->pcdata->stances[7],
		ch->pcdata->stances[8], ch->pcdata->stances[9],
		ch->pcdata->stances[10] );

	fprintf( fp, "Newstats     %d %d %d %d %d\n", ch->pcdata->body,
		ch->pcdata->mind, ch->pcdata->spirit, ch->pcdata->will, ch->pcdata->primal );

	fprintf( fp, "Kills        %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->kills[0],	ch->pcdata->kills[1],
		ch->pcdata->kills[2],   ch->pcdata->kills[3],
		ch->pcdata->kills[4],   ch->pcdata->kills[5],
		ch->pcdata->kills[6],   ch->pcdata->kills[7],
		ch->pcdata->kills[8],   ch->pcdata->kills[9],
		ch->pcdata->kills[10],   ch->pcdata->kills[11],
		ch->pcdata->kills[12],   ch->pcdata->kills[13],
		ch->pcdata->kills[14] );

	fprintf( fp, "Extras       %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->extras[0],	ch->pcdata->extras[1],
		ch->pcdata->extras[2],	ch->pcdata->extras[3],
		ch->pcdata->extras[4],	ch->pcdata->extras[5],
		ch->pcdata->extras[6],	ch->pcdata->extras[7],
		ch->pcdata->extras[8],	ch->pcdata->extras[9] );

	fprintf( fp, "Extras2      %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->extras2[0],		ch->pcdata->extras2[1],
		ch->pcdata->extras2[2],         ch->pcdata->extras2[3],
		ch->pcdata->extras2[4],         ch->pcdata->extras2[5],
		ch->pcdata->extras2[6],         ch->pcdata->extras2[7],
		ch->pcdata->extras2[8],         ch->pcdata->extras2[9] );

        fprintf( fp, "Clan         %d %d %d %d %d %d %d %d %d %d\n",
                 ch->pcdata->clan[0],         ch->pcdata->clan[1],
                 ch->pcdata->clan[2],         ch->pcdata->clan[3],
                 ch->pcdata->clan[4],         ch->pcdata->clan[5],
                 ch->pcdata->clan[6],         ch->pcdata->clan[7],
                 ch->pcdata->clan[8],         ch->pcdata->clan[9] );

	fprintf( fp, "Powers       %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->powers[0],	ch->pcdata->powers[1],
		ch->pcdata->powers[2],	ch->pcdata->powers[3],
		ch->pcdata->powers[4],	ch->pcdata->powers[5],
		ch->pcdata->powers[6],	ch->pcdata->powers[7],
		ch->pcdata->powers[8],	ch->pcdata->powers[9] );

	fprintf( fp, "Suit         %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                ch->pcdata->suit[0],    ch->pcdata->suit[1],
                ch->pcdata->suit[2],    ch->pcdata->suit[3],
                ch->pcdata->suit[4],    ch->pcdata->suit[5],
                ch->pcdata->suit[6],    ch->pcdata->suit[7],
                ch->pcdata->suit[8],    ch->pcdata->suit[9],
                ch->pcdata->suit[10],   ch->pcdata->suit[11],
                ch->pcdata->suit[12],   ch->pcdata->suit[13],
                ch->pcdata->suit[14] );

	fprintf( fp, "Torso        %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->pcdata->runes[TORSO][0],	ch->pcdata->runes[TORSO][1],
		ch->pcdata->runes[TORSO][2],	ch->pcdata->runes[TORSO][3],
		ch->pcdata->runes[TORSO][4],	ch->pcdata->runes[TORSO][5],
		ch->pcdata->runes[TORSO][6],	ch->pcdata->runes[TORSO][7],
		ch->pcdata->runes[TORSO][8],	ch->pcdata->runes[TORSO][9],
		ch->pcdata->runes[TORSO][10],	ch->pcdata->runes[TORSO][11],
		ch->pcdata->runes[TORSO][12],	ch->pcdata->runes[TORSO][13],
		ch->pcdata->runes[TORSO][14] );
	fprintf( fp, "Leftarm      %d %d %d %d %d\n",
		ch->pcdata->runes[LEFTARM][0],	ch->pcdata->runes[LEFTARM][1],
		ch->pcdata->runes[LEFTARM][2],	ch->pcdata->runes[LEFTARM][3],
		ch->pcdata->runes[LEFTARM][4] );
	fprintf( fp, "Rightarm     %d %d %d %d %d\n",
		ch->pcdata->runes[RIGHTARM][0],	ch->pcdata->runes[RIGHTARM][1],
		ch->pcdata->runes[RIGHTARM][2],	ch->pcdata->runes[RIGHTARM][3],
		ch->pcdata->runes[RIGHTARM][4] );
	fprintf( fp, "Leftleg      %d %d %d %d %d %d %d %d\n",
		ch->pcdata->runes[LEFTLEG][0],	ch->pcdata->runes[LEFTLEG][1],
		ch->pcdata->runes[LEFTLEG][2],	ch->pcdata->runes[LEFTLEG][3],
		ch->pcdata->runes[LEFTLEG][4],	ch->pcdata->runes[LEFTLEG][5],
		ch->pcdata->runes[LEFTLEG][6],	ch->pcdata->runes[LEFTLEG][7] );
	fprintf( fp, "Rightleg     %d %d %d %d %d %d %d %d\n",
		ch->pcdata->runes[RIGHTLEG][0],	ch->pcdata->runes[RIGHTLEG][1],
		ch->pcdata->runes[RIGHTLEG][2],	ch->pcdata->runes[RIGHTLEG][3],
		ch->pcdata->runes[RIGHTLEG][4],	ch->pcdata->runes[RIGHTLEG][5],
		ch->pcdata->runes[RIGHTLEG][6],	ch->pcdata->runes[RIGHTLEG][7] );

	fprintf( fp, "Ansi         %d\n", ch->pcdata->ansi );

	fprintf( fp, "Condition    %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2] );

	fprintf( fp, "Pagelen      %d\n",   ch->pcdata->pagelen     );

	/* save note boards, and number of boards in case that changes */
	fprintf( fp, "Boards       %d ", MAX_BOARD );
	for ( i = 0; i < MAX_BOARD; i++ )
	  fprintf( fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i] );
	fprintf( fp, "\n" );


	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Skill        %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	/* "slick" recursion to keep pkills in order */
	if ( ch->pcdata->pkills != NULL )
	  fwrite_pkill( ch->pcdata->pkills, fp );
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	fprintf( fp, "Affect %3d %3d %3d %3d %10d\n",
	    paf->type,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    /* if ( ch->level < obj->level */
    if ( obj->item_type == ITEM_KEY
    ||   obj->item_type == ITEM_POTION )
        return;

    if ( IS_OBJ_STAT(obj,ITEM_UNIQUE) && ch->level < LEVEL_HERO )
    {
      if ( obj->wear_loc != -1 )
        unequip_char( ch, obj );
      return;
    }

    fprintf( fp, "#OBJECT\n" );
    fprintf( fp, "Nest         %d\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    fprintf( fp, "Owner        %s~\n",	obj->owner		     );
    fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum	     );
    fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags	     );
    fprintf( fp, "WearFlags    %d\n",	obj->wear_flags		     );
    fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %d\n",	obj->weight		     );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %d\n",	obj->cost		     );
    fprintf( fp, "Values       %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	fprintf( fp, "Affect       %d %d %d %d %d\n",
	    paf->type,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    static PC_DATA pcdata_zero;
    char strsave[MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int i;

    if ( char_free == NULL )
    {
	ch				= alloc_perm( sizeof(*ch), PERM_CHAR );
    }
    else
    {
	ch				= char_free;
	char_free			= char_free->next;
	char_free_num--;
    }
    clear_char( ch );

    if ( pcdata_free == NULL )
    {
	ch->pcdata			= alloc_perm( sizeof(*ch->pcdata), PERM_PC );
    }
    else
    {
	ch->pcdata			= pcdata_free;
	pcdata_free			= pcdata_free->next;
	pcdata_free_num--;
    }
    *ch->pcdata				= pcdata_zero;

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->prompt                          = str_dup( "<%hhp %mm %vmv> " );
    ch->last_note                       = 0;
    ch->act				= PLR_BLANK
					| PLR_COMBINE
					| PLR_PROMPT;
    ch->totalexp			= 0;
    ch->pcdata->board			= &boards[DEFAULT_BOARD];
    ch->pcdata->actnew			= 0;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->target			= str_dup( "" );
    ch->pcdata->temp			= str_dup( "" );
    ch->pcdata->humiliate		= str_dup( "" );
    ch->pcdata->ignore			= str_dup( "" );
    ch->pcdata->created			= str_dup( "" );
    ch->pcdata->legend			= str_dup( "" );
    ch->pcdata->cflag			= str_dup( "" );
    ch->pcdata->host			= str_dup( d->host );
    ch->pcdata->body			= 10;
    ch->pcdata->mind			= 10; 
    ch->pcdata->spirit			= 10;
    ch->pcdata->will			= 10;
    ch->pcdata->primal			= 0;
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->pagelen                 = 20;
    ch->pcdata->security		= 0;	/* OLC */
    ch->pcdata->condition[COND_FULL]	= 48;
    for ( i = 0; i < MAX_EXTRAS; i++ )
	ch->pcdata->extras[i] = 0;
    for ( i = 0; i < MAX_EXTRAS; i++ )
        ch->pcdata->extras2[i] = 0;
    for ( i = 0; i < 10; i++ )
        ch->pcdata->clan[i] = 0;
    for ( i = 0; i < 10; i++ )
        ch->pcdata->suit[i] = 0;

    found = FALSE;
    fclose( fpReserve );

    /* parsed player file directories by Yaz of 4th Realm */
    /* decompress if .gz file exists - Thx Alander */
#if !defined(macintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    "/", capitalize( name ), ".gz" );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose( fp );
	sprintf( buf, "gzip -dfq %s", strsave );
	system( buf );
    }
#endif

#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    "/", capitalize( name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
#endif
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    PKILL_DATA *pkill;
    char *word;
    bool fMatch;
    int i = 0, pkinf, pkvnum, pktime, pklost;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_number( fp ) );
	    KEY( "Armor",	ch->armor,		fread_number( fp ) );
	    KEY( "Ansi",	ch->pcdata->ansi,	fread_number( fp ) );

	    if ( !str_cmp( word, "Alignment" ) )
	    {
	      fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }

	    if ( !str_cmp( word, "Affect" ) )
	    {
		AFFECT_DATA *paf;

		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf), PERM_AFF );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}

		paf->type	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'B':
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );

	    if ( !str_cmp( word, "Boards" ) )
	    { int num = fread_number( fp );  /* == number of boards saved */
	      char *boardname;
	      for ( ; num; num-- )
	      { boardname = fread_word( fp );
		i = board_lookup( boardname ); /* find number of board */
		if ( i == BOARD_NOTFOUND ) /* does board still exist? */
		{ sprintf( buf, "fread_char: %s has an unknown board name: %s. Skipped.", ch->name, boardname );
		  log_string( buf );
		  fread_number( fp );
		}
		else /* save it */
		  ch->pcdata->last_note[i] = fread_number( fp );
	      }

		fMatch = TRUE;
		break;
	    }

	    break;

	case 'C':
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cflag",	ch->pcdata->cflag,	fread_string( fp ) );
	    KEY( "Created",	ch->pcdata->created,	fread_string( fp ) );

            if ( !str_cmp( word, "Clan" ) )
            {
              for ( i = 0; i < 10; i++ )
                ch->pcdata->clan[i] = fread_number( fp );

              if ( clan_table[ch->pcdata->clan[0]].active == FALSE )
                for ( i = 0; i < 10; i++ )
                  ch->pcdata->clan[i] = 0;

              fMatch = TRUE;
              break;
            }

	    if ( !str_cmp( word, "Condition" ) )
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Deaf",	ch->deaf,		fread_number( fp ) );
	    KEY( "Description",	ch->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
	      /*
	       * Pfile has been loaded, any updates have taken place,
	       * now make sure they don't take place again and corrupt
	       * something.
	       */
	      ch->pcdata->version = PFILE_VERSION;
	      return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );

	    if ( !str_cmp( word, "Extras" ) )
	    { for ( i = 0; i < MAX_EXTRAS; i++ )
	      {
	        ch->pcdata->extras[i] = fread_number( fp );
	        if ( ch->pcdata->version < 1 && i < 4 )
	          ch->pcdata->kills[i] = ch->pcdata->extras[i];
	        if ( ch->pcdata->version < 2 && i < 4 )
	          ch->pcdata->extras[i] = 0;
	      }
	      fMatch = TRUE;
	      break;
	    }

	    if ( !str_cmp( word, "Extras2" ) )
	    { for ( i = 0; i < MAX_EXTRAS; i++ )
	        ch->pcdata->extras2[i] = fread_number( fp );

	      if ( ch->pcdata->version < 3 )
	        ch->pcdata->extras2[2] = 0;
	      fMatch = TRUE;
	      break;
	    }

	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
	    break;

	case 'H':
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Humiliate",	ch->pcdata->humiliate,	fread_string( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'I':
	    KEY( "Ignore",	ch->pcdata->ignore,	fread_string( fp ) );
	    break;

	case 'K':
	    if ( !str_cmp( word, "Kills" ) )
	    {
	      for ( i = 0; i < 15; i++ )
	        ch->pcdata->kills[i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }

	case 'L':
	    KEY( "Language",	ch->language,		fread_number( fp ) );
            KEY( "Legend",      ch->pcdata->legend,     fread_string( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );

	    if ( !str_cmp( word, "Leftarm" ) )
	    { for ( i = 0; i < 5; i++ )
		ch->pcdata->runes[LEFTARM][i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }
	    if ( !str_cmp( word, "Leftleg" ) )
	    { for ( i = 0; i < 8; i++ )
		ch->pcdata->runes[LEFTLEG][i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }

	    break;

	case 'M':
	    KEY( "Mname",	ch->pcdata->mname,	fread_string( fp ) );
	    KEY( "Mmaster",	ch->pcdata->mmaster,	fread_string( fp ) );

	    break;

	case 'N':
	    KEY( "Newact",	ch->pcdata->actnew,	fread_number( fp ) );
	    if ( !str_cmp( word, "Name" ) )
	    {
		/*
		 * Name already set externally.
		 */
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Newstats" ) )
	    {
	      ch->pcdata->body = fread_number( fp );
	      ch->pcdata->mind = fread_number( fp );
	      ch->pcdata->spirit = fread_number( fp );
	      ch->pcdata->will = fread_number( fp );
	      ch->pcdata->primal = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    } 
	    KEY( "Note",        ch->last_note,          fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Pagelen",     ch->pcdata->pagelen,    fread_number( fp ) );
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prompt",	ch->prompt,		fread_string( fp ) );

	   if ( !str_cmp( word, "Powers" ) )
	   {
	     for ( i = 0; i < 10; i++ )
	       ch->pcdata->powers[i] = fread_number( fp );
	     if ( ch->class == CLASS_FIST
	       && ch->pcdata->powers[F_LEARNED] >= 1020
	       && ch->pcdata->powers[F_LEARNED] <= 1025 )
	     {
	       ch->pcdata->powers[F_LEARNED] += 1980;
	     }
	     if ( ch->class == CLASS_FIST && ch->pcdata->powers[F_MASTER] < 234567 )
	     {
	       ch->pcdata->powers[F_MASTER] = 234567;
	     }

	     if ( IS_CLASS(ch,CLASS_SORCERER) && ch->pcdata->version < 3 )
	     {
	       ch->pcdata->powers[0] = 0;
	       ch->pcdata->powers[1] = 0;
	     }
	     fMatch = TRUE;
	     break;
	   }
	
	    if ( !str_cmp( word, "Pkill" ) )
	    {
		pkinf = fread_number( fp );
		pklost = fread_number( fp );
		pkvnum = fread_number( fp );
		pktime = fread_number( fp );

		/* pk records decay after a week */
		if ( pktime + 604800 < current_time || pktime < PKILL_WIPE )
		{
		  fread_to_eol( fp );
		  fMatch = TRUE;
		  break;
		}
		pkill = new_pkill();
		pkill->inf = pkinf;
		pkill->lost = pklost;
		pkill->vnum = pkvnum;
		pkill->time = pktime;
		pkill->humil = (bool) fread_number( fp );
		pkill->suit = (bool) fread_number( fp );
		pkill->victim = fread_string( fp );
		pkill->next = ch->pcdata->pkills;
		ch->pcdata->pkills = pkill;
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'R':
	    KEY( "Race",        ch->race,		fread_number( fp ) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_TEMPLE );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Rightarm" ) )
	    { for ( i = 0; i < 5; i++ )
		ch->pcdata->runes[RIGHTARM][i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }

	    if ( !str_cmp( word, "Rightleg" ) )
	    { for ( i = 0; i < 8; i++ )
		ch->pcdata->runes[RIGHTLEG][i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "Security",	ch->pcdata->security,	fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Suit" ) )
            {
              for ( i = 0; i < 15; i++ )
                ch->pcdata->suit[i] = fread_number( fp );

              /* gundams are wiped out on load */
              if ( ch->pcdata->suit[SUIT_NUMBER] >= 5 )
              {
                for ( i = 0; i < 10; i++ )
                  ch->pcdata->suit[i] = 0;
              }

              fMatch = TRUE;
              break;
            }

	    if ( !str_cmp( word, "Skill" ) )
	    {
		int sn;
		int value;

		value = fread_number( fp );
		sn    = skill_lookup( fread_word( fp ) );
		if ( sn < 0 )
		    bug( "Fread_char: unknown skill.", 0 );
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    if (!str_cmp( word, "Stances" ) )
	    {
		for ( i = 0; i <= MAX_STANCES; i++ )
		  ch->pcdata->stances[i] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Totalexp",	ch->totalexp,		fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" ) )
	    {
		ch->pcdata->title = fread_string( fp );
		/* removed , damnit
		if ( isalpha(ch->pcdata->title[0])
		||   isdigit(ch->pcdata->title[0]) )
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		} end removed */
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Torso" ) )
	    { for ( i = 0; i < 15; i++ )
	        ch->pcdata->runes[TORSO][i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }

	    break;

	case 'V':
	    KEY( "Version",	ch->pcdata->version,	fread_number(fp) );

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizbit",	ch->wizbit,		fread_number( fp ) );

	    if ( !str_cmp( word, "Weapons" ) )
            {
	      for ( i = 0; i < MAX_WEAPONS ; i++ )
		ch->pcdata->weapons[i] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }
		
	    break;
	}

	/* Make sure old chars have this field - Kahn */
	if ( !ch->pcdata->pagelen )
	    ch->pcdata->pagelen = 20;
	if ( !ch->prompt || ch->prompt == '\0' )
	    ch->prompt = str_dup( "<%h %m %v> " );
	if ( !ch->pcdata->bamfin || ch->pcdata->bamfin[0] == '\0' )
	  ch->pcdata->bamfin = str_dup( "has joined the chaos." );
	if ( !ch->pcdata->bamfout || ch->pcdata->bamfout[0] == '\0' )
	  ch->pcdata->bamfout = str_dup( "has left the building." );
	if ( !ch->pcdata->mname )
	  ch->pcdata->mname = str_dup( "none" );
	if ( !ch->pcdata->mmaster )
	  ch->pcdata->mmaster = str_dup( ch->name );
	if ( !ch->pcdata->ignore )
	  ch->pcdata->mmaster = str_dup( "" );
	if ( !ch->pcdata->cflag )
	  ch->pcdata->cflag = str_dup( "" );



	if ( !fMatch )
	{
	    bug( "Fread_char: no match.", 0 );
	    fread_to_eol( fp );
	}
    }

}



void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;

    if ( obj_free == NULL )
    {
	obj		= alloc_perm( sizeof(*obj), PERM_OTHER );
    }
    else
    {
	obj		= obj_free;
	obj_free	= obj_free->next;
	obj_free_num--;
    }

    *obj		= obj_zero;
    obj->name		= str_dup( "" );
    obj->short_descr	= str_dup( "" );
    obj->description	= str_dup( "" );
    obj->owner		= str_dup( "" );

    fNest		= FALSE;
    fVnum		= FALSE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Affect" ) )
	    {
		AFFECT_DATA *paf;

		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf), PERM_AFF );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}

		paf->type	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;

		if ( extra_descr_free == NULL )
		{
		    ed			= alloc_perm( sizeof(*ed), PERM_DESC );
		}
		else
		{
		    ed			= extra_descr_free;
		    extra_descr_free	= extra_descr_free->next;
		}

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fVnum )
		{
		    bug( "Fread_obj: incomplete object, no vnum.", 0 );
		    free_string( obj->name        );
		    free_string( obj->description );
		    free_string( obj->short_descr );
		    free_string( obj->owner	  );
		    obj->next = obj_free;
		    obj_free  = obj;
		    obj_free_num++;
		    return;
		}
                else if ( !fNest )
                {
                    bug( "Fread_obj: incomplete object, bad nest.", 0 );
                    free_string( obj->name        );
                    free_string( obj->description );
                    free_string( obj->short_descr );
                    free_string( obj->owner       );
                    obj->next = obj_free;
                    obj_free  = obj;
                    obj_free_num++;
                    return;
                 }
		else
		{
		    obj->next	= object_list;
		    object_list	= obj;
		    obj->pIndexData->count++;
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    break;


	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'O':
	    KEY( "Owner",	obj->owner,		fread_string( fp ) );
	    break;

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) )
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		{
		  obj->pIndexData = get_obj_index( OBJ_VNUM_DUMMY );
		  // bug( "Fread_obj: bad vnum %d.", vnum );
		}
		else
		  fVnum = TRUE;

		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

void fwrite_corpse( OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_corpse( obj->next_content, fp, iNest );

    /* Uniques do not save over copyovers, would dupe 'em */
    if ( IS_OBJ_STAT(obj,ITEM_UNIQUE) )
      return;

    fprintf( fp, "#OBJECT\n" );
    fprintf( fp, "Nest         %d\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    fprintf( fp, "Owner        %s~\n",	obj->owner		     );
    fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum	     );
    fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags	     );
    fprintf( fp, "WearFlags    %d\n",	obj->wear_flags		     );
    fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %d\n",	obj->weight		     );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %d\n",	obj->cost		     );
    fprintf( fp, "Values       %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );

    if ( iNest == 0 )
      fprintf( fp, "Room         %d\n",   obj->in_room->vnum           );
    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	fprintf( fp, "Affect       %d %d %d %d %d\n",
	    paf->type,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_corpse( obj->contains, fp, iNest + 1 );

    return;
}

void fread_corpse( FILE *fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *pRoomIndex;
    char *word;
    int iNest;
    int room = ROOM_VNUM_TEMPLE;
    bool fMatch;
    bool fNest;
    bool fVnum;

    if ( obj_free == NULL )
    {
	obj		= alloc_perm( sizeof(*obj), PERM_OTHER );
    }
    else
    {
	obj		= obj_free;
	obj_free	= obj_free->next;
	obj_free_num--;
    }

    *obj		= obj_zero;
    obj->name		= str_dup( "" );
    obj->short_descr	= str_dup( "" );
    obj->description	= str_dup( "" );
    obj->owner		= str_dup( "" );

    fNest		= FALSE;
    fVnum		= FALSE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Affect" ) )
	    {
		AFFECT_DATA *paf;

		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf), PERM_AFF );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}

		paf->type	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;

		if ( extra_descr_free == NULL )
		{
		    ed			= alloc_perm( sizeof(*ed), PERM_DESC );
		}
		else
		{
		    ed			= extra_descr_free;
		    extra_descr_free	= extra_descr_free->next;
		}

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    bug( "Fread_corpses: incomplete object.", 0 );
		    free_string( obj->name        );
		    free_string( obj->description );
		    free_string( obj->short_descr );
		    free_string( obj->owner	  );
		    obj->next = obj_free;
		    obj_free  = obj;
		    obj_free_num++;
		    return;
		}
		else
		{
		    obj->next	= object_list;
		    object_list	= obj;
		    obj->pIndexData->count++;

		    /* if room was removed in boot, send corpse to temple */
		    if ( ( pRoomIndex = get_room_index( room ) ) == NULL )
		      room = ROOM_VNUM_TEMPLE;

		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_room( obj, pRoomIndex );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_corpses: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'O':
	    KEY( "Owner",	obj->owner,		fread_string( fp ) );
	    break;

	case 'R':
	    KEY( "Room",	room,			fread_number( fp ) );
	    break;
	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_corpses: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_corpses: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) )
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_corpses: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

void save_copyover()
{ char copyfile[MAX_INPUT_LENGTH];
  FILE *fp;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  fclose( fpReserve );
  sprintf( copyfile, "%s", "../area/copyover.are" );

  if ( ( fp = fopen( copyfile, "w" ) ) == NULL )
  { bug( "Save_copyover: fopen", 0 );
    perror( copyfile );
  }

  for ( obj = object_list; obj != NULL; obj = obj_next )
  { obj_next = obj->next;
      /* potential dupe bug, better to risk corpses being destroyed */
      if ( obj->item_type == ITEM_CORPSE_PC && obj->contains != NULL && obj->in_obj == NULL)
        fwrite_corpse( obj, fp, 0 );
  }

  fprintf( fp, "#END\n" );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void load_copyover()
{ char copyfile[MAX_INPUT_LENGTH];
  FILE *fp;

  fclose( fpReserve );
  sprintf( copyfile, "%s", "../area/copyover.are" );

  if ( ( fp = fopen( copyfile, "r" ) ) == NULL )
  { bug( "Load_copyover: fopen", 0 );
    perror( copyfile );
  }

  for ( ; ; )
  { char letter;
    char *word;

    letter = fread_letter( fp );
    if ( letter == '*' )
    {
        fread_to_eol( fp );
        continue;
    }

    if ( letter != '#' )
    {
        bug( "Load_copyover: # not found.", 0 );
        break;
    }
  
    word = fread_word( fp );
    if ( !str_cmp( word, "OBJECT" ) ) fread_corpse( fp );
    else if ( !str_cmp( word, "END"    ) ) break;
    else
    {
        bug( "Load_copyover: bad section.", 0 );
        break;
    }
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void fwrite_pkill( PKILL_DATA *pkill, FILE *fp )
{
  if ( pkill->next != NULL )
    fwrite_pkill( pkill->next, fp );

  fprintf( fp, "Pkill        %d %d %d %d %d %d %s~\n",
	pkill->inf,     pkill->lost,    pkill->vnum,
	pkill->time,    pkill->humil,   pkill->suit,
	pkill->victim );

  return;
}
