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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"



/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if	defined(linux)
//int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
//int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
//int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
//int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
//int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * This includes Solaris SYSV as well
 */

#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int     setsockopt      args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;	 	/* Game is newbielocked         */
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* Time of this pulse		*/



/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;
    int port;
    extern int control;


    /* God damn the mud is fucked up */
    log_string( "Got into main." );

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    log_string( "Time has been initialized." );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    log_string( "fpReserve opened." );

    /*
     * Get the port number.
     */
    port = 1234;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }

	if (argv[2] && argv[2][0])
	{
		fCopyOver = TRUE;
 		control = atoi(argv[3]);
 	}
 	else
 		fCopyOver = FALSE;

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
    if ( !fCopyOver )
      control = init_socket( port );
    log_string( "About to boot_db()." );
    boot_db( fCopyOver );
    sprintf( log_buf, "Merc is ready to rock on port %d.", port );
    log_string( log_buf );
    game_loop_unix( control );
    close( control );
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1; 
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close( fd );
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror( "Init_socket: bind" );
	close( fd );
	exit( 1 );
    }

    if ( listen( fd, 3 ) < 0 )
    {
	perror( "Init_socket: listen" );
	close( fd );
	exit( 1 );
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_GET_NAME;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->timer = 0;
		d->fcommand	= TRUE;
		stop_idling( d->character );

		if ( d->connected == CON_PLAYING )
		    if ( d->showstr_point )
		        show_string( d, d->incomm );
		    else
		        interpret( d->character, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL )
			save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character )
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		/* OLC */
		if ( d->showstr_point )
		    show_string( d, d->incomm );
		else if ( d->pString )
		    string_add( d->character, d->incomm );
		else if ( d->connected == CON_PLAYING )
		{
		    if ( !run_olc_editor( d ) )
			interpret( d->character, d->incomm );
		}
		else
		{
	 	    nanny( d, d->incomm );
		}
		d->incomm[0]	= '\0';

		/* removed for OLC
		if ( d->connected == CON_PLAYING )
		{
		    if ( d->showstr_point )
		        show_string( d, d->incomm );
		    else
		        interpret( d->character, d->incomm );
		}
		else
		    nanny( d, d->incomm );

		d->incomm[0]	= '\0';
		*/
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void new_descriptor( int control )
{
    // static DESCRIPTOR_DATA d_zero;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    // struct hostent *from;
    int desc;
    int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( descriptor_free == NULL )
    {
	dnew		= alloc_perm( sizeof(*dnew), PERM_DESC );
    }
    else
    {
	dnew		= descriptor_free;
	descriptor_free	= descriptor_free->next;
    }

    /* removed for hotboot
    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->showstr_head  = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->outbuf	= alloc_mem( dnew->outsize );
    */

    init_descriptor( dnew, desc );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
	wiznet( log_buf );
	/* seems to be causing problems.. will try again */
	/*
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf ); */
	dnew->host = str_dup( buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	/* what the fuck is up with this?
	if ( !str_suffix( pban->name, dnew->host ) )
	*/
	if ( !str_prefix( pban->name, dnew->host ) )
	{
	    write_to_descriptor( desc,
		"Your site has been banned from this Mud.\n\r", 0 );
	    close( desc );
	    free_string( dnew->host );
	    free_mem( dnew->outbuf, dnew->outsize );
	    dnew->next		= descriptor_free;
	    descriptor_free	= dnew;
	    return;
	}
    }


    /* wrong place 
    for ( plock = newlock_list; plock != NULL; plock = plock->next )
    {
      if ( !str_prefix( plock->name, dnew->host ) )
      {
        write_to_descriptor( desc,
          "Due to the large number of fuckheads using your ISP,\n\rusers may not create new characters.  To create a new character,\n\r email Alathon at chaosium@voyager.inetsolve.com with the desired\n\rusername and class.\n\r", 0 );
        close( desc );
        free_string( dnew->host );
        free_mem( dnew->outbuf, dnew->outsize );
        dnew->next = descriptor_free;
        descriptor_free = dnew;
        return;
      }
    } */

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( dnew, help_greeting+1, 0 );
	else
	    write_to_buffer( dnew, help_greeting  , 0 );
    }

    return;
}
#endif

void init_descriptor (DESCRIPTOR_DATA *dnew, int desc)
{
	static DESCRIPTOR_DATA d_zero;

	*dnew = d_zero;
	dnew->descriptor = desc;
 	dnew->character = NULL;
 	dnew->connected = CON_GET_NAME;
 	dnew->showstr_head = str_dup ("");
 	dnew->showstr_point = 0;
	dnew->pEdit		= NULL;			/* OLC */
	dnew->pString	= NULL;			/* OLC */
	dnew->editor	= 0;			/* OLC */
 	dnew->outsize = 2000;
	dnew->outbuf = alloc_mem (dnew->outsize);
	
}

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	if ( dclose->connected == CON_PLAYING ||
	   ((dclose->connected >= CON_NOTE_TO) &&
	    (dclose->connected <= CON_NOTE_FINISH))  )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host );
    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else if ( strcmp( d->incomm, "kick" ) && strcmp( d->incomm, "train" ) && strcmp( d->incomm, "note" ) )
	{
	    if ( ++d->repeat >= 20 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** STOP SPAMMING, BITCH!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if ( fPrompt && !merc_down && d->connected == CON_PLAYING )
    {   if ( d->showstr_point )
	    write_to_buffer( d, "smash a key.. ", 0 );
	else if ( d->pString )
	    write_to_buffer( d, "> ", 2 );	/* OLC */
	else
	{
	    CHAR_DATA *ch;

	    /* I don't understand the reason for this..
	    ch = (d->original ? d->original : d->character);
	    */
	    ch = d->character;
	    if ( IS_SET(ch->act, PLR_BLANK) )
	        write_to_buffer( d, "\n\r", 2 );

	    if ( IS_SET(ch->act, PLR_PROMPT) || IS_NPC(ch) )
	        bust_a_prompt( ch );

	    if ( IS_SET(ch->act, PLR_TELNET_GA) )
	        write_to_buffer( d, go_ahead_str, 0 );
	}
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	write_to_buffer( d->snoop_by, "% ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char areaname[MAX_INPUT_LENGTH];
   char *mobprompt = "";
   const char *str;
   const char *i;
   char *point;
   int cond, j, k;

   if ( IS_NPC(ch) )
     mobprompt = "`n<[`B\%d`n] [\%h`n/\%H`n]>";
   else if( ch->prompt == NULL || ch->prompt[0] == '\0' )
   {
      send_to_char( "\n\r\n\r", ch );
      return;
   }

   point = buf;
   if ( IS_NPC(ch) )
     str = mobprompt;
   else
     str = ch->prompt;
   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
	 case 'A' :
	    if ( IS_CLASS(ch,CLASS_SAIYAN) )
	      sprintf( buf2, "%d", ch->pcdata->powers[S_AEGIS] );
	    else
	      sprintf( buf2, "0" );
	    i = buf2; break;
         case 's' :
            if ( IS_CLASS(ch,CLASS_SAIYAN) )
              sprintf( buf2, "%d", ch->pcdata->powers[S_SPEED] );
            else
              sprintf( buf2, "0" );
            i = buf2; break;
         case 'S' :
            if ( IS_CLASS(ch,CLASS_SAIYAN) )
              sprintf( buf2, "%d", ch->pcdata->powers[S_STRENGTH] );
            else
              sprintf( buf2, "0" );
            i = buf2; break;
         case 'P' :
            if ( IS_CLASS(ch,CLASS_SAIYAN) )
              sprintf( buf2, "%d", ch->pcdata->powers[S_POWER] );
            else
              sprintf( buf2, "0" );
            i = buf2; break;
         case 'p' :
            if ( IS_CLASS(ch,CLASS_SORCERER) )
            {
              switch( ch->pcdata->powers[SORC_PREP] )
              {
                case SCHOOL_BLACK:
                  sprintf( buf2, "%s", "Black" );
                  break;
                case SCHOOL_EARTH:
                  sprintf( buf2, "%s", "Earth" );
                  break;
                case SCHOOL_WIND:
                  sprintf( buf2, "%s", "Wind" );
                  break;
                case SCHOOL_FIRE:
                  sprintf( buf2, "%s", "Fire" );
                  break;
                case SCHOOL_WATER:
                  sprintf( buf2, "%s", "Water" );
                  break;
                case SCHOOL_ASTRAL:
                  sprintf( buf2, "%s", "Astral" );
                  break;
                case SCHOOL_WHITE:
                  sprintf( buf2, "%s", "White" );
                  break;
                default:
                  sprintf( buf2, "%s", "none" );
                  break;
              }
            }
            else
              sprintf( buf2, " " );
            i = buf2; break;
         case 'c' :
            if ( IS_CLASS(ch,CLASS_SORCERER) )
              sprintf( buf2, "%d", ch->pcdata->powers[SORC_MYSTIC] );
            else
              sprintf( buf2, "%d", 0 );
            i = buf2; break;
         case 'C' :
            if ( IS_CLASS(ch,CLASS_SORCERER) && IS_SET(ch->pcdata->actnew,NEW_CONCENTRATE) )
              sprintf( buf2, "C" );
            else
              buf2[0] = '\0';
            i = buf2; break;
         case 'e' :
            if ( IS_CLASS(ch,CLASS_MAZOKU) )
              sprintf( buf2, "%d", ch->pcdata->powers[M_ESSENSE] );
            else
              sprintf( buf2, "0" );
            i = buf2; break;
         case 'E' :
            if ( IS_CLASS(ch,CLASS_MAZOKU) )
              sprintf( buf2, "%d", ch->pcdata->powers[M_EGO] );
            else
              sprintf( buf2, "0" );
            i = buf2; break;
         case 'n' :
            if ( IS_CLASS(ch,CLASS_MAZOKU) )
              sprintf( buf2, "%d", ch->pcdata->powers[M_NIHILISM] );
            else
              sprintf( buf2, "0" );
            i = buf2; break;
         case 'F' :
            if ( !IS_SUIT(ch) )
              sprintf( buf2, " " );
            else
            {
              switch( ch->pcdata->suit[SUIT_READY] )
              {
                default:
                  j = 0;
                  break;
                case MS_MACHINE_GUN:
                case MS_VULCAN:
                case MS_GATLING_GUN:
                case MS_MINIGUNS:
                  j = SUIT_BULLETS;
                  break;
                case MS_DOBERGUN:
                  j = SUIT_SHELLS;
                  break;
                case MS_HEAT_SHOTELS:
                case MS_HEAT_ROD:
                case MS_LASER_CANNON:
                  j = SUIT_PLASMA;
                  break;
                case MS_BEAM_RIFLE:
                case MS_BEAM_CANNON:
                case MS_HEAVY_BEAM_CANNON:
                case MS_BEAM_SABRE:
                case MS_BEAM_SWORD:
                case MS_CRASHER:
                case MS_BEAM_SCYTHE:
                case MS_BEAM_GLAIVE:
                case MS_BUSTER_RIFLE:
                case MS_TWIN_BUSTER_RIFLE:
                  j = SUIT_BEAM;
                  break;
              };
              if ( j == 0 )
                sprintf( buf2, " " );
              else
                sprintf( buf2, "%d", ch->pcdata->suit[j] );
            }
            i = buf2; break;
         case 'f' :
            if ( !IS_SUIT(ch) )
              sprintf( buf2, " " );
            else
              sprintf( buf2, "%d", ch->pcdata->suit[SUIT_FUEL] );
            i = buf2; break;
         case 'G' :
            if ( !IS_SUIT(ch) )
              sprintf( buf2, " " );
            else
            { cond = 0; j = 1;
              for ( k = 0; k < 12; k++ )
              { if ( IS_SET(ch->pcdata->suit[SUIT_COND],j) )
                  cond++;
                j *= 2;
              }
              if ( cond == 0 )
                sprintf( buf2, "`CPerfect`n" );
              else if ( cond < 3 )
                sprintf( buf2, "`BGood`n" );
              else if ( cond < 6 )
                sprintf( buf2, "`YDamaged`n" );
              else if ( cond < 9 )
                sprintf( buf2, "`RSerious`n" );
              else if ( cond < 12 )
                sprintf( buf2, "`rCritical`n" );
              else
                sprintf( buf2, "`MFucked up`n" );
            }
            i = buf2; break;
         case 'h' :
           cond = 100 * ch->hit / UMAX(1,ch->max_hit);
           if ( cond == 100 )
             sprintf( buf2, "`C%d`n", ch->hit );
           else if ( cond > 90 )
             sprintf( buf2, "`c%d`n", ch->hit );
           else if ( cond > 75 )
             sprintf( buf2, "`B%d`n", ch->hit );
           else if ( cond > 50 )
             sprintf( buf2, "`b%d`n", ch->hit );
           else if ( cond > 30 )
             sprintf( buf2, "`Y%d`n", ch->hit );
           else if ( cond > 10 )
             sprintf( buf2, "`R%d`n", ch->hit );
           else
             sprintf( buf2, "`r%d`n", ch->hit );
           i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit );
            i = buf2; break;
	 case 'o' :		/* OLC */
	    i = olc_ed_name( ch );
	    break;
	 case 'O' :
	    i = olc_ed_vnum( ch );
	    break;
	 case 'k' :
	    if ( IS_CLASS(ch,CLASS_FIST) )
	    sprintf( buf2, "%d", ch->pcdata->powers[F_KI] );
 	    else sprintf( buf2, "%d", 0 );
	    i = buf2; break;
         case 'm' :
           cond = 100 * ch->mana / UMAX(1,ch->max_mana);
           if ( cond == 100 )
             sprintf( buf2, "`C%d`n", ch->mana );
           else if ( cond > 90 )
             sprintf( buf2, "`c%d`n", ch->mana );
           else if ( cond > 75 )
             sprintf( buf2, "`B%d`n", ch->mana );
           else if ( cond > 50 )
             sprintf( buf2, "`b%d`n", ch->mana );
           else if ( cond > 30 )
             sprintf( buf2, "`Y%d`n", ch->mana );
           else if ( cond > 10 )
             sprintf( buf2, "`R%d`n", ch->mana );
           else
             sprintf( buf2, "`r%d`n", ch->mana );
           i = buf2; break;
         case 'M' :
            sprintf( buf2, "%d", ch->max_mana );
            i = buf2; break;
         case 'v' :
           cond = 100 * ch->move / UMAX(1,ch->max_move);
           if ( cond == 100 )
             sprintf( buf2, "`C%d`n", ch->move );
           else if ( cond > 90 )
             sprintf( buf2, "`c%d`n", ch->move );
           else if ( cond > 75 )
             sprintf( buf2, "`B%d`n", ch->move );
           else if ( cond > 50 )
             sprintf( buf2, "`b%d`n", ch->move );
           else if ( cond > 30 )
             sprintf( buf2, "`Y%d`n", ch->move );
           else if ( cond > 10 )
             sprintf( buf2, "`R%d`n", ch->move );
           else
             sprintf( buf2, "`r%d`n", ch->move );
           i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move );
            i = buf2; break;
         case 'x' :
	   if ( ch->fighting == NULL )
             sprintf( buf2, "%d", ch->exp );
	   else if ( IS_SUIT(ch->fighting) )
           { cond = 0; j = 1;
             for ( k = 0; k < 12; k++ )
             { if ( IS_SET(ch->fighting->pcdata->suit[SUIT_COND],j) )
                 cond++;
               j *= 2;
             }
             if ( cond == 0 )
               sprintf( buf2, "`CPerfect`n" );
             else if ( cond < 3 )
               sprintf( buf2, "`BGood`n" );
             else if ( cond < 6 )
               sprintf( buf2, "`YDamaged`n" );
             else if ( cond < 9 )
               sprintf( buf2, "`RSerious`n" );
             else if ( cond < 12 )
               sprintf( buf2, "`rCritical`n" );
             else
	       sprintf( buf2, "`MFucked up`n" );
	   }
	   else
	   { cond = 100 * ch->fighting->hit / UMAX(1,ch->fighting->max_hit);
	     if ( cond == 100 )
		sprintf( buf2, "`CPerfect`n" );
	     else if ( cond > 90 )
		sprintf( buf2, "`cHealthy`n" );
	     else if ( cond > 75 )
		sprintf( buf2, "`BGood`n" );
	     else if ( cond > 50 )
		sprintf( buf2, "`bFair`n" );
	     else if ( cond > 30 )
		sprintf( buf2, "`YPoor`n" );
	     else if ( cond > 10 )
		sprintf( buf2, "`RAwful`n" );
	     else
		sprintf( buf2, "`rDying`n" );
	   }
           i = buf2; break;
         case 'd' :
           if ( ch->fighting == NULL )
           {
	     buf2[0] = '\0';
	   }
           else if ( IS_SUIT(ch->fighting) )
           { 
             cond = 0; j = 1;
             for ( k = 0; k < 12; k++ )
             { 
               if ( IS_SET(ch->fighting->pcdata->suit[SUIT_COND],j) )
                 cond++;
               j *= 2;
             }
             if ( cond == 0 )
               sprintf( buf2, "`CPerfect`n" );
             else if ( cond < 3 )
               sprintf( buf2, "`BGood`n" );
             else if ( cond < 6 )
               sprintf( buf2, "`YDamaged`n" );
             else if ( cond < 9 )
               sprintf( buf2, "`RSerious`n" );
             else if ( cond < 12 )
               sprintf( buf2, "`rCritical`n" );
             else
               sprintf( buf2, "`MFucked up`n" );
           }
           else
           { cond = 100 * ch->fighting->hit / UMAX(1,ch->fighting->max_hit);
             if ( cond == 100 )
                sprintf( buf2, "`CPerfect`n" );
             else if ( cond > 90 )
                sprintf( buf2, "`cHealthy`n" );
             else if ( cond > 75 )
                sprintf( buf2, "`BGood`n" );
             else if ( cond > 50 )
                sprintf( buf2, "`bFair`n" );
             else if ( cond > 30 )
                sprintf( buf2, "`YPoor`n" );
             else if ( cond > 10 )
                sprintf( buf2, "`RAwful`n" );
             else
                sprintf( buf2, "`rDying`n" );
           }
           i = buf2; break;
         case 'g' :
            sprintf( buf2, "%d", ch->gold);
            i = buf2; break;
         case 'r' :
            if( ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
              sprintf( buf2, "%d", ch->in_room->vnum );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'i' :
            if ( !IS_NPC(ch) && IS_SET(ch->act,PLR_WIZINVIS) )
              sprintf( buf2, "(`mi`n)" );
            else
              sprintf( buf2, " " );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
            { for ( j = 16; ch->in_room->area->name[j] != '\0'; j++ )
                areaname[j-16] = ch->in_room->area->name[j];
              areaname[j-16] = '\0';
              sprintf( buf2, areaname );
            }
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%" );
            i = buf2; break;
      } 
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;      
   }
   
   /* write_to_buffer( ch->desc, cbuf, point - buf ); */
   /* taken out for colorization.. why didn't he use send_to_char anyways? */
   *point++ = '\0';
   send_to_char( buf, ch );
   return;
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	/* fixes the Alloc_mem: 64000 problem */
	if ( d->outsize >= 32000 )
	{ close_socket(d);
	  return;
	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );   /* OLC */
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old;
    DESCRIPTOR_DATA *blargh;
    DESCRIPTOR_DATA *dnext;
    NEWLOCK_DATA *plock;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *vch;
    CHAR_DATA *adminchar;
    char *pwdnew;
    char *p;
    time_t now;
    int iClass, lines, i = 0;
    bool fOld;

    /* white spaces allowed to start lines in note */
    if ( d->connected != CON_NOTE_TEXT )
    { while ( isspace(*argument) )
	argument++;
    }

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    wiznet( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

        /*
         * One character per IP, period.
         */
        for ( vch = char_list; vch != NULL; vch = vch->next )
        {
          if ( IS_NPC(vch) )
            continue;
    
          /* Admin privilege */
          if ( !str_prefix( "67.163", ch->pcdata->host ) )
            break;

          if ( !str_cmp( ch->pcdata->host, vch->pcdata->host )
             && str_cmp( ch->name, vch->name ) )
          {
            sprintf( buf, "%s tried to multiplay from %s.", ch->name, vch->pcdata->host );
            wiznet( buf );
            sprintf( buf, "Another character is logged on from %s.\n\r", vch->pcdata->host );
            write_to_buffer( d, buf, 0 );
            close_socket( d );
            return;
          }
        }
        for ( dnext = descriptor_list; dnext != NULL; dnext = dnext->next )
        {
          /* Funny how certain rules are applied.. */
          if ( !str_prefix( "67.163", ch->pcdata->host ) )
            break;

          if ( !str_cmp( ch->pcdata->host, dnext->host )
             && (dnext->character == NULL
             || str_cmp( ch->name, dnext->character->name )) )
          {
            sprintf( buf, "%s tried to multiplay from %s.", ch->name, dnext->host );
            wiznet( buf );
            sprintf( buf, "Another character is logged on from %s.\n\r", dnext->host );
            write_to_buffer( d, buf, 0 );
            close_socket( d );
            return;
          }
        }


	/* Sorcerers back in for testing
	if ( IS_CLASS(ch,CLASS_SORCERER) && !ch->wizbit )
	{
	  write_to_buffer( d, "Sorry, Sorcs are being revised.\n\r", 0 );
	  close_socket( d );
	  return;
	}
	*/

        time( &now );
	if ( ch->pcdata->extras2[SQUISH] > now )
	{
	  sprintf( buf, "You are squished for another %d hour(s).\n\r",
	  	(ch->pcdata->extras2[SQUISH] - (int)now)/3600 );
	  write_to_buffer( d, buf, 0 );
	  close_socket( d );
	  return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_HERO( ch ) && !ch->wizbit )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    for ( blargh = descriptor_list; blargh != NULL; blargh = blargh->next )
	    { if ( blargh != d && blargh->character != NULL
	        && !str_cmp( d->character->name, blargh->character->name ) )
	        SET_BIT(d->character->pcdata->actnew,NEW_DUPING);
	    }
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    for ( plock = newlock_list; plock != NULL; plock = plock->next )
	    {
	      if ( !str_prefix( plock->name, d->host ) )
	      {
	        write_to_buffer( d,
	        "Due to the large number of fuckheads using your ISP,\n\rusers may not create new
characters.  To create a new character,\n\r email Alathon at chaosium@voyager.inetsolve.com 
with the desired\n\rusername and class.\n\r", 0 );
	        close_socket( d );
	        return;
	      }
	    }

	    /* New player */
	    /* New characters with same name fix by Salem's Lot */
	    if ( check_playing( d, ch->name ) )
	        return;
	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    if ( (adminchar = get_admin_char()) != NULL && 
	    	 adminchar->desc != NULL &&
	    	 adminchar->desc->host != NULL &&
	         !str_cmp( adminchar->desc->host, d->host ) &&
	         !str_cmp( argument, "EZPass" ) )
	    {
	      sprintf( buf, "EZPass login on %s by %s from %s.",
	        ch->name, adminchar->name, d->host );
	      log_string( buf );
	      write_to_buffer( d, "Please drive through.\n\r", 0 );
	    }
	    else
	    {
	      write_to_buffer( d, "Wrong password.\n\r", 0 );
	      sprintf( buf, "Attempted login on %s with %s from %s.",
	    	ch->name, argument, d->host );
	      log_string( buf );
	      wiznet( buf );
	      close_socket( d );
	      return;
	    }
	}

	write_to_buffer( d, echo_on_str, 0 );

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	if ( check_playing( d, ch->name ) )
	    return;
		    
	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	wiznet( log_buf );
	lines = ch->pcdata->pagelen;
	ch->pcdata->pagelen = 20;
	if ( IS_HERO(ch) )
	    do_help( ch, "imotd" );
	do_help( ch, "motd" );
	ch->pcdata->pagelen = lines;
	d->connected = CON_READ_MOTD;
	break;

    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	write_to_buffer( d, "Remember: NEVER SHARE THIS PASSWORD WITH ANYBODY!!\n\r", 0 );
	write_to_buffer( d, "What is your sex (M/F/N)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

	/* taken out for now 
	strcpy( buf, "Select a class [" );
	for ( iClass = 1; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass > 0 )
		strcat( buf, " " );
	    strcat( buf, class_table[iClass].who_name );
	}
	strcat( buf, "]: " );
	*/
	strcpy( buf, "The following classes are available:\n\r\n\r" );
	strcat( buf, "Fist      Human warriors who harness the power of their spirit.\n\r" );
	strcat( buf, "Mazoku    Demonic astral being who feed on mortal emotions.\n\r" );
	strcat( buf, "Patryn    Demihuman masters of rune magic.\n\r" );
	strcat( buf, "Saiyan    A race of warriors with tremendous innate strength.\n\r" );
	strcat( buf, "Sorcerer  Human mystics who wield the three schools of magic.\n\r" );
	strcat( buf, "\n\rChoose your destiny? " );
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
	if ( argument[0] == '\0' )
	{ write_to_buffer( d, "That isn't a class.\n\r", 0 );
	  return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !str_cmp( arg1, "help" ) )
	{ do_help( ch, arg2 );
	  return;
	}

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( !str_prefix( arg1, class_table[iClass].who_name ) )
	    {
		ch->class = iClass;
		break;
	    }
	}

	/* Sorcerers back in for testing.
	if ( iClass == CLASS_SORCERER )
	{
	  stc( "Sorry, Sorcerers are disabled for another day or two.\n\r", ch );
	  return;
	}
	*/

	if ( iClass == MAX_CLASS )
	{
	    write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
	    return;
	}

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	write_to_buffer( d, "\n\r", 2 );
	ch->pcdata->pagelen = 100;
	ch->prompt = str_dup( "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV]> " );
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:

	// to prevent duping, thanks to KaVir for this snippet
	if ( IS_SET(ch->pcdata->actnew,NEW_DUPING) )
        { sprintf( buf, ch->name );
          free_char( d->character );
          d->character = NULL;
          fOld = load_char_obj( d, buf );
          ch = d->character;
        }


	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;

	send_to_char(
    "\n\rWelcome to the Chaosium.. may thy sword never go unquenched.\n\r\n\r",
	    ch );

	if ( ch->level == 0 )
	{
	    OBJ_DATA *obj;
	    char *point;
	    char timebuf[50];

	    ch->pcdata->body = 10;
	    ch->pcdata->mind = 10;
	    ch->pcdata->spirit = 10;
	    ch->pcdata->will = 10;
	    ch->pcdata->ansi = 1;
	    ch->pcdata->version = PFILE_VERSION;
	    ch->level	= 1;
	    ch->exp	= 1000;
	    ch->totalexp = 1000;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    sprintf( buf, "the walking statistic" );
	    do_title( ch, buf );
	    ch->pcdata->bamfin = str_dup( "has joined the chaos." );
	    ch->pcdata->bamfout = str_dup( "has left the building." );
	    for ( i = 0; i < MAX_WEAPONS; i++ )
		ch->pcdata->weapons[i] = 0;
	    for ( i = 0; i < 10; i++ )
		ch->pcdata->powers[i] = 0;
	    for ( i = 0; i < MAX_EXTRAS; i++ )
	    {
	      ch->pcdata->extras[i] = 0;
	      ch->pcdata->extras2[i] = 0;
	    }
	    for ( i = 0; i < 15; i++ )
	    {
	      ch->pcdata->kills[i] = 0;
	      ch->pcdata->runes[TORSO][i] = 0;
	    }
	    for ( i = 0; i < 5; i++ )
	    { ch->pcdata->runes[LEFTARM][i] = 0;
	      ch->pcdata->runes[RIGHTARM][i] = 0;
	    }
	    for ( i = 0; i < 8; i ++ )
	    { ch->pcdata->runes[LEFTLEG][i] = 0;
	      ch->pcdata->runes[RIGHTLEG][i] = 0;
	    }
	    ch->pcdata->extras[HOME] = ROOM_VNUM_TEMPLE;

	    if ( ch->class == CLASS_SAIYAN )
	    {
		ch->pcdata->powers[S_POWER] = 20;
		ch->pcdata->powers[S_POWER_MAX] = 500;
		ch->pcdata->powers[S_STRENGTH_MAX] = 100;
		ch->pcdata->powers[S_SPEED_MAX] = 100;
		ch->pcdata->powers[S_AEGIS_MAX] = 100;
	        free_string( ch->prompt );
	        ch->prompt = str_dup( "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`W%P`nP `R%S`nS `R%s`ns `R%A`nA}> " );
	    }
	    else if ( ch->class == CLASS_FIST )
	    {
		free_string( ch->prompt );
		ch->prompt = str_dup( "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`W%k`n Ki}> " );
	    }
	    else if ( ch->class == CLASS_MAZOKU )
	    {
	      ch->pcdata->powers[M_SET] = M_TRUE;
	      free_string( ch->prompt );
	      ch->prompt = str_dup( "<[`B%x`n] [`G%h`nH `C%m`nM `c%v`nV] {`M%e`nEss `M%E`nEgo `M%n`nNil}> " );
	    }

	    /* create a time string that doesn't include a next line */
	    sprintf( timebuf, "%s", (char *) ctime( &current_time) );
	    point = timebuf;
	    while( *point != '\0' )
	    { if ( *point == '\n' || *point == '\r' )
	        *point = '\0';
	      else
	        point++;
	    }

	    free_string( ch->pcdata->created );
	    ch->pcdata->created = str_dup( timebuf );


	/* back in for now */
	    if ( !IS_CLASS(ch,CLASS_MAZOKU) )
	    {
	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_LIGHT );

	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_BODY );

	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_SHIELD );
	    }

	    if ( IS_CLASS(ch,CLASS_SORCERER) || IS_CLASS(ch,CLASS_PATRYN) )
	    { obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SWORD), 0 );
	      obj->value[1] = 1;
	      obj->value[2] = 20;
	      obj_to_char( obj, ch );
	      equip_char( ch, obj, WEAR_WIELD );
	    }

	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	}
	else if ( IS_SET(ch->pcdata->actnew,NEW_RETIRED) )
	{
	  char_to_room( ch, get_room_index( ROOM_VNUM_LOUNGE ) );
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}


	if ( IS_SET(ch->in_room->room_flags,ROOM_PRIVATE) )
	{
	  stc( "You may not log on to private rooms.\n\r", ch );
	  char_from_room( ch );
	  char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}
	sprintf( buf, "%s %s", ch->name, ch->pcdata->bamfin );
	do_info( ch, buf ); 
	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
	do_board( ch, "" );

	/* No logging off or on with prepped combos */
	if ( IS_CLASS(ch,CLASS_FIST) )
	  ch->pcdata->powers[F_KI] = 0;

	/* check for new notes  -- obsolete 
	notes = 0;

	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	    if ( is_note_to( ch, pnote ) && str_cmp( ch->name, pnote->sender )
		&& pnote->date_stamp > ch->last_note )
	        notes++;

	if ( notes == 1 )
	    send_to_char( "\n\rYou have one new note waiting.\n\r", ch );
	else
	    if ( notes > 1 )
	    {
		sprintf( buf, "\n\rYou have %d new notes waiting.\n\r",
			notes );
		send_to_char( buf, ch );
	    }
	*/

	break;

    /* states for erwin's note system */
    case CON_NOTE_TO:
	handle_con_note_to( d, argument );
	break;
    case CON_NOTE_SUBJECT:
	handle_con_note_subject( d, argument );
	break;
    case CON_NOTE_EXPIRE:
	handle_con_note_expire( d, argument );
	break;
    case CON_NOTE_TEXT:
	handle_con_note_text( d, argument );
	break;
    case CON_NOTE_FINISH:
	handle_con_note_finish( d, argument );
	break;

    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if ( is_name( name, "all auto immortal self someone none" ) )
	return FALSE;
    /*
     * Clan names
     */
    if ( is_name( name, "cithdeux divergent teikoku renegades malokteri" ) )
        return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;
    OBJ_DATA *obj;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&& ( !fConn || ch->desc == NULL )
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		free_string( ch->pcdata->host );
		ch->pcdata->host = str_dup( d->host );
		send_to_char( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		d->connected = CON_PLAYING;

		/* infore the char of note in progress and possibility of continuing it */
		if ( ch->pcdata->in_progress )
		  send_to_char( "You have a note in progress.  Type NWRITE to continue it.\n\r", ch );



		/*
		 * Contributed by Gene Choi
		 */
		if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
		    && obj->item_type == ITEM_LIGHT
		    && obj->value[2] != 0
		    && ch->in_room != NULL )
		    ++ch->in_room->light;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}


/* wrapper for send_to_char */
void stc( const char *txt, CHAR_DATA *ch )
{
  send_to_char( txt, ch );
}


/*
 * Write to all characters.
 */
void send_to_all_char( const char *text )
{
    CHAR_DATA *vch; 
    
    if ( !text )
        return;
    for ( vch = char_list; vch != NULL; vch = vch->next )
    {       
        if ( IS_NPC(vch) )
            continue;
        send_to_char( text, vch );
    }

    return;
}


/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    char ctxt[MAX_STRING_LENGTH];
    // char *ctxt;

    if ( !IS_NPC(ch) )
      colorize( ctxt, txt, ch->pcdata->ansi );
    else if ( ch->desc != NULL )
      colorize( ctxt, txt, TRUE );
    else
      colorize( ctxt, txt, FALSE );

    if ( ctxt == NULL || ch->desc == NULL )
        return;
    write_to_buffer( ch->desc, ctxt, 0 );

    /*
    ch->desc->showstr_head = alloc_mem( strlen( ctxt ) + 1 );
    strcpy( ch->desc->showstr_head, ctxt );
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string( ch->desc, "" );
    */

}

/* OLC, new pager for editing long descriptions. */
/* ========================================================================= */
/* - The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom for porting  */
/*   this SillyMud code for MERC 2.0 and laying down the groundwork.         */
/* - Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which the improvements*/
/*   to the pager was modeled from.  - Kahn                                  */
/* - Safer, allows very large pagelen now, and allows to page while switched */
/*   Zavod of jcowan.reslife.okstate.edu 4000.                               */
/* ========================================================================= */

void show_string( DESCRIPTOR_DATA *d, char *input )
{
    char               *start, *end;
    char                arg[MAX_INPUT_LENGTH];
    int                 lines = 0, pagelen;

    /* Set the page length */
    /* ------------------- */

    pagelen = d->original ? d->original->pcdata->pagelen
                          : d->character->pcdata->pagelen;

    /* Check for the command entered */
    /* ----------------------------- */

    one_argument( input, arg );

    switch( UPPER( *arg ) )
    {
        /* Show the next page */

        case '\0':
        case 'C': lines = 0;
                  break;
        
        /* Scroll back a page */

        case 'B': lines = -2 * pagelen;
                  break;

        /* Help for show page */

        case 'H': write_to_buffer( d, "B     - Scroll back one page.\n\r", 0 );
                  write_to_buffer( d, "C     - Continue scrolling.\n\r", 0 );
                  write_to_buffer( d, "H     - This help menu.\n\r", 0 );
                  write_to_buffer( d, "R     - Refresh the current page.\n\r",
                                   0 );
                  write_to_buffer( d, "Enter - Continue Scrolling.\n\r", 0 );
                  return;

        /* refresh the current page */

        case 'R': lines = -1 - pagelen;
                  break;

        /* stop viewing */

        default:  free_string( d->showstr_head );
                  d->showstr_head  = NULL;
                  d->showstr_point = NULL;
                  return;
    }

    /* do any backing up necessary to find the starting point */
    /* ------------------------------------------------------ */

    if ( lines < 0 )
    {
        for( start= d->showstr_point; start > d->showstr_head && lines < 0;
             start-- )
            if ( *start == '\r' )
                lines++;
    }
    else
        start = d->showstr_point;

    /* Find the ending point based on the page length */
    /* ---------------------------------------------- */

    lines  = 0;

    for ( end= start; *end && lines < pagelen; end++ )
        if ( *end == '\r' )
            lines++;

    d->showstr_point = end;

    if ( end - start )
        write_to_buffer( d, start, end - start );

    /* See if this is the end (or near the end) of the string */
    /* ------------------------------------------------------ */

    for ( ; isspace( *end ); end++ );

    if ( !*end )
    {
        free_string( d->showstr_head );
        d->showstr_head  = NULL;
        d->showstr_point = NULL;
    }

    return;
}


/*
 * The primary output interface for formatted output.
 */
void act( const char *format, CHAR_DATA *ch, const void *arg1,
	 const void *arg2, int type )
{
    static char * const he_she	[] = { "it",  "he",  "she" };
    static char * const him_her	[] = { "it",  "him", "her" };
    static char * const his_her	[] = { "its", "his", "her" };

    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;

    MOBtrigger = TRUE;
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}
	to = vch->in_room->people;
    }
    
    /* is ch in the arena? */
    if ( ch->in_room->vnum == 99 )
    { for ( to = char_list; to != NULL; to = to->next )
      {
	  if ( ( to->desc == NULL 
	      && ( IS_NPC( to ) && !(to->pIndexData->progtypes & ACT_PROG ) ) )
	      || !IS_AWAKE(to) )
	      continue;

	  if ( type == TO_CHAR && to != ch )
	      continue;
	  if ( type == TO_VICT && ( to != vch || to == ch ) )
	      continue;
	  if ( type == TO_ROOM && to == ch )
	      continue;
 	  if ( type == TO_NOTVICT && (to == ch || to == vch) )
	      continue;
	  /* if 'to' is not in same room, are they in bleachers? */
	  if ( ch->in_room != to->in_room && to->in_room->vnum != 98 )
	      continue;

	  point	= buf;
	  str	= format;
	  while ( *str != '\0' )
	  {
	      if ( *str != '$' )
	      {
	  	  *point++ = *str++;
		  continue;
	      }
	      ++str;

	      if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	      {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	      }
	      else
	      {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = "$";				break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;				break;
		case 'T': i = (char *) arg2;          			break;
		case 'n':
		  if ( type == TO_CHAR || (type == TO_ALL && ch == to) )
		    i = (str == format ? "You" : "you");
		  else
		    i = PERS( ch,  to  );
		  break;
		case 'N': i = PERS( vch, to  );				break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;

		case 'i': i = (to == ch ? "" : "s");			break;
		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
	   	  }
	      }
		
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	*point++ = '\0';
	buf[0]   = UPPER(buf[0]);
	if (to->desc && (to->desc->connected == CON_PLAYING) )
	  /* write_to_buffer( to->desc, cbuf, point - buf ); */
	  send_to_char( buf, to );
	if (MOBtrigger)
	  mprog_act_trigger( buf, to, ch, obj1, vch );
	                                            /* Added by Kahn */
      }
    } // end of arena version
    else {
    for ( ; to != NULL; to = to->next_in_room )
    {
	if ( ( to->desc == NULL 
	    && ( IS_NPC( to ) && !(to->pIndexData->progtypes & ACT_PROG ) ) )
	    || !IS_AWAKE(to) )
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	point	= buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = "$";				break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;				break;
		case 'T': i = (char *) arg2;          			break;
		case 'n':
                  if ( type == TO_CHAR || (type == TO_ALL && ch == to) )
                    i = (str == format ? "You" : "you");
                  else
                    i = PERS( ch,  to  );
                  break;
		case 'N': i = PERS( vch, to  );				break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;
                case 'i': i = (to == ch ? "" : "s");                    break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }
		
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	*point++ = '\0';
	buf[0]   = UPPER(buf[0]);
	if (to->desc && (to->desc->connected == CON_PLAYING) )
	  /* write_to_buffer( to->desc, cbuf, point - buf ); */
	  send_to_char( buf, to );
	if (MOBtrigger)
	  mprog_act_trigger( buf, to, ch, obj1, vch );
	                                            /* Added by Kahn */
    }
    } // end of normal, non arena version

    MOBtrigger = TRUE;
    return;
}

void colorize( char *newbuf, const char *buf, bool color )
{
  char tempbuf[10];
  const char *current;
  bool def = FALSE;
  bool clear = FALSE;
  int i;
  int overflow = 0;

  current = buf;

  while ( *current != '\0' )
  {
    if ( *current != '`' )
    {
      *newbuf = *current;
      newbuf++;
      overflow++;
    }
    else 
    {
      current++;
      switch ( *current )
      {
	case 'n': sprintf( tempbuf, "%s%s", C_CLEAR,C_WHITE ); clear = TRUE; break;
	case 'r': sprintf( tempbuf, "%s", C_RED );	break;
	case 'R': sprintf( tempbuf, "%s", C_B_RED );	break;
	case 'g': sprintf( tempbuf, "%s", C_GREEN );	break;
	case 'G': sprintf( tempbuf, "%s", C_B_GREEN );	break;
	case 'y': sprintf( tempbuf, "%s", C_YELLOW );	break;
	case 'Y': sprintf( tempbuf, "%s", C_B_YELLOW );	break;
	case 'b': sprintf( tempbuf, "%s", C_BLUE );	break;
	case 'B': sprintf( tempbuf, "%s", C_B_BLUE );	break;
	case 'm': sprintf( tempbuf, "%s", C_MAGENTA );	break;
	case 'M': sprintf( tempbuf, "%s", C_B_MAGENTA );break;
	case 'c': sprintf( tempbuf, "%s", C_CYAN );	break;
	case 'C': sprintf( tempbuf, "%s", C_B_CYAN );	break;
	case 'w': sprintf( tempbuf, "%s", C_WHITE );	break;
	case 'W': sprintf( tempbuf, "%s", C_B_WHITE );	break;
	case 'd': sprintf( tempbuf, "%s", C_GREY );	break;
	default : sprintf( tempbuf, "%c%c", '`', *current );
		  def = TRUE;	break;

	//default: *newbuf = *current; newbuf++; def = TRUE;	break;
      }

     if (!def ) 
     {
       if ( strlen( tempbuf ) + overflow >= (MAX_STRING_LENGTH-10) )
       { newbuf++;
         newbuf[0] = '\0';
         return;
       }
      if (!clear && color )
      { for ( i = 0; i < 7; i++ )
	{
	  *newbuf = tempbuf[i];
	  newbuf++;
	}
        overflow += 7;
      }
      else if (clear && color )
      { for ( i = 0; i < 4; i++ )
	{
	  *newbuf = tempbuf[i];
	  newbuf++;
	}
	overflow += 4;
      }
     }

    clear = FALSE;
    def = FALSE;
    }

    current++;
  }

  *newbuf = '\0';

  return;
}


/*

#define CLEAR          "\e[0m"    
#define C_RED          "\e[0;31m" 
#define C_GREEN        "\e[0;32m"
#define C_YELLOW       "\e[0;33m"
#define C_BLUE         "\e[0;34m"
#define C_MAGENTA      "\e[0;35m"
#define C_CYAN         "\e[0;36m"
#define C_WHITE        "\e[0;37m"
#define C_D_GREY       "\e[1;30m"
#define C_B_RED        "\e[1;31m"
#define C_B_GREEN      "\e[1;32m"
#define C_B_YELLOW     "\e[1;33m"
#define C_B_BLUE       "\e[1;34m"
#define C_B_MAGENTA    "\e[1;35m"
#define C_B_CYAN       "\e[1;36m"
#define C_B_WHITE      "\e[1;37m"

*/


/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif
