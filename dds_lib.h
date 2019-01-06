/*****************************************************************************

		dds_lib.h -- Deyu Deyu Software's standard library include file
		Copyright(C) 1997 - 2000 by Deyu Deyu Software

*****************************************************************************/

#pragma once
#include <tchar.h>

#ifdef  __cplusplus
extern "C" {
#endif	/* def __cplusplus */

#define IsBlank( c )	(( c ) == _T( ' ' ) || ( c ) == _T( '\t' ))
#define SkipBlank( p )	do{ while( IsBlank( *( p ))) ++( p ); } while( 0 )
#define SearchExt( p )	StrTokFile( NULL, ( p ), STF_EXT )
#define ZeroToNull( p )	( *( p ) ? ( p ) : NULL )

/*** string library *********************************************************/

enum{
	STF_DRV		= 1 << 0,
	STF_PATH	= 1 << 1,
	STF_NODE	= 1 << 2,
	STF_EXT		= 1 << 3,
	STF_SHORT	= 1 << 4,
	STF_LONG	= 1 << 5,
	STF_FULL	= 1 << 6,
	
	STF_NAME	= STF_NODE | STF_EXT,
	STF_PATH2	= STF_DRV  | STF_PATH
};

TCHAR *	StrTokFile( TCHAR *, const TCHAR *, UINT );
TCHAR *	GetFullPathWithCDir( TCHAR *szDst, const TCHAR *szFileName, const TCHAR *szCDir );
BOOL IsExt( const TCHAR *szFileName, const TCHAR *szExt );
TCHAR *	ChangeExt( TCHAR *, TCHAR *, TCHAR * );
TCHAR *	StrCatPathChar( TCHAR * );
TCHAR *	StrShift( TCHAR *, int );
TCHAR *	StrQuote( TCHAR * );
TCHAR *	StrRemoveCR( TCHAR * );
TCHAR *	StrGetParam( TCHAR *, TCHAR ** );
TCHAR *	StrSearchMem( TCHAR *, TCHAR *, UINT, UINT );
int		StrReplaceEnv( TCHAR *, TCHAR * );

/*** shell exec *************************************************************/

enum{
	EXECFILE_CHDIR		= 1 << 0,	// %1 の DIR に CD
	EXECFILE_RET_CMD	= 1 << 1,	// 最終的に実行した cmd を szCmdLine に返す
};

BOOL ExecuteFile(
	TCHAR				*szCmdLine,
	STARTUPINFO			*pStartupInfo,
	PROCESS_INFORMATION	*ppiRet,
	UINT				uOption
);

#ifdef  __cplusplus
}
#endif	/* def __cplusplus */
