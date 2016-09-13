ACKNOWLEDGMENTS:

The outlook of this note board system is partly based on the one of Mystic
Adventure (mud.gel.ulaval.ca 4000). Many of the bugs fixed from version 1 to
2 were found with the help of players of Our Place (ourplace.org 6543).

See the file License for important licensing information.


FILE LIST

readme.txt	(This file) - the small changes here and there

changes.txt Changes since version 1.

board.c		Main board file. Remember to include in Makefile.

board.h		Include file. Insert #include "board.h" in merc.h, at about
			line 150 (before site ban structure)

help.txt	A help entry for HELP NOTE.


New in version 3
----------------

INSTALL.ROM       Further installation instructions for ROM by
                  yago@cerberus.uab.es.

patch_rom24_board  A patch also by Yago



PATCHING INSTRUCTIONS:

If there is + before a line, it means that only that line of code should
be inserted  - the lines without + around it are just for reference. !
denotes lines that were partially changed. This is partially modified
output of `rcsdiff -bBp *.[ch]`

Other things you will need to do:

- Remove the old HELP file entry NOTE, and insert the new
- Define NOTE_DIR to some directory that fits you (in board.h) *AND*
  remember to create that directory.
- Add the BOARD command to your cmdtable in interp.c.

Note that it is only the BOARD and NOTE commands that are used. Though the
do_nwrite etc. look like commands, they are just local functions.


ABBREVIATIONS:

is_full_name() - If your MUD uses abbreviations (easily achieved by changing
str_cmp into str_prefix in is_name) - you will need to create such a
function that is basically is_name, but without the abbreviating ability.
Simply copy MERC's or Envy's is_name() which does not use str_prefix but
str_cmp.

If you already are on a MERC/Envy MUD, without abbreviations, simply:

#define is_full_name is_name

FUTURE:

Threads. Replying to a note. Quoting, using an external editor (will have to
modify tintin to do that). Changing/appending to a note. Online note board
creation. Usenet-like board hierarchy. SMTP/NNTP interface (yup,
alt.rec.games.mud.diku.your-mud.announce). Boards shared with other MUDs.


PATCHES:


act_comm.c	1996/01/01 15:02:57
===================================

do_note, is_note_to etc.: Remove them all.

Free note in progress when quitting.

*************** void do_quit( CHAR_DATA *ch, char *argum
+ 
+ 	/* Free note that might be there somehow */        
+     if (ch->pcdata->in_progress)
+     	free_note (ch->pcdata->in_progress);
          
      
      d = ch->desc;


comm.c	1996/01/01 19:18:22
===========================


Change close_socket() so notewriters are not thrown completely out.
  
*************** void close_socket( DESCRIPTOR_DATA *dclo
      {
  	sprintf( log_buf, "Closing link to %s.", ch->name );
  	log_string( log_buf );
! 
! 	/* If ch is writing note or playing, just lose link otherwise clear char */
! 	if ( (dclose->connected == CON_PLAYING) || 
! 	  ((dclose->connected >= CON_NOTE_TO) && 
! 	   (dclose->connected <= CON_NOTE_FINISH)))
  	{
  	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
  	    ch->desc = NULL;

*************** void nanny( DESCRIPTOR_DATA *d, char *ar

      CHAR_DATA *ch;
! /*    NOTE_DATA *pnote; */ /* Remove note variable */
      char *pwdnew;
      char *p;
      int iClass;
      int lines;
! /*    int notes; */ /* Remove this one too */
      bool fOld;

+ 	/* Delete leading spaces UNLESS character is writing a note */
+ 	if (d->connected != CON_NOTE_TEXT)
+ 	{
  	    while ( isspace(*argument) )
  		argument++;
+ 	}
  
      ch = d->character;


A bit further down: Completely remove the current checking for note and add
this instead. 
  
*************** void nanny( DESCRIPTOR_DATA *d, char *ar

--- 1902,2190 ----
  	}
  
  	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
  	do_look( ch, "auto" );
+ 	do_board (ch, ""); /* Show board status */


Add this after the last break after CON_READ_MOTD:

	break; /* this is the break from CON_READ_MOTD */


	/* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
	/* ch MUST be PC here; have nwrite check for PC status! */
		
	case CON_NOTE_TO:
		handle_con_note_to (d, argument);
		break;
		
	case CON_NOTE_SUBJECT:
		handle_con_note_subject (d, argument);
		break; /* subject */
	
	case CON_NOTE_EXPIRE:
		handle_con_note_expire (d, argument);
		break;

	case CON_NOTE_TEXT:
		handle_con_note_text (d, argument);
		break;
		
	case CON_NOTE_FINISH:
		handle_con_note_finish (d, argument);
		break;

	} /* This is the closing brace for the big switch in nanny()

	return;
} /* this is the end of nanny()




Add information about note in progress if a character should lose link.
  
*************** bool check_reconnect( DESCRIPTOR_DATA *d
  		log_string( log_buf );
  		d->connected = CON_PLAYING;
  
+ 		/* Inform the character of a note in progress and the possbility of continuation! */		
+ 		if (ch->pcdata->in_progress)
+ 			send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r",ch);
+ 
  		/*
  		 * Contributed by Gene Choi
  		 */

In act(), do not send any acts to characters that are anything but playing.

*************** void act( const char *format, CHAR_DATA 

OLD:

  	*point++ = '\n';
  	*point++ = '\r';
  	buf[0]   = UPPER(buf[0]);
! 	if (to->desc)
  	  write_to_buffer( to->desc, buf, point - buf );
      }

NEW:

  	*point++ = '\n';
  	*point++ = '\r';
  	buf[0]   = UPPER(buf[0]);
! 	if (to->desc && (to->desc->connected == CON_PLAYING))
  	  write_to_buffer( to->desc, buf, point - buf );
      }


db.c	1995/12/30 21:49:16
===========================

At the top of the file: Remove note_list. Remove load_notes().

  char                    log_buf         [2*MAX_INPUT_LENGTH];
  KILL_DATA               kill_table      [MAX_LEVEL];
! /* NOTE_DATA *             note_list; */
  OBJ_DATA *              object_list;


Add loading of notes at startup.
The save_notes() updates the note boards with purged messages.

*************** void boot_db( void )
          fix_exits( );
          fBootDb = FALSE;
          area_update(TRUE);
! /*        load_notes( ); */
!  	  load_boards(); /* Load all boards */
!     save_boards(); 
          load_war();
          load_disabled();
      }

merc.h	1996/01/01 14:23:50
===================================

Remove pnote from char_data:

*************** struct  char_data
      MOB_INDEX_DATA *    pIndexData;
      DESCRIPTOR_DATA *   desc;
      AFFECT_DATA *       affected;
-     NOTE_DATA *         pnote;
      OBJ_DATA *          carrying;
      ROOM_INDEX_DATA *   in_room;
      ROOM_INDEX_DATA *   was_in_room;

Add these fields to pc_data: (remember to replace the OLD time_t last_note
with the array below)

*************** struct  pc_data
  	
+  	BOARD_DATA *			board; /* The current board */
+  	time_t				last_note[MAX_BOARD]; /* last note for the boards */
+ 	NOTE_DATA *			in_progress;

Add a time_t expire field to the note_data:

*************** struct  note_data
{
    NOTE_DATA * next;
    char *      sender;
    char *      date;
    char *      to_list;
    char *      subject;
    char *      text;
    time_t      date_stamp;
+   time_t      expire;
};



save.c	1996/01/01 14:50:32
===========================


When saving, add saving of board status.

First, add this variable:

*************** void save_char_obj( CHAR_DATA *ch )

  void fwrite_char( CHAR_DATA *ch, FILE *fp )
  {
      AFFECT_DATA *paf;
!     int sn,i;
  
      fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"		);


Then add saving of board status:
  
*************** void fwrite_char( CHAR_DATA *ch, FILE *f
  
  	fprintf( fp, "Pagelen      %d\n",   ch->pcdata->pagelen     );
  	
+ 	/* Save note board status */
+ 	/* Save number of boards in case that number changes */
+ 	fprintf (fp, "Boards       %d ", MAX_BOARD);
+ 	for (i = 0; i < MAX_BOARD; i++)
+ 		fprintf (fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
+ 	fprintf (fp, "\n");
+ 	
+ 		
  	for ( sn = 0; sn < MAX_SKILL; sn++ )
  	{
  	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )


When laoding a character, add setting of default board.

*************** bool load_char_obj( DESCRIPTOR_DATA *d, 

      ch->act				= PLR_BLANK
  					| PLR_COMBINE
  					| PLR_PROMPT;
+ 
+ 	/* every characters starts at default board from login.. this board
+ 	   should be read_level == 0 !
+ 	*/   
+ 	ch->pcdata->board		= &boards[DEFAULT_BOARD];
+ 				
      ch->pcdata->pwd			= str_dup( "" );
      ch->pcdata->bamfin			= str_dup( "" );
      ch->pcdata->bamfout			= str_dup( "" );


Finally, loading of board data.

*************** void fread_char( CHAR_DATA *ch, FILE *fp
  	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
  	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
  	    KEY( "Battles", ch->pcdata->battles, fread_number( fp ));

+ 
+ 		/* Read in board status */	    
+ 	    if (!str_cmp(word, "Boards" ))
+ 	    {
+ 			int i,num = fread_number (fp); /* number of boards saved */
+ 			char *boardname;
+ 			
+ 			for (; num ; num-- ) /* for each of the board saved */
+ 			{
+ 				boardname = fread_word (fp);
+ 				i = board_lookup (boardname); /* find board number */
+ 				
+ 				if (i == BOARD_NOTFOUND) /* Does board still exist ? */
+ 				{
+ 					sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);					
+ 					log_string (buf);
+ 					fread_number (fp); /* read last_note and skip info */
+ 				}
+ 				else /* Save it */
+ 					ch->pcdata->last_note[i] = fread_number (fp);
+ 			}	 /* for */
+ 			
+ 			fMatch = TRUE;
+ 	    } /* Boards */
+ 	    
  	    break;
  
  	case 'C':


Other places
============


You will need to change the places where something can be sent to a
character using send_to_char to check if the character is writing a note.
For example, when the "You are hungry" message is sent. It would look
something like:


if (ch->desc && ch->desc->connected == CON_PLAYING)
	send_to_char ("You are hungry.\n\r",ch);

The MUD first checks if the character has a descriptor at all, and then if
the character is playing.

Also, if you are using Envy or so, consider adding a call to do_afk() when
character starts writing a note:

	do_afk (ch,"");

This will make tells to that character get a message that the character is
AFK, instead of just them disappearing. You could, of course, increase the
checks in do_tell and do_reply so they check for chararcters writing a note.



