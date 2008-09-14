/*****************************************************************************

		string.c -- dds string library
		Copyright(C) 1997 - 2000 by Deyu Deyu Software

2001.02.04	StrTokFile : 該当 str がないときは NULL を返す ( szDst == NULL時 )
2001.03.29	StrGetParam : szDst == NULL の時はポインタだけ返す
2002.02.04	WIN32 固有部分を切り離し可能にした
			StrQuote : " がないときだけ " を付加する
			StrExpandEnv 追加

*****************************************************************************/

#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
#endif
#include <stdlib.h>

#include "dds.h"
#include "dds_lib.h"

/*** macro ******************************************************************/

#define StrCpy_fromP2P( dst, from, to ) \
	while(( from ) < ( to )) ( *( dst )++ = *( from )++ )

/*** strtokfile *************************************************************/
/*
	filename operation の真打登場
	szDst == NULL のときは，szFileName 内の sub string を返す
	該当部分がなければ，ret = NULL
*/

char *StrTokFile( char *szDst, char *szFileName, char cMode ){
	
	char	*pPath,
			*pNode,
			*pExt,
			*pSce = szFileName,
			*pDst = szDst;
	
	DWORD	dwLen;
	
	#ifdef _WIN32
		if( szDst ){
			// full path 変換
			if( cMode & STF_FULL ){
				if(( dwLen = GetFullPathName( pSce, MAX_PATH, szDst, NULL )) && dwLen < MAX_PATH )
					pSce = szDst;
			}
			
			// LFN 変換
			if( cMode & STF_LONG ){
				if(( dwLen = GetLongPathName( pSce, szDst, MAX_PATH )) && dwLen < MAX_PATH )
					pSce = szDst;
				
			// SFN 変換
			}else if( cMode & STF_SHORT ){
				if(( dwLen = GetShortPathName( pSce, szDst, MAX_PATH )) && dwLen < MAX_PATH )
					pSce = szDst;
			}
		}
	#endif // _WIN32
	
	// パス名
	if( pSce[ 0 ] != '\0' && pSce[ 1 ] == ':' )
		pPath = pSce + 2;
	else
		pPath = pSce;
	
	// ノード名
	if( pNode = strrchr( pPath, '\\' )) ++pNode;
	else pNode = pPath;
	
	// 拡張子
	if( !( pExt = strrchr( pNode, '.' ))) pExt = strchr( pNode, '\0' );
	
	if( szDst == NULL ){
		if( cMode & STF_PATH )	return( ZeroToNull( pPath ));
		if( cMode & STF_NODE )	return( ZeroToNull( pNode ));
		if( cMode & STF_EXT )	return( ZeroToNull( pExt ));
		return( pSce );
	}
	
	// szDst にコピー
	// StrCpy_fromP2P の arg[ 1 ] は破壊される
	if(( cMode & ( STF_DRV | STF_PATH | STF_NODE | STF_EXT )) == 0 ){
		strcpy( pDst, pSce );
	}else{
		if( cMode & STF_DRV )  StrCpy_fromP2P( pDst, pSce,  pPath );
		if( cMode & STF_PATH ) StrCpy_fromP2P( pDst, pPath, pNode );
		if( cMode & STF_NODE ) StrCpy_fromP2P( pDst, pNode, pExt );
		
		*pDst = '\0';
		
		if( cMode & STF_EXT )  strcpy( pDst, pExt );
	}
	
	return( ZeroToNull( szDst ));
}

/*** IsExt ******************************************************************/
/* ファイル名 vs 拡張子の比較	szExt には '.' を含まない */

BOOL IsExt( char *szFileName, char *szExt ){
	
	char *pExt;
	
	return(
		( pExt = SearchExt( szFileName )) == NULL ?
			szExt == NULL :
			!stricmp( ++pExt, szExt )
	);
}

/*** change ext *************************************************************/
/* szExt に . は付けない */
/* szExt == NULL のときは，拡張子を削除 */

char *ChangeExt( char *szDst, char *szFileName, char *szExt ){
	
	StrTokFile( szDst, szFileName, STF_PATH2 | STF_NODE );
	return(
		( szExt )	? strcat( strcat( szDst, "." ), szExt )
					: szDst
	);
}

/*** 最後尾に \ を付加 ******************************************************/

char *StrCatPathChar( char *szPath ){
	
	char *p;
	static const char szPathChar[] = "\\";
	
	if( *szPath == '\0' ) return( strcpy( szPath, szPathChar ));
	
	p = strchr( szPath, '\0' ) - 1;
	if( *p != '\\' ) strcpy( ++p, szPathChar );
	
	return( szPath );
}

/*** string の shift ********************************************************/
/* iCnt > 0 で右シフト，< 0 で左シフト */

char *StrShift( char *szBuf, int iCnt ){
	
	char *p;
	
	if( iCnt > 0 ){
		for( p = strchr( szBuf, '\0' ); p >= szBuf; --p ) p[ iCnt ] = *p;
		
	}else if( iCnt < 0 ){
		
		iCnt = -iCnt;
		
		if( strlen( szBuf ) > ( UINT )iCnt )
			for( p = szBuf; *p = p[ iCnt ]; ++p );
		else
			*szBuf = '\0';
	}
	
	return( szBuf );
}

/*** "quote" されてなかったら "quote" ***************************************/

char *StrQuote( char *szBuf ){
	
	if( *szBuf != '\"' ){
		StrShift( szBuf, 1 );
		*szBuf = '\"';
	}
	
	if( strchr( szBuf, '\0' )[ -1 ] != '\"' ) strcat( szBuf, "\"" );
	return( szBuf );
}

/*** remove \n code *********************************************************/

char *StrRemoveCR( char *szBuf ){
	
	char *p;
	
	if( p = strchr( szBuf, '\n' )) *p = '\0';
	return( szBuf );
}

/*** 非破壊的 get param *****************************************************/
/*
	"" で囲まれたものは，"" は除去される
	pNextParam 先頭のブランクは除去される
	これ以上ないときは，*szDst == '\0' && ret == NULL
	szDst == NULL 時は，*ppBuf 内の文字列を返す
*/
char *StrGetParam(		// )
	char *szDst,		// get した param string の格納先
	char **ppBuf		// buffer への pp
){
	BOOL	bQuoted	= FALSE;
	char	*pDst	= szDst;
	char	*pSce	= *ppBuf;		// get 元の string
	
	SkipBlank( pSce );
	if( !szDst ) szDst = pSce;		// NULL 時は *ppBuf 内の文字列を返す
	
	while(( bQuoted || !IsBlank( *pSce ))&& *pSce != '\n' && *pSce != '\0' ){
		if( *pSce == '\"' ){
			bQuoted = !bQuoted;
			++pSce;
		}else if( pDst ){
			*pDst++ = *pSce++;
		}else{
			++pSce;
		}
	}
	
	if( pDst ) *pDst = '\0';
	
	SkipBlank( pSce );
	*ppBuf = pSce;
	return( ZeroToNull( szDst ));
}

/*** バッファ内から特定のデータ列を探す *************************************/
/*
	pBuf1 内の見つけた ptr を返す
	なければ NULL
*/
char *StrSearchMem(		// )
	char	*pBuf1,		// この中から探し出す
	char	*pBuf2,		// 探したいデータ
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

int StrReplaceEnv( char *szDst, char *szSrc ){
	
	char	*pEnvName = NULL;
	char	*pEnvValue;
	int		iReplaceCnt = 0;
	
	for( ; *szSrc; ++szSrc ){
		
		*szDst++ = *szSrc;
		
		if( *szSrc == '%' ){
			// 環境変数始めの %
			if( pEnvName == NULL )	pEnvName = szDst;
			else{
				
				// 環境変数終わりの %
				szDst[ -1 ] = '\0';	// 変数名を \0 ターミネート
				
				if(( pEnvValue = getenv( pEnvName )) != NULL ){
					strcpy( pEnvName - 1, pEnvValue );	// 見つかった環境変数をコピー
					szDst = strchr( pEnvName, '\0' );	// 次のポインタ
					
					++iReplaceCnt;
					pEnvName = NULL;
					
				}else{
					szDst[ -1 ] = '%';	// 置換できなかったので % を元に戻す
					pEnvName = szDst;	// 終わりだと思っていたのを始めにする
				}
			}
		}
	}
	*szDst = '\0';
	return( iReplaceCnt );
}
