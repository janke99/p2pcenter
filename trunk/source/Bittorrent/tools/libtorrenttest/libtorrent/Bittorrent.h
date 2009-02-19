#pragma once
#include "ibittorrent.h"

class CBittorrent :
	public IBittorrent
{
public:
	CBittorrent(void);
	virtual ~CBittorrent(void);

	virtual bool InitModule( IBittorrentNotify* pNotify );
	virtual void ReleaseModule();
	virtual DWORD OpenSource( const char* szUrl, bool bSource );
	virtual void CloseSource( DWORD dwChannelID );
	virtual void RequestSegment( DWORD dwChannelID, DWORD dwStartPos, DWORD dwLength );
	virtual bool ReadSegment( DWORD dwChannelID, DWORD dwStartPos, char* pBuffer, DWORD& dwLength );
	virtual LONGLONG GetChannelSize( DWORD dwChannelID );
	virtual bool GetAllChannelID( list<DWORD>& listChannels);
	virtual bool GetChannelMonitorInfo( DWORD dwChannelID, stMonitorInfo& monInfo);
	virtual bool GetChannelMonitorInfo( const char* szChannelHash, stMonitorInfo& monInfo);
	virtual void tick_it();

private:
	IBittorrentNotify* notify_;
};
