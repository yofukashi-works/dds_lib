/*****************************************************************************

		cmd_opt.h -- command line option parser header file
		Copyright(C) 2000 by D.D.S

*****************************************************************************/

/*** constant definition ****************************************************/

enum{									// �I�v�V�����^�C�v
	CMDOPT_TYPE_INT,					// �����l
	CMDOPT_TYPE_STR,					// ������
	CMDOPT_TYPE_STR_NONULL,				// ��k��������
	
	CMDOPT_TYPE_MASK		= 0x7F,		// type mask �l
	CMDOPT_TYPE_SPECIFIED	= 0x80		// �w�肳�ꂽ�t���O
};


enum{									// �G���[�^�C�v
	CMDOPT_ERR_SUCCESS = 0,				// ����
	CMDOPT_ERR_UNKNOWN,					// �I�v�V�����������Ȃ�
	CMDOPT_ERR_NOSTR					// �����񂪎擾�ł��Ȃ�
};

/*** new type ***************************************************************/

typedef struct {
	char	cOptChar,		// �I�v�V�������� ignore case ���͏������Ŏw��
			cType;			// �I�v�V�����E�ϐ��^�C�v
	void	*pVar;			// �ϐ��ւ̃|�C���^
} CMDOPT_LIST;

typedef void ( *CMDOPT_ERR_PROC )(	// �G���[�n���h���֐�
	char,							// �G���[�����̃I�v�V����
	UINT );							// �G���[�R�[�h

/*** extern & prototype *****************************************************/

UINT GetCmdLineOption( char *, CMDOPT_LIST *, char **, CMDOPT_ERR_PROC );
BOOL IsCmdLineOptSpecified( void *, CMDOPT_LIST * );
