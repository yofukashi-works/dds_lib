#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "dds.h"
#include "dds_lib.h"

/*** test bench *************************************************************/

void main( int argc, char **argv ){
	
	char	szBuf[ 260 ];
	int		i;
	char	*p;
	
	puts( argv[ 1 ] );
	
	for( p = argv[ 1 ]; *p; ++p ){
		if( *p == '#' ) *p = '%';
	}
	
	i = StrReplaceEnv( szBuf, argv[ 1 ] );
	printf( "%d:%s\n", i, szBuf );
}
