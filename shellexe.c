/*****************************************************************************

		shellexe.c -- ���Ɛ� shell execute
		Copyright(C) 2000-'01 by D.D.S

2000.04.01	%L �� %1 �Ɠ��ꎋ
2000.07.08	$1 $* ���g�p�\��
2000.08.25	GetRegStr: (�W��) �̒l���Ȃ������������� "" �����^�[�� ( NT �΍� )
2001.02.03	Magic ID �̃T�|�[�g�� #! �����ɂ���
			�t�@�C���擪 SEARCH_LEN ���� Magic ID2 �� Magic ID �Ƃ݂Ȃ�
2002.01.01	HKCR\.ext �� (�W��) ���Ȃ��Ƃ��� Unknown ���g�p����
2002.02.04	%hoge% ��u������
2003.02.21	���� StartupInfo, uOption �ǉ�
2005.03.10	uOption �̔���� & ����Ȃ��� | �ɂȂ��Ă��Q|�P|��
2005.03.14	ShellExecute() �� Scripter ���ĂԂ� MAGIC_ID �� Hit ���Ă��Q|�P|��
2005.05.21	EXECFILE_CHDIR ���w�肵�Ȃ��Ƃ� szPath ���s�肾����

*****************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "dds.h"
#include "dds_lib.h"

/*** macros *****************************************************************/

#define IsLFN( s )	( strchr( s, ' ' ) || strchr( s, '\t' ))

/*** const definition *******************************************************/

#define BUFSIZE		1024			// �o�b�t�@�T�C�Y
#define SEARCH_LEN	( 1024 * 64 )	// #! ���T�[�`����͈�

#define EXECDIR_FIRST_FILE			// current dir �� %1 �� dir �ɕύX
#define RETURN_EXEC_CMD				// szExecCmd �� szCmdLine �ɕԂ�

#define MAGIC_ID	"#!"
#define MAGIC_ID2	"#Executable" MAGIC_ID

/*** get a string from REGISTORY ********************************************/

char *GetRegStr(
	HKEY	hKey,			// HKEY_*
	char	*pSubKey,		// sub key name
	char	*pValueName,	// member name
	char	*szDst			// output buffer
){
	
	HKEY	hSubKey;					/* for getting default action		*/
	DWORD	dwBufSize = BUFSIZE;		/* data buf size for RegQueryValue	*/
	
	if( RegOpenKeyEx( hKey, pSubKey, 0, KEY_READ, &hSubKey )== ERROR_SUCCESS ){
		LONG lResult = RegQueryValueEx(
			hSubKey, pValueName, NULL, NULL,
			( LPBYTE )szDst, &dwBufSize
		);
		RegCloseKey( hSubKey );
		
		// ���Ȃ��I�������C���Ȃ����^�[��
		if( lResult == ERROR_SUCCESS ) return( szDst );
		
		// (�W��) �̒l���Ȃ�������������C"" �����^�[�� ( NT �΍� )
		if( pValueName == NULL ){
			*szDst = '\0';
			return( szDst );
		}
	}
	*szDst = '\0';
	return( NULL );
}

/*** reg ����W���A�N�V�����R�}���h��ǂ� �Ȃ���� Unknown\open *************/

char *GetDefaultAction( char *szAction, char *szFileName ){
	
	char	szVerb[ BUFSIZE ],
			*pExt;
	
	do{
		// �g���q���擾
		if(( pExt = SearchExt( szFileName )) == NULL ) break;
		
		// HKCR\.ext �̒l���擾
		GetRegStr( HKEY_CLASSES_ROOT, pExt, NULL, szAction );
		if( *szAction == '\0' ) break;
		
		// szAction = HKCR\extfile + \shell\ (^^;
		strcat( szAction, "\\shell\\" );
		
		// HKCR\extfile\shell\(�W��) ���擾
		GetRegStr( HKEY_CLASSES_ROOT, szAction, NULL, szVerb );
		
		// HKCR\extfile\shell\szVerb\command	�Ȃ����
		// HKCR\extfile\shell\open\command
		strcat( strcat( szAction, *szVerb ? szVerb : "open" ), "\\command" );
		
		// ���̒l���擾
		GetRegStr( HKEY_CLASSES_ROOT, szAction, NULL, szAction );
		if( *szAction != '\0' ) return( szAction );
		
	}while( 0 );
	
	// �g���q���Ȃ� or ���s�\�ȃA�N�V�������擾�ł��Ȃ�����
	strcpy( szAction, "Unknown\\shell\\edit\\command" );
	return( GetRegStr( HKEY_CLASSES_ROOT, szAction, NULL, szAction ));
}

/*** �^����ꂽ�t�@�C���� shell esc cmd ��Ԃ� �Ȃ���� NULL ****************/

char *GetShellEscAction( char *szAction, char *szFileName ){
	
	FILE	*fp;
	char	*pCmdLineParam,
			*p,
			szBuf		[ SEARCH_LEN ],
			szCmdFile	[ BUFSIZE ];
	
	static char	szMagicID[]  = MAGIC_ID,
				szMagicID2[] = MAGIC_ID2;
	
	// open ���s�� NULL return
	if(( fp = fopen( szFileName, "rb" )) == NULL ) return( NULL );
	
	// ��s�ǂ�
	fgets( szBuf, SEARCH_LEN, fp );
	
	// search #! /hogehoge/....
	if( p = strstr( szBuf, szMagicID )){
		p += sizeof( szMagicID ) - 1;
		
	}else if(
		fread( szBuf, 1, SEARCH_LEN, fp ),
		p = StrSearchMem( szBuf, szMagicID2, SEARCH_LEN, sizeof( szMagicID2 ) - 1 )
	){
		p += sizeof( szMagicID2 ) - 1;
	}
	
	fclose( fp );
	
	// shell esc ��������Ȃ�
	if( p == NULL || *p == '\0' ) return( NULL );
	
	/*** shell esc ���������� *********************************************/
	
	// p �̒���� 0x0D / 0x0A �� 0 �ɂ���
	
	pCmdLineParam = p;
	if( p = strchr( pCmdLineParam, 0xD )) *p = '\0';
	if( p = strchr( pCmdLineParam, 0xA )) *p = '\0';
	
	// ���s�t�@�C������؂�o��
	StrGetParam( szCmdFile, &pCmdLineParam );
	
	// / �� \ �ϊ�
	for( p = szCmdFile; *p != '\0'; ++p ) if( *p == '/' ) *p = '\\';
	
	// cmd line param �ƌ������Ċ���!!
	sprintf( szAction,
		( *pCmdLineParam != '\0' ) ? "\"%s\" %s" : "\"%s\"",
		szCmdFile, pCmdLineParam );
	
	return( szAction );
}

/*** get default action of the file type ************************************/
/*
szCmdLine �� %1 �����ʂ��ēK�؂Ȏ��s�t�@�C�������s����D
%1 �̃V�F���G�X�P�[�v���F������
%1 �̃p�X�� cd ���Ă�����s����
�߂�l�� CreateProcess �̂���
*/
BOOL ExecuteFile(
	char				*szCmdLine,
	STARTUPINFO			*pStartupInfo,
	PROCESS_INFORMATION	*ppiRet,
	UINT				uOption
){
	
	char	szFirstParam[ BUFSIZE ],	// %1
			*pOtherParam,				// %*
			szAction[ BUFSIZE ],		// %1 %* �����܂ރR�}���h
			szExecCmd[ BUFSIZE ],		// ���ۂɎ��s���� cmd + param
			szPath[ BUFSIZE ],			// %1 �̃p�X
			*pAction,
			*pExecCmd;
	
	BOOL	bRet;
	
	// for CreateProcess
	
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	
	// �A�N�V���� get!!
	pOtherParam = szCmdLine;
	StrGetParam( szFirstParam, &pOtherParam );
	
	// Esc sequence
	if( GetShellEscAction( szAction, szFirstParam ) == NULL ){
		
		// �g���q�����Ȃ���s�`���̊m�F������
		if(
			!SearchExt( szFirstParam ) &&
			( int )FindExecutable( szFirstParam, NULL, szExecCmd ) > 32 ){
			
			strcpy( szFirstParam, szExecCmd );
		}
		if( GetDefaultAction( szAction, szFirstParam ) == NULL )
			strcpy( szAction, "start" );	// ����[ (^^;
	}
	
	// ���ϐ���u��
	// szExecCmd �� tmp �I�Ɏg���Ă�D�ŁC���ۂɒu�����ꂽ���̂�
	// ����Ƃ��ɂ��� szAction �ɏ����߂��Ă���
	if( StrReplaceEnv( szExecCmd, szAction )) strcpy( szAction, szExecCmd );
	
	// %1 %* �������Ă�ꍇ�͂����ǉ�
	if( !strstr( szAction, "%1" ) &&
		!strstr( szAction, "%L" ) &&
		!strstr( szAction, "$1" )) strcat( szAction, " \"%1\"" );
	
	if( !strstr( szAction, "%*" ) &&
		!strstr( szAction, "$*" )) strcat( szAction, " %*" );
	
	// %1 ��LFN �� "%1" �łȂ������Ƃ��̑΍�
	if( IsLFN( szFirstParam ) && strstr( szAction, "\"%1\"" ) == NULL ){
		
		strcpy( szExecCmd, szFirstParam );
		GetShortPathName( szExecCmd, szFirstParam, BUFSIZE );
	}
	
	// %? �̒u��
	pAction  = szAction;
	pExecCmd = szExecCmd;
	
	do{
		if( *pAction == '%' || *pAction == '$' ){
			switch( *++pAction ){
			  case '1':
			  case 'L':
				strcpy( pExecCmd, szFirstParam );
				pExecCmd += strlen( szFirstParam );
				
			  Case '*':
				strcpy( pExecCmd, pOtherParam );
				pExecCmd += strlen( pOtherParam );
				
			  Default:
				*pExecCmd++ = pAction[ -1 ];
				*pExecCmd++ = *pAction;
			}
		}else{
			// % �ȊO�̕����͂��̂܂܃R�s�[
			*pExecCmd++ = *pAction;
		}
	}while( *pAction++ );
	
	// �s���̃X�y�[�X�폜
	for( pExecCmd = szExecCmd + strlen( szExecCmd ) - 1;
		IsBlank( *pExecCmd ) && pExecCmd >= szExecCmd;
		--pExecCmd ){
		
		*pExecCmd = '\0';
	}
	
	// ���s������
	
	if( pStartupInfo ){
		si = *pStartupInfo;
	}else{
		GetStartupInfo( &si );
	}
	
	if( uOption & EXECFILE_CHDIR ){
		StrTokFile( szPath, szFirstParam, STF_DRV | STF_PATH );
	}else{
		*szPath = '\0';
	}
	
	// ���悢����s
	DebugMsgD( "CreateProcess >>%s<<\n", szExecCmd );
	
	bRet = CreateProcess(
		NULL, szExecCmd,				/* exec file, params				*/
		NULL, NULL,						/* securities						*/
		FALSE,							/* inherit flag						*/
		NORMAL_PRIORITY_CLASS,			/* creation flags					*/
		NULL,							/* env								*/
		( uOption | EXECFILE_CHDIR ) && *szPath ? szPath : NULL,
										/* current dir.						*/
		&si, &pi
	);
	
	// ���s����?
	if( bRet ){
		// ppiRet �Ɋi�[�v�����o�Ă��邩?
		if( ppiRet != NULL ){
			
			// �i�[�D�n���h�������̂� caller �̐ӔC
			*ppiRet = pi;
		}else{
			
			// �n���h���́C����Ȃ��C����Ȃ��D
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}
	}
	
	if( uOption & EXECFILE_RET_CMD ) strcpy( szCmdLine, szExecCmd );
	
	return( bRet );
}
