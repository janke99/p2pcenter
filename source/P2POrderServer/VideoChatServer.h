#pragma once

#include "ClientMgr.h"
#include "BaseCommand.h"
#include "thread/KThread.h"
#include "../../inc/MediaDefine.h"

#define TIMEOUT_INTERVAL 33000

class CChatClient
{
public:

	CChatClient(void) :
	  m_dwType(0)
	  {
		  Active();
	  }

	  virtual ~CChatClient(void)
	  {
	  }

	  void Active()
	  {
		  m_dwLastActive = GetTickCount();
	  }

public:
	string m_sUserSession;
	DWORD m_nLinkNo;
	DWORD m_dwServerSession;
	DWORD m_dwLastActive;
	map<DWORD, CChatClient *> m_SourceSubscribes;		//订阅自己的用户列表
	map<DWORD, CChatClient *> m_TargetSubscribes;		//自己订阅的用户列表
	DWORD m_dwType;	//1:source, 2:play
};


class CVideoChatServer : 
	public ITcpServerNotify,
	public CKThread
{
public:
	CVideoChatServer(void);
	virtual ~CVideoChatServer(void);

	bool Connect( unsigned short usPort = 10100 );
	void Release();

	// From ITcpServerNotify
	virtual void onAccept(unsigned long sessionID);
	virtual void onClosed(unsigned long sessionID);
	virtual void onRecvData(char * data,int len,unsigned long sessionID);

protected:
	void CVideoChatServer::UpdateServerTick();
	virtual void ThreadProcMain(void);
	void DeliveData( PFRAME_HEADER pFH, BYTE* pData, unsigned int nLinkNo);
	CChatClient* FindClientByUSession( string sUSession);
	CChatClient* FindClientBySession( DWORD dwSession);

private:
	CKEvent m_recvEvent;
	CKPtrMap<DWORD, CChatClient> m_vccMgr;
	ITcpServer* m_pTcpServer;
	bool m_bWantStop;
};
