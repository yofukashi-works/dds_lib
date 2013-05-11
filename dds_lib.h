/*****************************************************************************

		dds_lib.h -- Deyu Deyu Software's standard library include file
		Copyright(C) 1997 - 2000 by Deyu Deyu Software

*****************************************************************************/

#if !defined _DDS_LIB_H
#define _DDS_LIB_H

#ifdef  __cplusplus
extern "C" {
#endif	/* def __cplusplus */

#define IsBlank( c )	(( c ) == ' ' || ( c ) == '\t' )
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

char *	StrTokFile( char *, const char *, UINT );
char *	GetFullPathWithCDir( char *szDst, const char *szFileName, const char *szCDir );
BOOL IsExt( const char *szFileName, const char *szExt );
char *	ChangeExt( char *, char *, char * );
char *	StrCatPathChar( char * );
char *	StrShift( char *, int );
char *	StrQuote( char * );
char *	StrRemoveCR( char * );
char *	StrGetParam( char *, char ** );
char *	StrSearchMem( char *, char *, UINT, UINT );
int		StrReplaceEnv( char *, char * );

/*** shell exec *************************************************************/

enum{
	EXECFILE_CHDIR		= 1 << 0,	// %1 ‚Ì DIR ‚É CD
	EXECFILE_RET_CMD	= 1 << 1,	// ÅI“I‚ÉŽÀs‚µ‚½ cmd ‚ð szCmdLine ‚É•Ô‚·
};

BOOL ExecuteFile(
	char				*szCmdLine,
	STARTUPINFO			*pStartupInfo,
	PROCESS_INFORMATION	*ppiRet,
	UINT				uOption
);

#ifdef  __cplusplus
}
#endif	/* def __cplusplus */
#endif	/* !def _DDS_LIB_H */
