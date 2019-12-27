// ***************************************************************
//  TcpipSocket.h
//  Version:  1.0   
//  Created: 2006-12-7
//  Author:	 陈庆明
//  -------------------------------------------------------------
//  Purpose:
//  -------------------------------------------------------------
//  Copyright (C) 2006 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef _TCPIP_SOCKET_H__
#define _TCPIP_SOCKET_H__

//#include <WinSock2.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//各种返回值函义
/*
S_OK	成功
E_FAIL	错误
E_TIMEOUT	超时
E_INVALIDARG 参数错误
E_ABORT	用户终止
E_NOCONNECTION 连接不存在
E_OUTOFMEMORY  内存不足
*/

#define E_TIMEOUT RPC_E_TIMEOUT	
#define E_NOCONNECTION RPC_E_CONNECTION_TERMINATED
// #define TCPIP_SUCCEEDED		0
// #define TCPIP_ERROR			SOCKET_ERROR
// #define TCPIP_TIMEOUT		(-2)
// #define TCPIP_USERCANCEL	(-3)
// #define TCPIP_INVALIDARG	(-4)
// #define TCPIP_NOCONNECTION	(-5)

class CTcpipSocket
{
public:
	CTcpipSocket();
	virtual ~CTcpipSocket();
	static BOOL Startup();
	static void Cleanup();

	int Create(); //创建
	int SetHost(LPCTSTR pHostName);	//创建
	int Connect(WORD uPort, int nMillTimeOut, LPCTSTR pHostName = NULL);

	int Recv(PBYTE pBuffer, UINT uMaxLen, int nMillTimeOut);
	int RecvBlock(PBYTE pBuffer, UINT uMaxLen, int nMillTimeOut);
	int Send(const PBYTE pBuffer, UINT uLen, int nMillTimeOut);
	int SendBlock(const PBYTE pBuffer, UINT uLen, int nMillTimeOut);

	void Close();

protected:
	int EnableNonBlock(BOOL bNonBlock);

private:
	SOCKET m_hSock;
	DWORD  m_dwRemoteIP;
	bool   m_bConnected;
	bool   m_bCancel; 
};

#endif // _TCPIP_SOCKET_H__
