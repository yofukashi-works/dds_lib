#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "dds.h"
#include "dds_lib.h"

/*** test bench *************************************************************/

void main( int argc, TCHAR **argv ){
	
	TCHAR	szBuf[ 260 ];
	int		i;
	TCHAR	*p;
	
	_putts( argv[ 1 ] );
	
	for( p = argv[ 1 ]; *p; ++p ){
		if( *p == _T( '#' )) *p = _T( '%' );
	}
	
	i = StrReplaceEnv( szBuf, argv[ 1 ] );
	_tprintf( _T( "%d:%ls\n" ), i, szBuf );
}
