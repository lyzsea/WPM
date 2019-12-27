// HttpClient.h: interface for the CHttpClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPCLIENT_H__0A1022FF_FF3E_4690_8726_E8D888C51D96__INCLUDED_)
#define AFX_HTTPCLIENT_H__0A1022FF_FF3E_4690_8726_E8D888C51D96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define _WIN32_WINNT  0x0500

#include <string>
using namespace std;

#include <Sddl.h>

#include "TcpipSocket.h"

#define myheapalloc(x) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x))
#define myheapfree(x)  (HeapFree(GetProcessHeap(), 0, x))

class IHttpEvent
{
public:
	virtual void __stdcall OnHttpError(DWORD dwCode, LPCTSTR lpError) = 0;
	virtual void __stdcall OnProgress(DWORD dwLength, DWORD dwTotalSize) = 0;
};
//各种返回值函义
/*
S_OK	成功
S_FALSE 文件没有改变
E_CHUNKED_ENCODING //CHUNKED编码
E_FAIL	错误
E_TIMEOUT	超时
E_INVALIDARG 参数错误
E_ABORT	用户终止
E_NOCONNECTION 连接不存在
E_OUTOFMEMORY  内存不足
E_CANTOPENFILE 文件无法打开
E_OVERFLOW	   设置长度溢出
*/
#define E_OVERFLOW DISP_E_OVERFLOW
#define E_CHUNKED_ENCODING CRYPT_E_BAD_ENCODE
#define E_CANTOPENFILE MK_E_CANTOPENFILE
#define E_REDIRECTION  _HRESULT_TYPEDEF_(0x8019012EL)

class CHttpClient  
{
public:
	enum{get=0, post};
	CHttpClient();
	virtual ~CHttpClient();
	void SetTimeOut(int nTimeOut);
	HRESULT DownLoadFile(LPCTSTR lpUrl, LPCTSTR lpFileName, std::wstring& strUrlRedirecttion);
	
	HRESULT		DownLoadFile1(LPCTSTR lpUrl, LPCTSTR lpFileName, std::wstring& strUrlRedirecttion);
	BOOL		GetProxyEnable(TCHAR * szIP, ULONG uLen, WORD & dwPort);
	void		GetProxyIpAndPort(TCHAR * szIP, ULONG uLen, WORD & dwPort, TCHAR * szValue);
	HRESULT		DownLoadFileFromProxy(TCHAR * szIP, WORD & dwPort, LPCTSTR lpUrl, LPCTSTR lpFileName, std::wstring& strUrlRedirecttion);

	HRESULT		ResumeDown1(LPCTSTR lpUrl, LPCTSTR lpFileName);
	HRESULT		ResumeDownFromProxy(TCHAR * szIP, WORD dwPort, LPCTSTR lpUrl, LPCTSTR lpFileName);
//	BOOL		GetInternetSettingPath(TCHAR * szPath);

	HRESULT ResumeDown(LPCTSTR lpUrl, LPCTSTR lpFileName);//断点续传
	HRESULT ReportData(LPCTSTR lpUrl, LPCTSTR lpParam = NULL, DWORD dwParamSize = 0, 
					   OUT PBYTE pOutBuf = NULL, IN OUT DWORD* pdwSize = NULL, UINT uType = get, 
					   LPCSTR lpContentType = NULL);
	void Abort();
	void SetCallbackPoint(IHttpEvent* pEvent);
	BOOL GetServerTime(FILETIME* ft);
protected:
	HRESULT SendRequest(LPCTSTR lpHostName, LPCTSTR lpFilePath, LPCTSTR lpParam, DWORD dwParamSize, UINT uType, LPCSTR lpContentType = NULL);
	HRESULT SendResumeRequest(LPCTSTR lpHostName, LPCTSTR lpFilePath, DWORD dwSize);
	HRESULT RecvHttpResponseHead(int nFileSize, std::wstring& strUrlRedirect);
	int RecvData(HANDLE hFile, int nContentLen, BOOL bChunked);
private:
	int RecvChunkedSize();
	int m_nTimeOut;
	CTcpipSocket m_Socket;
	IHttpEvent* m_pEvent;
	LARGE_INTEGER m_timeServer;
};

#endif // !defined(AFX_HTTPCLIENT_H__0A1022FF_FF3E_4690_8726_E8D888C51D96__INCLUDED_)
