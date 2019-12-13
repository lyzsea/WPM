// HttpClient.cpp: implementation of the CHttpClient class.
//
//////////////////////////////////////////////////////////////////////
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <wininet.h>
#include <Sddl.h>
#include <tchar.h>
#include <comutil.h>

#include "HttpClient.h"

#include "UTL_STRNUM.H"
#include "UTL_FileTime.H"

#pragma comment(lib, "Iphlpapi")
#pragma comment(lib, "wininet")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Shlwapi")

extern std::string g_strGuid;
typedef std::basic_string<TCHAR> tstring ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHttpClient::CHttpClient()
{
	CTcpipSocket::Startup();
	m_nTimeOut = 20*1000;
	m_pEvent = NULL;
	m_timeServer.QuadPart = 0;
}

CHttpClient::~CHttpClient()
{
	CTcpipSocket::Cleanup();
}

BOOL CHttpClient::GetServerTime(FILETIME* ft)
{
	if (m_timeServer.QuadPart == 0)
		return FALSE;
	ft->dwHighDateTime = m_timeServer.HighPart;
	ft->dwLowDateTime = m_timeServer.LowPart;
	return TRUE;
}
void CHttpClient::SetTimeOut(int nTimeOut)
{
	m_nTimeOut = nTimeOut;
}

HRESULT  CHttpClient::DownLoadFile1(LPCTSTR lpUrl, LPCTSTR lpFileName, std::wstring& strUrlRedirecttion)
{
	HRESULT hr = S_OK;
	
	TCHAR szIP[256] = {0};
	WORD dwPort	= 0;
	if(this->GetProxyEnable(szIP, 256, dwPort))
	{
		hr = this->DownLoadFileFromProxy(szIP, dwPort, lpUrl, lpFileName, strUrlRedirecttion);
	}
	else
	{
		hr = this->DownLoadFile(lpUrl, lpFileName, strUrlRedirecttion);
	}
	
	return hr;
}

HRESULT  CHttpClient::DownLoadFileFromProxy(TCHAR * szIP, WORD & dwPort, LPCTSTR lpUrl, LPCTSTR lpFileName, std::wstring& strUrlRedirecttion)
{
	HRESULT hr = m_Socket.Create();
	if (FAILED(hr))
		return hr;
	
	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH]={0};
	TCHAR szFilePath[INTERNET_MAX_PATH_LENGTH]={0};
	
	URL_COMPONENTS uc ;
	ZeroMemory( &uc, sizeof( uc ) ) ;
	uc.dwStructSize = sizeof( uc ) ;
	uc.lpszHostName = szHostName;
	uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH ;
	uc.lpszUrlPath = szFilePath;
	uc.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
	if (InternetCrackUrl(lpUrl, lstrlen(lpUrl), ICU_DECODE, &uc ) == FALSE)
	{
		m_Socket.Close();
		return E_INVALIDARG;
	}
	
	if (0 == szFilePath[0])
	{
		szFilePath[0] = '/';
		szFilePath[1] = 0;
	}
	//_bstr_t szTempIP = szIP;
	hr = m_Socket.SetHost(szIP);
	if (FAILED(hr))
	{//服务器不对
		m_Socket.Close();
		return hr;
	}
	
	hr = E_FAIL;
	do {
		int nCode = m_Socket.Connect(dwPort, m_nTimeOut);
		if (FAILED(nCode))
		{
			hr = nCode;
			break;
		}
		
		//已连接上，发送请求包
		HRESULT hCode = SendRequest(szHostName, lpUrl, NULL, 0, get, NULL);
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}
		int nContent = RecvHttpResponseHead(0, strUrlRedirecttion);
		if (nContent == E_REDIRECTION && strUrlRedirecttion.length() > 0)
		{
			if (strUrlRedirecttion.find(L"/") == 0)
			{
				strUrlRedirecttion = std::wstring(L"http://") + std::wstring(szHostName) + strUrlRedirecttion; 
			}
		}
		if (nContent == E_REDIRECTION && strUrlRedirecttion.length() > 0)
		{
			if (strUrlRedirecttion.find(L"/") == 0)
			{
				strUrlRedirecttion = std::wstring(L"http://") + std::wstring(szHostName) + strUrlRedirecttion; 
			}
		}

		if (nContent == S_FALSE)
			return S_FALSE;//下载文件，如果服务器上文件没有改变，则不再下载
		BOOL bChunkedEncoding = FALSE;
		if (FAILED(nContent))
		{
			if (nContent == E_CHUNKED_ENCODING)
				bChunkedEncoding = TRUE;
			else
			{
				hr = nContent;
				break;
			}
		}
		
		HANDLE hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
			NULL,	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (INVALID_HANDLE_VALUE == hFile)
		{
			hr = E_CANTOPENFILE;
			break;
		}
		
		nCode = RecvData(hFile, nContent, bChunkedEncoding);
		if (SUCCEEDED(nCode))
		{//成功接收所有数据
			hr = S_OK;			
		}else
			hr = nCode;
		//::SetFileTime(hFile, NULL, NULL, &ftLastModify);
		CloseHandle(hFile);
	} while(0);
	
	m_Socket.Close();
	return hr;
}

BOOL  CHttpClient::GetProxyEnable(TCHAR * szIP, ULONG uLen, WORD & dwPort)
{
	/*	unsigned long nSize		= 4096; 
	TCHAR szBuf[4096]		= {0}; 
	TCHAR szProxy[MAX_PATH] = {0};
	
	INTERNET_PROXY_INFO* pInfo = (INTERNET_PROXY_INFO*)szBuf; 
	if(!InternetQueryOption(NULL, INTERNET_OPTION_PROXY, pInfo, &nSize)) 
	{ 
		return FALSE;
	}
	if(pInfo->dwAccessType != INTERNET_OPEN_TYPE_PROXY)
	{
		return FALSE;
	}

	_tcscpy(szProxy, pInfo->lpszProxy);

	this->GetProxyIpAndPort(szIP, dwPort, szProxy);

	if(dwPort <= 0 || _tcslen(szIP) == 0 )
		return FALSE;*/

	BOOL bRe			= TRUE;
	HKEY hKey			= NULL;
	DWORD dwValue		= {0};
	DWORD dwType		= REG_DWORD;
	DWORD dwDataSize	= sizeof(DWORD);
	
	DWORD dwTypeS		= REG_SZ;
	DWORD dwSize		= 256;
	TCHAR szValue[256]	= {0};

//	TCHAR szInSpath[256]	= {0};
//	if(!(this->GetInternetSettingPath(szInSpath)))
//		return FALSE;

	if(::RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), &hKey) == ERROR_SUCCESS)
	{
		do 
		{
			if(::RegQueryValueEx(hKey, _T("ProxyEnable"), NULL, &dwType, (LPBYTE)(&dwValue), &dwDataSize) != ERROR_SUCCESS)
			{
				bRe = FALSE;
				break;
			}
			if(!dwValue)
			{
				bRe = FALSE;
				break;
			}
			if(::RegQueryValueEx(hKey, _T("ProxyServer"), NULL, &dwTypeS, (LPBYTE)szValue, &dwSize) != ERROR_SUCCESS)
			{
				bRe = FALSE;
				break;
			}
			if(dwSize <= 0)
			{
				bRe = FALSE;
				break;
			}
			
			this->GetProxyIpAndPort(szIP, uLen, dwPort, szValue);
			
		}while (FALSE);
		
		::RegCloseKey(hKey);
	} 
	else
		bRe = FALSE;
	
	if(dwPort <= 0 || _tcslen(szIP) == 0 )
		bRe = FALSE;
	
	return bRe;
}
/*
BOOL  CHttpClient::GetInternetSettingPath(TCHAR * szPath)
{
	if(szPath == NULL)
		return FALSE;

	BOOL  fAPISuccess		= FALSE;
	TCHAR szUser[MAX_PATH]	= {0};
	DWORD dwUsize			= MAX_PATH;
	LPVOID	pUserSID		= NULL;
	DWORD  cbUserSID		= 0;
	TCHAR *  szDomain		= NULL;
	DWORD	cbDomain		= 0;
	SID_NAME_USE  snuType;

	if(::GetUserName(szUser, &dwUsize) == FALSE)
		return FALSE;

	fAPISuccess = LookupAccountName(NULL, szUser, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);
	if(fAPISuccess)
		return FALSE;

	pUserSID = myheapalloc(cbUserSID);
	szDomain = (TCHAR *) myheapalloc(cbDomain * sizeof(TCHAR));
	
	fAPISuccess = LookupAccountName(NULL, szUser, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);
	if(!fAPISuccess)
		return FALSE;

	LPTSTR lpBuf = NULL;
	fAPISuccess = ConvertSidToStringSid(pUserSID, &lpBuf);
	if(!fAPISuccess)
		return FALSE;

	if(pUserSID) 
		myheapfree(pUserSID);
	if(szDomain) 
		myheapfree(szDomain);

	strcpy(szPath, lpBuf);
	strcat(szPath, _T("\\"));
	strcat(szPath, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"));

	return TRUE;
}
*/
void  CHttpClient::GetProxyIpAndPort(TCHAR * szIP, ULONG uLen, WORD & dwPort, TCHAR * szValue)
{
/*	TCHAR  * p		= NULL;
	TCHAR szD[]		= "http=";
	p = _tcsstr(szValue, szD);
	int n = 0;
	while(p[n] != '\0' )
	{
		if(p[n] == ':')
		{
			_tcsncpy(szIP, p + 5, n - 5);
			p = p + n + 1;
			dwPort = atoi(p);
			break;
		}
		
		n++;
	}

	TCHAR  * p		= NULL;
	TCHAR szD[]		= "http=";
	p = _tcsstr(szValue, szD);
	if(p == NULL)
		return;

	int n = 0;
	while(p[n] != '\0' )
	{
		if(p[n] == ':')
		{
			_tcsncpy(szIP, p + 5, n - 5);
			p = p + n + 1;
			n = 0;
			continue;
		}
		else if(p[n] == ';')
		{
			TCHAR szP[20] = {0};
			_tcsncpy(szP, p, n);
			dwPort = atoi(szP);
			break;
		}
		
		 n++;
	}*/

	TCHAR  * p		= NULL;
	TCHAR szD[]		= _T("http=");
	p = _tcsstr(szValue, szD);
	if(p == NULL)
	{
		tstring strV = _T("");
		strV.assign(szValue, _tcslen(szValue));
		int np = strV.find(':');
		_tcsncpy_s(szIP, uLen, (TCHAR * )strV.c_str(), np);
		tstring strPort = _T("");
		strPort.assign(strV.c_str() + np + 1, strV.size() - np - 1);
		dwPort = (WORD)_ttol(strPort.c_str());
		return;
	}
	
	int n = 0;
	while(p[n] != '\0' )
	{
		if(p[n] == ':')
		{
			_tcsncpy_s(szIP, uLen, p + 5, n - 5);
			p = p + n + 1;
			n = 0;
			continue;
		}
		else if(p[n] == ';')
		{
			TCHAR szP[20] = {0};
			_tcsncpy_s(szP, 20, p, n);
			dwPort = (WORD)_ttol(szP);
			break;
		}
		
		n++;
	}

}

HRESULT CHttpClient::DownLoadFile(LPCTSTR lpUrl, LPCTSTR lpFileName, std::wstring& strUrlRedirecttion)
{
	HRESULT hr = m_Socket.Create();
	if (FAILED(hr))
		return hr;
	
	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH]={0};
	TCHAR szFilePath[INTERNET_MAX_PATH_LENGTH]={0};

	URL_COMPONENTS uc ;
	ZeroMemory( &uc, sizeof( uc ) ) ;
	uc.dwStructSize = sizeof( uc ) ;
	uc.lpszHostName = szHostName;
	uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH ;
	uc.lpszUrlPath = szFilePath;
	uc.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
	if (InternetCrackUrl(lpUrl, lstrlen(lpUrl), ICU_DECODE, &uc ) == FALSE)
	{
		m_Socket.Close();
		return E_INVALIDARG;
	}

	if (0 == szFilePath[0])
	{
		szFilePath[0] = '/';
		szFilePath[1] = 0;
	}
	hr = m_Socket.SetHost(szHostName);
	if (FAILED(hr))
	{//服务器不对
		m_Socket.Close();
		return hr;
	}
	
	hr = E_FAIL;
	do {
		int nCode = m_Socket.Connect(uc.nPort, m_nTimeOut);
		if (FAILED(nCode))
		{
			hr = nCode;
			break;
		}

		//已连接上，发送请求包
		HRESULT hCode = SendRequest(szHostName, szFilePath, NULL, 0, get, NULL);
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}

		int nContent = RecvHttpResponseHead(0, strUrlRedirecttion);
		if (nContent == E_REDIRECTION && strUrlRedirecttion.length() > 0)
		{
			if (strUrlRedirecttion.find(L"/") == 0)
			{
				strUrlRedirecttion = std::wstring(L"http://") + std::wstring(szHostName) + strUrlRedirecttion; 
			}
		}
		if (nContent == S_FALSE)
			return S_FALSE;//下载文件，如果服务器上文件没有改变，则不再下载
		BOOL bChunkedEncoding = FALSE;
		if (FAILED(nContent))
		{
			if (nContent == E_CHUNKED_ENCODING)
				bChunkedEncoding = TRUE;
			else
			{
				hr = nContent;
				break;
			}
		}

		HANDLE hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
								  NULL,	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			hr = E_CANTOPENFILE;
			break;
		}
		
		nCode = RecvData(hFile, nContent, bChunkedEncoding);
		if (SUCCEEDED(nCode))
		{//成功接收所有数据
			hr = S_OK;			
		}else
			hr = nCode;
		//::SetFileTime(hFile, NULL, NULL, &ftLastModify);
		CloseHandle(hFile);
	} while(0);

	m_Socket.Close();
	return hr;
}

int CHttpClient::RecvData(HANDLE hFile, int nContentLen, BOOL bChunked)
{
	#define HTTP_BUFFER_SIZE 1024*5
	PBYTE pBuffer = new BYTE[HTTP_BUFFER_SIZE+12]; //开出缓冲

	HRESULT hr = E_FAIL;
	BOOL bBreak = FALSE;
	do 
	{
		if (bChunked)
		{
			nContentLen = RecvChunkedSize();
			if(nContentLen == 0)
			{//数据接收完成
				hr = S_OK;
				break;
			}else if (FAILED(nContentLen))
			{
				hr = nContentLen;
				break;
			}
		}
		int  nSize = 0;
		DWORD dwTotalSize = nContentLen;
		DWORD dwRecvSize = 0;

		while(nContentLen > 0)
		{
			if (m_pEvent)
				m_pEvent->OnProgress(dwRecvSize, dwTotalSize);
			//每一次接收1024
			int nRecv = nContentLen > 1024 ? 1024 : nContentLen;
			int nRecved = m_Socket.RecvBlock(pBuffer+nSize, nRecv, m_nTimeOut);
			if (FAILED(nRecved))
			{//接收失败
				hr = nRecved;
				bBreak = TRUE;
				break;
			}
			
			if (nRecved == nRecv)
			{//一块接收成功
				dwRecvSize += nRecv;
				nSize += nRecv;
				if (nSize >= HTTP_BUFFER_SIZE || nContentLen == nRecv)
				{//如果接收到的数据大于
					DWORD dwBytesWritten = 0;
					if (FALSE == WriteFile(hFile, pBuffer, nSize, &dwBytesWritten, NULL))
					{
						hr = E_CANTOPENFILE;
						bBreak = TRUE;
						break;
					}
					FlushFileBuffers(hFile);
					nSize = 0;
				}
				
				nContentLen -= nRecv;
			}
		}
		if (bBreak)
			break;

		if (nContentLen != 0)
			break;
		if (bChunked)
		{//如果是Chunked编码接收掉一个'\r\n'
			SHORT sRecv = 0;
			int nRecv = m_Socket.RecvBlock((LPBYTE)&sRecv, 2, m_nTimeOut);	
			if (nRecv <= 0 && sRecv != '\n\r')
			{
				hr = nRecv;
				break;
			}
		}else
		{
			hr = S_OK;
		}

		if (m_pEvent)
		 	m_pEvent->OnProgress(dwRecvSize, dwTotalSize);
	} while(bChunked);

	if (pBuffer)
	{
		delete pBuffer;
		pBuffer = NULL;
	}

	return hr;
}

int CHttpClient::RecvChunkedSize()
{
	TCHAR szChunkSize[20]={0};
	szChunkSize[0]='0';

	int nBytes = 0;
	for(;nBytes < 20;)
	{//此间接收Chunk的大小
		char cRecv = 0;
		int nRecv = m_Socket.Recv((LPBYTE)&cRecv, 1, m_nTimeOut);	
		if (nRecv <= 0)
		{
			if (nRecv == 0)
				return E_FAIL;

			return nRecv;
		}
		
		if (cRecv == '\r')
			continue;

		if (cRecv == '\n')
			break;

		szChunkSize[nBytes++] = cRecv;
	} 

	if (nBytes >= 20)
		return E_FAIL;

	UINT uChunkSize = 0;
	if (!Hex2Integer(szChunkSize, nBytes, &uChunkSize))
		return E_FAIL;

	return uChunkSize;
}

HRESULT CHttpClient::ReportData(LPCTSTR lpUrl, LPCTSTR lpParam, DWORD dwParamSize, OUT PBYTE pOutBuf, IN OUT DWORD* pdwSize, UINT uType, LPCSTR lpContentType)
{
	HRESULT hr = m_Socket.Create();
	if (FAILED(hr))
		return hr;

	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH]={0};
	TCHAR szFilePath[INTERNET_MAX_PATH_LENGTH]={0};
	TCHAR szCodedPath[INTERNET_MAX_PATH_LENGTH]={0};
	DWORD dwEscapeFNLen = sizeof(szFilePath);

	URL_COMPONENTS uc ;
	ZeroMemory( &uc, sizeof( uc ) ) ;
	uc.dwStructSize = sizeof( uc ) ;
	uc.lpszHostName = szHostName;
	uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH ;
	uc.lpszUrlPath = szCodedPath;
	uc.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
	if (InternetCrackUrl(lpUrl, lstrlen(lpUrl), ICU_DECODE, &uc ) == FALSE)
	{
		m_Socket.Close();
		return E_INVALIDARG;
	}
	
	UrlEscape(szCodedPath, szFilePath, &dwEscapeFNLen, URL_ESCAPE_SPACES_ONLY );


	if (0 == szFilePath[0])
	{
		szFilePath[0] = '/';
		szFilePath[1] = 0;
	}
	hr = m_Socket.SetHost(szHostName);
	if (FAILED(hr))
	{
		m_Socket.Close();
		return hr;
	}

	BOOL bNeedRecv = !!(pOutBuf && pdwSize);

	hr = E_FAIL;

	do 
	{
		HRESULT hCode = m_Socket.Connect(uc.nPort, m_nTimeOut);
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}

		//已连接上，发送请求包
		hCode = SendRequest(szHostName, szFilePath, lpParam, dwParamSize, uType, lpContentType);
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}

		//接收回传数据头
		if (!bNeedRecv)
		{
			hr = S_OK;
			break;
		}

		std::wstring strUrlRedirecttion;
		int nContent = RecvHttpResponseHead(0, strUrlRedirecttion);
		if (nContent == E_REDIRECTION && strUrlRedirecttion.length() > 0)
		{
			if (strUrlRedirecttion.find(L"/") == 0)
			{
				strUrlRedirecttion = std::wstring(L"http://") + std::wstring(szHostName) + strUrlRedirecttion; 
			}
		}
		//此部分暂时不启用Chunked编码
//		BOOL bChunkedEncoding = FALSE;
		if (FAILED(nContent))
		{
			hr = nContent;
//			if (nContent == CHUNKED_ENCODING)
//				bChunkedEncoding = TRUE;
//			else
				break;
		}

		if ((DWORD)nContent > *pdwSize)
			nContent = *pdwSize;

		if (nContent)
		{
			hCode = m_Socket.RecvBlock(pOutBuf, nContent, m_nTimeOut);
			if (hCode == nContent)
			{
				hr = S_OK;
			}else
				hr = hCode;

		}else
			hr = S_OK;

		*pdwSize = nContent;
	} while(0);
	m_Socket.Close();
	return hr;
}

HRESULT CHttpClient::SendRequest(LPCTSTR lpHostName, LPCTSTR lpFilePath, LPCTSTR lpParam, DWORD dwParamSize, UINT uType, LPCSTR lpContentType)
{
const char GETFORMAT[] = "GET %s%s%s HTTP/1.1\r\n"
						 "Host: %s\r\n"
						 "%s"//此行主要是为了适应If-Modified-Since: Wed, 07 Jun 2006 19:35:34 GMT
						 "User-Agent: Mozilla/4.0  %s\r\n"
						 "Cache-Control: no-cache\r\n\r\n";

const char POSTFORMAT[] = "POST %s HTTP/1.1\r\n"
					      "Host: %s\r\n"
						  "%s"//此行主要是为了适应If-Modified-Since: Wed, 07 Jun 2006 19:35:34 GMT
 						  "Content-Type: %s\r\n" //Content-Type: multipart/form-data; boundary=---------------------------7d72fd2c2c0b6a
												 //Content-Type: application/x-www-form-urlencoded
						  "User-Agent: Mozilla/4.0\r\n"
						  "Content-Length: %u\r\n"
						  "Pragma: no-cache\r\n\r\n";


	int nMaxLen = INTERNET_MAX_URL_LENGTH + MAX_PATH + dwParamSize;
	char* pSendBuffer = new char[nMaxLen];
	if (NULL == pSendBuffer)
		return E_OUTOFMEMORY;

	char szFormat[MAX_PATH] = {0};
// 	if (lpLocalModifyTime)
// 	{//如果本地修改时间存在
// 		TCHAR szTime[100]={0};
// 		if (FileTimeToString(*lpLocalModifyTime, szTime, 100))
// 		{
//             std::string strTemp = ws2s(szTime, CP_ACP);
// 			_snprintf_s(szFormat, MAX_PATH,"If-Modified-Since: %s\r\n",strTemp.c_str());
// 		}else
// 			szFormat[0] = 0;
// 	}else
		szFormat[0] = 0;


	int nSendLen = 0;
	switch(uType)
	{
	case get:
		{
			if (dwParamSize && lpParam)
			{
				std::string strPath = ws2s(lpFilePath, CP_ACP);
				std::string strHostName = ws2s(lpHostName, CP_ACP);
				std::string strParam = ws2s(lpParam, CP_ACP);
				nSendLen = _snprintf_s(pSendBuffer, nMaxLen, _TRUNCATE, GETFORMAT, strPath.c_str(), "?", strParam.c_str(), strHostName.c_str(), szFormat, g_strGuid.c_str());
			}
			else
			{
				std::string strPath = ws2s(lpFilePath, CP_ACP);
				std::string strHostName = ws2s(lpHostName, CP_ACP);
				nSendLen = _snprintf_s(pSendBuffer, nMaxLen, _TRUNCATE, GETFORMAT, strPath.c_str(), "", "", strHostName.c_str(), szFormat, g_strGuid.c_str());
			}
		}
		break;
	case post:
		{
			if (lpContentType == NULL)
				lpContentType = "application/x-www-form-urlencoded";
			std::string strPath = ws2s(lpFilePath, CP_ACP);
			std::string strHostName = ws2s(lpHostName, CP_ACP);
			nSendLen = _snprintf_s(pSendBuffer, nMaxLen, _TRUNCATE, POSTFORMAT, strPath.c_str(), strHostName.c_str(), szFormat, lpContentType, dwParamSize);
			if (lpParam && dwParamSize)
			{//把参数加到尾里面去
				memcpy(pSendBuffer+nSendLen, lpParam, dwParamSize);
				nSendLen += dwParamSize;
			}
		}
		break;
	}


	int nCode = m_Socket.SendBlock((PBYTE)pSendBuffer, nSendLen, m_nTimeOut);
	if (SUCCEEDED(nCode))
	{
		delete[] pSendBuffer;
		return S_OK;
	}

	delete[] pSendBuffer;
	return nCode;
}

HRESULT CHttpClient::SendResumeRequest(LPCTSTR lpHostName, LPCTSTR lpFilePath, DWORD dwSize)
{
const char GETFORMAT[] = "GET %s HTTP/1.1\r\n"
						 "User-Agent: Mozilla/4.0\r\n"
						 "Host: %s\r\n"
						 "Range:bytes=%d-\r\n"
						 "Cache-Control: no-cache\r\n\r\n";

	int nMaxLen = INTERNET_MAX_URL_LENGTH + MAX_PATH;
	char* pSendBuffer = new char[nMaxLen];
	if (NULL == pSendBuffer)
		return E_OUTOFMEMORY;
	std::string strPath = ws2s(lpFilePath, CP_ACP);
	std::string strHostName = ws2s(lpHostName, CP_ACP);
	int nSendLen = _snprintf_s(pSendBuffer, nMaxLen, _TRUNCATE, GETFORMAT, strPath.c_str(), strHostName.c_str(), dwSize);

	HRESULT hr = m_Socket.SendBlock((PBYTE)pSendBuffer, nSendLen, m_nTimeOut);

	delete[] pSendBuffer;
	return hr;
}
/*如果是chunked编码，会收到如下数据包
HTTP/1.1 200 OK
Content-Type: image/x-icon
Last-Modified: Wed, 07 Jun 2006 19:35:34 GMT
Expires: Sun, 17 Jan 2038 19:14:07 GMT
Server: GWS/2.1
Transfer-Encoding: chunked
Date: Fri, 20 Apr 2007 20:10:53 GMT
*/
//内容返回长度
HRESULT CHttpClient::RecvHttpResponseHead(int nFileSize, std::wstring& strUrlRedirect)
{
	const unsigned int BUFFER_LEN = 10;
	unsigned int uBuffLen = BUFFER_LEN;
	char* pBuff = new char[uBuffLen + 1];
	unsigned int nBytes = 0;
	int nEnter = 0;
	union
	{
		WORD wValue;
		BYTE pValue[2];
	} RecvBuf;

	RecvBuf.wValue = 0;
	do 
	{
		RecvBuf.wValue = 0;
		int nRecv = m_Socket.Recv(RecvBuf.pValue, 1, m_nTimeOut);	
		if (FAILED(nRecv))
			return nRecv;

		if (0 == nRecv)
			return E_FAIL;
		if (nBytes >= uBuffLen)
		{
			uBuffLen += BUFFER_LEN;
			char* pBuffNew = new char[uBuffLen + 1];
			memcpy(pBuffNew, pBuff, uBuffLen - BUFFER_LEN);
			delete pBuff;
			pBuff = pBuffNew;
		}
		*(short*)(pBuff+nBytes) = RecvBuf.wValue;
		if (RecvBuf.pValue[0] == '\n')
			nEnter ++;
		else if (RecvBuf.pValue[0] != '\r')
			nEnter = 0;
		nBytes ++;
		
	} while(nEnter != 2);

	//char szTime[100];
    

	int nCode = 0;
	sscanf_s(pBuff, "%*s %d %*s", &nCode, sizeof(nCode));

	if (nCode == 304)
	{
		delete pBuff;
		return S_FALSE; 
	}

    
	//首先查找文件的最后修改日期
//	Last-Modified: Wed, 07 Jun 2006 19:35:34 GMT
// 	LPSTR lpLastModify = StrStrIA(pBuff, "Last-Modified:");
// 	BOOL bGetTime = FALSE;
// 	if (lpLastModify)
// 	{
// 		lpLastModify += sizeof("Last-Modified:")-1;
// 
// 		sscanf_s(lpLastModify, "%*[ ]%100[^\r\n]", szTime, sizeof(szTime));
// 		std::wstring strTime = s2ws(szTime, CP_ACP);
// 		if (TimeStringToFileTime(strTime.c_str(), ftLastModify))
// 			bGetTime = TRUE;
// 	}
// 
// 
// 	if (!bGetTime)
// 	{//没有时间就
// 		SYSTEMTIME st;
// 		GetSystemTime(&st);
// 		SystemTimeToFileTime(&st, &ftLastModify);
// 	}
	
	if (nCode == 416)
	{//服务器不能满足客户在请求中指定的Range头
		delete pBuff;
		return E_OVERFLOW;
		//如果是chunked编码,说明文件头取错,下面这段代码不同的情况下不同
// 		if (StrStrI(szBuf, "Transfer-Encoding: chunked"))
// 		{//此处应该返回错误
// 			return 0;
// 		}
// 
// 		LPSTR pRange = StrStrI(szBuf, "Content-Range:");
// 		if (pRange == NULL)
// 		{
// 			return E_FAIL;
// 		}
// 
// 		int nLen = 0;
// 		sscanf(pRange,"%*[^/]/%d", &nLen);
// 		if (nLen == nFileSize)
// 		{
// 			return S_OK;
// 		}
// 		return E_FAIL;
	}
	if (nCode >= 300)
	{
		if (nCode == 300 || nCode == 301 || nCode == 302 || nCode == 303 || nCode == 307)// || nCode == 301 || )
		{
			LPSTR lpLocation = StrStrIA(pBuff, "Location:");
			if (lpLocation)
			{
				lpLocation += sizeof("Location:") - 1;
				char szUrlNew[INTERNET_MAX_URL_LENGTH] = {0};
				sscanf_s(lpLocation, "%*[ ]%[^\r^\n]", szUrlNew, INTERNET_MAX_URL_LENGTH);
				if (strlen(szUrlNew) > 0)
				{
					strUrlRedirect = s2ws(szUrlNew, CP_ACP);
					delete pBuff;
					return E_REDIRECTION;
				}
			}
		}
		delete pBuff;
		return E_FAIL;
	}

	//如果是chunked编码
	if (StrStrIA(pBuff, "Transfer-Encoding: chunked"))
		return E_CHUNKED_ENCODING;

	LPSTR pContent = StrStrIA(pBuff, "Content-Length:");
	if (pContent == NULL)
	{//正常下载没有长度,可能是chunked编码

		delete pBuff;
		return E_FAIL;
	}

	pContent += sizeof("Content-Length");
	int nLen = E_FAIL;
	sscanf_s(pContent,"%d", &nLen, sizeof(nLen));
	
	delete pBuff;
	return nLen;
}

HRESULT  CHttpClient::ResumeDown1(LPCTSTR lpUrl, LPCTSTR lpFileName)
{
	HRESULT hr = S_OK;
	
	TCHAR szIP[256] = {0};
	WORD dwPort	= 0;
	if(this->GetProxyEnable(szIP, 256, dwPort))
	{
		hr = this->ResumeDownFromProxy(szIP, dwPort, lpUrl, lpFileName);
	}
	else
	{
		hr = this->ResumeDown(lpUrl, lpFileName);
	}
	
	return hr;
}

HRESULT  CHttpClient::ResumeDownFromProxy(TCHAR * szIP, WORD dwPort, LPCTSTR lpUrl, LPCTSTR lpFileName)
{
	HRESULT hr = m_Socket.Create();
	if (FAILED(hr))
		return hr;
	
	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH]={0};
	TCHAR szFilePath[INTERNET_MAX_PATH_LENGTH]={0};
	
	URL_COMPONENTS uc ;
	ZeroMemory( &uc, sizeof( uc ) ) ;
	uc.dwStructSize = sizeof( uc ) ;
	uc.lpszHostName = szHostName;
	uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH ;
	uc.lpszUrlPath = szFilePath;
	uc.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
	if (InternetCrackUrl(lpUrl, lstrlen(lpUrl), ICU_DECODE, &uc ) == FALSE)
	{
		m_Socket.Close();
		return E_INVALIDARG;
	}
	
	if (0 == szFilePath[0])
	{
		szFilePath[0] = '/';
		szFilePath[1] = 0;
	}
	hr = m_Socket.SetHost(szIP);
	if (FAILED(hr))
	{
		m_Socket.Close();
		return hr;
	}
	
	
	hr = E_FAIL;
	do {
		HRESULT hCode = m_Socket.Connect(dwPort, m_nTimeOut);
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}
		
		//这里取出本地文件长度
		WIN32_FIND_DATA wfd={0};
		HANDLE hFind = FindFirstFile(lpFileName, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			wfd.nFileSizeLow = 0;			
		}else
		{
			FindClose(hFind);
		}
		
		hCode = SendResumeRequest(szHostName, lpUrl, wfd.nFileSizeLow);
		//已连接上，发送请求包
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}
		
		std::wstring strUrlRedirecttion;
		int nContent = RecvHttpResponseHead(wfd.nFileSizeLow, strUrlRedirecttion);
		BOOL bChunkedEncoding = FALSE;
		if (nContent < 0)
		{
			if (nContent == E_CHUNKED_ENCODING)
				bChunkedEncoding = TRUE;
			else
			{
				hr = nContent;
				break;
			}
		}
		
		if (nContent == 0)
		{
			hr = S_OK;
			break;
		}
		
		HANDLE hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE,  FILE_SHARE_READ, 
			NULL,	OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (INVALID_HANDLE_VALUE == hFile)
		{
			hr = E_CANTOPENFILE;
			break;
		}
		
		SetFilePointer(hFile, 0,0,FILE_END); //本地文件Seek到尾部
		hr = RecvData(hFile, nContent, bChunkedEncoding);
		
		//::SetFileTime(hFile, NULL, NULL, &ftLastModify);
		CloseHandle(hFile);
	} while(FALSE);
	
	m_Socket.Close();
	return hr;	
}

HRESULT CHttpClient::ResumeDown(LPCTSTR lpUrl, LPCTSTR lpFileName)
{//断点续传
	HRESULT hr = m_Socket.Create();
	if (FAILED(hr))
		return hr;
	
	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH]={0};
	TCHAR szFilePath[INTERNET_MAX_PATH_LENGTH]={0};

	URL_COMPONENTS uc ;
	ZeroMemory( &uc, sizeof( uc ) ) ;
	uc.dwStructSize = sizeof( uc ) ;
	uc.lpszHostName = szHostName;
	uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH ;
	uc.lpszUrlPath = szFilePath;
	uc.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
	if (InternetCrackUrl(lpUrl, lstrlen(lpUrl), ICU_DECODE, &uc ) == FALSE)
	{
		m_Socket.Close();
		return E_INVALIDARG;
	}

	if (0 == szFilePath[0])
	{
		szFilePath[0] = '/';
		szFilePath[1] = 0;
	}
	hr = m_Socket.SetHost(szHostName);
	if (FAILED(hr))
	{
		m_Socket.Close();
		return hr;
	}


	hr = E_FAIL;
	do {
		HRESULT hCode = m_Socket.Connect(uc.nPort, m_nTimeOut);
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}

		//这里取出本地文件长度
		WIN32_FIND_DATA wfd={0};
		HANDLE hFind = FindFirstFile(lpFileName, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			wfd.nFileSizeLow = 0;			
		}else
		{
			FindClose(hFind);
		}
	
		hCode = SendResumeRequest(szHostName, szFilePath, wfd.nFileSizeLow);
		//已连接上，发送请求包
		if (FAILED(hCode))
		{
			hr = hCode;
			break;
		}
		
		std::wstring strUrlRedirecttion;
		int nContent = RecvHttpResponseHead(wfd.nFileSizeLow, strUrlRedirecttion);
		BOOL bChunkedEncoding = FALSE;
		if (nContent < 0)
		{
			if (nContent == E_CHUNKED_ENCODING)
				bChunkedEncoding = TRUE;
			else
			{
				hr = nContent;
				break;
			}
		}

		if (nContent == 0)
		{
			hr = S_OK;
			break;
		}
		
		HANDLE hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE,  FILE_SHARE_READ, 
								  NULL,	OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			hr = E_CANTOPENFILE;
			break;
		}

		SetFilePointer(hFile, 0,0,FILE_END); //本地文件Seek到尾部
		hr = RecvData(hFile, nContent, bChunkedEncoding);

		//::SetFileTime(hFile, NULL, NULL, &ftLastModify);
		CloseHandle(hFile);
	} while(FALSE);

	m_Socket.Close();
	return hr;	
}

void CHttpClient::Abort()
{
	m_Socket.Close();
}

void CHttpClient::SetCallbackPoint(IHttpEvent* pEvent)
{
	m_pEvent = pEvent;
}
