#pragma once

#include "ClientMgr.h"
#include "BaseCommand.h"
#include "thread/KThread.h"

class COrderApp : 
	public ITcpServerNotify,
	public CKThread
{
public:
	COrderApp(void);
	virtual ~COrderApp(void);

	bool Connect( unsigned short usPort = 8905 );
	void Release();

	// From ITcpServerNotify
	virtual void onAccept(unsigned long sessionID);
	virtual void onClosed(unsigned long sessionID);
	virtual void onRecvData(char * data,int len,unsigned long sessionID);

	void OnRecvUdpData( char* pData, int nLen, unsigned long ulIP, unsigned short usPort );

	virtual void ThreadProcMain(void);

	void TransInputCommand( string sCmd);

private:
	CClientMgr m_clientMgr;
	ITcpServer* m_pTcpServer;
	bool m_bWantStop;
};
