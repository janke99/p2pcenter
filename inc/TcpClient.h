#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#ifdef TCPCLIENT_EXPORTS
#define TCPCLIENT_API __declspec(dllexport)
#else
#define TCPCLIENT_API __declspec(dllimport)
#endif

class ITcpClientNotify
{
public:
	virtual void OnReceivedData(char* pData,int nLen)=0;
	virtual void OnConnected(void)=0;
	virtual void OnDisconnected(void)=0;

};

class ITcpClient
{
public:
	virtual bool connectServer(const char * serverip,unsigned short port)=0;
	virtual bool sendData(char* pData,int nLen)=0;
	virtual void disconnect(void)=0;
	virtual void releaseConnection()=0;

};

TCPCLIENT_API ITcpClient* CreateTcpClient(ITcpClientNotify * notify);
TCPCLIENT_API bool initNetwork();
TCPCLIENT_API void uninitNetwork();
#endif