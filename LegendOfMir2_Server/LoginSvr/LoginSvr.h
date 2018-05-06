#pragma once

#define PACKET_KEEPALIVE				"%++$"

#define LOGPARAM_STR			1
#define LOGPARAM_INT			2

/************************************************************************/
/* �û���Ϣ����                                                                     */
/************************************************************************/
class CUserInfo
{
public:
	SOCKET		sock;
	TCHAR		szSockHandle[4];

	TCHAR		szUserID[11];				// User ID
	TCHAR		szAddress[20];				// User's local address 
	
	BYTE		btPayMode;
	
	int			nClientVersion;
	int			nCertification;
	BOOL		fVersionAccept;
	BOOL		fSelServerOk;

	int			nServerID;
};

/************************************************************************/
/* ��½���ض���                                                                     */
/************************************************************************/
class CGateInfo
{
public:
	SOCKET					sock;

	CWHList<CUserInfo*>		xUserInfoList;
	
	// For Overlapped I/O
	OVERLAPPED				Overlapped;
	WSABUF					DataBuf;
	CHAR					Buffer[DATA_BUFSIZE];
	int						bufLen;

	CWHQueue				g_SendToGateQ;

public:
	/************************************************************************/
	/* ���͵���½����                                                                     */
	/************************************************************************/
	void	SendToGate(SOCKET cSock, char *pszPacket);
	/************************************************************************/
	/* ����һ���û�                                                                     */
	/************************************************************************/
	void	ReceiveSendUser(char *pszPacket);
	/************************************************************************/
	/* �ر�һ���û�                                                                     */
	/************************************************************************/
	void	ReceiveCloseUser(char *pszPacket);
	/************************************************************************/
	/* ��һ���û�                                                                     */
	/************************************************************************/
	void	ReceiveOpenUser(char *pszPacket);
	/************************************************************************/
	/* �������ݿ��������Ϣ                                                                     */
	/************************************************************************/
	void	ReceiveServerMsg(char *pszPacket);
	/************************************************************************/
	/* �½�һ���û�                                                                     */
	/************************************************************************/
	void	MakeNewUser(char *pszPacket);
	/************************************************************************/
	/* �����û�ʵ�����                                                                     */
	/************************************************************************/
	bool	ParseUserEntry( char *buf, _AUSERENTRYINFO *userInfo );
	/************************************************************************/
	/* ��������û�                                                                     */
	/************************************************************************/
	void	ProcAddUser(SOCKET s, char *pszData);
	/************************************************************************/
	/* �����½�û�                                                                     */
	/************************************************************************/
	void	ProcLogin(SOCKET s, char *pszData);
	/************************************************************************/
	/* ����ѡ�������                                                                     */
	/************************************************************************/
	void	ProcSelectServer(SOCKET s, WORD wServerIndex);

	void	Close();
	/************************************************************************/
	/* ������                                                                     */
	/************************************************************************/
	__inline void SendKeepAlivePacket() { send(sock, PACKET_KEEPALIVE, sizeof(PACKET_KEEPALIVE), 0); }

	CGateInfo()
	{
		bufLen	= 0;
	}

	int  Recv()
	{
		DWORD nRecvBytes = 0, nFlags = 0;

		DataBuf.len = DATA_BUFSIZE - bufLen;
		DataBuf.buf = Buffer + bufLen;

		memset( &Overlapped, 0, sizeof( Overlapped ) );

		return WSARecv( sock, &DataBuf, 1, &nRecvBytes, &nFlags, &Overlapped, 0 );
	}

	bool HasCompletionPacket()
	{
		return memchr( Buffer, '$', bufLen ) ? true : false;
	}

	// �ӽ��յ��Ļ���������ȡ��Ϣ
	char * ExtractPacket( char *pPacket )
	{
		int packetLen = (char *) memchr( Buffer, '$', bufLen ) - Buffer + 1;

		memcpy( pPacket, Buffer, packetLen );

		memmove( Buffer, Buffer + packetLen, DATA_BUFSIZE - packetLen );
		bufLen -= packetLen;

		return pPacket + packetLen;
	}
};

/************************************************************************/
/* ��Ϸ����������ʵ��                                                                     */
/************************************************************************/
struct GAMESERVERINFO
{
	int  index;
	char name[50];
	char ip[50];
	int  connCnt;	// ���� �� ����
};

// ���ط�����IP��Ϣ
struct GATESERVERINFO
{
	char ip[50];
};


typedef struct tag_TSENDBUFF
{
	SOCKET			sock;
	char			szData[1024];
}_TSENDBUFF, *_LPTSENDBUFF;

void InsertLogMsg(UINT nID);
void InsertLogMsg(LPTSTR lpszMsg);
void InsertLogMsgParam(UINT nID, void *pParam, BYTE btFlags = LOGPARAM_STR);
