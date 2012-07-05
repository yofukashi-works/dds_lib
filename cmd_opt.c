/*****************************************************************************

		cmd_opt.c -- command line option parser
		Copyright(C) 2000 by D.D.S

2001.02.11	opt �� "-opt" �N�H�[�g����Ă����Ƃ� opt �ƔF�����Ȃ�����
2002.02.04	Win32 ���Ƃ����łȂ��Ƃ� DebugMsg �̎��̂�؂�ւ���

*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
#else
 #include <stdio.h>
#endif
#include <stdlib.h>

#include "dds.h"
#include "dds_lib.h"
#include "cmd_opt.h"

/*** macros *****************************************************************/

#define CMDOPT_DEF_SEP_CHAR		'-'		// �I�v�V�����E�p�����[�^�E�Z�p���[�^
//#define CMDOPT_DEF_BREAK_ON_ERR		// �G���[�����������^�[��
//#define CMDOPT_DEF_IGNORE_CASE		// �啶���E���������ꎋ
//#define CMDOPT_DEF_SWITCH_DEFAULT		// opt �l�ȗ����̓f�t�H���g�𔽓]

#define SetIfNotNull( p, v ) if(( p )!= NULL ) *( p ) = ( v );

#define OPT_CHAR( u )		( opt[ u ].cOptChar )
#define OPT_VAR_TYPE( u )	( opt[ u ].cType & CMDOPT_TYPE_MASK )
#define OPT_PVAR( u )		( opt[ u ].pVar )

#ifdef _WIN32
 #define DEBUG_MSG DebugMsgD
#else
 #define DEBUG_MSG DebugMsg
#endif

/*** nop function for error handler proc ************************************/

static void ErrProcNop( char cOptChar, UINT uErrCode ){}

/*** get command line option ************************************************/
/*
�R�}���h���C���I�v�V������ǂ�
���������� 0 ��Ԃ�
���s������ pOtherParam �ɂ��̃|�C���^��Ԃ�
pOtherParam �́C�擪�̃u�����N�͏��������
*/

UINT GetCmdLineOption(
	char			*szCmdLine,			// �R�}���h���C��������
	CMDOPT_LIST		*opt,				// �I�v�V�������X�g
	char			**pOtherParam,		// opt �ȊO�̎c��̕���������� ptr
	CMDOPT_ERR_PROC ErrorProc ){		// �G���[�n���h���֐�
	
	char	*p = szCmdLine;
	
	UINT	uRet = CMDOPT_ERR_SUCCESS,	// �߂�l
			u;
	
	// �G���[�n���h���ݒ�
	
	if( ErrorProc == NULL ) ErrorProc = ErrProcNop;
	
	// �J�n
	
	for(;;){
		SkipBlank( p );
		
		// �p�����[�^�擪�� '-' �ȊO�Ȃ�Copt ��͏I���D
		
		if( *p == '\"' && p[ 1 ] == CMDOPT_DEF_SEP_CHAR ) ++p;
		else if( *p != CMDOPT_DEF_SEP_CHAR ) break;
		
		// �p�����[�^�� 1��荞�� //////////////////////////////////////////
		
		++p;
		while( *p != '\0' && *p != ' ' && *p != '\t' ){
			
			if( *p == '\"' ){
				++p;
				continue;
			}
			
			// �I�v�V�������� *p �����X�g����T�[�` //////////////////////////
			
			for( u = 0; OPT_CHAR( u ) != '\0'; ++u ){
				
#				ifdef CMDOPT_DEF_IGNORE_CASE
					if( OPT_CHAR( u ) == tolower( *p )) break;
					
#				else	// CMDOPT_DEF_IGNORE_CASE
					if( OPT_CHAR( u ) == *p ) break;
					
#				endif	// CMDOPT_DEF_IGNORE_CASE
			}
			
			// unknown opt ��? ///////////////////////////////////////////////
			
			if( OPT_CHAR( u ) == '\0' ){
				ErrorProc( *p, CMDOPT_ERR_UNKNOWN );
				
#				ifdef CMDOPT_DEF_BREAK_ON_ERR
					SetIfNotNull( pOtherParam, p );
					return( CMDOPT_ERR_UNKNOWN );
					
#				else	// CMDOPT_DEF_BREAK_ON_ERR
					++p;
#				endif	//CMDOPT_DEF_BREAK_ON_ERR
				
			}else{
				// option type ���̏��� //////////////////////////////////////
				++p;
				opt[ u ].cType |= CMDOPT_TYPE_SPECIFIED;
				
				switch( OPT_VAR_TYPE( u )){
				  case CMDOPT_TYPE_INT: /*** INT �^ *************************/
					if( isdigit( *p )){
						*( int *)OPT_PVAR( u ) = strtol( p, &p, 10 );
					}else{
#						ifdef CMDOPT_DEF_SWITCH_DEFAULT
							*( int *)OPT_PVAR( u ) =
								( *( int *)OPT_PVAR( u ) == 0 ) ? 1 : 0;
							
#						else	// CMDOPT_DEF_SWITCH_DEFAULT
							*( int *)OPT_PVAR( u ) = 1;
							
#						endif	// CMDOPT_DEF_SWITCH_DEFAULT
					}
				  Case CMDOPT_TYPE_STR: /*** ������^ ***********************/
				  case CMDOPT_TYPE_STR_NONULL:
					StrGetParam(( char *)OPT_PVAR( u ), &p );
					
					if( OPT_VAR_TYPE( u ) == CMDOPT_TYPE_STR_NONULL &&
						*( char *)OPT_PVAR( u ) == '\0' ){
						
						ErrorProc( OPT_CHAR( u ), CMDOPT_ERR_NOSTR );
						
#						ifdef CMDOPT_DEF_BREAK_ON_ERR
							SetIfNotNull( pOtherParam, p );
							return( CMDOPT_ERR_NOSTR );
							
#						else	//CMDOPT_DEF_BREAK_ON_ERR
							opt[ u ].cType &= CMDOPT_TYPE_MASK;
							
#						endif	//CMDOPT_DEF_BREAK_ON_ERR
					}
					goto Brk1Param;
				}
			}
		} // 1�p�����[�^�̏I��
		
	  Brk1Param: ;	// opt char �ȍ~�����ׂĂ��� opt �̊Ǌ��̏ꍇ������jmp
	}
	
	// �����I��
	SetIfNotNull( pOtherParam, p );
	
	// Debug information
	IfDebug{
		DEBUG_MSG( "GetCmdLineOption:report start >>>>\n" );
		
		for( u = 0; OPT_CHAR( u ) != '\0'; ++u ){
			DEBUG_MSG( "'%c' %d ",
				OPT_CHAR( u ),
				( opt[ u ].cType & CMDOPT_TYPE_SPECIFIED ) != 0 );
			
			switch( OPT_VAR_TYPE( u )){
				case CMDOPT_TYPE_INT:	DEBUG_MSG( "%u\n", *( UINT *)OPT_PVAR( u ));
				
				Case CMDOPT_TYPE_STR_NONULL:
				case CMDOPT_TYPE_STR:	DEBUG_MSG( "\"%s\"\n", ( char *)OPT_PVAR( u ));
				
				Default:				DEBUG_MSG( "Unknown var type\n" );
			}
		}
		
		DEBUG_MSG( "GetCmdLineOption:report end   <<<<\n" );
	}
	
	return( uRet );
}

/*** check is option specified? *********************************************/

BOOL IsCmdLineOptSpecified( void *pVar, CMDOPT_LIST *opt ){
	
	UINT	u;
	
	for( u = 0; OPT_CHAR( u ) != '\0'; ++u ){
		if( OPT_PVAR( u ) == pVar )
			return(( opt[ u ].cType & CMDOPT_TYPE_SPECIFIED ) != 0 );
	}
	return( FALSE );
}
