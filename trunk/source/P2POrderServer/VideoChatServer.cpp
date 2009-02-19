#include "StdAfx.h"
#include ".\videochatserver.h"
#include "misc/IniFile.h"
#include "log/KLog.h"

CVideoChatServer::CVideoChatServer(void) :
m_pTcpServer(NULL),
m_bWantStop(false)
{
	printf( "Server Current version is %s\n", SERVER_VERSION );
}

CVideoChatServer::~CVideoChatServer(void)
{
	if ( m_pTcpServer )
	{
		m_pTcpServer->ReleaseConnections();
		m_pTcpServer = NULL;
	}
}

bool CVideoChatServer::Connect( unsigned short usPort )
{
#ifdef _DEBUG
	m_pTcpServer = CreateTcpServer( this);
#else
	m_pTcpServer = CreateTcpServer( this);
#endif

	if ( ! m_pTcpServer )
		return false;

	if ( !m_pTcpServer->Connect( usPort ) )
		return false;

	printf( "listen tcp port %d succeed\n", usPort );

	return StartThread();
}

void CVideoChatServer::Release(void)
{
	m_bWantStop = true;
	m_recvEvent.Set();
	WaitForStop();

	if ( m_pTcpServer )
	{
		m_pTcpServer->ReleaseConnections();
		m_pTcpServer = NULL;
	}
}

void CVideoChatServer::onAccept(unsigned long nLinkNo)
{
	printf( "Accept new link session id: %d\n", nLinkNo);
}

void CVideoChatServer::onClosed(unsigned long nLinkNo)
{
	printf( "Close link session id: %d\n", nLinkNo);

	m_vccMgr.Remove( nLinkNo );
}

void CVideoChatServer::onRecvData(char* pData, int nLen, unsigned long nLinkNo)
{
	static UINT nRecvCount = 0;
	printf("%u - Len(%u) LinkNo(%u)\n", ++nRecvCount, nLen, nLinkNo);

	if ( nLen >= sizeof(FRAME_HEADER))
	{
		PFRAME_HEADER pFH = (PFRAME_HEADER)pData;
		if ( pFH->dwDataLen == (nLen - sizeof(FRAME_HEADER)))
		{
			DeliveData( pFH, (BYTE *)pData + sizeof(FRAME_HEADER), nLinkNo);
		}
	}
}

void CVideoChatServer::ThreadProcMain(void)
{
	while ( !m_bWantStop )
	{
		m_recvEvent.Wait( 1000 );
		UpdateServerTick();
	}
}

void CVideoChatServer::UpdateServerTick()
{
}

CChatClient* CVideoChatServer::FindClientByUSession( string sUSession)
{
	map<DWORD, CChatClient *>::iterator it = m_vccMgr.GetMapPtr()->begin();
	for ( ; it != m_vccMgr.GetMapPtr()->end(); it++)
	{
		CChatClient* p = it->second;
		if ( p->m_sUserSession == sUSession)
			return p;
	}

	return NULL;
}

CChatClient* CVideoChatServer::FindClientBySession( DWORD dwSession)
{
	map<DWORD, CChatClient *>::iterator it = m_vccMgr.GetMapPtr()->begin();
	for ( ; it != m_vccMgr.GetMapPtr()->end(); it++)
	{
		CChatClient* p = it->second;
		if ( p->m_dwServerSession == dwSession)
			return p;
	}

	return NULL;
}

void CVideoChatServer::DeliveData( PFRAME_HEADER pFH, BYTE* pData, unsigned int nLinkNo)
{
	switch( pFH->byStreamType) 
	{
	case STREAM_TYPE_VIDEO:
	case STREAM_TYPE_AUDIO:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( !p)	return;
			
			p->Active();
			map<DWORD, CChatClient *>::iterator it = p->m_SourceSubscribes.begin();
			while ( it != p->m_SourceSubscribes.end())
			{
				CChatClient* pFriend =m_vccMgr.FindNoLock( it->first); //修改SESSION,数据原样转发
				if ( pFriend)
				{
					pFH->dwSession =p->m_nLinkNo;
					m_pTcpServer->sendData( (char *)pFH, pFH->dwDataLen + sizeof(FRAME_HEADER), pFriend->m_nLinkNo);
					it++;
				}
				else
				{
					it = p->m_SourceSubscribes.erase(it);
				}
			}
		}
		break;

	case STREAM_TYPE_USERLOGIN:
		{
			if ( 16 == pFH->dwDataLen)
			{
				string sUserSession;
				sUserSession.append( (char *)pData, pFH->dwDataLen);

				CKAutoLock l(m_vccMgr.GetCritSec());
				CChatClient* p = FindClientByUSession( sUserSession);
				if ( p)
					m_vccMgr.RemoveNoLock( p->m_nLinkNo);

				p  =m_vccMgr.FindNoLock( nLinkNo);
				if ( p)
					m_vccMgr.RemoveNoLock( p->m_nLinkNo);

				p = new CChatClient();
				p->m_nLinkNo = nLinkNo;
				p->m_sUserSession =sUserSession;
				static DWORD dwCount = 0;
				p->m_dwServerSession =GetTickCount() + dwCount++;
				p->m_dwType =pFH->byReserve1;
				m_vccMgr.InsertNoLock( nLinkNo, p);

				char szData[128];
				PFRAME_HEADER pFrameHeader = (PFRAME_HEADER)szData;
				pFrameHeader->byCodecType = 0;
				pFrameHeader->byKeyFrame = 0;
				pFrameHeader->byReserve1 = 0;
				pFrameHeader->byStreamType = STREAM_TYPE_USERLOGIN;
				pFrameHeader->dwDataLen = (DWORD)sUserSession.length();
				pFrameHeader->dwSeqNum = pFH->dwSeqNum;
				pFrameHeader->dwSession = p->m_dwServerSession;
				pFrameHeader->dwTimeStamp = GetTickCount();
				memcpy( szData + sizeof(FRAME_HEADER), sUserSession.c_str(), sUserSession.length());
				m_pTcpServer->sendData( szData, pFrameHeader->dwDataLen + sizeof(FRAME_HEADER), nLinkNo);
			}
		}
		break;

	case STREAM_TYPE_USERLOGOUT:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( p)
			{
				p->Active();
				FRAME_HEADER FrameHeader;
				FrameHeader.byCodecType = 0;
				FrameHeader.byKeyFrame = 0;
				FrameHeader.byReserve1 = 0;
				FrameHeader.byStreamType = STREAM_TYPE_USERLOGOUT;
				FrameHeader.dwDataLen = 0;
				FrameHeader.dwSeqNum = pFH->dwSeqNum;
				FrameHeader.dwSession = p->m_dwServerSession;
				FrameHeader.dwTimeStamp = GetTickCount();

				m_pTcpServer->sendData( (char *)&FrameHeader, sizeof(FRAME_HEADER), nLinkNo);

				m_vccMgr.RemoveNoLock( p->m_nLinkNo);
			}
		}
		break;

	case STREAM_TYPE_PLAYERLIST:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( p)
			{
				p->Active();
			}
		}
		break;

	case STREAM_TYPE_SUBSCRIBE:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( p)
			{
				p->Active();
				if ( sizeof(DWORD) == pFH->dwDataLen)
				{
					DWORD dwRet = -1;
					DWORD dwFriendSSRC = *(DWORD *)pData;
					CChatClient* pFriend = FindClientBySession( dwFriendSSRC);
					if ( pFriend)
					{
						pFriend->m_SourceSubscribes[p->m_nLinkNo] =p;
						p->m_TargetSubscribes [pFriend->m_nLinkNo] =pFriend;
						dwRet = 0;
					}
					else
					{
						//朋友不存在 ret = -1
						dwRet = -1;
					}

					char szData[128];
					PFRAME_HEADER pFrameHeader = (PFRAME_HEADER)szData;
					pFrameHeader->byCodecType = 0;
					pFrameHeader->byKeyFrame = 0;
					pFrameHeader->byReserve1 = 0;
					pFrameHeader->byStreamType = STREAM_TYPE_SUBSCRIBE;
					pFrameHeader->dwDataLen = sizeof(dwRet);
					pFrameHeader->dwSeqNum = pFH->dwSeqNum;
					pFrameHeader->dwSession = p->m_dwServerSession;
					pFrameHeader->dwTimeStamp = GetTickCount();
					memcpy( szData + sizeof(FRAME_HEADER), &dwRet, sizeof(dwRet));
					m_pTcpServer->sendData( szData, pFrameHeader->dwDataLen + sizeof(FRAME_HEADER), nLinkNo);
				}
			}
		}
		break;

	case STREAM_TYPE_UNSUBSCRIBE:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( p)
			{
				p->Active();
				if ( sizeof(DWORD) == pFH->dwDataLen)
				{
					DWORD dwRet = -1;
					DWORD dwFriendSSRC = *(DWORD *)pData;
					CChatClient* pFriend = FindClientBySession( dwFriendSSRC);
					if ( pFriend)
					{
						pFriend->m_SourceSubscribes.erase(p->m_nLinkNo);
						p->m_TargetSubscribes.erase(pFriend->m_nLinkNo);
						dwRet = 0;
					}
					else
					{
						//朋友不存在 ret = -1
						dwRet = -1;
					}

					char szData[128];
					PFRAME_HEADER pFrameHeader = (PFRAME_HEADER)szData;
					pFrameHeader->byCodecType = 0;
					pFrameHeader->byKeyFrame = 0;
					pFrameHeader->byReserve1 = 0;
					pFrameHeader->byStreamType = STREAM_TYPE_UNSUBSCRIBE;
					pFrameHeader->dwDataLen = sizeof(dwRet);
					pFrameHeader->dwSeqNum = pFH->dwSeqNum;
					pFrameHeader->dwSession = p->m_dwServerSession;
					pFrameHeader->dwTimeStamp = GetTickCount();
					memcpy( szData + sizeof(FRAME_HEADER), &dwRet, sizeof(dwRet));
					m_pTcpServer->sendData( szData, pFrameHeader->dwDataLen + sizeof(FRAME_HEADER), nLinkNo);
				}
			}
		}
		break;

	case STREAM_TYPE_HEARTBEAT:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( p)
			{
				p->Active();
				FRAME_HEADER FrameHeader;
				FrameHeader.byCodecType = 0;
				FrameHeader.byKeyFrame = 0;
				FrameHeader.byReserve1 = 0;
				FrameHeader.byStreamType = STREAM_TYPE_HEARTBEAT;
				FrameHeader.dwDataLen = 0;
				FrameHeader.dwSeqNum = pFH->dwSeqNum;
				FrameHeader.dwSession = p->m_dwServerSession;
				FrameHeader.dwTimeStamp = GetTickCount();

				m_pTcpServer->sendData( (char *)&FrameHeader, sizeof(FRAME_HEADER), nLinkNo);
			}
		}
		break;

	case STREAM_TYPE_REPORTERROR:
		{
			CKAutoLock l(m_vccMgr.GetCritSec());
			CChatClient* p  =m_vccMgr.FindNoLock( nLinkNo);
			if ( p)
			{
				p->Active();
				string sError;
				sError.append( (char *)pData, pFH->dwDataLen);
			}
		}
		break;

	default:
		break;
	}
}
