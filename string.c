/*****************************************************************************

		string.c -- dds string library
		Copyright(C) 1997 - 2000 by Deyu Deyu Software

2001.02.04	StrTokFile : �Y�� str ���Ȃ��Ƃ��� NULL ��Ԃ� ( szDst == NULL�� )
2001.03.29	StrGetParam : szDst == NULL �̎��̓|�C���^�����Ԃ�
2002.02.04	WIN32 �ŗL������؂藣���\�ɂ���
			StrQuote : " ���Ȃ��Ƃ����� " ��t������
			StrExpandEnv �ǉ�

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
	filename operation �̐^�œo��
	szDst == NULL �̂Ƃ��́CszFileName ���� sub string ��Ԃ�
	�Y���������Ȃ���΁Cret = NULL
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
			// full path �ϊ�
			if( cMode & STF_FULL ){
				if(( dwLen = GetFullPathName( pSce, MAX_PATH, szDst, NULL )) && dwLen < MAX_PATH )
					pSce = szDst;
			}
			
			// LFN �ϊ�
			if( cMode & STF_LONG ){
				if(( dwLen = GetLongPathName( pSce, szDst, MAX_PATH )) && dwLen < MAX_PATH )
					pSce = szDst;
				
			// SFN �ϊ�
			}else if( cMode & STF_SHORT ){
				if(( dwLen = GetShortPathName( pSce, szDst, MAX_PATH )) && dwLen < MAX_PATH )
					pSce = szDst;
			}
		}
	#endif // _WIN32
	
	// �p�X��
	if( pSce[ 0 ] != '\0' && pSce[ 1 ] == ':' )
		pPath = pSce + 2;
	else
		pPath = pSce;
	
	// �m�[�h��
	if( pNode = strrchr( pPath, '\\' )) ++pNode;
	else pNode = pPath;
	
	// �g���q
	if( !( pExt = strrchr( pNode, '.' ))) pExt = strchr( pNode, '\0' );
	
	if( szDst == NULL ){
		if( cMode & STF_PATH )	return( ZeroToNull( pPath ));
		if( cMode & STF_NODE )	return( ZeroToNull( pNode ));
		if( cMode & STF_EXT )	return( ZeroToNull( pExt ));
		return( pSce );
	}
	
	// szDst �ɃR�s�[
	// StrCpy_fromP2P �� arg[ 1 ] �͔j�󂳂��
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
/* �t�@�C���� vs �g���q�̔�r	szExt �ɂ� '.' ���܂܂Ȃ� */

BOOL IsExt( char *szFileName, char *szExt ){
	
	char *pExt;
	
	return(
		( pExt = SearchExt( szFileName )) == NULL ?
			szExt == NULL :
			!stricmp( ++pExt, szExt )
	);
}

/*** change ext *************************************************************/
/* szExt �� . �͕t���Ȃ� */
/* szExt == NULL �̂Ƃ��́C�g���q���폜 */

char *ChangeExt( char *szDst, char *szFileName, char *szExt ){
	
	StrTokFile( szDst, szFileName, STF_PATH2 | STF_NODE );
	return(
		( szExt )	? strcat( strcat( szDst, "." ), szExt )
					: szDst
	);
}

/*** �Ō���� \ ��t�� ******************************************************/

char *StrCatPathChar( char *szPath ){
	
	char *p;
	static const char szPathChar[] = "\\";
	
	if( *szPath == '\0' ) return( strcpy( szPath, szPathChar ));
	
	p = strchr( szPath, '\0' ) - 1;
	if( *p != '\\' ) strcpy( ++p, szPathChar );
	
	return( szPath );
}

/*** string �� shift ********************************************************/
/* iCnt > 0 �ŉE�V�t�g�C< 0 �ō��V�t�g */

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

/*** "quote" ����ĂȂ������� "quote" ***************************************/

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

/*** ��j��I get param *****************************************************/
/*
	"" �ň͂܂ꂽ���̂́C"" �͏��������
	pNextParam �擪�̃u�����N�͏��������
	����ȏ�Ȃ��Ƃ��́C*szDst == '\0' && ret == NULL
	szDst == NULL ���́C*ppBuf ���̕������Ԃ�
*/
char *StrGetParam(		// )
	char *szDst,		// get ���� param string �̊i�[��
	char **ppBuf		// buffer �ւ� pp
){
	BOOL	bQuoted	= FALSE;
	char	*pDst	= szDst;
	char	*pSce	= *ppBuf;		// get ���� string
	
	SkipBlank( pSce );
	if( !szDst ) szDst = pSce;		// NULL ���� *ppBuf ���̕������Ԃ�
	
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

/*** �o�b�t�@���������̃f�[�^���T�� *************************************/
/*
	pBuf1 ���̌����� ptr ��Ԃ�
	�Ȃ���� NULL
*/
char *StrSearchMem(		// )
	char	*pBuf1,		// ���̒�����T���o��
	char	*pBuf2,		// �T�������f�[�^
	UINT	uBufSize,	// pBuf1 �T�C�Y
	UINT	uLen		// pBuf2 �T�C�Y
){
	UINT	u;
	
	for( u = 0; u <= uBufSize - uLen; ++u ){
		if( pBuf1[ u ] == *pBuf2 && memcmp( pBuf1 + u, pBuf2, uLen ) == 0 ){
			return( pBuf1 + u );
		}
	}
	return( NULL );
}

/*** %hoge% �`���̊��ϐ���W�J ********************************************/
/*
	%%hoge%% �ł� %hoge% ������Βu���\
	% �̏������s�\���Ȃ��́C���ϐ����Ȃ��ꍇ�͒u�����Ȃ�
	ret : �u���ł������ϐ��̐�
*/

int StrReplaceEnv( char *szDst, char *szSrc ){
	
	char	*pEnvName = NULL;
	char	*pEnvValue;
	int		iReplaceCnt = 0;
	
	for( ; *szSrc; ++szSrc ){
		
		*szDst++ = *szSrc;
		
		if( *szSrc == '%' ){
			// ���ϐ��n�߂� %
			if( pEnvName == NULL )	pEnvName = szDst;
			else{
				
				// ���ϐ��I���� %
				szDst[ -1 ] = '\0';	// �ϐ����� \0 �^�[�~�l�[�g
				
				if(( pEnvValue = getenv( pEnvName )) != NULL ){
					strcpy( pEnvName - 1, pEnvValue );	// �����������ϐ����R�s�[
					szDst = strchr( pEnvName, '\0' );	// ���̃|�C���^
					
					++iReplaceCnt;
					pEnvName = NULL;
					
				}else{
					szDst[ -1 ] = '%';	// �u���ł��Ȃ������̂� % �����ɖ߂�
					pEnvName = szDst;	// �I��肾�Ǝv���Ă����̂��n�߂ɂ���
				}
			}
		}
	}
	*szDst = '\0';
	return( iReplaceCnt );
}
