/********************************************************************
	created:	10:08:2008   15:49
	author:		Kevin
	
	purpose:	

	email:		kan8888.com@163.com
	msn:		fuwenke@gmail.com
	QQ:			799383417
	site:		opensource.kan8888.com
*********************************************************************/
 
#pragma once

#ifndef SAFE_SYSFREE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_SYSFREE
#define SAFE_SYSFREE( x )   \
	if ( NULL != x )            \
{                           \
	SysFreeString( x );       \
	x = NULL;               \
}
#endif

#ifndef BREAK_IF_FAILED
#define BREAK_IF_FAILED(hr) if(FAILED(hr)) break;
#endif

#ifndef SAFE_ADDREF

#define SAFE_ADDREF( x )    \
	if ( x )                \
{                       \
	x->AddRef();        \
}

#endif

typedef enum{
	VIDEO_CODER_H264 = 0,
	VIDEO_CODER_XVID,
	VIDEO_CODER_MPEG4,
	VIDEO_CODER_WMV9,
	VIDEO_CODER_H263,
	VIDEO_CODER_MSMPEGV2,
	VIDEO_CODER_MSMPEGV3,
	VIDEO_CODER_WMV1,
	VIDEO_CODER_MAX
}VIDEO_CODER_TYPE;

typedef enum{
	AUDIO_CODER_ACC = 0,
	AUDIO_CODER_G729A
}AUDIO_CODER_TYPE;

typedef enum {
	STREAM_TYPE_VIDEO = 0,
	STREAM_TYPE_AUDIO,
	STREAM_TYPE_USERLOGIN,
	STREAM_TYPE_USERLOGOUT,
	STREAM_TYPE_PLAYERLIST,
	STREAM_TYPE_SUBSCRIBE,
	STREAM_TYPE_UNSUBSCRIBE,
	STREAM_TYPE_HEARTBEAT,
	STREAM_TYPE_REPORTERROR
}FRAME_STREAM_TYPE;

typedef struct 
{
	BYTE  byStreamType;		// FRAME_STREAM_TYPE
	BYTE  byCodecType;		// 编码器类型 VIDEO_CODER_TYPE or AUDIO_CODER_TYPE
	BYTE  byKeyFrame;		// 关键帧
	BYTE  byReserve1;		// 保留字节
	DWORD dwSession;		// 包唯一标识符
	DWORD dwSeqNum;			// 包单向序号
	DWORD dwTimeStamp;		// 时间戳
	DWORD dwDataLen;		// 实际据包长度
}FRAME_HEADER, *PFRAME_HEADER;

typedef enum{
	VIDEO_FILE_AVI = 0
}VIDEO_FILE_TYPE;

typedef enum{
	VIDEO_FILE_MP3 = 0,
	VIDEO_FILE_WMA,
	VIDEO_FILE_RM,
	VIDEO_FILE_WAV
}AUDIO_FILE_TYPE;

typedef enum {
	VIDEO_SIZE_176X144 = 0,
	VIDEO_SIZE_320X240,
	VIDEO_SIZE_352X288,
	VIDEO_SIZE_640X480,
	//VIDEO_SIZE_704X576,
	VIDEO_SIZE_FULLSCREEN
}VIDEO_SIZE_TYPE;

typedef enum{
	VIDEO_RGB=0,
	VIDEO_I420,
	VIDEO_H264,
	VIDEO_WMV9,
	VIDEO_RV40
}VIDEO_RGB_TYPE;

typedef enum{
	VIDEO_FRAME_2  = 0,
	VIDEO_FRAME_5,
	VIDEO_FRAME_10,
	VIDEO_FRAME_15,
	VIDEO_FRAME_20,
	VIDEO_FRAME_25,
	VIDEO_FRAME_30
}VIDEO_FRAME_TYPE;

const GUID MEDIASUBTYPE_I420 = {0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
