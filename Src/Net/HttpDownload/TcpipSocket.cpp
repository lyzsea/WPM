// ***************************************************************
//  TcpipSocket.cpp
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

#include <WinSock2.h>
#include "UTL_FileTime.H"
#include "TcpipSocket.h"


CTcpipSocket::CTcpipSocket()
{
	m_hSock = INVALID_SOCKET;
	m_bCancel = FALSE;
}

CTcpipSocket::~CTcpipSocket()
{
	Close();
}

BOOL CTcpipSocket::Startup()
{
	WSADATA wsaData;
	if (0 == WSAStartup( MAKEWORD(2,2), &wsaData ))
		return TRUE;
	return FALSE;
}

void CTcpipSocket::Cleanup()
{
	WSACleanup();
}

int CTcpipSocket::Create()
{
	if (INVALID_SOCKET != m_hSock)
		Close();
	{
		m_bCancel = false;
		m_bConnected = false;

		if(INVALID_SOCKET == (m_hSock = socket(AF_INET, SOCK_STREAM, 0)))
		{
			return E_FAIL;
		}

		if (E_FAIL == EnableNonBlock(TRUE))
		{
			closesocket(m_hSock);
			m_hSock = INVALID_SOCKET;
			return E_FAIL;
		}
	}

	return S_OK;
}

int CTcpipSocket::EnableNonBlock(BOOL bNonBlock)
{
	if (INVALID_SOCKET == m_hSock)
		return E_FAIL;

	if (SOCKET_ERROR == ioctlsocket(m_hSock, FIONBIO, (unsigned long*)&bNonBlock))
		return E_FAIL;

	return S_OK;
}

int CTcpipSocket::SetHost(LPCTSTR pHostName)
{
	if (m_bCancel)
		return E_ABORT;

	if (NULL == pHostName || 0 == pHostName[0])
		return E_INVALIDARG;
//解析DNS
	std::string strName = ws2s(pHostName, CP_ACP);
	m_dwRemoteIP = inet_addr(strName.c_str());
	if (m_dwRemoteIP == INADDR_NONE)
	{
		hostent *pHost = ::gethostbyname(strName.c_str());
		if (NULL == pHost)
			return E_FAIL;
		m_dwRemoteIP = *((unsigned long*)pHost->h_addr_list[0]);
	}


	return S_OK;
}

int CTcpipSocket::Connect(WORD uPort, int nMillTimeOut, LPCTSTR pHostName)
{
	int hr;

	if (pHostName && pHostName[0])
	{
		hr = SetHost(pHostName);
		if (S_OK != hr)
			return hr;
	}

	if (INVALID_SOCKET == m_hSock)
	{
		hr = Create();
		if (S_OK != hr)
			return hr;
	}
	
	if (m_bCancel)
		return E_ABORT;

	struct sockaddr_in inAddr;
	inAddr.sin_addr.S_un.S_addr = m_dwRemoteIP;
	inAddr.sin_family = AF_INET;
	inAddr.sin_port = ::htons(uPort);
	if (SOCKET_ERROR == ::connect(m_hSock, (PSOCKADDR)&inAddr, sizeof(inAddr)))
	{//
		int nErr = WSAGetLastError();
		if(!(nErr==WSAEWOULDBLOCK || nErr == WSAEINPROGRESS))
			return E_FAIL;
	}

	DWORD dwStartTime = GetTickCount();

	do 
	{
		if (m_bCancel)
			return E_ABORT;
		
		fd_set         fd_write;  
		fd_set		   fd_exp;
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_exp);
		FD_SET(m_hSock, &fd_write);
		FD_SET(m_hSock, &fd_exp);

		timeval	timeout={0, 50*1000};// 5ms查看一次是否有连接成功允许
		switch(select(m_hSock+1, 0, &fd_write, &fd_exp, &timeout))
		{
		case 0://超时
			if (m_bCancel)
				return E_ABORT;
			break;
		case SOCKET_ERROR://失败
			{
				if (m_bCancel)
					return E_ABORT;
				return E_FAIL;
			}
		default:
			if (FD_ISSET(m_hSock, &fd_write))
			{
				if (m_bCancel)
						return E_ABORT;

				m_bConnected = true;
				return S_OK;
			}
			else if(FD_ISSET(m_hSock, &fd_exp))
			{
				return E_FAIL;
			}
		}
	} while(nMillTimeOut == INFINITE || nMillTimeOut > (int)((GetTickCount() - dwStartTime)));

	return E_TIMEOUT;
}


void CTcpipSocket::Close()
{
	m_bCancel = true;
	m_bConnected = false;
	if (INVALID_SOCKET !=m_hSock)
	{
		closesocket(m_hSock);
		m_hSock = INVALID_SOCKET;
	}
}

int CTcpipSocket::Recv(PBYTE pBuffer, UINT uMaxLen, int nMillTimeOut)
{
	if (m_hSock == INVALID_SOCKET || !m_bConnected)
		return E_NOCONNECTION;

	DWORD dwStartTime = GetTickCount();
	do 
	{
		if (m_bCancel)
			return E_ABORT;
		
		fd_set         fd_read;                      
		fd_set		   fd_exp;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_exp);
		
		FD_SET(m_hSock, &fd_read);
		FD_SET(m_hSock, &fd_exp);

		timeval	timeout={0,50*1000}; // 5ms查看一次是否有连接成功允许
		
		switch(select(m_hSock+1, &fd_read, 0, &fd_exp, &timeout))
		{
		case -1://错误
			return E_FAIL;
		case 0://超时
			break;
		default:
			{
				if (FD_ISSET(m_hSock, &fd_read))
				{
					if (m_bCancel)
						return E_ABORT;
					int nRecv = recv(m_hSock, (char*)pBuffer, uMaxLen, 0);
					if (nRecv > 0)
//					if (nRecv >= 0) 2008-12-1刘金鑫修改，发现recv有可能返回0,这导致CPU 100%
						return nRecv;
					else
						return E_FAIL;
				}else if(FD_ISSET(m_hSock, &fd_exp))
					return E_FAIL;
			}
		}

	} while(nMillTimeOut == INFINITE || nMillTimeOut > (int)(GetTickCount() - dwStartTime));

	return E_TIMEOUT;
}

int CTcpipSocket::RecvBlock(PBYTE pBuffer, UINT uMaxLen, int nMillTimeOut)
{
	DWORD dwTotalRecved = 0;
	for(;dwTotalRecved < uMaxLen;)
	{//接收数据
		if (m_bCancel)
			return E_ABORT;

		int nRecv = Recv(pBuffer + dwTotalRecved, uMaxLen - dwTotalRecved, nMillTimeOut);
		if (FAILED(nRecv))
		{
			return nRecv;
		}
		else if (nRecv > 0)
		{
			dwTotalRecved += nRecv;
			if (dwTotalRecved == uMaxLen)
				return dwTotalRecved;
		}
	}

	return E_TIMEOUT;
}

int CTcpipSocket::Send(const PBYTE pBuffer, UINT uLen, int nMillTimeOut)
{
	if (m_hSock == INVALID_SOCKET || !m_bConnected)
		return E_NOCONNECTION;

	DWORD dwStartTime = GetTickCount();
	do 
	{
		if (m_bCancel)
			return E_ABORT;
		
		fd_set         fd_write;                      
		fd_set		   fd_exp;

		FD_ZERO(&fd_write);
		FD_ZERO(&fd_exp);
		
		FD_SET(m_hSock, &fd_write);
		FD_SET(m_hSock, &fd_exp);

		timeval	timeout={0,50*1000}; // 5ms查看一次是否有连接成功允许
		
		switch(select(m_hSock+1, 0, &fd_write, &fd_exp, &timeout))
		{
		case -1://错误
			return E_FAIL;
		case 0://超时
			break;
		default:
			{
				if (FD_ISSET(m_hSock, &fd_write))
				{
					if (m_bCancel)
						return E_ABORT;
					int nSend = send(m_hSock, (const char*)pBuffer, uLen, 0);
					if (nSend >= 0)
						return nSend;
					else
						return E_FAIL;
				}else if(FD_ISSET(m_hSock, &fd_exp))
					return E_FAIL;
			}
		}

	} while(nMillTimeOut == INFINITE || nMillTimeOut > (int)(GetTickCount() - dwStartTime));

	return E_TIMEOUT;
}

int CTcpipSocket::SendBlock(const PBYTE pBuffer, UINT uLen, int nMillTimeOut)
{
	DWORD dwTotalSend = 0;

	for(;dwTotalSend < uLen;)
	{//接收数据
		if (m_bCancel)
			return E_ABORT;

		int nSend = Send(pBuffer + dwTotalSend, uLen - dwTotalSend, nMillTimeOut);
		if (FAILED(nSend))
		{
			return nSend;
		}
		else if (nSend > 0)
		{
			dwTotalSend += nSend;
			if (dwTotalSend == uLen)
				return dwTotalSend;
		}
	}

	return S_OK;
}