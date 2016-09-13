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



/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#define DECLARE_CHANT_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_CHANT_FUN( fun )	CHANT_FUN fun
#endif



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short    int			sh_int;
typedef unsigned char			bool;
#endif




/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef	struct	chant_data		CHANT_DATA;
typedef	struct	newlock_data		NEWLOCK_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct  editing_data            EDITING_DATA;	/* OLC */
typedef	struct	inf_data		INF_DATA;
typedef	struct	pkill_data		PKILL_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  mob_prog_data           MPROG_DATA;         /* MOBprogram */
typedef struct  mob_prog_act_list       MPROG_ACT_LIST;     /* MOBprogram */
typedef struct  auction_data            AUCTION_DATA; /* auction data */
typedef	struct	vote_data		VOTE_DATA; /* vote info */
typedef	struct	challenge_data		CHALLENGE_DATA;	    /* Challenge info */


/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );
typedef void CHANT_FUN	args( ( int cn, int rank, CHAR_DATA *ch, void *vo ) );



/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 8192
#define MAX_INPUT_LENGTH	  320

/*
 * Current pfile version
 * 00: pfiles before 02/08/01
 * 01: pfiles with kills array.
 * 02: slots 0 and 1 in extras are now for flees
 * 03: removed prepping chants (0-1 in powers, 2 in extras2)
 */
#define	PFILE_VERSION		3


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_SKILL		  163
#define MAX_CHANT		   81
#define MAX_CLASS		    6
#define MAX_LEVEL		   20
#define LEVEL_HERO		   (MAX_LEVEL - 4)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 3)
#define L_IMM			   (MAX_LEVEL - 3)
#define MAX_WEAPONS		   13
#define MAX_STANCES		   10
#define MAX_RUNESPELLS		   32
#define	MAX_MATERIA		   16
#define MAX_EXTRAS		   10
#define	MAX_CLANS		    6
#define	MAX_SUITS		   15
#define	MAX_DAMAGE_TYPES           13
#define	MAX_FINGER_LINES	   12
#define MAX_LEADERS		   10
#define	MAX_LOGOUTS		   68
#define	MAX_WGEN		   35
#define	CHANT_DRAGON_SLAVE	   14
#define	L_HERO			LEVEL_HERO
#define	PKILL_WIPE	   1021756101

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (30 * PULSE_PER_SECOND)
#define PULSE_AREA		  (60 * PULSE_PER_SECOND)
#define PULSE_REGEN		  ( 5 * PULSE_PER_SECOND)
#define PULSE_AUCTION             (10 * PULSE_PER_SECOND) /* 10 seconds */
/* Save the database - OLC 1.1b */
#define PULSE_DB_DUMP		  (1800* PULSE_PER_SECOND ) /* 30 minutes  */

#define	SECONDS_PER_WEEK       604800

#include "board.h"


/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    char *	name;
};

struct newlock_data
{
    NEWLOCK_DATA *next;
    char *	name;
};

#define C_CLEAR        "\e[0m"    // no color
#define C_RED          "\e[0;31m" // dark colors
#define C_GREEN        "\e[0;32m"
#define C_YELLOW       "\e[0;33m"
#define C_BLUE         "\e[0;34m"
#define C_MAGENTA      "\e[0;35m"
#define C_CYAN         "\e[0;36m"
#define C_WHITE        "\e[0;37m" // i.e. grey
#define C_GREY         "\e[1;30m" // bright colors
#define C_B_RED        "\e[1;31m"
#define C_B_GREEN      "\e[1;32m"
#define C_B_YELLOW     "\e[1;33m"
#define C_B_BLUE       "\e[1;34m"
#define C_B_MAGENTA    "\e[1;35m"
#define C_B_CYAN       "\e[1;36m"
#define C_B_WHITE      "\e[1;37m"

#define CLASS_NONE		0
#define CLASS_SAIYAN		1
#define CLASS_PATRYN		2
#define CLASS_FIST		3
#define CLASS_SORCERER		4
#define	CLASS_MAZOKU		5


/* slots for the Kills array in pcdata */
#define	PK			0
#define	PD			1
#define	MK			2
#define MD			3
#define SECOND			4
#define FIRST			5
#define POWERFUL		6
#define MANIACAL		7
#define UNHOLY			8
#define	LASTKILL		9
#define LASTFAIR		10
#define	ESCAPECAPS		11

/* slots for the Extras array in pcdata */

#define	PUSSY			3
#define STATUS			4
#define LEGEND			5
#define HOME			6
#define TIMER			7
#define	TOKENS			8
#define	TIE			9

/* slots for Extras2 in pcdata */
#define	QUEST_TYPE		0
#define QUEST_INFO		1
#define	UNIQUE_TIMER		2
#define	EVAL			3
#define	PKCOUNT			4
#define	BOUNTY			5
#define	QUEST_COMPLETED		6
#define	SQUISH			7
#define	GLOBALS			8
#define	EXCLANS			9

/* quest values checked in quest code */
#define	QUEST_NONE		0
#define QUEST_MOB		1
#define QUEST_ITEM		2
#define	QUEST_FINISHED		3
#define	QUEST_FAILED		4


/* clan information, all integer */
#define	CLAN			0
#define	CLAN_RANK		1

/* clan diplomacy */
#define CLAN_ALLIED		0
#define	CLAN_AMITY		1
#define	CLAN_NEUTRAL		2
#define	CLAN_HOSTILE		3
#define	CLAN_WAR		4


#define	CLAN_TEMP		9

/* Define suit information, all ineger */
#define	SUIT_EQ			0
#define	SUIT_COND		1
#define SUIT_ARMOR		2
#define	SUIT_FUEL		3
#define	SUIT_READY		4
#define	SUIT_BULLETS		5
#define	SUIT_SHELLS		6
#define	SUIT_PLASMA		7
#define	SUIT_BEAM		8
#define	SUIT_NUMBER		9
#define	SUIT_WEAPON		10
#define	SUIT_COMBAT		11
#define	SUIT_REFLEX		12
#define	SUIT_AIM		13
#define	SUIT_MISSILES		14

/*
 * Bits for items mobile suits can ready
 */
#define	MS_BUCKLER		1
#define	MS_SHIELDS		2
#define	MS_MACHINE_GUN		4
#define	MS_BEAM_RIFLE		8
#define	MS_BEAM_CANNON		16
#define	MS_LASER_CANNON		32
#define	MS_HEAVY_BEAM_CANNON	64
#define	MS_VULCAN		128
#define	MS_MISSILES		256
#define	MS_BEAM_SABRE		512
#define	MS_BEAM_SWORD		1024
#define	MS_DOBERGUN		2048
#define	MS_HEATER		4096
#define	MS_CRASHER		8192
#define	MS_BUSTER_RIFLE		16384
#define	MS_TWIN_BUSTER_RIFLE	32768
#define	MS_BEAM_SCYTHE		65536
#define	MS_GATLING_GUN		131072
#define	MS_MINIGUNS		262144
#define	MS_HEAT_SHOTELS		524288
#define	MS_BEAM_GLAIVE		1048576
#define	MS_DRAGON_FANG		2097152
#define	MS_HEAT_ROD		4194304
#define	MS_CLOAKER		8388608
#define	MS_FLAMETHROWER		16777216


/*
 * Stances, normal and advanced; 5 each.
 */
#define STANCE_LION		1
#define STANCE_LYNX		2
#define STANCE_SNAKE		3
#define STANCE_BADGER		4
#define STANCE_FERRET		5
#define STANCE_HAWK		6
#define STANCE_EAGLE		7
#define STANCE_VULTURE		8
#define STANCE_SPARROW		9
#define STANCE_STORK	       10

/*
 * Saiyan Powers slots
 */
#define S_POWER			0
#define S_POWER_MAX		1
#define	S_STRENGTH		2
#define S_STRENGTH_MAX		3
#define S_SPEED			4
#define S_SPEED_MAX		5
#define S_AEGIS			6
#define S_AEGIS_MAX		7
#define S_TECH			8

#define S_KIBOLT		1
#define S_KIWAVE		2
#define S_KIBLAST		4
#define S_KIBOMB		8
#define S_KIMOVE		16
#define S_KISENSE		32
#define S_FLIGHT		64
#define S_KAMEHAMEHA		128
#define S_KIKOUHOU		256
#define S_MASENKOUHA		512
#define S_SOLARFIST		1024
#define S_HAWKEYES		2048
#define S_HASHYUKEN		4096
#define S_BATTLESENSE		8192
#define S_ZANZOUKEN		16384
#define S_KAIOUKEN		32768
#define S_KIWALL		65536
#define S_RYUKEN		131072
#define	S_KIAIHOU		262144


/*
 * Fist Powers slots and bits
 */
#define F_KI			0
#define F_KI_MAX		1
#define F_LEARNED		2
#define F_TORSO			3
#define F_ARMS			4
#define F_HANDS			5
#define F_LEGS			6
#define F_DISC			7
#define	F_MASTER		8

#define F_SHINKICK		1300
#define F_JAB			1301
#define F_SPINKICK		1302
#define F_KNEE			1303
#define F_ELBOW			1304
#define F_UPPERCUT		1305
#define F_STOMP			1306
#define F_JUMPKICK		1307
#define F_DEATHTOUCH		1308
#define	F_PALMTHRUST		1309

#define DAM_KIFLAME		1100

/*
 * Patryn bits and such
 */
#define	P_LEARNED		0
#define P_LEARNED_NUM		1
#define P_AIR			2
#define P_EARTH			3
#define P_FIRE			4
#define P_WATER			5
#define P_ENERGY		6
#define P_NEGATIVE		7
#define	P_BITS			8

#define	P_DEFENSES		1

#define TORSO			0
#define LEFTARM			1
#define RIGHTARM		2
#define LEFTLEG			3
#define RIGHTLEG		4

#define DAM_SHOCKSHIELD		1200
#define DAM_LIGHTNING		1201
#define DAM_EARTH		1202
#define DAM_FIRE		1203
#define DAM_WATER		1204
#define DAM_ENERGY		1205
#define DAM_NEGATIVE		1206
/*
 * Runes for Patryns
 */
#define RUNE_NONE		0
#define	RUNE_AIR		1
#define RUNE_EARTH		2
#define RUNE_FIRE		4
#define	RUNE_WATER		8
#define	RUNE_ENERGY		16
#define	RUNE_NEGATIVE		32
#define	RUNE_LIFE		64
#define	RUNE_DEATH		128
#define	RUNE_CREATION		256
#define	RUNE_DESTRUCTION	512
#define	RUNE_PROTECTION		1024
#define RUNE_TRANSFORMATION	2048
#define RUNE_MOVEMENT		4096
#define	RUNE_ALL		8192
#define	RUNE_ABJURATION		16384
/*
 * Bits for Sorcerers
 */
#define SORC_PREP	0
#define	SORC_MYSTIC	1
#define SCHOOL_BLACK	2
#define SCHOOL_EARTH	3
#define SCHOOL_WIND	4
#define SCHOOL_FIRE	5
#define SCHOOL_WATER	6
#define SCHOOL_ASTRAL	7
#define SCHOOL_WHITE	8
#define SORC_SPEC	9

#define DOLPH_ZORK	50
#define RUBYEYE_BLADE	51
#define	LAGUNA_BLADE	52
#define	DAM_FLAME_BREATH	1495


/* Bits for Mazoku */
#define M_LEARNED	0
#define	M_SET		1
#define	M_CTYPE		2
#define	M_CTIME		3
#define	M_EGO           4
#define	M_NIHILISM	5
#define	M_ESSENSE	6
#define	M_MATTER	7
#define	M_ASTRAL	8
#define	M_FOCUS		9

#define	M_HUMAN		1
#define	M_BATTLE	2
#define	M_TRUE		4
#define	M_HANDS		8
#define M_CLAWS		16
#define	M_SPIKES	32
#define	M_BLADES	64
#define	M_TENTACLES	128

#define	M_THIRD		512
#define	M_FOURTH	1024
#define	M_FIFTH		2048
#define	M_SIXTH		4096
#define	M_WINGS		8192
#define	M_EYES		16384
#define	M_TELEPORT	32768
#define	M_CHARGE	65536
#define	M_BLAST		131072
#define	M_BOLT		262144
#define	M_BOMB		524288
#define	M_ASTRIKE	1048576

#define	DAM_CERULEAN	1500
#define	DAM_OBSIDIAN	1501
#define	DAM_CRIMSON	1502
#define	DAM_EMERALD	1503



/* damage types for mobile suit attacks */
#define	DAM_BULLETS	1140
#define DAM_BEAMRIFLE	1141
#define	DAM_BEAMSABRE   1142
#define	DAM_BEAMSWORD	1143
#define	DAM_SHELLS	1144
#define	DAM_EXPLOSION	1145
#define	DAM_HEATROD     1146
#define	DAM_MISSILES	1147




/*
 * New bits for various PC states
 */

#define	NEW_KAME_1		2
#define	NEW_FLEEING		4
#define	NEW_FAIR_GAME		8
#define	NEW_CHAMPION		16
#define	NEW_VOTED		32
#define	NEW_KAME_2		64
#define	NEW_LEGEND		128
#define	NEW_INNERFIRE		256
#define NEW_FIGUREEIGHT		512
#define NEW_NOBLOCK		1024
#define NEW_CIRCLE		2048
#define	NEW_LAGUNABLADE		4096
#define	NEW_RECOVERY		8192
#define	NEW_CONCENTRATE		16384
#define	NEW_NEGATIVE_BLOCK	32768
#define	NEW_DRAGON_SLAVE	65536
#define	NEW_VAN_REHL		131072
#define	NEW_PHOENIX_AURA	262144
#define	NEW_FIRE_BLOCK		524288
#define	NEW_AIR_BLOCK		1048576
#define	NEW_KIAIHOU		2097152
#define	NEW_DUPING		4194304
#define	NEW_SUICIDE		8388608
#define	NEW_TARD		16777216
#define	NEW_BALL_LIGHTNING	33554432
#define	NEW_RETIRED		67108864
#define	NEW_MASENKOUHA		134217728


/*
 * damage bits for unis
 */
#define	DAM_SHOCKWAVE		3000

/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
};



/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_SEX			 6
#define CON_GET_NEW_CLASS		 7
#define CON_READ_MOTD			 8
#define CON_BREAK_CONNECT		 9
#define	CON_COPYOVER_RECOVER		10

/*
 * Connected states for OLC.
 */
#define CON_AEDITOR                     -1
#define CON_REDITOR                     -2
#define CON_MEDITOR                     -3
#define CON_OEDITOR                     -4

/*
 * Online editing structure.
 */
struct  editing_data	/* OLC */
{
    void *              pIndex;
    int                 vnum;
    int                 type;
    int                 command;
    char *              text;
    char **             destination;
};

/*
 * Area flags. OLC
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */
#define		AREA_VERBOSE	8       /* for OLC */


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    sh_int		descriptor;
    sh_int		connected;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *              showstr_head;
    char *              showstr_point;
    char *		outbuf;
    int			outsize;
    int			outtop;
    void *              pEdit;		/* OLC */
    char **             pString;	/* OLC */
    int			editor;		/* OLC */
    int			timer;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define	TO_ALL		    4



/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    AREA_DATA * area;
    sh_int	level;
    char *	keyword;
    char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    sh_int	keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */
struct	class_type
{
    char 	who_name	[9];	/* Three-letter name for 'who'	*/
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	weapon;			/* First weapon			*/
    sh_int	guild;			/* Vnum of guild room		*/
    sh_int	body;			/* max trainable body		*/
    sh_int 	mind;			/* max trainable mind		*/
    sh_int	spirit;			/* "   "         spirit		*/
    sh_int	will;			/* "   "	 willpower	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
};



/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t      date_stamp;
    time_t	expire;
};



/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    sh_int		type;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
};

/*
 * A record of damage inflicted
 */
struct	inf_data
{
  INF_DATA *	next;
  char *	victim;
  int		damage;
  time_t	time;
  bool		pc;
};

/*
 * A record of a pkill
 */
struct pkill_data
{
  PKILL_DATA *	next;
  char *	victim;
  int		inf;
  int		lost;
  int		vnum;
  int		time;
  bool		humil;
  bool		suit;
};

/*
 * A chant waiting to go off.
 */
struct	chant_data
{
  CHANT_DATA *	next;
  sh_int	cn;
  sh_int	wait;
  sh_int	rank;
  sh_int	line;
  char *	target;
};



struct  auction_data
{
    OBJ_DATA  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    int		min;    /* lowest bet allowed, set by seller */
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
};

struct	challenge_data
{
  bool active;
  CHAR_DATA *challenger;
  CHAR_DATA *opponent;
  int challenge_time;
  int fight_time;
};

struct	vote_data
{
  bool active;
  sh_int num;
  sh_int pulses;
  char *question;
  int votes[6];
  char *line1;
  char *line2;
  char *line3;
  char *line4;
  char *line5;
};




/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    sh_int		number;
    sh_int		killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAMPIRE	   3



/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		      1		/* Auto set for mobs	*/
#define ACT_SENTINEL		      2		/* Stays in one room	*/
#define ACT_SCAVENGER		      4		/* Picks up objects	*/
#define	ACT_FLAT_LEVEL		      8         /* Non-randomized level */
#define	ACT_AGG_ALL		     16         /* Attack PC's and NPCs */
#define ACT_AGGRESSIVE		     32		/* Attacks PC's		*/
#define ACT_STAY_AREA		     64		/* Won't leave area	*/
#define ACT_WIMPY		    128		/* Flees when hurt	*/
#define ACT_PET			    256		/* Auto set for pets	*/
#define ACT_TRAIN		    512		/* Can train PC's	*/
#define ACT_PRACTICE		   1024		/* Can practice PC's	*/
#define	ACT_VALHERU		   2048		/* Attacks non-clannies */
#define	ACT_CITHDEUX		   4096		/* Attacks non-clannies */
#define	ACT_SYNDICATE		   8192		/* Attacks non-clannies */
#define	ACT_BROTHERHOOD		  16384		/* Attacks non-clannies */
#define	ACT_EXODUS		  32768		/* Attacks non-clannies */
#define	ACT_MERCENARY		  65536		/* Attacks non-clannies */
#define	ACT_NO_RANGED		 131072		/* Block ranged attacks */
#define	ACT_FINISH		 262144		/* Cap morted players   */


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		      1
#define AFF_INVISIBLE		      2
#define AFF_DETECT_EVIL		      4
#define AFF_DETECT_INVIS	      8
#define AFF_DETECT_MAGIC	     16
#define AFF_DETECT_HIDDEN	     32
#define AFF_CHAOS_STRING	     64
#define AFF_SANCTUARY		    128
#define AFF_FAERIE_FIRE		    256
#define AFF_INFRARED		    512
#define AFF_CURSE		   1024
#define AFF_VISFARANK		   2048
#define AFF_POISON		   4096
#define AFF_PROTECT		   8192
#define AFF_VAS_GLUUDO		  16384
#define AFF_SNEAK		  32768
#define AFF_HIDE		  65536
#define AFF_SLEEP		 131072
#define AFF_CHARM		 262144
#define AFF_FLYING		 524288
#define AFF_PASS_DOOR		1048576
#define	AFF_NO_FLEE		2097152
#define	AFF_HOLY_RESIST		4194304
#define	AFF_NO_MOVE		8388608
#define	AFF_RAYWING	       16777216
#define	AFF_WINDY_SHIELD       33554432
#define	AFF_STEELY	       67108864
#define	AFF_HENSHIN           134217728




/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3
#define	OBJ_VNUM_GEN_WEAPON	      4

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_FINAL_TURD	     16

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22
#define	OBJ_VNUM_MATERIA	     27

#define OBJ_VNUM_SCHOOL_MACE	   3700
#define OBJ_VNUM_SCHOOL_DAGGER	   3701
#define OBJ_VNUM_SCHOOL_SWORD	   3702
#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER     3716

#define	OBJ_VNUM_PORTAL		     23
#define	OBJ_VNUM_GATE		     24
#define	OBJ_VNUM_MRISWITH            34

#define	OBJ_SUIT_LEO		     60
#define	OBJ_SUIT_ARIES		     61
#define	OBJ_SUIT_TAURUS		     62
#define	OBJ_SUIT_VIRGO		     63
#define	OBJ_SUIT_MERCURIUS	     64
#define	OBJ_SUIT_VAYEATE	     65

#define	OBJ_VNUM_ASTRAL_RING		200
#define	OBJ_VNUM_ASTRAL_AMULET		201
#define	OBJ_VNUM_ASTRAL_SHIRT		202
#define	OBJ_VNUM_ASTRAL_CAP		203
#define	OBJ_VNUM_ASTRAL_PANTS		204
#define	OBJ_VNUM_ASTRAL_BOOTS		205
#define	OBJ_VNUM_ASTRAL_GLOVES		206
#define	OBJ_VNUM_ASTRAL_SLEEVES		207
#define	OBJ_VNUM_ASTRAL_SHIELD		208
#define	OBJ_VNUM_ASTRAL_CLOAK		209
#define	OBJ_VNUM_ASTRAL_BELT		210
#define	OBJ_VNUM_ASTRAL_BRACER		211
#define	OBJ_VNUM_ASTRAL_SCEPTRE		212




/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define	ITEM_PORTAL		     27
#define	ITEM_GATE		     28
#define	ITEM_SUIT		     29
#define	ITEM_MUNITION		     30
#define	ITEM_ACCESSORY		     31
#define	ITEM_MATERIA		     32


/*
 * Weapon flags.
 * Added in game, goes in v0
 */
#define	WEAP_RUNE_AIR			1
#define	WEAP_RUNE_EARTH			2
#define	WEAP_RUNE_FIRE			4
#define	WEAP_RUNE_WATER			8
#define	WEAP_RUNE_ENERGY		16
#define	WEAP_RUNE_NEGATIVE		32

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		      1
#define ITEM_HUM		      2
#define ITEM_DARK		      4
#define ITEM_LOCK		      8
#define ITEM_EVIL		     16
#define ITEM_INVIS		     32
#define ITEM_MAGIC		     64
#define ITEM_NODROP		    128
#define ITEM_BLESS		    256
#define ITEM_ANTI_GOOD		    512
#define ITEM_ANTI_EVIL		   1024
#define ITEM_ANTI_NEUTRAL	   2048
#define ITEM_NOREMOVE		   4096
#define ITEM_INVENTORY		   8192
#define	ITEM_NO_LOCATE		  16384
#define	ITEM_UNIQUE		  32768
#define	ITEM_ASTRAL		  65536
#define	ITEM_NOLOOT		 131072
#define	ITEM_HARDENED		 262144


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		      1
#define ITEM_WEAR_FINGER	      2
#define ITEM_WEAR_NECK		      4
#define ITEM_WEAR_BODY		      8
#define ITEM_WEAR_HEAD		     16
#define ITEM_WEAR_LEGS		     32
#define ITEM_WEAR_FEET		     64
#define ITEM_WEAR_HANDS		    128 
#define ITEM_WEAR_ARMS		    256
#define ITEM_WEAR_SHIELD	    512
#define ITEM_WEAR_ABOUT		   1024 
#define ITEM_WEAR_WAIST		   2048
#define ITEM_WEAR_WRIST		   4096
#define ITEM_WIELD		   8192
#define ITEM_HOLD		  16384



/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24



/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_TEMPLE	   3001
#define ROOM_VNUM_ALTAR		   3054
#define ROOM_VNUM_SCHOOL	   3700
#define	ROOM_VNUM_LOUNGE	     97



/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		      1
#define ROOM_NO_MOB		      4
#define ROOM_INDOORS		      8
#define ROOM_PRIVATE		    512
#define ROOM_SAFE		   1024
#define ROOM_SOLITARY		   2048
#define ROOM_PET_SHOP		   4096
#define ROOM_NO_RECALL		   8192
#define	ROOM_HQ			  32768
#define	ROOM_ARENA		  65536
#define	ROOM_HANGER		 131072
#define	ROOM_REGEN		 262144


/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5
#define MAX_DIR			      6	/* OLC */



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      1
#define EX_CLOSED		      2
#define EX_LOCKED		      4
#define EX_PICKPROOF		     32



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_MAX		     11



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define MAX_WEAR		     18



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2



/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      1		/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		      2
#define	PLR_NOFOLLOW                  4
#define PLR_AUTOEXIT		      8
#define PLR_AUTOLOOT		     16
#define PLR_AUTOSAC                  32
#define PLR_BLANK		     64
#define PLR_BRIEF		    128
#define	PLR_FIREBACK		    256
#define PLR_COMBINE		    512
#define PLR_PROMPT		   1024
#define PLR_TELNET_GA		   2048
#define PLR_HOLYLIGHT		   4096
#define PLR_WIZINVIS		   8192
#define	PLR_IMMTALK		  16384
#define	PLR_SILENCE		  32768
#define PLR_NO_EMOTE		  65536
#define	PLR_NODESC		 131072
#define PLR_NO_TELL		 262144
#define PLR_LOG			 524288
#define PLR_DENY		1048576
#define PLR_FREEZE		2097152
#define	PLR_NO_NOTE		4194304
#define PLR_KILLER		8388608
#define PLR_TRUESIGHT	       16777216


/*
 * Obsolete bits.
 */
#if 0
#define PLR_AUCTION		      4	/* Obsolete	*/
#define PLR_CHAT		    256	/* Obsolete	*/
#define PLR_NO_SHOUT		 131072	/* Obsolete	*/
#endif



/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		      1
#define	CHANNEL_CHAT		      2
#define	CHANNEL_WIZNET		      4
#define	CHANNEL_IMMTALK		      8
#define	CHANNEL_MUSIC		     16
#define	CHANNEL_QUESTION	     32
#define	CHANNEL_SHOUT		     64
#define	CHANNEL_YELL		    128
#define	CHANNEL_INFO		    256
#define	CHANNEL_FATALITY	    512
#define	CHANNEL_TELL		   1024


/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    char *              spec_name;	/* OLC */
    SHOP_DATA *		pShop;
    AREA_DATA *		area;		/* OLC */
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    sh_int		vnum;
    sh_int		count;
    sh_int		killed;
    sh_int		sex;
    sh_int		level;
    int			act;
    int			affected_by;
    sh_int		clan;
    sh_int		hitroll;		/* Unused */
    sh_int		ac;			/* Unused */
    int			hitnodice;		/* Unused */
    int			hitsizedice;		/* Unused */
    sh_int		hitplus;		/* Unused */
    sh_int		damnodice;		/* Unused */
    sh_int		damsizedice;		/* Unused */
    sh_int		damplus;		/* Unused */
    int			gold;			/* Unused */
    MPROG_DATA *        mobprogs;               /* Used by MOBprogram */
    int                 progtypes;              /* Used by MOBprogram */
};



/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		reply;
    SPEC_FUN *		spec_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    OBJ_DATA *		carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    PC_DATA *		pcdata;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *              prompt;
    sh_int		sex;
    sh_int		class;
    sh_int		race;
    sh_int		level;
    sh_int		trust;
    bool                wizbit;
    int			played;
    time_t		logon;
    time_t		save_time;
    time_t              last_note;
    sh_int		timer;
    sh_int		wait;
    int			hit;
    int			max_hit;
    int			mana;
    int			max_mana;
    int			move;
    int			max_move;
    int			gold;
    int			exp;
    int			act;
    int			affected_by;
    sh_int		position;
    sh_int		practice;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor;
    sh_int		wimpy;
    sh_int		deaf;
    MPROG_ACT_LIST *    mpact;                  /* Used by MOBprogram */
    int                 mpactnum;               /* Used by MOBprogram */
    int			totalexp;
    sh_int		language;
};



/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *		target;
    sh_int		condition	[3];
    sh_int              pagelen;
    sh_int		learned		[MAX_SKILL];
    sh_int		weapons		[MAX_WEAPONS];
    int			body;
    int			mind;
    int			spirit;
    int			will;
    sh_int		ansi;
    sh_int		stances		[MAX_STANCES+2];
    int			powers		[10];
    int			actnew;
    int			primal;
    sh_int		runes		[5][15];
    BOARD_DATA *	board;
    time_t		last_note	[MAX_BOARD];
    NOTE_DATA *		in_progress;
    int			extras		[MAX_EXTRAS];
    char *		humiliate;
    char *		mname;
    char *		mmaster;
    char *		temp;
    char *		created;
    int			extras2		[MAX_EXTRAS];
    sh_int		clan		[10];
    int			suit		[15];
    char *		ignore;
    int			kills		[15];
    int			version;
    char *		legend;
    int 		security;	/* OLC */ /* Builder security */
    CHANT_DATA *	chant;
    char *		cflag;
    char *		host;
    INF_DATA *		inf;		/* damage inflicted list */
    PKILL_DATA *	pkills;		/* list of kills */
};

/*
 * MOBprogram block
*/

struct  mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *           buf;
    CHAR_DATA *      ch;
    OBJ_DATA *       obj;
    void *           vo;
};

struct  mob_prog_data
{
    MPROG_DATA *next;
    int         type;
    char *      arglist;
    char *      comlist;
};

bool    MOBtrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024


/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		16

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *		area;	/* OLC */
    char *		name;
    char *		short_descr;
    char *		description;
    sh_int		vnum;
    sh_int		item_type;
    int			extra_flags;
    sh_int		wear_flags;
    sh_int		count;
    sh_int		weight;
    int			cost;			/* Unused */
    int			value	[4];
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		owner;
    sh_int		item_type;
    int			extra_flags;
    sh_int		wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		level;
    sh_int		timer;
    int			value	[4];
    int			extra	[10];
};



/*
 * Exit data.
 */
struct	exit_data
{
    ROOM_INDEX_DATA *	to_room;
    EXIT_DATA	*	next;
    sh_int		vnum;
    sh_int		exit_info;
    sh_int		key;
    char *		keyword;
    char *		description;
    int			rs_flags;	/* OLC */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
};


struct	area_data
{
    AREA_DATA *		next;
    char *		name;
    int                 recall;
    int 		age;
    int 		nplayer;
    char *		filename;	/* OLC */
    char *		builders;	/* OLC - Listing of builders */
    int			security;	/* OLC - Value 0-infinity  */
    int			lvnum;		/* OLC - Lower vnum */
    int			uvnum;		/* OLC - Upper vnum */
    int			vnum;		/* OLC - Area vnum  */
    int			area_flags;	/* OLC */
};



/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[6];
    char *		name;
    char *		description;
    sh_int		vnum;
    int			room_flags;
    sh_int		light;
    sh_int		sector_type;
    sh_int		clan;
    RESET_DATA *	reset_first;	/* OLC */
    RESET_DATA *	reset_last;	/* OLC */
};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED          -1
#define TYPE_HIT                1000
#define TYPE_LSLASH		1025
#define	TYPE_USTROKE		1026

/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define	TAR_CHAR_GLOBAL		    5



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Waiting time after use	*/
    bool	room;			/* Wear off message to room?	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
};

struct chant_type
{
  char *	name;
  CHANT_FUN *	cast;
  sh_int	school;
  bool		prep;
  sh_int	rank;
  sh_int	cost;
  sh_int	lines;
  sh_int	lag;
  sh_int	target;
  char *	line1;
  char *	line2;
  char * 	line3;
  char *	line4;
  char *	line5;
  sh_int	wait;
  sh_int	placeholder;
};

struct lltype
{
  char *	name;
  char *	clan;
  int		pkills;
  int		pdeaths;
  int		assessment;
  time_t	lastkill;
};

struct suit_type
{
  char *	name;
  int		eq;
  int		standard;
  int		requisite;
  int		fight;
  int		weapon;
  int		speed;
  int		power;
  int		armor;
};


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	sh_int	gsn_backstab;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_awareness;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;
extern	sh_int	gsn_whirl;

extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;
extern	sh_int	gsn_infrared;

extern	sh_int	gsn_kiwall;
extern	sh_int	gsn_kaiouken;

extern	sh_int	gsn_wind_ward;
extern	sh_int	gsn_earth_ward;
extern	sh_int	gsn_flame_ward;
extern	sh_int	gsn_water_ward;
extern	sh_int	gsn_spirit_ward;
extern	sh_int	gsn_negative_ward;
extern	sh_int	gsn_wind_curse;
extern	sh_int	gsn_earth_curse;
extern	sh_int	gsn_flame_curse;
extern	sh_int	gsn_water_curse;

extern	sh_int	gsn_golden_blaze;
extern	sh_int	gsn_judicators_ire;
extern	sh_int	gsn_dark_blaze;

extern	sh_int	gsn_mos_varim;
extern	sh_int	gsn_balus_wall;
extern	sh_int	gsn_defense;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))	/* OLC */
#define STR(dat, field)         (( (dat)->field != NULL                    \
                                             ? (dat)->field                \
                                             : (dat)->pIndexData->field ))



/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define CH(descriptor)		((descriptor)->original ? \
				(descriptor)->original : (descriptor)->character)
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))


#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define PC(ch,field)            ((ch)->pcdata->field)	/* OLC */
#define GET_PC(ch,field,nopc)   ((ch->pcdata != NULL ? ch->pcdata->field    \
                                                     : (nopc)) )	/* OLC */
#define	IS_TIED(ch)		(!IS_NPC(ch) && ch->pcdata->extras[TIE]>0)
#define GET_AC(ch)		((ch)->armor				    \
				    + ( IS_AWAKE(ch)			    \
				    ? 0		    \
				    : 0 ))
#define GET_HITROLL(ch)		((ch)->hitroll+ch->level)
#define GET_DAMROLL(ch)		((ch)->damroll+ch->level)

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))

#define MANA_COST(ch, sn)       (IS_NPC(ch) ? 0 : skill_table[sn].min_mana)

#define IS_CLASS(ch,cl)		(!IS_NPC(ch) && ch->class == cl)
#define IS_WIELDING(ch,weap)	(get_eq_char(ch,WEAR_WIELD) != NULL &&      \
				(get_eq_char(ch,WEAR_WIELD))->pIndexData->  \
				 vnum == weap )

#define	IS_SUIT(ch)		(!IS_NPC(ch) && 			    \
				 ch->pcdata->suit[SUIT_NUMBER] > 0 )

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_SUMMONED( obj )      (obj->pIndexData->vnum == 50 ||             \
				 obj->pIndexData->vnum == 51 ||             \
				 obj->pIndexData->vnum == 52)
#define IS_OWNED( obj )		( str_cmp( obj->owned, "none" ) &&          \
				  str_cmp( obj->owned, "" ) )


/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )



/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
};



/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char * const	name;
    char * const	char_no_arg;
    char * const	others_no_arg;
    char * const	char_found;
    char * const	others_found;
    char * const	vict_found;
    char * const	char_auto;
    char * const	others_auto;
};


/*
 * Structure for stance types and affects.
 */
struct stance_type
{
  char * name;
  int attacks;
  int redux;
  sh_int damage[MAX_STANCES+1];
  sh_int dodge[MAX_STANCES+1];
  sh_int parry[MAX_STANCES+1];
};

/*
 * Structure for materia table.
 */
struct materia_type
{
  int type;
  char *name;
  int maxap;
  char *spell[4];
  int ap[4];
  SPELL_FUN * cast[4];
};

/*
 * Structure for generated weapon descriptions.
 */
struct wgen_type
{
  int type;
  char *name;
};

  

/* 
 * Structure for clan information
 */
struct	clan_type
{
  char * name;
  char * title;
  int gold;
  int pkills;
  int pdeaths;
  int mobkills;
  int mobdeaths;
  int kills[MAX_CLANS+1];
  int diplomacy[MAX_CLANS+1];
  bool active;
  char rank[6][30];
};


/*
 * Global constants.
 */

extern	const	struct	class_type	class_table	[MAX_CLASS];
extern	const	struct	cmd_type	cmd_table	[];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	const	struct	skill_type	skill_table	[MAX_SKILL];
extern	const	struct	chant_type	chant_table	[MAX_CHANT];
extern	const	struct	social_type	social_table	[];
extern	const	struct	stance_type	stance_table	[MAX_STANCES+1];
extern	const	struct	suit_type	suit_table	[MAX_SUITS+1];
extern	const	struct	materia_type	materia_table	[MAX_MATERIA+1];
extern	const	struct	wgen_type	wgen_table	[MAX_WGEN];



/*
 * Global variables.
 */

extern		BAN_DATA	  *	ban_list;
extern		NEWLOCK_DATA	  *     newlock_list;
extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		NOTE_DATA	  *	note_list;
extern		OBJ_DATA	  *	object_list;

extern		AFFECT_DATA	  *	affect_free;
extern		BAN_DATA	  *	ban_free;
extern		CHANT_DATA	  *	chant_free;
extern		NEWLOCK_DATA	  *     newlock_free;
extern		CHAR_DATA	  *	char_free;
extern		DESCRIPTOR_DATA	  *	descriptor_free;
extern		EXTRA_DESCR_DATA  *	extra_descr_free;
extern		INF_DATA	  *	inf_free;
extern		PKILL_DATA	  * 	pkill_free;
extern		NOTE_DATA	  *	note_free;
extern		OBJ_DATA	  *	obj_free;
extern		PC_DATA		  *	pcdata_free;

extern		int			char_free_num;
extern		int			pcdata_free_num;
extern		int			obj_free_num;

extern		int			perm_list	[10];
extern		int			act_prog_num;

extern		HELP_DATA 	  *	help_first;


#define	PERM_OLC	0
#define	PERM_ROOM	1
#define	PERM_DESC	2
#define	PERM_CHAR	3
#define	PERM_PC		4
#define	PERM_NOTE	5
#define	PERM_AFF	6
#define	PERM_INDEX	7
#define	PERM_ALLOC	8
#define	PERM_OTHER	9

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		struct  lltype		leaderboard	[MAX_LEADERS+5];
extern		struct  lltype		loserboard	[10];
extern		struct	clan_type	clan_table	[MAX_CLANS+1];
extern          AUCTION_DATA      *     auction;
extern		VOTE_DATA	  *	voting;
extern		const	char	*	dip_table	[5];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN(	do_accept	);
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN( do_ansi		);
DECLARE_DO_FUN(	do_answer	);
DECLARE_DO_FUN(	do_apurge	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN(	do_astrike	);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_auto         );
DECLARE_DO_FUN( do_autoexit     );
DECLARE_DO_FUN( do_autoloot     );
DECLARE_DO_FUN( do_autosac      );
DECLARE_DO_FUN( do_autostance	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN(	do_banis	);
DECLARE_DO_FUN(	do_banish	);
DECLARE_DO_FUN( do_bandage	);
DECLARE_DO_FUN(	do_battlesense	);
DECLARE_DO_FUN(	do_bigbang	);
DECLARE_DO_FUN(	do_bits		);
DECLARE_DO_FUN( do_blank        );
DECLARE_DO_FUN( do_bodytrain	);
DECLARE_DO_FUN(	do_bounty	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief        );
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buggedmob	);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_cflag	);
DECLARE_DO_FUN(	do_challenge	);
DECLARE_DO_FUN(	do_changeclass	);
DECLARE_DO_FUN(	do_channels	);
DECLARE_DO_FUN( do_chant	);
DECLARE_DO_FUN(	do_charge	);
DECLARE_DO_FUN(	do_chat		);
DECLARE_DO_FUN( do_circle	);
DECLARE_DO_FUN(	do_claim	);
DECLARE_DO_FUN(	do_induct	);
DECLARE_DO_FUN(	do_clanbalance	);
DECLARE_DO_FUN(	do_clanboards	);
DECLARE_DO_FUN(	do_clandeposit	);
DECLARE_DO_FUN(	do_clanedit	);
DECLARE_DO_FUN(	do_clanrank	);
DECLARE_DO_FUN(	do_clantalk	);
DECLARE_DO_FUN( do_clearstats	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_combo	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_concentrate	);
DECLARE_DO_FUN(	do_config	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_contracts	);
DECLARE_DO_FUN(	do_copyover	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN(	do_cset		);
DECLARE_DO_FUN(	do_dare		);
DECLARE_DO_FUN(	do_defenses	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN(	do_develop	);
DECLARE_DO_FUN( do_dice		);
DECLARE_DO_FUN(	do_dim_mak	);
DECLARE_DO_FUN(	do_diplomacy	);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN( do_discipline	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN( do_elbow	);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_enter	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN( do_erase	);
DECLARE_DO_FUN( do_escape	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_extraset	);
DECLARE_DO_FUN( do_eyesight	);
DECLARE_DO_FUN( do_fatality	);
DECLARE_DO_FUN(	do_fightlist	);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_finger	);
DECLARE_DO_FUN( do_finish	);
DECLARE_DO_FUN(	do_fire		);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_flex		);
DECLARE_DO_FUN( do_flight	);
DECLARE_DO_FUN(	do_fly		);
DECLARE_DO_FUN( do_focus	);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_form		);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN( do_gift		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN(	do_gouge	);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_grow		);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_hawkeyes	);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN( do_home		);
DECLARE_DO_FUN(	do_humiliate	);
DECLARE_DO_FUN(	do_humiliation	);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN(	do_imbue	);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_info		);
DECLARE_DO_FUN( do_innerfire	);
DECLARE_DO_FUN(	do_install	);
DECLARE_DO_FUN(	do_instantiate	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN( do_jab		);
DECLARE_DO_FUN( do_junk		);
DECLARE_DO_FUN( do_kaiouken	);
DECLARE_DO_FUN( do_kamehameha	);
DECLARE_DO_FUN(	do_karma	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN( do_kiblast	);
DECLARE_DO_FUN( do_kibolt	);
DECLARE_DO_FUN( do_kibomb	);
DECLARE_DO_FUN( do_kikouhou	);
DECLARE_DO_FUN( do_kisense	);
DECLARE_DO_FUN( do_kiwall	);
DECLARE_DO_FUN( do_kiwave	);
DECLARE_DO_FUN( do_knee		);
DECLARE_DO_FUN(	do_lamerboards	);
DECLARE_DO_FUN(	do_lash		);
DECLARE_DO_FUN( do_leaderboards	);
DECLARE_DO_FUN( do_learn	);
DECLARE_DO_FUN(	do_leave	);
DECLARE_DO_FUN( do_legend	);
DECLARE_DO_FUN( do_lenter	);
DECLARE_DO_FUN( do_level	);
DECLARE_DO_FUN( do_levitate	);
DECLARE_DO_FUN( do_lexit	);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN(	do_llbackup	);
DECLARE_DO_FUN(	do_llsave	);
DECLARE_DO_FUN(	do_llsort	);
DECLARE_DO_FUN(	do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_loot		);
DECLARE_DO_FUN( do_loserboards	);
DECLARE_DO_FUN( do_maccept	);
DECLARE_DO_FUN( do_masenkouha	);
DECLARE_DO_FUN( do_master	);
DECLARE_DO_FUN( do_mbanish	);
DECLARE_DO_FUN(	do_mburst	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mbanish	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_minionset	);
DECLARE_DO_FUN( do_mjoin	);
DECLARE_DO_FUN(	do_mlist	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN( do_mname	);
DECLARE_DO_FUN(	do_morph	);
DECLARE_DO_FUN(	do_mount	);
DECLARE_DO_FUN( do_mpareadam	);
DECLARE_DO_FUN( do_mpasound     );
DECLARE_DO_FUN( do_mpat         );
DECLARE_DO_FUN(	do_mpdamage	);
DECLARE_DO_FUN( do_mpecho       );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat     );
DECLARE_DO_FUN( do_mpforce      );
DECLARE_DO_FUN( do_mpgoto       );
DECLARE_DO_FUN( do_mpjunk       );
DECLARE_DO_FUN( do_mpkill       );
DECLARE_DO_FUN(	do_mplist	);
DECLARE_DO_FUN( do_mpmload      );
DECLARE_DO_FUN( do_mpoload      );
DECLARE_DO_FUN( do_mppurge      );
DECLARE_DO_FUN(	do_mpretarget	);
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_mptransfer   );
DECLARE_DO_FUN(	do_mptransall	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN( do_mtalk	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN(	do_music	);
DECLARE_DO_FUN(	do_newlock	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN(	do_nonote	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN(	do_owhere	);
DECLARE_DO_FUN( do_pagelen      );
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN(	do_phoenixaura	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN(	do_pkstat	);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN( do_powerset	);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN(	do_prepare	);
DECLARE_DO_FUN(	do_preserve	);
DECLARE_DO_FUN( do_prompt       );
DECLARE_DO_FUN(	do_punch	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_quest	);
DECLARE_DO_FUN(	do_question	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_rage		);
DECLARE_DO_FUN(	do_rake		);
DECLARE_DO_FUN(	do_ready	);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_reclaim	);
DECLARE_DO_FUN(	do_redit	);
DECLARE_DO_FUN(	do_reduction	);
DECLARE_DO_FUN(	do_reform	);
DECLARE_DO_FUN(	do_release	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN(	do_repair	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN( do_research	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_retir	);
DECLARE_DO_FUN(	do_retire	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN( do_roundhouse	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN(	do_rub		);
DECLARE_DO_FUN( do_runestats	);
DECLARE_DO_FUN( do_runetrain	);
DECLARE_DO_FUN( do_runeweave	);
DECLARE_DO_FUN( do_ryuken	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_salvo	);
DECLARE_DO_FUN(	do_sap		);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_sblast	);
DECLARE_DO_FUN(	do_sbolt	);
DECLARE_DO_FUN(	do_sbomb	);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN(	do_send		);
DECLARE_DO_FUN(	do_shell	);
DECLARE_DO_FUN( do_shinkick	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_shunkanidou	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_silence	);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_slash	);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN( do_slist        );
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_smother	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN(	do_socials	);
DECLARE_DO_FUN( do_solarflare	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_specialize	);
DECLARE_DO_FUN( do_spells       );
DECLARE_DO_FUN( do_spend	);
DECLARE_DO_FUN( do_spinkick	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_squish	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN( do_stance	);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN(	do_status	);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN(	do_suicide	);
DECLARE_DO_FUN(	do_swave	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tard		);
DECLARE_DO_FUN(	do_retarget	);
DECLARE_DO_FUN( do_tattoo	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN( do_technique	);
DECLARE_DO_FUN(	do_teleport	);
DECLARE_DO_FUN(	do_tie		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN( do_timer	);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_token	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_uniques	);
DECLARE_DO_FUN(	do_untie	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN( do_uppercut	);
DECLARE_DO_FUN(	do_users	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN(	do_vote		);
DECLARE_DO_FUN(	do_voteset	);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN( do_whirl	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_withdraw	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizify       );
DECLARE_DO_FUN( do_wizlist      );
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN(	do_wreck	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);



/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_protection	);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);

/*
 * Spells used through materia.
 */
DECLARE_SPELL_FUN(      spell_fire		);
DECLARE_SPELL_FUN(      spell_fire2		);
DECLARE_SPELL_FUN(      spell_fire3		);
DECLARE_SPELL_FUN(      spell_ice		);
DECLARE_SPELL_FUN(      spell_ice2		);
DECLARE_SPELL_FUN(      spell_ice3		);
DECLARE_SPELL_FUN(      spell_bolt		);
DECLARE_SPELL_FUN(      spell_bolt2		);
DECLARE_SPELL_FUN(      spell_bolt3		);
DECLARE_SPELL_FUN(      spell_cure		);
DECLARE_SPELL_FUN(      spell_cure2		);
DECLARE_SPELL_FUN(      spell_regen		);
DECLARE_SPELL_FUN(      spell_cure3		);
DECLARE_SPELL_FUN(      spell_quake		);
DECLARE_SPELL_FUN(      spell_quake2		);
DECLARE_SPELL_FUN(      spell_quake3		);
DECLARE_SPELL_FUN(      spell_bio		);
DECLARE_SPELL_FUN(      spell_bio2		);
DECLARE_SPELL_FUN(      spell_bio3		);
DECLARE_SPELL_FUN(      spell_poisona		);
DECLARE_SPELL_FUN(      spell_esuna		);
DECLARE_SPELL_FUN(      spell_resist		);
DECLARE_SPELL_FUN(      spell_life		);
DECLARE_SPELL_FUN(      spell_life2		);
DECLARE_SPELL_FUN(      spell_barrier		);
DECLARE_SPELL_FUN(      spell_shell		);
DECLARE_SPELL_FUN(      spell_reflect		);
DECLARE_SPELL_FUN(      spell_wall		);
DECLARE_SPELL_FUN(      spell_sleepel		);
DECLARE_SPELL_FUN(      spell_silence		);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(      spell_slow		);
DECLARE_SPELL_FUN(      spell_stop		);
DECLARE_SPELL_FUN(      spell_demi		);
DECLARE_SPELL_FUN(      spell_demi2		);
DECLARE_SPELL_FUN(      spell_demi3		);
DECLARE_SPELL_FUN(      spell_debarrier		);
DECLARE_SPELL_FUN(      spell_despell		);
DECLARE_SPELL_FUN(      spell_death		);
DECLARE_SPELL_FUN(      spell_comet		);
DECLARE_SPELL_FUN(      spell_comet2		);
DECLARE_SPELL_FUN(      spell_freeze		);
DECLARE_SPELL_FUN(      spell_break		);
DECLARE_SPELL_FUN(      spell_tornado		);
DECLARE_SPELL_FUN(      spell_flare		);
DECLARE_SPELL_FUN(      spell_ultima		);



/*
 * Chant declarations, defined in sorcerer.c
 */
DECLARE_CHANT_FUN(	chant_balus_rod		);
DECLARE_CHANT_FUN(	chant_disfang		);
DECLARE_CHANT_FUN(	chant_dolph_zork	);
DECLARE_CHANT_FUN(	chant_dynast_brass	);
DECLARE_CHANT_FUN(	chant_blast_ash		);
DECLARE_CHANT_FUN(	chant_ferrous_bleed	);
DECLARE_CHANT_FUN(	chant_hell_blast	);
DECLARE_CHANT_FUN(	chant_zelas_brid	);
DECLARE_CHANT_FUN(	chant_dolph_strash	);
DECLARE_CHANT_FUN(	chant_ruby_eye_blade	);
DECLARE_CHANT_FUN(	chant_gaav_flare	);
DECLARE_CHANT_FUN(	chant_dynast_breath	);
DECLARE_CHANT_FUN(	chant_laguna_blast	);
DECLARE_CHANT_FUN(	chant_dragon_slave	);
DECLARE_CHANT_FUN(	chant_laguna_blade	);

DECLARE_CHANT_FUN(	chant_lighting		);
DECLARE_CHANT_FUN(	chant_dicleary		);
DECLARE_CHANT_FUN(	chant_holy_bless	);
DECLARE_CHANT_FUN(	chant_flow_break	);
DECLARE_CHANT_FUN(	chant_chaos_strings	);
DECLARE_CHANT_FUN(	chant_recovery		);
DECLARE_CHANT_FUN(	chant_sleeping		);
DECLARE_CHANT_FUN(	chant_mos_varim		);
DECLARE_CHANT_FUN(	chant_visfarank		);
DECLARE_CHANT_FUN(	chant_vas_gluudo	);
DECLARE_CHANT_FUN(	chant_laphas_seed	);
DECLARE_CHANT_FUN(	chant_holy_resist	);
DECLARE_CHANT_FUN(	chant_defense		);
DECLARE_CHANT_FUN(	chant_megido_flare	);
DECLARE_CHANT_FUN(	chant_ressurection	);

DECLARE_CHANT_FUN(	chant_levitation	);
DECLARE_CHANT_FUN(	chant_scatter_brid	);
DECLARE_CHANT_FUN(	chant_digger_bolt	);
DECLARE_CHANT_FUN(	chant_diem_wing		);
DECLARE_CHANT_FUN(	chant_damu_brass	);
DECLARE_CHANT_FUN(	chant_raywing		);
DECLARE_CHANT_FUN(	chant_mono_volt		);
DECLARE_CHANT_FUN(	chant_dimilar_ai	);
DECLARE_CHANT_FUN(	chant_windy_shield	);
DECLARE_CHANT_FUN(	chant_bomb_di_wind	);

DECLARE_CHANT_FUN(	chant_gray_bomb		);
DECLARE_CHANT_FUN(	chant_earth_5		);
DECLARE_CHANT_FUN(	chant_dill_brand	);
DECLARE_CHANT_FUN(	chant_dug_break		);
DECLARE_CHANT_FUN(	chant_gozu_vro		);
DECLARE_CHANT_FUN(	chant_bephis_bring	);
DECLARE_CHANT_FUN(	chant_earth_30		);
DECLARE_CHANT_FUN(	chant_mega_brand	);
DECLARE_CHANT_FUN(	chant_dug_haut		);
DECLARE_CHANT_FUN(	chant_vlave_howl	);

DECLARE_CHANT_FUN(	chant_flare_bit		);
DECLARE_CHANT_FUN(	chant_flare_arrow	);
DECLARE_CHANT_FUN(	chant_val_flare		);
DECLARE_CHANT_FUN(	chant_burst_rondo	);
DECLARE_CHANT_FUN(	chant_fireball		);
DECLARE_CHANT_FUN(	chant_flare_lance	);
DECLARE_CHANT_FUN(	chant_rune_flare	);
DECLARE_CHANT_FUN(	chant_balus_wall	);
DECLARE_CHANT_FUN(	chant_burst_flare	);
DECLARE_CHANT_FUN(	chant_blast_bomb	);

DECLARE_CHANT_FUN(	chant_aqua_create	);
DECLARE_CHANT_FUN(	chant_freeze_arrow	);
DECLARE_CHANT_FUN(	chant_dark_mist		);
DECLARE_CHANT_FUN(	chant_ly_briem		);
DECLARE_CHANT_FUN(	chant_freeze_brid	);
DECLARE_CHANT_FUN(	chant_icicle_lance	);
DECLARE_CHANT_FUN(	chant_sea_cucumber_spin	);
DECLARE_CHANT_FUN(	chant_vice_freeze	);
DECLARE_CHANT_FUN(	chant_demona_crystal	);
DECLARE_CHANT_FUN(	chant_van_rehl		);

DECLARE_CHANT_FUN(	chant_bram_blazer	);
DECLARE_CHANT_FUN(	chant_elmekia_lance	);
DECLARE_CHANT_FUN(	chant_assha_dist	);
DECLARE_CHANT_FUN(	chant_astral_detect	);
DECLARE_CHANT_FUN(	chant_astral_break	);
DECLARE_CHANT_FUN(	chant_shadow_snap	);
DECLARE_CHANT_FUN(	chant_elmekia_flame	);
DECLARE_CHANT_FUN(	chant_astral_vine	);
DECLARE_CHANT_FUN(	chant_vision		);
DECLARE_CHANT_FUN(	chant_ra_tilt		);









/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
//char *	crypt		args( ( const char *key, const char *salt ) );
#define crypt(s1,s2) (s1)
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""		/* Player files			*/
#define NULL_FILE	"proto.are"	/* To reserve one stream	*/
#define MOB_DIR         ""              /* MOBProg files                */
#endif

#if defined(MSDOS)
#define PLAYER_DIR	""		/* Player files                 */
#define NULL_FILE	"nul"		/* To reserve one stream	*/
#define MOB_DIR         ""              /* MOBProg files                */
#endif

#if defined(unix)
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define	PLAYER_TEMP_DIR	"../player/temp" /* temps of player files	*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define MOB_DIR         "MOBProgs/"     /* MOBProg files                */
#endif

#if defined(linux)
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define MOB_DIR         "MOBProgs/"     /* MOBProg files                */
#endif

#define AREA_LIST	"area.lst"	/* List of areas		*/

#define BUG_FILE	"bugs.txt"      /* For 'bug' and bug( )		*/
#define IDEA_FILE	"ideas.txt"	/* For 'idea'			*/
#define TYPO_FILE	"typos.txt"     /* For 'typo'			*/
#define NOTE_FILE	"notes.txt"	/* For 'notes'			*/
#define SHUTDOWN_FILE	"shutdown.txt"	/* For 'shutdown'		*/
#define	COPYOVER_FILE	"copyover.txt"	/* For 'copyover'		*/
#define	EXE_FILE	"../src/chaosium"	/* For 'copyover'		*/




/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN

/* act_comm.c */
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
bool	is_note_to	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	talk_auction	args( (char *argument) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
void	init_descriptor	args( ( DESCRIPTOR_DATA *dnew, int desc ) );
void	stc		args( ( const char *txt, CHAR_DATA *ch ) );
void	wiznet		args( ( char *argument ) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
int	assess		args( ( CHAR_DATA *ch ) );

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door ) );
void	unstance	args( ( CHAR_DATA *ch ) );
void	update_llboards	args( ( CHAR_DATA *ch, bool fair ) );
void	sort_llboards	args( ( void ) );

/* act_obj.c */
void	rand_obj	args( ( CHAR_DATA *ch ) );

/* act_wiz.c */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
void	copyover	args( ( void ) );



/* comm.c */
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_all_char	args( ( const char *txt ) );
void    show_string     args( ( DESCRIPTOR_DATA *d, char *input ) );
void	act	        args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	colorize	args( ( char *newbuf, const char *buf, bool color ) );

/* db.c */
void	boot_db		args( ( bool fCopyOver ) );
void	area_update	args( ( bool startup ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
RID *	get_rand_room	args( ( void ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
char *	fread_worda	args( ( FILE *fp ) );
void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem, int requested) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
INF_DATA * new_inf	args( ( void ) );
PKILL_DATA * new_pkill	args( ( void ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	args( ( const char *str ) );
void	tail_chain	args( ( void ) );
int	isquare		args( ( int num ) );
void	memory_check	args( ( void ) );
int	has_uniques	args( ( CHAR_DATA *ch ) );

/* fight.c */
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	raw_kill	args( ( CHAR_DATA *victim, bool pdox ) );
int	stanced		args( ( CHAR_DATA *ch ) );
void	stancecheck	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	chant_damage	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	add_inf		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );
void	clear_inf	args( ( CHAR_DATA *ch ) );
void	inf_update	args( ( void ) );
int	get_inf_dam	args( ( CHAR_DATA *ch, char *name ) );

/* handler.c */
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_age		args( ( CHAR_DATA *ch ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
bool	is_full_name	args( ( const char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_same_clan	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD * 	get_full_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int amount ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
/*char *	item_type_name	args( ( OBJ_DATA *obj ) ); OLC */
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( int vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char *	expand_bits	args( ( int bits ) );
void	eval		args( ( CHAR_DATA *ch ) );
int	parsebet	args( ( const int currentbet, const char *argument ) );
int	advatoi		args( ( const char *s ) );
void	obj_affect_remove args( ( OBJ_DATA *obj, AFFECT_DATA *af ) );
bool	timer_check	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dec_duration	args( ( CHAR_DATA *ch, sh_int gsn, int dur ) );
CD *	get_admin_char	args( ( void ) );

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
bool    IS_SWITCHED     args( ( CHAR_DATA *ch ) );

/* magic.c */
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );

/* mob_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args( (const char *s1, const char *s2 ) );
#endif

void    mprog_wordlist_check    args( ( char * arg, CHAR_DATA *mob,
                CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type ) );
void    mprog_percent_check     args( ( CHAR_DATA *mob,
                CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type ) );
void    mprog_act_trigger       args( ( char* buf, CHAR_DATA* mob,
                        CHAR_DATA* ch, OBJ_DATA* obj, void* vo ) );
void    mprog_bribe_trigger     args( ( CHAR_DATA* mob, CHAR_DATA* ch,
                        int amount ) );
void    mprog_entry_trigger     args( ( CHAR_DATA* mob ) );
void    mprog_give_trigger      args( ( CHAR_DATA* mob, CHAR_DATA* ch,
                        OBJ_DATA* obj ) );
void    mprog_greet_trigger     args( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args( ( CHAR_DATA* mob ) );
void    mprog_random_trigger    args( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args( ( char* txt, CHAR_DATA* mob ) );

/* mob_commands.c */
char *  mprog_type_to_name      args( ( int type ) );

/* patryn.c */
int	get_runes	args( ( CHAR_DATA *ch, int type, int part ) );

/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );
void	save_char_fin	args( ( CHAR_DATA *ch ) );
void	save_copyover	args( ( void ) );
void	load_copyover	args( ( void ) );
char	*initial	args( ( const char *str ) );

/* sorcerer.c */
void	chant_cast	args( ( CHAR_DATA *ch, CHANT_DATA *cha ) );
void	lose_chant	args( ( CHAR_DATA *ch ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );

/* suit.c */
bool	is_cloaked	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
/* update.c */
void	advance_level	args( ( CHAR_DATA *ch ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
void	save_llboards	args( ( bool flip ) );
int	isquare		args( ( int num ) );

/* string.c */
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );	/* OLC */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );	/* OLC */
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );	/* OLC */
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );	/* OLC */
char *	string_replace	args( ( char * orig, char * old, char * new ) );	/* OLC */
char *	string_proper	args( ( char * argument ) );	/* OLC */
char *	string_unpad	args( ( char * argument ) );	/* OLC */
int	arg_count	args( ( char *argument ) );	/* OLC */
char *	all_capitalize	args( ( const char *argument ) );	/* OLC */

/* olc.c */
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    redit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    medit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void	add_reset	args( ( ROOM_INDEX_DATA *area, RESET_DATA *pReset, int index ) );

/* bit.c */
char *	room_bit_name		args( ( int room_flags ) );	/* OLC */
int	room_name_bit		args( ( char *buf ) );		/* OLC */
int	sector_number		args( ( char *argument ) );	/* OLC */
char *	sector_name		args( ( int sect ) );		/* OLC */
int	item_name_type		args( ( char *name ) ); 	/* OLC */
char *	item_type_name		args( ( int item_type ) );	/* OLC */
int	extra_name_bit		args( ( char* buf ) );		/* OLC */
char *	extra_bit_name		args( ( int extra_flags ) ); 	/* OLC */
int	wear_name_bit		args( ( char* buf ) );		/* OLC */
char *	wear_bit_name		args( ( int wear ) );		/* OLC */
int	act_name_bit		args( ( char* buf ) );		/* OLC */
int	affect_name_bit		args( ( char* buf ) );		/* OLC */
int	affect_name_loc		args( ( char* name ) );		/* OLC */
int     wear_name_loc   	args( ( char *buf ) );		/* OLC */
char *	wear_loc_name		args( ( int wearloc ) );	/* OLC */
char *	act_bit_name		args( ( int act ) );		/* OLC */
int	get_weapon_type		args( ( char *arg ) );		/* OLC */
char *	get_type_weapon		args( ( int arg ) );		/* OLC */
int	get_container_flags	args( ( char *arg ) );		/* OLC */
char *	get_flags_container	args( ( int arg ) );		/* OLC */
int	get_liquid_type		args( ( char *arg ) );		/* OLC */
char *	get_type_liquid		args( ( int arg ) ) ;		/* OLC */

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type
{
    char *	spec_name;
    SPEC_FUN *	spec_fun;
};



/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
    char * name;
    int  bit;
    bool settable;
};



/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY	1



/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */



#define MAX_DIR	6
#define NO_FLAG -99	/* Must not be used in flags or stats. */



/*
 * Interp.c
 */
DECLARE_DO_FUN( do_aedit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_redit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_oedit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_medit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_resets	);


/*
 * Global Constants
 */
extern	char *	const	dir_name        [];
extern	const	int	rev_dir         [];
extern	const	struct	spec_type	spec_table	[];



/*
 * Global variables
 */
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern  	SHOP_DATA *             shop_last;

extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;

extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];



/* db.c */
void	reset_area      args( ( AREA_DATA * pArea, bool start ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom, bool start ) );

/* string.c */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *	string_replace	args( ( char * orig, char * old, char * new ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *	string_unpad	args( ( char * argument ) );
char *	string_proper	args( ( char * argument ) );

/* olc.c */
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );

/* special.c */
char *	spec_string	args( ( SPEC_FUN *fun ) );	/* OLC */

/* bit.c */
extern const struct flag_type 	area_flags[];
extern const struct flag_type	sex_flags[];
extern const struct flag_type	exit_flags[];
extern const struct flag_type	door_resets[];
extern const struct flag_type	room_flags[];
extern const struct flag_type	sector_flags[];
extern const struct flag_type	type_flags[];
extern const struct flag_type	extra_flags[];
extern const struct flag_type	wear_flags[];
extern const struct flag_type	act_flags[];
extern const struct flag_type	affect_flags[];
extern const struct flag_type	apply_flags[];
extern const struct flag_type	wear_loc_strings[];
extern const struct flag_type	wear_loc_flags[];
extern const struct flag_type	weapon_flags[];
extern const struct flag_type	container_flags[];
extern const struct flag_type	liquid_flags[];


#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
