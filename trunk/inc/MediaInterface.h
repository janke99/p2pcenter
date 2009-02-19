/********************************************************************
	created:	10:08:2008   15:49
	author:		Kevin
	
	purpose:	

	email:		kan8888.com@163.com
	msn:		fuwenke@gmail.com
	QQ:		799383417
	site:		opensource.kan8888.com
*********************************************************************/
 
#pragma once

#include "MediaDefine.h"

#define MEDIAINTERFACE_API __declspec(dllexport)

class IVideoCaptureNotify
{
public:
	virtual void OnVideoFrameArrive(LPVOID lpData, DWORD dwSize) = 0;
};

class VideoCaptureItem
{
public:
	VideoCaptureItem(void) :
	  m_hWnd(NULL)
		  , m_stSize(VIDEO_SIZE_176X144)
		  , m_rtRgb(VIDEO_I420)
		  , m_nFps(10)
		  , m_nDevID(0)
	  {
	  };

	  HWND 				m_hWnd;	
	  VIDEO_SIZE_TYPE	m_stSize;		
	  VIDEO_RGB_TYPE	m_rtRgb;			
	  int				m_nFps;					
	  int  				m_nDevID;
};

class IVideoCapture
{
public:
	virtual bool Connect(IVideoCaptureNotify* pNotify, const VideoCaptureItem& Item) = 0;
	virtual void Release(void) = 0;
	virtual void SetVideoWindowPosition(int x, int y, int cx, int cy) = 0;
	virtual void StartVideoCapture(void) = 0;
	virtual void StopVideoCapture(void) = 0;
	virtual void ShowVideoWindow(bool bShow) = 0;
	virtual bool EnumVCapDev( char acName[10][260] , long &lNums ) = 0;
	virtual bool EnumVCompress( char acName[256][260] , long &lNums ) = 0;
	virtual void GetVideoFormat(char *pbFormat) = 0;
};

class IVideoPlayNotify
{
};

class VideoPlayItem
{
public:
	VideoPlayItem(void) :
	  m_hWnd(NULL)
		  , m_stSize(VIDEO_SIZE_176X144)
		  , m_rtRgb(VIDEO_I420)
	  {
	  };

	  HWND m_hWnd;
	  VIDEO_SIZE_TYPE	m_stSize;		
	  VIDEO_RGB_TYPE	m_rtRgb;
};

class IVideoPlay
{
public:
	virtual bool Connect(IVideoPlayNotify* pNotify, const VideoPlayItem& Item) = 0;
	virtual void Release(void) = 0;
	virtual void SetVideoWindowPosition(int x, int y, int cx, int cy) = 0;
	virtual void StartVideoPlay(void) = 0;
	virtual void StopVideoPlay(void) = 0;
	virtual void ShowVideoWindow(bool bShow) = 0;
	virtual void DeliverData(LPVOID lpData, DWORD dwSize) = 0;
};

class IVideoEnDecodeNotify
{
public:
	virtual void OnVideoEnDecodeFrame(LPVOID lpData, DWORD dwSize) = 0;
};

class VideoEncodeItem
{
public:
	VideoEncodeItem(void)
		: m_stSize(VIDEO_SIZE_176X144)
		, m_nMinKeyInterval(25)
		, m_nMaxKeyInterval(100)
		, m_nBitRate(1024000)
		, m_nQuality(0)
		, m_nFps(10)	
	{
	};

	VIDEO_SIZE_TYPE	m_stSize;	
	int				m_nMinKeyInterval;
	int				m_nMaxKeyInterval;
	int				m_nBitRate;
	int				m_nQuality;
	int				m_nFps;
};

class IVideoEncoder
{
public:
	virtual bool Connect(IVideoEnDecodeNotify* pNotify, const VideoEncodeItem& Item) = 0;
	virtual void Release(void) = 0;
	virtual int Encode(BYTE* pInData, int nLen, BYTE* pOutBuf, int& nOutLen, int& nKeyFrame) = 0;
};

class VideoDecodeItem
{
public:
	VideoDecodeItem(void)
		: m_stSize(VIDEO_SIZE_176X144)
	{
	};

	VIDEO_SIZE_TYPE	m_stSize;	
};

class IVideoDecoder
{
public:
	virtual bool Connect(IVideoEnDecodeNotify* pNotify, const VideoDecodeItem& Item) = 0;
	virtual void Release(void) = 0;
	virtual int Decode(BYTE* pInData, int nLen, BYTE* pOutBuf, int& nOutLen) = 0;
};

MEDIAINTERFACE_API IVideoCapture *CreateIVideoCapture();
MEDIAINTERFACE_API IVideoPlay *CreateIVideoPlay();
MEDIAINTERFACE_API IVideoEncoder *CreateIVideoEncoder(VIDEO_CODER_TYPE coderType);
MEDIAINTERFACE_API IVideoDecoder *CreateIVideoDecoder(VIDEO_CODER_TYPE coderType);

//MEDIAINTERFACE_API IAudioCapture *CreateIAudioCapture();
//MEDIAINTERFACE_API IAudioPlay *CreateIAudioPlay();
//MEDIAINTERFACE_API IAudioEncoder *CreateIAudioEncoder(AUDIO_CODER_TYPE coderType);
//MEDIAINTERFACE_API IAudioDecoder *CreateIAudioDecoder(AUDIO_CODER_TYPE coderType);
//
//MEDIAINTERFACE_API ISocketServer* CreateSocketServer();
//MEDIAINTERFACE_API ISocketClient* CreateSocketClient();
//
//MEDIAINTERFACE_API IRTPServer* CreateRTPServer();
//MEDIAINTERFACE_API IRTPClient* CreateRTPClient();

MEDIAINTERFACE_API bool CoInit();
MEDIAINTERFACE_API UnCoInit();

//MEDIAINTERFACE_API bool InitSocket();
//MEDIAINTERFACE_API UnInitSocket();

