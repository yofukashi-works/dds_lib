/*****************************************************************************

		string.c -- dds string library
		Copyright(C) 1997 - 2000 by Deyu Deyu Software

2001.02.04	StrTokFile : 該当 _tcs がないときは NULL を返す ( szDst == NULL時 )
2001.03.29	StrGetParam : szDst == NULL の時はポインタだけ返す
2002.02.04	WIN32 固有部分を切り離し可能にした
			StrQuote : " がないときだけ " を付加する
			StrExpandEnv 追加

*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <direct.h>
#endif
#include <stdlib.h>

#include "dds.h"
#include "dds_lib.h"

/*** macro ******************************************************************/

#define StrCpy_fromP2P( dst, from, to ) \
	while(( from ) < ( to )) ( *( dst )++ = *( from )++ )

/*** _tcstokfile *************************************************************/
/*
	filename operation の真打登場
	szDst == NULL のときは，szFileName 内の sub string を返す
	該当部分がなければ，ret = NULL
*/

TCHAR *StrTokFile( TCHAR *szDst, const TCHAR *szFileName, UINT uMode ){
	
	const TCHAR	*pPath,
				*pNode,
				*pExt,
				*pSce = szFileName;
	TCHAR		*pDst = szDst;
	
	DWORD	dwLen;
	
	#ifdef _WIN32
		if( szDst ){
			// full path 変換
			if( uMode & STF_FULL ){
				if(( dwLen = GetFullPathName( pSce, MAX_PATH, szDst, NULL )) && dwLen < MAX_PATH )
					pSce = szDst;
			}
			
			// LFN 変換
			if( uMode & STF_LONG ){
				if(( dwLen = GetLongPathName( pSce, szDst, MAX_PATH )) && dwLen < MAX_PATH )
					pSce = szDst;
				
			// SFN 変換
			}else if( uMode & STF_SHORT ){
				if(( dwLen = GetShortPathName( pSce, szDst, MAX_PATH )) && dwLen < MAX_PATH )
					pSce = szDst;
			}
		}
	#endif // _WIN32
	
	// パス名
	if( pSce[ 0 ] != _T( '\0' ) && pSce[ 1 ] == _T( ':' ))
		pPath = pSce + 2;
	else
		pPath = pSce;
	
	// ノード名
	if( pNode = _tcsrchr( pPath, _T( '\\' ))) ++pNode;
	else pNode = pPath;
	
	// 拡張子
	if( !( pExt = _tcsrchr( pNode, _T( '.' )))) pExt = _tcschr( pNode, _T( '\0' ));
	
	if( szDst == NULL ){
		if( uMode & STF_PATH )	return(( TCHAR *)ZeroToNull( pPath ));
		if( uMode & STF_NODE )	return(( TCHAR *)ZeroToNull( pNode ));
		if( uMode & STF_EXT )	return(( TCHAR *)ZeroToNull( pExt ));
		return(( TCHAR *)pSce );
	}
	
	// szDst にコピー
	// StrCpy_fromP2P の arg[ 1 ] は破壊される
	if(( uMode & ( STF_DRV | STF_PATH | STF_NODE | STF_EXT )) == 0 ){
		_tcscpy( pDst, pSce );
	}else{
		if( uMode & STF_DRV )  StrCpy_fromP2P( pDst, pSce,  pPath );
		if( uMode & STF_PATH ) StrCpy_fromP2P( pDst, pPath, pNode );
		if( uMode & STF_NODE ) StrCpy_fromP2P( pDst, pNode, pExt );
		
		*pDst = _T( '\0' );
		
		if( uMode & STF_EXT )  _tcscpy( pDst, pExt );
	}
	
	return( ZeroToNull( szDst ));
}


/*** cdir を指定したフルパス名取得 ******************************************/
// szCDir はファイルでもよい

TCHAR *GetFullPathWithCDir( TCHAR *szDst, const TCHAR *szFileName, const TCHAR *szCDir ){
	
	TCHAR	szCWDir[ MAX_PATH ];
	
	_tgetcwd( szCWDir, MAX_PATH );	// カレント dir
	
	// szCDir の dir 部分取得
	StrTokFile( szDst, szCDir, STF_PATH2 | STF_LONG | STF_FULL );
	
	if( _tchdir( szDst ) != 0 ) return NULL;				// CDir に cd
	StrTokFile( szDst, szFileName, STF_FULL | STF_LONG );	// szFileName の FullPath
	_tchdir( szCWDir );										// pwd を元に戻す
	
	return szDst;
}

/*** IsExt ******************************************************************/
/* ファイル名 vs 拡張子の比較	szExt には '.' を含まない */

BOOL IsExt( const TCHAR *szFileName, const TCHAR *szExt ){
	
	TCHAR *pExt;
	
	return(
		( pExt = SearchExt( szFileName )) == NULL ?
			szExt == NULL :
			!_tcsicmp( ++pExt, szExt )
	);
}

/*** change ext *************************************************************/
/* szExt に . は付けない */
/* szExt == NULL のときは，拡張子を削除 */

TCHAR *ChangeExt( TCHAR *szDst, TCHAR *szFileName, TCHAR *szExt ){
	
	StrTokFile( szDst, szFileName, STF_PATH2 | STF_NODE );
	return(
		( szExt )	? _tcscat( _tcscat( szDst, _T( "." )), szExt )
					: szDst
	);
}

/*** 最後尾に \ を付加 ******************************************************/

TCHAR *StrCatPathChar( TCHAR *szPath ){
	
	TCHAR *p;
	static const TCHAR szPathChar[] = _T( "\\" );
	
	if( *szPath == _T( '\0' )) return( _tcscpy( szPath, szPathChar ));
	
	p = _tcschr( szPath, _T( '\0' )) - 1;
	if( *p != _T( '\\' )) _tcscpy( ++p, szPathChar );
	
	return( szPath );
}

/*** string の shift ********************************************************/
/* iCnt > 0 で右シフト，< 0 で左シフト */

TCHAR *StrShift( TCHAR *szBuf, int iCnt ){
	
	TCHAR *p;
	
	if( iCnt > 0 ){
		for( p = _tcschr( szBuf, _T( '\0' )); p >= szBuf; --p ) p[ iCnt ] = *p;
		
	}else if( iCnt < 0 ){
		
		iCnt = -iCnt;
		
		if( _tcslen( szBuf ) > ( UINT )iCnt )
			for( p = szBuf; *p = p[ iCnt ]; ++p );
		else
			*szBuf = _T( '\0' );
	}
	
	return( szBuf );
}

/*** "quote" されてなかったら "quote" ***************************************/

TCHAR *StrQuote( TCHAR *szBuf ){
	
	if( *szBuf != _T( '\"' )){
		StrShift( szBuf, 1 );
		*szBuf = _T( '\"' );
	}
	
	if( _tcschr( szBuf, _T( '\0' ))[ -1 ] != _T( '\"' )) _tcscat( szBuf, _T( "\"" ));
	return( szBuf );
}

/*** remove \n code *********************************************************/

TCHAR *StrRemoveCR( TCHAR *szBuf ){
	
	TCHAR *p;
	
	if( p = _tcschr( szBuf, _T( '\n' ))) *p = _T( '\0' );
	return( szBuf );
}

/*** 非破壊的 get param *****************************************************/
/*
	"" で囲まれたものは，"" は除去される
	pNextParam 先頭のブランクは除去される
	これ以上ないときは，*szDst == '\0' && ret == NULL
	szDst == NULL 時は，*ppBuf 内の文字列を返す
*/
TCHAR *StrGetParam(		// )
	TCHAR *szDst,		// get した param string の格納先
	TCHAR **ppBuf		// buffer への pp
){
	BOOL	bQuoted	= FALSE;
	TCHAR	*pDst	= szDst;
	TCHAR	*pSce	= *ppBuf;		// get 元の string
	
	SkipBlank( pSce );
	if( !szDst ) szDst = pSce;		// NULL 時は *ppBuf 内の文字列を返す
	
	while(( bQuoted || !IsBlank( *pSce ))&& *pSce != _T( '\n' ) && *pSce != _T( '\0' )){
		if( *pSce == _T( '\"' )){
			bQuoted = !bQuoted;
			++pSce;
		}else if( pDst ){
			*pDst++ = *pSce++;
		}else{
			++pSce;
		}
	}
	
	if( pDst ) *pDst = _T( '\0' );
	
	SkipBlank( pSce );
	*ppBuf = pSce;
	return( ZeroToNull( szDst ));
}

/*** バッファ内から特定のデータ列を探す *************************************/
/*
	pBuf1 内の見つけた ptr を返す
	なければ NULL
*/
TCHAR *StrSearchMem(		// )
	TCHAR	*pBuf1,		// この中から探し出す
	TCHAR	*pBuf2,		// 探したいデータ
	UINT	uBufSize,	// pBuf1 サイズ
	UINT	uLen		// pBuf2 サイズ
){
	UINT	u;
	
	for( u = 0; u <= uBufSize - uLen; ++u ){
		if( pBuf1[ u ] == *pBuf2 && memcmp( pBuf1 + u, pBuf2, uLen ) == 0 ){
			return( pBuf1 + u );
		}
	}
	return( NULL );
}

/*** %hoge% 形式の環境変数を展開 ********************************************/
/*
	%%hoge%% でも %hoge% があれば置換可能
	% の書式が不十分なもの，環境変数がない場合は置換しない
	ret : 置換できた環境変数の数
*/

int StrReplaceEnv( TCHAR *szDst, TCHAR *szSrc ){
	
	TCHAR	*pEnvName = NULL;
	TCHAR	*pEnvValue;
	int		iReplaceCnt = 0;
	
	for( ; *szSrc; ++szSrc ){
		
		*szDst++ = *szSrc;
		
		if( *szSrc == _T( '%' )){
			// 環境変数始めの %
			if( pEnvName == NULL )	pEnvName = szDst;
			else{
				
				// 環境変数終わりの %
				szDst[ -1 ] = _T( '\0' );	// 変数名を \0 ターミネート
				
				if(( pEnvValue = _tgetenv( pEnvName )) != NULL ){
					_tcscpy( pEnvName - 1, pEnvValue );	// 見つかった環境変数をコピー
					szDst = _tcschr( pEnvName, _T( '\0' ));	// 次のポインタ
					
					++iReplaceCnt;
					pEnvName = NULL;
					
				}else{
					szDst[ -1 ] = _T( '%' );	// 置換できなかったので % を元に戻す
					pEnvName = szDst;	// 終わりだと思っていたのを始めにする
				}
			}
		}
	}
	*szDst = _T( '\0' );
	return( iReplaceCnt );
}
