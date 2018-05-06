#pragma once

#define PACKET_KEEPALIVE				"%++$"

#define LOGPARAM_STR			1
#define LOGPARAM_INT			2

/************************************************************************/
/* 用户信息对象                                                                     */
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
/* 登陆网关对象                                                                     */
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
	/* 发送到登陆网关                                                                     */
	/************************************************************************/
	void	SendToGate(SOCKET cSock, char *pszPacket);
	/************************************************************************/
	/* 发送一个用户                                                                     */
	/************************************************************************/
	void	ReceiveSendUser(char *pszPacket);
	/************************************************************************/
	/* 关闭一个用户                                                                     */
	/************************************************************************/
	void	ReceiveCloseUser(char *pszPacket);
	/************************************************************************/
	/* 打开一个用户                                                                     */
	/************************************************************************/
	void	ReceiveOpenUser(char *pszPacket);
	/************************************************************************/
	/* 接收数据库服务器消息                                                                     */
	/************************************************************************/
	void	ReceiveServerMsg(char *pszPacket);
	/************************************************************************/
	/* 新建一个用户                                                                     */
	/************************************************************************/
	void	MakeNewUser(char *pszPacket);
	/************************************************************************/
	/* 解析用户实体对象                                                                     */
	/************************************************************************/
	bool	ParseUserEntry( char *buf, _AUSERENTRYINFO *userInfo );
	/************************************************************************/
	/* 处理添加用户                                                                     */
	/************************************************************************/
	void	ProcAddUser(SOCKET s, char *pszData);
	/************************************************************************/
	/* 处理登陆用户                                                                     */
	/************************************************************************/
	void	ProcLogin(SOCKET s, char *pszData);
	/************************************************************************/
	/* 处理选择服务器                                                                     */
	/************************************************************************/
	void	ProcSelectServer(SOCKET s, WORD wServerIndex);

	void	Close();
	/************************************************************************/
	/* 心跳包                                                                     */
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

	// 从接收到的缓冲区中提取消息
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
/* 游戏服务器对象实体                                                                     */
/************************************************************************/
struct GAMESERVERINFO
{
	int  index;
	char name[50];
	char ip[50];
	int  connCnt;	// 立加 荐 包府
};

// 网关服务器IP信息
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
