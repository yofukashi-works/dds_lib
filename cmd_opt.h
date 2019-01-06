/*****************************************************************************

		cmd_opt.h -- command line option parser header file
		Copyright(C) 2000 by D.D.S

*****************************************************************************/

/*** constant definition ****************************************************/

enum{									// オプションタイプ
	CMDOPT_TYPE_INT,					// 整数値
	CMDOPT_TYPE_STR,					// 文字列
	CMDOPT_TYPE_STR_NONULL,				// 非ヌル文字列
	
	CMDOPT_TYPE_MASK		= 0x7F,		// type mask 値
	CMDOPT_TYPE_SPECIFIED	= 0x80		// 指定されたフラグ
};


enum{									// エラータイプ
	CMDOPT_ERR_SUCCESS = 0,				// 成功
	CMDOPT_ERR_UNKNOWN,					// オプション文字がない
	CMDOPT_ERR_NOSTR					// 文字列が取得できない
};

/*** new type ***************************************************************/

typedef struct {
	TCHAR	cOptChar,		// オプション文字 ignore case 時は小文字で指定
			cType;			// オプション・変数タイプ
	void	*pVar;			// 変数へのポインタ
} CMDOPT_LIST;

typedef void ( *CMDOPT_ERR_PROC )(	// エラーハンドラ関数
	TCHAR,							// エラー原因のオプション
	UINT );							// エラーコード

/*** extern & prototype *****************************************************/

UINT GetCmdLineOption( TCHAR *, CMDOPT_LIST *, TCHAR **, CMDOPT_ERR_PROC );
BOOL IsCmdLineOptSpecified( void *, CMDOPT_LIST * );
