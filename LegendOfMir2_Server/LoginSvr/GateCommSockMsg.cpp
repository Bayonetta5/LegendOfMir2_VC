#include "stdafx.h"

extern HWND								g_hStatusBar;

extern SOCKET							g_gcSock;

UINT WINAPI ThreadFuncForMsg(LPVOID lpParameter);

extern HANDLE							g_hIOCP;
extern BOOL								g_fTerminated;

CWHList<CGateInfo*>						g_xGateList;
CWHList<GAMESERVERINFO*>				g_xGameServerList;
CWHList<GATESERVERINFO*>				g_xGateServerList;
char									g_szServerList[1024];

unsigned long							g_hThreadForMsg = 0;

void UpdateStatusBar(BOOL fGrow)
{
	static long	nNumOfCurrSession = 0;

	TCHAR	szText[20];

	(fGrow ? InterlockedIncrement(&nNumOfCurrSession) : InterlockedDecrement(&nNumOfCurrSession));
	
	wsprintf(szText, _TEXT("%d Sessions"), nNumOfCurrSession);

	SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(3, 0), (LPARAM)szText);
}

BOOL InitServerThreadForMsg()
{
	UINT	dwThreadIDForMsg = 0;

	if (!g_hThreadForMsg)
	{
		g_hThreadForMsg	= _beginthreadex(NULL, 0, ThreadFuncForMsg,	NULL, 0, &dwThreadIDForMsg);

		if (g_hThreadForMsg)
			return TRUE;
	}

	return FALSE;
}

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int					nLen = sizeof(SOCKADDR_IN);

	SOCKET				Accept;
	SOCKADDR_IN			Address;

	while (TRUE)
	{
		Accept = accept(g_gcSock, (struct sockaddr FAR *)&Address, &nLen);

		if (g_fTerminated)
			return 0;

		CGateInfo* pGateInfo = new CGateInfo;

		if (pGateInfo)
		{
			pGateInfo->sock = Accept;

			CreateIoCompletionPort((HANDLE)pGateInfo->sock, g_hIOCP, (DWORD)pGateInfo, 0);

			if (g_xGateList.AddNewNode(pGateInfo))
			{
				int zero = 0;
				
				setsockopt(pGateInfo->sock, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) );

				pGateInfo->Recv();

				UpdateStatusBar(TRUE);

#ifdef _DEBUG
				TCHAR szGateIP[256];
				wsprintf(szGateIP, _T("%d.%d.%d.%d"), Address.sin_addr.s_net, Address.sin_addr.s_host, 
															Address.sin_addr.s_lh, Address.sin_addr.s_impno);

				InsertLogMsgParam(IDS_ACCEPT_GATESERVER, szGateIP);
#endif
			}
		}
	}

	return 0;
}

void CloseGate(CGateInfo* pGateInfo)
{
}

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	DWORD					dwBytesTransferred = 0;
	CGateInfo*				pGateInfo = NULL;
	LPOVERLAPPED			lpOverlapped = NULL;
	char					szTmp[DATA_BUFSIZE];

	while (TRUE)
	{
		if ( GetQueuedCompletionStatus(
									(HANDLE)CompletionPortID, 
									&dwBytesTransferred, 
									(LPDWORD)&pGateInfo, 
									(LPOVERLAPPED *)&lpOverlapped, 
									INFINITE) == FALSE )
		{
			return 0;
		}

		//如果已经关闭服务器
		if (g_fTerminated)
		{
			PLISTNODE		pListNode;

			if (g_xGateList.GetCount())
			{
				pListNode = g_xGateList.GetHead();

				while (pListNode)
				{
					pGateInfo = g_xGateList.GetData(pListNode);

					if (pGateInfo)
						pGateInfo->Close();

					delete pGateInfo;
					pGateInfo = NULL;

					pListNode = g_xGateList.RemoveNode(pListNode);
				}
			}

			return 0;
		}
		
		//如果socket发生io异常
		if ( dwBytesTransferred == 0 )
		{
			pGateInfo->Close();
			continue;
		}

		pGateInfo->bufLen += dwBytesTransferred;

		//校验是否存在完成的数据包
		while ( pGateInfo->HasCompletionPacket() )
		{
			*(pGateInfo->ExtractPacket( szTmp ) - 1) = '\0';

			switch ( szTmp[1] )
			{
				case '-':
					pGateInfo->SendKeepAlivePacket();//登陆网关与登陆服务器的心跳包
					break;
				case 'A':
					pGateInfo->ReceiveSendUser(&szTmp[2]);//接收发送用户
					break;
				case 'O':
					pGateInfo->ReceiveOpenUser(&szTmp[2]);//打开一个用户
					break;
				case 'X':
					pGateInfo->ReceiveCloseUser(&szTmp[2]);//关闭一个用户
					break;
				case 'S':
					pGateInfo->ReceiveServerMsg(&szTmp[2]);//选择服务器相关消息
					break;
				case 'M':
					pGateInfo->MakeNewUser(&szTmp[2]);//创建一个新的用户
					break;
			}
		}

		if ( pGateInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING )
		{
			InsertLogMsg(_TEXT("WSARecv() failed"));
			continue;
		}
	}

	return 0;
}
