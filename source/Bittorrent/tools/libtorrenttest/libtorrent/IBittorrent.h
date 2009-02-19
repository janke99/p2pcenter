#pragma once

#include <string>
#include <list>
#include <windows.h>
using namespace std;

#ifdef LIBTORRENT_EXPORTS
#define BITTORRENT_API __declspec(dllexport)
#else
#define BITTORRENT_API __declspec(dllimport)
#endif

class IBittorrentNotify
{
public:
	virtual OnConnect() = 0;
	virtual OnDisConnect() = 0;
};

// 参考 CONNNECT_STATUS in P2PPeer.h
typedef enum CONNTYPE
{
	CONNTYPE_NONE = 0,
	CONNTYPE_HANDSHAKING,
	CONNTYPE_CONNECTED,
	CONNTYPE_DISCONNECTED
};

struct stPeerInfo {
	DWORD		dwID;
	string		sExternalIP;
	WORD		wExternalUDPPort;
	WORD		wExternalTCPPort;
	bool		bSource;					// 是否为源

	CONNTYPE	ConnType;					// 连接类型
	DWORD		dwUploadBytes;				// 上传能力,十秒内的平均上传字节 单位 BYTE/秒
	DWORD		dwRecvBytes;				// 本地统计上传能力,十秒内的平均接收字节 单位 BYTE/秒,对此PEER的一个评级

	DWORD		dwRequestBlocks;			// 请求的块数
	DWORD		dwReceiveBlocks;			// 收到的块数
	DWORD		dwGetBlocks;				// 被请求的块数
	DWORD		dwSendBlocks;				// 发送的块数

	double		dbHealthRate;				// 发送健康率
	double		dbReturnRate;				// 请求接收率
};

struct stMonitorInfo {
	list<stPeerInfo> Peers;
	string sChannelName;
};

class IBittorrent
{
public:
	virtual bool InitModule( IBittorrentNotify* pNotify ) = 0;
	virtual void ReleaseModule() = 0;
	virtual DWORD OpenSource( const char* szUrl, bool bSource ) = 0;
	virtual void CloseSource( DWORD dwChannelID ) = 0;
	virtual void RequestSegment( DWORD dwChannelID, DWORD dwStartPos, DWORD dwLength ) = 0;
	virtual bool ReadSegment( DWORD dwChannelID, DWORD dwStartPos, char* pBuffer, DWORD& dwLength ) = 0; // dwLength 缓冲长度,返回读取实际长度
	virtual LONGLONG GetChannelSize( DWORD dwChannelID ) = 0;
	virtual bool GetAllChannelID( list<DWORD>& listChannels) = 0;
	virtual bool GetChannelMonitorInfo( DWORD dwChannelID, stMonitorInfo& monInfo) = 0;
	virtual bool GetChannelMonitorInfo( const char* szChannelHash, stMonitorInfo& monInfo) = 0;
	virtual void tick_it(){};
};

BITTORRENT_API IBittorrent* CreateIBittorrent();
