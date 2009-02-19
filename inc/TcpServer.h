#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#ifdef TCPSERVER_EXPORTS
#define TCPSERVER_API __declspec(dllexport)
#else
#define TCPSERVER_API __declspec(dllimport)
#endif


class ITcpServerNotify
{
public:
	virtual void onAccept(unsigned long sessionID)=0;
	virtual void onClosed(unsigned long sessionID)=0;
	virtual void onRecvData(char * data,int len,unsigned long sessionID)=0;
};

class ITcpServer
{
public:
	virtual bool Connect(unsigned short usPort)=0;
	virtual void ReleaseConnections(void)=0;
	virtual bool sendData(char * data,int len,unsigned long sessionID)=0;
};

TCPSERVER_API ITcpServer* CreateTcpServer(ITcpServerNotify * notify);
TCPSERVER_API bool initNetwork();
TCPSERVER_API void uninitNetwork();

#endif