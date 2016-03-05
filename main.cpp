#include "main.h"
#include <stdlib.h>
#include <time.h>

#ifdef _WINDOWS
#include <winsock.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

HashEntry Sikou::HashTbl[1024*1024];

int Sikou::MakeMoveFirst(int SorE,int depth,Te teBuf[],KyokumenKomagumi k)
{
	int teNum=0;
	if (HashTbl[k.HashVal & 0xfffff].HashVal!=k.HashVal) {
		return 0;
	}
	if (HashTbl[k.HashVal & 0xfffff].Tesu%2!=k.Tesu%2) {
		// ��Ԃ��Ⴄ�B
		return 0;
	}

	// �ǖʂ���v�����Ǝv����
	Te te=HashTbl[k.HashVal & 0xfffff].Best;
	if (!te.IsNull()) {
		if (k.IsLegalMove(SorE,te)) {
			teBuf[teNum++]=te;
		}
	}
	te=HashTbl[k.HashVal & 0xfffff].Second;
	if (!te.IsNull()) {
		if (k.IsLegalMove(SorE,te)) {
			teBuf[teNum++]=te;
		}
	}
	if (depth>1) {
		te=Best[depth-1][depth];
		if (!te.IsNull() && k.IsLegalMove(SorE,te)) {
			teBuf[teNum++]=te;
		}
	}
	return teNum;
}

Te Stack[32];	// 32�ƌ��������͓K���B�ő�[�������̒��x�܂ł����s���Ȃ����Ƃ����҂��Ă���B

// ��S�͂Œǉ��B�����@�ɂ��T���B
// ����́A���̃A���S���Y������{�ɐi�߂Ă����B
int Sikou::NegaAlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax,bool bITDeep)
{
	if (depth==1) {
		// �����`�F�b�N
		int sennitite=0;
		for(int i=k.Tesu;i>0;i-=2) {
			if (k.HashHistory[i]==k.HashVal) {
				sennitite++;
			}
		}
		if (sennitite>=4) {
			// �����
			sennitite=0;
			// �A������̐����`�F�b�N
			int i;
			for(i=k.Tesu;sennitite<=3&&i>0;i-=2) {
				if (!Kyokumen::OuteHistory[i]) {
					break;
				}
				if (k.HashHistory[i]==k.HashVal) {
					sennitite++;
				}
			}
			if (sennitite==4) {
				// �A������̐������������Ă���
				return VAL_INFINITE;
			}
			sennitite=0;
			for(i=k.Tesu;sennitite<=3&&i>1;i-=2) {
				if (!Kyokumen::OuteHistory[i-1]) {
					break;
				}
				if (k.HashHistory[i]==k.HashVal) {
					sennitite++;
				}
			}
			if (sennitite==4) {
				// �A������̐����������Ă���
				return -VAL_INFINITE;
			}
			return 0;
		}
	}
	if (depth==depthMax) {
		int value=k.Evaluate()+k.BestEval(SorE);
		// �����̎�Ԃ��猩�����_��Ԃ�
		if (SorE==SELF) {
			return value;
		} else {
			return -value;
		}
	}
	if (HashTbl[k.HashVal & 0x0fffff].HashVal==k.HashVal) {
		HashEntry e=HashTbl[k.HashVal & 0x0fffff];
		if (e.value>=beta && e.Tesu>=k.Tesu && e.Tesu%2==k.Tesu%2 && e.depth<=depth && e.remainDepth>=depthMax-depth && e.flag!=UPPER_BOUND) {
			return e.value;
		}
		if (e.value<=alpha && e.Tesu>=k.Tesu && e.Tesu%2==k.Tesu%2 && e.depth<=depth && e.remainDepth>=depthMax-depth && e.flag!=LOWER_BOUND) {
			return e.value;
		}
	} else if (depthMax-depth>2 && bITDeep) {
		// ���߂ĖK�ꂽ�ǖʂŁA�[�����c���Ă���̂ő��d�����[�����s���B
		return ITDeep(SorE,k,alpha,beta,depth,depthMax);
	}
	Te teBuf[600];
	int retval=-VAL_INFINITE-1;
	if (depth<2 && k.Mate(SorE,7,teBuf[0])==1) {
		Best[depth][depth]=teBuf[0];
		Best[depth][depth+1]=Te(0);
		retval=VAL_INFINITE+1;
		goto HashAdd;
	}
	int teNum;
	if (depth==0) {
		teNum=MakeMoveFirst(SorE,depth,teBuf,k);
	} else {
		teNum=MakeMoveFirst(SorE,depth,teBuf,k);
	}
	int i;
	k.EvaluateTe(SorE,teNum,teBuf);
	for(i=0;i<teNum;i++) {
		KyokumenKomagumi kk(k);
		if (teBuf[i].IsNull()) {
			continue;
		}
		Stack[depth]=teBuf[i];
		kk.Move(SorE,teBuf[i]);
		int v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax);
		// �w����̕]�����}�C�i�X�̎肪���O�Ɏw����Ă��āA��������肪���l���X�V���Ȃ��悤�Ȃ�A
		// �ǂ݂�[�����ēǂݒ���
		if (depth>1 && Stack[depth-1].value<0 && Stack[depth-1].to==Stack[depth].to && v<=retval) {
			v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax+2);
		}
		if (v>retval) {
			retval=v;
			Best[depth][depth]=teBuf[i];
			for(int i=depth+1;i<depthMax;i++) {
				Best[depth][i]=Best[depth+1][i];
			}
			if (depth==0) {
				printf("retval:%d,te:",retval);
				for(int j=0;j<depthMax;j++) {
					Best[0][j].Print();
				}
				printf("\n");
			}
			if (retval>=beta) {
				goto HashAdd;
			}
		}
	}
	teNum=k.MakeLegalMoves(SorE,teBuf);
	if (teNum==0) {
		// ����
		return -VAL_INFINITE;
	}
	k.EvaluateTe(SorE,teNum,teBuf);
	for(i=0;i<teNum;i++) {
		if ((teBuf[i].value<-100 || i>30) && i>0 && retval>-VAL_INFINITE) {
			break;
		}
		KyokumenKomagumi kk(k);
		Stack[depth]=teBuf[i];
		kk.Move(SorE,teBuf[i]);
		int v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax);
		// �w����̕]�����}�C�i�X�̎肪���O�Ɏw����Ă��āA��������肪���l���X�V���Ȃ��悤�Ȃ�A
		// �ǂ݂�[�����ēǂݒ���
		if (depth>1 && Stack[depth-1].value<0 && Stack[depth-1].to==Stack[depth].to && v<=beta) {
			v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax+2);
		}
		if (v>retval) {
			retval=v;
			Best[depth][depth]=teBuf[i];
			for(int i=depth+1;i<depthMax;i++) {
				Best[depth][i]=Best[depth+1][i];
			}
			if (depth==0) {
				printf("retval:%d,te:",retval);
				for(int j=0;j<depthMax;j++) {
					Best[0][j].Print();
				}
				printf("\n");
			}
			if (retval>=beta) {
				goto HashAdd;
			}
		}
	}
HashAdd:
	HashEntry e;
	e=HashTbl[k.HashVal & 0x0fffff];
	if (e.HashVal==k.HashVal) {
		e.Second=e.Best;
	} else {
		if (e.Tesu-e.depth==k.Tesu-depth && e.remainDepth>depthMax-depth) {
			// �n�b�V���ɂ���f�[�^�̕����d�v�Ȃ̂ŏ㏑�����Ȃ�
			if (depth==0) {
				k.Print();
				Best[depth][depth].Print();
			}
			goto NotAdd;
		}
		e.HashVal=k.HashVal;
		e.Second=Te(0);
	}
	if (retval>alpha) {
		e.Best=Best[depth][depth];
	} else {
		e.Best=Te(0);
	}
	e.value=retval;
	if (retval<=alpha) {
		e.flag=UPPER_BOUND;
	} else if (retval>=beta) {
		e.flag=LOWER_BOUND;
	} else {
		e.flag=EXACTLY_VALUE;
	}
	e.depth=depth;
	e.remainDepth=depthMax-depth;
	e.Tesu=k.Tesu;
	HashTbl[k.HashVal & 0x0fffff]=e;
NotAdd:
	return retval;
}

int Sikou::ITDeep(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax)
{
	int retval;
	int i;
	for(i=depth+1;i<=depthMax;i++) {
		retval=NegaAlphaBeta(SorE,k,alpha,beta,depth,i,false);
	}
	return retval;
}

Joseki joseki("public.bin");
Kyokumen *shoki;


// �{�i�I�ɐ�ǂ݂�����v�l���[�`���ɂȂ�܂����B
Te Sikou::Think(int SorE,KyokumenKomagumi k)
{
	int teNum;
	Te teBuf[600];
	int hindo[600];
	int i,j;
	joseki.fromJoseki(*shoki,SELF,k,k.Tesu,teNum,teBuf,hindo);
	// ��ԕp�x�̍�����Ղ�I�ԁB
	if (teNum>0) {
		int max,maxhindo;
		max=0;
		maxhindo=hindo[max];
		for(i=1;i<teNum;i++) {
			if (hindo[i]>maxhindo) {
				maxhindo=hindo[i];
				max=i;
			}
		}
		return teBuf[max];
	}
	TsumeHash::Clear();

	// depthMax�͓K���Ɏc�莞�Ԃɍ��킹�Ē�������Ȃǂ̍H�v���K�v�ł��B
	int depthMax=5;
	for(i=0;i<MAX_DEPTH;i++) {
		for(j=0;j<MAX_DEPTH;j++) {
			Best[i][j]=Te(0);
		}
	}
	int bestVal=ITDeep(SorE,k,-VAL_INFINITE+1,VAL_INFINITE-1,0,depthMax);
//	int bestVal=NegaAlphaBeta(SorE,k,-VAL_INFINITE+1,VAL_INFINITE-1,0,depthMax);
	printf("bestVal:%d,te:",bestVal);
	for(i=0;i<depthMax;i++) {
		Best[0][i].Print();
	}
	printf("\n");

	return Best[0][0];
}

// �񍇖@�Ȏ肩�ǂ������肷��֐��ł��B
bool IsIllegal(Te te,int teNum,Te *teBuf)
{
	// �v����ɁA��̈ꗗ�̒��ɂ�������A
	for(int i=0;i<teNum;i++) {
		if (te==teBuf[i]) {
			// Illegal�ł͂Ȃ��A�Ƃ������Ƃ�false��Ԃ��܂��B
			return false;
		}
	}
	// ��̈ꗗ�̒��ɂȂ���́A��@�Ȏ聁�w���Ă͂����Ȃ���ł��B
	return true;
}

void usage()
{
	fprintf(stderr,"shogi CPU|HUMAN|LAN CPU|HUMAN|LAN [User Password [Server [Port]]]\n");
	fprintf(stderr,"User and Password must set if match with LAN.\n");
	fprintf(stderr,"Server default is usapyon.dip.jp, Port default is 4081\n");
}

enum Teban {
	CPU,
	HUMAN,
	LAN
};

char *tebanStr[]={
	"CPU",
	"HUMAN",
	"LAN"
};

Teban teban[2];

Te TeHistory[1000];
char *CSAKomaStr[]={
//  �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� ��
	"","","","","","","","","","","","","","","","","",
//   ��   ��   �j   ��   ��   �p   ��   ��   ��   ��   �\   �S  ��  �n   ��  ��
	"FU","KY","KE","GI","KI","KA","HI","OU","TO","NY","NK","NG","","UM","RY","",
//   ��   ��   �j   ��   ��   �p   ��   ��   ��   ��   �\   �S  ��  �n   ��
	"FU","KY","KE","GI","KI","KA","HI","OU","TO","NY","NK","NG","","UM","RY"
};

int ByouHistory[1000];

#ifdef _WINDOWS
SOCKET s;
#else 
int s;
#endif

void CsaSend(char *buf)
{
	int ret;
	int n = strlen(buf);
	printf("send:%s",buf);
	ret=send(s, buf, n, 0);
	if (ret<0) {
		fprintf(stderr,"CSASend:SOCKET ERROR:SOCK NOT OPEN?\n");
		exit(1);
	}
}

void CsaRecv(char *buf)
{
	int sum = 0,nRecv;
	char c[1];

	for (;;) {
		// 1byte���ǂݎ��B'\n'������܂�
		nRecv = recv( s, c, 1, 0);
		if ( nRecv < 0 ) {
			fprintf(stderr,"CSARecv:SOCKET ERROR:SOCK NOT OPEN?\n");
			exit(1);
		}
		buf[sum++] = c[0];
		if (c[0]=='\n') {
			break;
		}
	}
	buf[sum++]='\0';
	printf("recv:%s",buf);
}

// ���{�̃��C���ł��B
int main(int argc,char *argv[])
{
	long start=clock();
	Kyokumen::HashInit();
	int SikouJikanTotal[2]={0,0};

	int i;
	int j;
	if (argc<3) {
		usage();
		exit(1);
	}
	for(i=1;i<=2;i++) {
		for(j=CPU;j<=LAN;j++) {
			if (strcmp(argv[i],tebanStr[j])==0) {
				break;
			}
		}
		teban[i-1]=(Teban)j;
		if (j==3) {
			usage();
			exit(1);
		}
	}
	char User[256];
	char Password[256];
	if (argc>=5) {
		strcpy(User,argv[3]);
		strcpy(Password,argv[4]);
	} else {
		strcpy(User,"");
		strcpy(Password,"");
	}
	char Server[256];
	if (argc>=6) {
		strcpy(Server,argv[5]);
	} else {
		strcpy(Server,"usapyon.dip.jp");
	}
	int Port;
	if (argc>=7) {
		Port=atoi(argv[6]);
	} else {
		Port=4081;
	}
	if (teban[0]==LAN && teban[1]==LAN) {
		fprintf(stderr,"Can't match LAN against LAN.\n");
		usage();
		exit(1);
	}
	if (teban[0]==LAN || teban[1]==LAN) {
		if (User[0]=='\0') {
			usage();
			exit(1);
		}
		// Socket���J��
#ifdef _WINDOWS
		WORD version=0x0101;
		WSADATA WSAData;
		WSAStartup(version,&WSAData);
#endif
		// �\�P�b�g�\�z
		s=socket(PF_INET,SOCK_STREAM,0);
		if (s<0) {
			fprintf(stderr,"Can't Create Socket.\n");
			exit(1);
		}
		// �ڑ���T�[�o������
		struct hostent *host=gethostbyname(Server);
		if (host==NULL) {
			fprintf(stderr,"Can't Find Server '%s'.\n",Server);
			exit(1);
		}
		// �ڑ��iconnect)
#ifdef _WINDOWS
		SOCKADDR_IN Addr;
#else
		struct sockaddr_in Addr;
#endif
		memset(&Addr, 0,sizeof(Addr));
		Addr.sin_family = AF_INET;
		Addr.sin_port = htons(Port);
		memcpy((char *)&Addr.sin_addr, (char *)host->h_addr,host->h_length);
		int rtn = connect(s, (sockaddr *)&Addr, sizeof(Addr));
		if (rtn<0) {
			fprintf(stderr,"Can't Connect to Server '%s' Port %d.\n",Server,Port);
			exit(1);
		}
		// User,Password��Login����
		char buf[256];
		sprintf(buf,"LOGIN %s %s\n",User,Password);
		CsaSend(buf);
		for(;;) {
			CsaRecv(buf); // LOGIN OK����������͂��B
			char tmp[256];
			sprintf(tmp,"LOGIN:%s OK\n",User);
			if (strcmp(buf,tmp)==0) {
				printf("���O�C������");
				break;
			} else {
				printf("���O�C���Ɏ��s���܂����B\r\n����: %s\n", buf);
#ifdef _WINDOWS
				closesocket(s);
#else
				close(s);
#endif
				exit(1);
			}
		}
		for(;;) {
			CsaRecv(buf);
			if (strcmp(buf,"Your_Turn:+\n")==0) {
				if (teban[0]==LAN) {
					swap(teban[0],teban[1]);
				}
			}
			if (strcmp(buf,"Your_Turn:-\n")==0) {
				if (teban[1]==LAN) {
					swap(teban[0],teban[1]);
				}
			}
			if (strcmp(buf,"END Game_Summary\n")==0) {
				break;
			}
		}
		CsaSend("AGREE\n");
		CsaRecv(buf);	// START������͂��B
	}
	
	// ����̏����z�u�ł��B���₷���ł���H�ϊ��͂��̕����G�ł����ǁB
	KomaInf HirateBan[9][9]={
		{EKY,EKE,EGI,EKI,EOU,EKI,EGI,EKE,EKY},
		{EMP,EHI,EMP,EMP,EMP,EMP,EMP,EKA,EMP},
		{EFU,EFU,EFU,EFU,EFU,EFU,EFU,EFU,EFU},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{SFU,SFU,SFU,SFU,SFU,SFU,SFU,SFU,SFU},
		{EMP,SKA,EMP,EMP,EMP,EMP,EMP,SHI,EMP},
		{SKY,SKE,SGI,SKI,SOU,SKI,SGI,SKE,SKY}
	};
	// ������͖ʓ|�ł�EHI�܂�0����ׂȂ��Ƃ����܂���B
	int HirateMotigoma[EHI+1]={
	// ����������������������j����p�򉤂ƈǌ\�S���n��������j����p��
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

	// �O��ڂŁA����̋ǖʂŁA������Ȃ�����J�n���܂��傤�B
	KyokumenKomagumi Hirate(0,HirateBan,HirateMotigoma);
	KyokumenKomagumi k(Hirate);
	KyokumenKomagumi::InitKanagomaValue();
	shoki=new Kyokumen(0,HirateBan,HirateMotigoma);
	k.Initialize();

	// ����͂܂��ȒP�Ȏv�l���Ȃ̂ŁA���������ȒP�ł��B
	Sikou sikou;

	// �����̋ǖʂŁA�ő�̎萔�͂T�V�X�肾�����ł��B
	Te teBuf[600];
	int teNum;

	// ��O�̃v���C���[����J�n���܂��B
	int SorE=SELF;

#ifdef _GCC_
	time_t temp;
	srand(time(&temp));
#else
	long temp;
	srand(time(&temp));
#endif

	int bSennitite=false;

	// ���������@�肪�Ȃ��Ȃ�����A�l�݁������ł��B
	// ���@�肪����Ԃ̓Q�[�����p�����܂��B
	// ��
	// �u�����v�ɂ��A���@�肪����ꍇ�ł������ɂȂ邱�Ƃ�����܂��B
	// �܂��A�u�����v���Ȃ�����I���܂���
	Te te;
	bool bLose=false;
	for(;;) {
		teNum=k.MakeLegalMoves(SorE,teBuf);
		long byouStart=clock();
		k.SenkeiInit();
		k.Print();
		int NowTeban,NextTeban;
		if (SorE==SELF) {
			NowTeban=teban[0];
			NextTeban=teban[1];
		} else {
			NowTeban=teban[1];
			NextTeban=teban[0];
		}
		bool bFirst=true;
		switch (NowTeban) {
			case HUMAN:
				if (teNum==0) {
					te=Te(0);
				} else do {
					if (!bFirst) {
						printf("���͂��ꂽ�肪�ُ�ł��B\n");
					}
					// �����͂��܂��B
					char buf[80];
					gets(buf);
					// ���͂̕��@:from,to,promote
					// �������A����łƂ���from��01�A����łƂ���from��02�c�Ƃ���B
					// promote�́A����Ƃ���*��t����B
					int from,to;
					int koma,capture;
					char promote[2];
					promote[0]='\0';
					if (strcmp(buf,"%TORYO")==0) {
						te=Te(0);
					} else {
						int ss=sscanf(buf,"%02x%02x%1s",&from,&to,&promote);
						if (ss<2) continue;
						if (from<OU) {
							koma=SorE|from;
							from=0;
						} else {
							koma=k.ban[from];
						}
						capture=k.ban[to];
						if (ss=3 && promote[0]=='*') {
							te=Te(from,to,koma,capture,1);
						} else {
							te=Te(from,to,koma,capture,0);
						}
						bFirst=false;
					}
				} while (IsIllegal(te,teNum,teBuf) && !te.IsNull());
				break;
			case CPU:
				te=sikou.Think(SorE,k);
				break;
			case LAN:
				{
					char buf[256];
					char komaStr[3];
					char c;
					CsaRecv(buf);	// ����̎w���肪�A���Ă���B�i�������w����ł��邱�Ƃ̓T�[�o�Ń`�F�b�N�ς݁j
					if (strcmp(buf,"%TORYO\n")==0) {
						te=Te(0);
					} else if (strcmp(buf,"#WIN\n")==0) {
						te=Te(0);
					} else if (strcmp(buf,"#TIME_UP\n")==0) {
						te=Te(0);
					} else if (strcmp(buf,"#LOSE\n")==0) {
						// �������錾�����B
						bLose=true;
						break;
					} else {
						sscanf(buf,"%c%02x%02x%2s,T%d\n",&c,&te.from,&te.to,komaStr,&ByouHistory[k.Tesu]);
						int i;
						for(i=0;i<=RY;i++) {
							if (strcmp(komaStr,CSAKomaStr[i|SELF])==0) {
								break;
							}
						}
						te.koma=i|SorE;
						te.promote=0;
						te.capture=k.ban[te.to];
						te.value=0;
						if (te.from!=0 && k.ban[te.from]!=te.koma) {
							te.promote=1;
							te.koma=k.ban[te.from];
						}
					}
				}
				break;
		}
		if (NextTeban==LAN) {
			// ���̎�𑗂�
			if (te.IsNull()) {
				CsaSend("%TORYO\n");
				char buf[256];
				CsaRecv(buf);
				CsaRecv(buf);
			} else {
				char buf[256];
				char komaStr[3];
				char c;
				int from,to;
				sprintf(buf,"%c%02x%02x%s\n",SorE==SELF?'+':'-',te.from,te.to,CSAKomaStr[te.koma|(te.promote?PROMOTED:0)]);

				CsaSend(buf);
				CsaRecv(buf);	// �w���肪�A���Ă���B
				if (strcmp(buf,"#TIME_UP\n")==0) {
					// ���Ԑ؂�
					CsaRecv(buf);
				}
				if (strcmp(buf,"#LOSE\n")==0) {
					// ���Ԑ؂ꕉ���B
					bLose=true;
					break;
				}
				sscanf(buf,"%c%02x%02x%2s,T%d\n",&c,&from,&to,komaStr,&ByouHistory[k.Tesu]);
			}
		}
		TeHistory[k.Tesu]=te;
		if (teban[0]!=LAN && teban[1]!=LAN) {
			// ���O�Ōv������B
			ByouHistory[k.Tesu]=(clock()-byouStart)/CLOCKS_PER_SEC;
			if (ByouHistory[k.Tesu]==0) {
				// �Œ�P�b�͕K���J�E���g����
				ByouHistory[k.Tesu]=1;
			}
		}
		te.Print();
		k.Move(SorE,te);
		int sennitite=0;
		int i;
		for(i=k.Tesu;i>0;i-=2) {
			if (k.HashHistory[i]==k.HashVal) {
				sennitite++;
			}
		}
		if (sennitite>=4) {
			bSennitite=true;
			break;
		}
		if (SorE==SELF) {
			SikouJikanTotal[0]+=ByouHistory[k.Tesu-1];
			SorE=ENEMY;
		} else {
			SikouJikanTotal[1]+=ByouHistory[k.Tesu-1];
			SorE=SELF;
		}
		printf("\n���v�l����:��� %2d:%02d ���%2d:%02d\n",
			SikouJikanTotal[0]/60,SikouJikanTotal[0]%60,
			SikouJikanTotal[1]/60,SikouJikanTotal[1]%60
		);
		if (te.IsNull() || bLose) {
			break;
		}
	}
	k.Print();
	if (SorE==SELF && te.IsNull()) {
		printf("���̏����B\n");
	} else if (SorE==ENEMY && te.IsNull()) {
		printf("���̏����B\n");
	} else if (SorE==SELF && (teNum==0 || bLose)) {
		printf("���̏����B\n");
	} else if (SorE==ENEMY && (teNum==0 || bLose)) {
		printf("���̏����B\n");
	} else if (bSennitite) {
		// �����ɂ��I��
		// ����̐����̔���
		printf("�����ł��B\n");
		int sennitite=0;
		if (Kyokumen::OuteHistory[k.Tesu]) {
			for(int i=k.Tesu;sennitite<=3&&i>0;i-=2) {
				if (!Kyokumen::OuteHistory[i]) {
					break;
				}
				if (k.HashHistory[i]==k.HashVal) {
					sennitite++;
				}
			}
			if (sennitite==4) {
				// �A������̐����
				printf("�A������̐����ł��B\n");
				if (SorE==SELF) { 
					printf("���̏����B\n");
				} else {
					printf("���̏����B\n");
				}
			}
		} else if (Kyokumen::OuteHistory[k.Tesu-1]) {
			// ������͖�����
			for(int i=k.Tesu;sennitite<=3&&i>0;i-=2) {
				if (!Kyokumen::OuteHistory[i-1]) {
					break;
				}
				if (k.HashHistory[i]==k.HashVal) {
					sennitite++;
				}
			}
			if (sennitite==4) {
				// �A������̐����
				printf("�A������̐����ł��B\n");
				if (SorE==SELF) {
					printf("���̏����B\n");
				} else {
					printf("���̏����B\n");
				}
			}
		}
	}
	FILE *fp=fopen("log.csa","w");
	if (fp==NULL) {
		fprintf(stderr,"log.csa�ɏ������݂ł��܂���B");
	} else {
		fprintf(fp,"N+\n");
		fprintf(fp,"N-\n");
		fprintf(fp,"P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n");
		fprintf(fp,"P2 * -HI *  *  *  *  * -KA * \n");
		fprintf(fp,"P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n");
		fprintf(fp,"P4 *  *  *  *  *  *  *  *  * \n");
		fprintf(fp,"P5 *  *  *  *  *  *  *  *  * \n");
		fprintf(fp,"P6 *  *  *  *  *  *  *  *  * \n");
		fprintf(fp,"P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n");
		fprintf(fp,"P8 * +KA *  *  *  *  * +HI * \n");
		fprintf(fp,"P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n");
		fprintf(fp,"+\n");
		for(i=0;i<k.Tesu;i++) {
			if (!TeHistory[i].IsNull()) {
				if (i%2==0) {
					fprintf(fp,"+");
				} else {
					fprintf(fp,"-");
				}
				fprintf(fp,"%02x%02x%s\n",TeHistory[i].from,TeHistory[i].to,CSAKomaStr[TeHistory[i].koma|(TeHistory[i].promote?PROMOTED:0)]);
				fprintf(fp,"T%d\n",ByouHistory[i]);
			} else {
				fprintf(fp,"%%TORYO\n");
			}
		}
		fclose(fp);
	}
	if (teban[0]==LAN || teban[1]==LAN) {
		CsaSend("LOGOUT\n");
#ifdef _WINDOWS
		closesocket(s);
#else
		close(s);
#endif
	}
	printf("%.3lfs",(double(clock()-start))/CLOCKS_PER_SEC);
	return 0;
}
 
