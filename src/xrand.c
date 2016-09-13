#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef	unsigned char	bool;
#define	TRUE		1

#define	TRIAL_MAX	1000000
#define SQUARE(x)	( (x) * (x) )

int	number_range	( int from, int to );
int	number_mm	( void );



int main( argc, argv )
    int argc;
    char *argv[];
{
    int		nBin;

    for ( nBin = 2; nBin < 1000; nBin++ )
    {
	int *	prgiBin;
	int	iRand;
	int	iBin;
	int	nMean;
	int	nSigma2;
	double	dSigma;
	
	prgiBin = calloc( nBin, sizeof(int) );
	for ( iRand = 0; iRand < TRIAL_MAX; iRand++ )
	    prgiBin[number_range(0, nBin-1)] += 1;

	nMean	= TRIAL_MAX / nBin;
	nSigma2 = 0;
	for ( iBin = 0; iBin < nBin; iBin++ )
	    nSigma2 += SQUARE( prgiBin[iBin] - nMean );
	dSigma   = sqrt( (double) (nSigma2 / (nBin - 1)) );
	printf( "%6d %6.4f\n", nBin, dSigma / nMean );
	free( prgiBin );
    }

    return 0;
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power;
    int number;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & (power - 1) ) >= to )
	;

    return from + number;
}



int number_mm( void )
{
    static int rgiState[3+55];
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[0];

    if ( piState[0] == 0 )
    {
	int iState;

	piState[3+0]	= 1;
	piState[3+1]	= 1;
	for ( iState = 3+2; iState < 3+55; iState++ )
	{
	    piState[iState] = (piState[iState-1] + piState[iState-2])
			    & ((1 << 30) - 1);
	}

	piState[0]	= 1;
	piState[1]	= 3+55 - 55;
	piState[2]	= 3+55 - 24;
    }

    iState1	 	= piState[1];
    iState2	 	= piState[2];
    iRand	 	= (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 3+55 )
	iState1 = 3+0;
    if ( ++iState2 == 3+55 )
	iState2 = 3+0;
    piState[1]		= iState1;
    piState[2]		= iState2;
    return iRand >> 6;
}

int isquare( int num )
{
  if ( num <= 0 )
    return 0;

  double x;
  x = sqrt( (double) (num) );
  return ( (int) (x) );
}



