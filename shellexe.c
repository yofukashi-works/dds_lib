/*****************************************************************************

		shellexe.c -- 自家製 shell execute
		Copyright(C) 2000-'01 by D.D.S

2000.04.01	%L を %1 と同一視
2000.07.08	$1 $* を使用可能に
2000.08.25	GetRegStr: (標準) の値がないだけだったら "" をリターン ( NT 対策 )
2001.02.03	Magic ID のサポートを #! だけにした
			ファイル先頭 SEARCH_LEN 内の Magic ID2 を Magic ID とみなす
2002.01.01	HKCR\.ext の (標準) がないときは Unknown を使用する
2002.02.04	%hoge% を置換する
2003.02.21	引数 StartupInfo, uOption 追加
2005.03.10	uOption の判定で & じゃなくて | になってた＿|?|○
2005.03.14	ShellExecute() で Scripter を呼ぶと MAGIC_ID が Hit してた＿|?|○
2005.05.21	EXECFILE_CHDIR を指定しないとき szPath が不定だった

*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

#include "dds.h"
#include "dds_lib.h"

/*** macros *****************************************************************/

#define IsLFN( s )	( _tcschr( s, _T( ' ' )) || _tcschr( s, _T( '\t' )))

/*** const definition *******************************************************/

#define BUFSIZE		1024			// バッファサイズ
#define SEARCH_LEN	( 1024 * 64 )	// #! をサーチする範囲

#define EXECDIR_FIRST_FILE			// current dir を %1 の dir に変更
#define RETURN_EXEC_CMD				// szExecCmd を szCmdLine に返す

#define MAGIC_ID	_T( "#!" )
#define MAGIC_ID2	_T( "#Executable" ) MAGIC_ID

/*** get a string from REGISTORY ********************************************/

TCHAR *GetRegStr(
	HKEY	hKey,			// HKEY_*
	TCHAR	*pSubKey,		// sub key name
	TCHAR	*pValueName,	// member name
	TCHAR	*szDst			// output buffer
){
	
	HKEY	hSubKey;					/* for getting default action		*/
	DWORD	dwBufSize = BUFSIZE;		/* data buf size for RegQueryValue	*/
	
	if( RegOpenKeyEx( hKey, pSubKey, 0, KEY_READ, &hSubKey )== ERROR_SUCCESS ){
		LONG lResult = RegQueryValueEx(
			hSubKey, pValueName, NULL, NULL,
			( LPBYTE )szDst, &dwBufSize
		);
		RegCloseKey( hSubKey );
		
		// 問題なく終わったら，問題なくリターン
		if( lResult == ERROR_SUCCESS ) return( szDst );
		
		// (標準) の値がないだけだったら，"" をリターン ( NT 対策 )
		if( pValueName == NULL ){
			*szDst = _T( '\0' );
			return( szDst );
		}
	}
	*szDst = _T( '\0' );
	return( NULL );
}

/*** reg から標準アクションコマンドを読む なければ Unknown\open *************/

TCHAR *GetDefaultAction( TCHAR *szAction, TCHAR *szFileName ){
	
	TCHAR	szVerb[ BUFSIZE ],
			*pExt;
	
	do{
		// 拡張子を取得
		if(( pExt = SearchExt( szFileName )) == NULL ) break;
		
		// HKCR\.ext の値を取得
		GetRegStr( HKEY_CLASSES_ROOT, pExt, NULL, szAction );
		if( *szAction == _T( '\0' )) break;
		
		// szAction = HKCR\extfile + \shell\ (^^;
		_tcscat( szAction, _T( "\\shell\\" ));
		
		// HKCR\extfile\shell\(標準) を取得
		GetRegStr( HKEY_CLASSES_ROOT, szAction, NULL, szVerb );
		
		// HKCR\extfile\shell\szVerb\command	なければ
		// HKCR\extfile\shell\open\command
		_tcscat( _tcscat( szAction, *szVerb ? szVerb : _T( "open" )), _T( "\\command" ));
		
		// ↑の値を取得
		GetRegStr( HKEY_CLASSES_ROOT, szAction, NULL, szAction );
		if( *szAction != _T( '\0' )) return( szAction );
		
	}while( 0 );
	
	// 拡張子がない or 実行可能なアクションが取得できなかった
	_tcscpy( szAction, _T( "Unknown\\shell\\edit\\command" ));
	return( GetRegStr( HKEY_CLASSES_ROOT, szAction, NULL, szAction ));
}

/*** 与えられたファイルの shell esc cmd を返す なければ NULL ****************/

TCHAR *GetShellEscAction( TCHAR *szAction, TCHAR *szFileName ){
	
	FILE	*fp;
	TCHAR	*pCmdLineParam,
			*p,
			szBuf		[ SEARCH_LEN ],
			szCmdFile	[ BUFSIZE ];
	
	static TCHAR	szMagicID[]  = MAGIC_ID,
				szMagicID2[] = MAGIC_ID2;
	
	// open 失敗で NULL return
	if(( fp = _tfopen( szFileName, _T( "rb" ))) == NULL ) return( NULL );
	
	// 一行読む
	fgets( szBuf, SEARCH_LEN, fp );
	
	// search #! /hogehoge/....
	if( p = _tcsstr( szBuf, szMagicID )){
		p += sizeof( szMagicID ) - 1;
		
	}else if(
		fread( szBuf, 1, SEARCH_LEN, fp ),
		p = StrSearchMem( szBuf, szMagicID2, SEARCH_LEN, sizeof( szMagicID2 ) - 1 )
	){
		p += sizeof( szMagicID2 ) - 1;
	}
	
	fclose( fp );
	
	// shell esc が見つからない
	if( p == NULL || *p == _T( '\0' )) return( NULL );
	
	/*** shell esc が見つかった *********************************************/
	
	// p の直後の 0x0D / 0x0A を 0 にする
	
	pCmdLineParam = p;
	if( p = _tcschr( pCmdLineParam, 0xD )) *p = _T( '\0' );
	if( p = _tcschr( pCmdLineParam, 0xA )) *p = _T( '\0' );
	
	// 実行ファイル名を切り出し
	StrGetParam( szCmdFile, &pCmdLineParam );
	
	// / → \ 変換
	for( p = szCmdFile; *p != _T( '\0' ); ++p ) if( *p == _T( '/' )) *p = '\\';
	
	// cmd line param と結合して完成!!
	_stprintf( szAction,
		( *pCmdLineParam != _T( '\0' )) ? _T( "\"%s\" %s" ) : _T( "\"%s\"" ),
		szCmdFile, pCmdLineParam );
	
	return( szAction );
}

/*** get default action of the file type ************************************/
/*
szCmdLine の %1 を識別して適切な実行ファイルを実行する．
%1 のシェルエスケープも認識する
%1 のパスに cd してから実行する
戻り値は CreateProcess のそれ
*/
BOOL ExecuteFile(
	TCHAR				*szCmdLine,
	STARTUPINFO			*pStartupInfo,
	PROCESS_INFORMATION	*ppiRet,
	UINT				uOption
){
	
	TCHAR	szFirstParam[ BUFSIZE ],	// %1
			*pOtherParam,				// %*
			szAction[ BUFSIZE ],		// %1 %* 等を含むコマンド
			szExecCmd[ BUFSIZE ],		// 実際に実行する cmd + param
			szPath[ BUFSIZE ],			// %1 のパス
			*pAction,
			*pExecCmd;
	
	BOOL	bRet;
	
	// for CreateProcess
	
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	
	// アクション get!!
	pOtherParam = szCmdLine;
	StrGetParam( szFirstParam, &pOtherParam );
	
	// Esc sequence
	if( GetShellEscAction( szAction, szFirstParam ) == NULL ){
		
		// 拡張子無しなら実行形式の確認をする
		if(
			!SearchExt( szFirstParam ) &&
			( int )FindExecutable( szFirstParam, NULL, szExecCmd ) > 32 ){
			
			_tcscpy( szFirstParam, szExecCmd );
		}
		if( GetDefaultAction( szAction, szFirstParam ) == NULL )
			_tcscpy( szAction, _T( "start" ));	// きゃー (^^;
	}
	
	// 環境変数を置換
	// szExecCmd は tmp 的に使ってる．で，実際に置換されたものが
	// あるときにだけ szAction に書き戻している
	if( StrReplaceEnv( szExecCmd, szAction )) _tcscpy( szAction, szExecCmd );
	
	// %1 %* が抜けてる場合はそれを追加
	if( !_tcsstr( szAction, _T( "%1" )) &&
		!_tcsstr( szAction, _T( "%L" )) &&
		!_tcsstr( szAction, _T( "$1" ))) _tcscat( szAction, _T( " \"%1\"" ));
	
	if( !_tcsstr( szAction, _T( "%*" )) &&
		!_tcsstr( szAction, _T( "$*" ))) _tcscat( szAction, _T( " %*" ));
	
	// %1 がLFN で _T( "%1" ) でなかったときの対策
	if( IsLFN( szFirstParam ) && _tcsstr( szAction, _T( "\"%1\"" )) == NULL ){
		
		_tcscpy( szExecCmd, szFirstParam );
		GetShortPathName( szExecCmd, szFirstParam, BUFSIZE );
	}
	
	// %? の置換
	pAction  = szAction;
	pExecCmd = szExecCmd;
	
	do{
		if( *pAction == _T( '%' ) || *pAction == _T( '$' )){
			switch( *++pAction ){
			  case _T( '1' ):
			  case _T( 'L' ):
				_tcscpy( pExecCmd, szFirstParam );
				pExecCmd += _tcslen( szFirstParam );
				
			  Case _T( '*' ):
				_tcscpy( pExecCmd, pOtherParam );
				pExecCmd += _tcslen( pOtherParam );
				
			  Default:
				*pExecCmd++ = pAction[ -1 ];
				*pExecCmd++ = *pAction;
			}
		}else{
			// % 以外の文字はそのままコピー
			*pExecCmd++ = *pAction;
		}
	}while( *pAction++ );
	
	// 行末のスペース削除
	for( pExecCmd = szExecCmd + _tcslen( szExecCmd ) - 1;
		IsBlank( *pExecCmd ) && pExecCmd >= szExecCmd;
		--pExecCmd ){
		
		*pExecCmd = _T( '\0' );
	}
	
	// 実行下準備
	
	if( pStartupInfo ){
		si = *pStartupInfo;
	}else{
		GetStartupInfo( &si );
	}
	
	if( uOption & EXECFILE_CHDIR ){
		StrTokFile( szPath, szFirstParam, STF_DRV | STF_PATH );
	}else{
		*szPath = _T( '\0' );
	}
	
	// いよいよ実行
	DebugMsgD( _T( "CreateProcess >>%s<<\n" ), szExecCmd );
	
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
	
	// 実行成功?
	if( bRet ){
		// ppiRet に格納要求が出ているか?
		if( ppiRet != NULL ){
			
			// 格納．ハンドルを閉じるのは caller の責任
			*ppiRet = pi;
		}else{
			
			// ハンドルは，いらない，いらない．
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}
	}
	
	if( uOption & EXECFILE_RET_CMD ) _tcscpy( szCmdLine, szExecCmd );
	
	return( bRet );
}
