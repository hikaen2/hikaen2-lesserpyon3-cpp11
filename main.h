#ifndef _MAIN_H_
#define _MAIN_H_

#include <algorithm>
#include <stdio.h>

#include "kyokumen.h"

#define MAX_DEPTH 32
#define VAL_INFINITE  999999

enum {
	EXACTLY_VALUE,		// �l�͋ǖʂ̕]���l���̂���
	LOWER_BOUND,		// �l�͋ǖʂ̕]���l�̉����l(val>=��)
	UPPER_BOUND			// �l�͋ǖʂ̕]���l�̏���l(val<=��)
};


class HashEntry {
public:
	uint64 HashVal;		// �n�b�V���l
	Te Best;			// �O��̔����[���ł̍őP��
	Te Second;			// �O�X��ȑO�̔����[���ł̍őP��
	int value;			// �����T���œ����ǖʂ̕]���l
	int flag;			// �����T���œ����l���A�ǖʂ̕]���l���̂��̂��A����l�������l��
	int Tesu;			// �����T�����s�����ۂ̎萔
	short depth;		// �����T�����s�����ۂ̐[��
	short remainDepth;	// �����T�����s�����ۂ̎c��[��
};


// �v�l���[�`���ł��B
class Sikou {
protected:
	static HashEntry HashTbl[1024*1024];	// 20bit�̑傫���A40M�o�C�g
	int MakeMoveFirst(int SorE,int depth,Te teBuf[],KyokumenKomagumi k);
public:
	// �Ƃ肠�����A���炩�̋ǖʂ�^���Ď��Ԃ��֐��ł��B
	Te Think(int SorE,KyokumenKomagumi k);
	// ��S�͂Œǉ��B����[���ł̍őP���ێ�����B
	Te Best[MAX_DEPTH][MAX_DEPTH];
	int MinMax(int SorE,KyokumenKomagumi &k,int depth,int depthMax);
	int AlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax);
	int NegaAlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax,bool bITDeep=true);
	int ITDeep(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax);	// NegaAlphaBeta��p���������[��
};
#endif
