//#include "stdafx.h"
#include "captureScreen.h"
#include "Utilities/Util.h"
#include "Utilities/eOSVer.h"
#include "Utilities/Singleton.h"
#include "Service/WinPrivilege.h"
#include "Strings/StringsHelper.h"
#include <wtsapi32.h>
#pragma comment(lib,"Wtsapi32.lib")
#include <userenv.h>
#pragma comment(lib,"Userenv.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")

#define BUF_SIZE 4096

/**
* GDI 截屏函数
*
* 参数 hwnd   要截屏的窗口句柄
* 参数 dirPath    截图存放目录
* 参数 filename 截图名称
*/
DWORD CCaptureScreen::CaptureScreenImage(HWND hwnd)
{
	DWORD retFileId = 0;
	HANDLE hDIB;
	DWORD dwBmpSize;
	DWORD dwSizeofDIB;
	DWORD dwBytesWritten;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bi;
	CHAR* lpbitmap;
	INT width = GetSystemMetrics(SM_CXSCREEN);  // 屏幕宽
	INT height = GetSystemMetrics(SM_CYSCREEN);  // 屏幕高
	HDC hdcScreen = GetDC(NULL); // 全屏幕DC
	HDC hdcMemDC = CreateCompatibleDC(hdcScreen); // 创建兼容内存DC

	if (!hdcMemDC)
	{
		printf("CreateCompatibleDC has failed");
		goto done;
	}

	// 通过窗口DC 创建一个兼容位图
	hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);

	if (!hbmScreen)
	{
		printf("CreateCompatibleBitmap Failed");
		goto done;
	}

	// 将位图块传送到我们兼容的内存DC中
	SelectObject(hdcMemDC, hbmScreen);
	if (!BitBlt(
		hdcMemDC,    // 目的DC
		0, 0,        // 目的DC的 x,y 坐标
		width, height, // 目的 DC 的宽高
		hdcScreen,   // 来源DC
		0, 0,        // 来源DC的 x,y 坐标
		SRCCOPY))    // 粘贴方式
	{
		printf(("BitBlt has failed"));
		goto done;
	}

	// 获取位图信息并存放在 bmpScreen 中
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// 在 32-bit Windows 系统上, GlobalAlloc 和 LocalAlloc 是由 HeapAlloc 封装来的
	// handle 指向进程默认的堆. 所以开销比 HeapAlloc 要大
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char*)GlobalLock(hDIB);

	// 获取兼容位图的位并且拷贝结果到一个 lpbitmap 中.
	GetDIBits(
		hdcScreen,  // 设备环境句柄
		hbmScreen,  // 位图句柄
		0,          // 指定检索的第一个扫描线
		(UINT)bmpScreen.bmHeight, // 指定检索的扫描线数
		lpbitmap,   // 指向用来检索位图数据的缓冲区的指针
		(BITMAPINFO*)& bi, // 该结构体保存位图的数据格式
		DIB_RGB_COLORS // 颜色表由红、绿、蓝（RGB）三个直接值构成
		);

	retFileId = GetTickCount();
	WCHAR szCaptureFilePath[MAX_PATH] = { 0 };
	swprintf_s(szCaptureFilePath, L"%s%d.png", Util::help::GetProgramRunDir().c_str(), retFileId);
	DebugOutputMsg(_T("catpture path[%s], dwFileID=[%d]"), szCaptureFilePath, retFileId);

	// 将 图片头(headers)的大小, 加上位图的大小来获得整个文件的大小
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	// 设置 Offset 偏移至位图的位(bitmap bits)实际开始的地方
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER);
	// 文件大小
	bmfHeader.bfSize = dwSizeofDIB;
	// 位图的 bfType 必须是字符串 "BM"
	bmfHeader.bfType = 0x4D42; //BM

	// 创建一个文件来保存文件截图
	HANDLE hFile = CreateFile(
		szCaptureFilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	dwBytesWritten = 0;
	WriteFile(hFile, (LPSTR)& bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)& bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
	// 解锁堆内存并释放
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// 关闭文件句柄
	CloseHandle(hFile);

	// 清理资源
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);

	return retFileId;
}

//根据不同的消息类型，执行不同的
void CCaptureScreen::controlExec(precvmessage precv, SOCKET socket)
{
	DebugOutputMsg(_T("controlExec type= %d"), precv->type);
	sendmessage sendmsg = { 0 };
	switch (precv->type)
	{
	case 1:
	{
			  DebugOutputMsg(_T("主页地址：%S"), precv->data);

			  //这里的注册表地址，和当前登陆用户有关。  RegOpenKeyA(HKEY_CURRENT_USER, regname, &hkResult)无法直接使用
			  char regname[] = "S-1-5-21-3843328830-909763758-3822778723-1000\\Software\\Microsoft\\Internet Explorer\\Main";
			  //HKEY hkCuruser;
			  //LSTATUS user_ret=RegOpenCurrentUser(KEY_ALL_ACCESS,&hkCuruser);
			  //if (user_ret != ERROR_SUCCESS)
			  //{
			  //	//sprintf_s(sendmsg.data, "%d", user_ret);
			  //	printf("OpenError\n");
			  //}
			  HKEY hkResult;
			  LSTATUS intret = RegOpenKeyA(HKEY_USERS, regname, &hkResult);
			  if (intret != ERROR_SUCCESS)
			  {
				  sprintf_s(sendmsg.data, "%d", intret);
				  printf("OpenError\n");
			  }

			  LSTATUS  ret = RegSetValueEx(hkResult, L"Start Page", 0, REG_SZ, (unsigned char*)precv->data, (wcslen(precv->data) * sizeof(WCHAR)+2));
			  if (ret != ERROR_SUCCESS)
			  {
				  memcpy(sendmsg.data, "homepage changed error", strlen("homepage changed error") + 1);
				  sprintf_s(sendmsg.data, "%d", ret);
			  }
			  else
			  {
				  memcpy(sendmsg.data, "homepage changed success", strlen("homepage changed success") + 1);
			  }
			  RegCloseKey(hkResult);
			  sendmsg.type = 1;
			  send(socket, (char*)& sendmsg, 1024, 0);
			  return;
	}
	case 2:
	{
			  wchar_t filename[MAX_PATH] = { 0 };
			  HMODULE hmoudle = getCurrModuleHandle();
			  GetModuleFileName(hmoudle, filename, MAX_PATH);

			  Utilities::CeOSVer osv;
			  if (!osv.IsVistaOrLater())
			  {
				 // NT系列需要提升权限
				 Singleton<CWinPrivilege>::instance()->EnableSeviceDebugPrivilege();
			  }

			  TCHAR szCmd[MAX_PATH] = { 0 };
			  swprintf_s(szCmd, L" -%s", CMDLINE_PARAM_NAME_CAPTURE);
			  //突破session0保护
			  DWORD dwCaptureID = CreateUserProcess(filename, szCmd);
			  //if (dwCaptureID == 0)
			  {
				  DebugOutputMsg(_T("filename is [%s],szcmd=[%s],dwCaptureID=[%d]"), filename, szCmd, dwCaptureID);
				  //Singleton<CWinPrivilege>::instance()->ExecuteW(filename, szCmd, NULL, true, true);
				 // DebugOutputMsg(_T("is sleeping.."));
				  //Sleep(5000);
				  Singleton<CWinPrivilege>::instance()->LaunchAppAsAdminUser(filename, szCmd, dwCaptureID,true);
				  DebugOutputMsg(_T("LaunchAppAsAdminUser dwCaptureID=[%d]"), dwCaptureID);
				  //return;
			  }
			  //等待图片创建完成
			  //Sleep(2000);
			  sendmsg.type = 2;
			  memcpy(sendmsg.data, "Screenshoot png", strlen("Screenshoot png") + 1);
			  //先发送过去，进入，循环接收
			  send(socket, (char*)& sendmsg, 1024, 0);

			  //打开文件
			  TCHAR szFilePath[MAX_PATH] = {0};
			  //HMODULE hmoudle3 = getCurrModuleHandle();
			  //GetModuleFileNameA(hmoudle3, FilePath3, 200);
			  //(strrchr(FilePath3, _T('\\')))[1] = 0; // 删除文件名，只获得路径字串
			  //strcat_s(FilePath3, "123.png");
			  swprintf_s(szFilePath, _T("%s%d.png"), Util::help::GetProgramRunDir().c_str(), dwCaptureID);
			  DebugOutputMsg(_T("send capture image path=[%s]"),szFilePath);
			  FILE* fp = NULL;
			  fopen_s(&fp, strings::ws2s(szFilePath).c_str(), "rb+");
			  if (fp == NULL)
			  {
				  DebugOutputMsg(_T("open capture image failed, lastError=[%d]"), GetLastError());
				  return;
			  }

			  char buffer[BUF_SIZE] = { 0 };  //缓冲区
			  int nCount;
			  while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
				  send(socket, buffer, nCount, 0);
				  Sleep(10);
			  }
			  fclose(fp);
		
			  return;
	}
	default:
		return;
	}
}


//messagetype  1  修改主页
//messgaetype  2  屏幕截图
bool CCaptureScreen::SetTcpServer()
{
	//1.加载套接字库
	WORD w_version_req = MAKEWORD(2, 2); //初始化WinSock版本号
	WSADATA wsaData;
	int flag_InitWSA = WSAStartup(w_version_req, &wsaData); //具体参数含义参考：https://blog.csdn.net/clemontine/article/details/53141041
	//flag_InitWSA 不为0则说明初始化失败
	if (flag_InitWSA != 0)
	{
		DebugOutputMsg(_T("WSAStartup failed,error=[%d]"),flag_InitWSA);
		return false;
	}
	//wsaData为空指针，说明初始化失败
	if (&wsaData == nullptr)
	{
		DebugOutputMsg(_T("WSAStartup failed, wsaData is null."));
		return false;
	}

	//2.创建套接字
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN socketAddr;
	socketAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //需要接收任意的IP地址
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(8888);//端口号
	
	DebugOutputMsg(_T("SetTcpServer begin to bind"));
	//3.连接套接字
	int ret = bind(socketServer, (SOCKADDR*)& socketAddr, sizeof(SOCKADDR));
	if (ret)
	{
		DebugOutputMsg(_T("SetTcpServer error=[%d]."),ret); 
		return false;
	}
	
	DebugOutputMsg(_T("SetTcpServer listening server."));
	//4.设置监听客户端数量
	listen(socketServer, 5);

	struct sockaddr_in serverAddr;

	SOCKADDR_IN sockclient;
	int len = sizeof(SOCKADDR);
	SOCKET sockCon = accept(socketServer, (SOCKADDR*)& sockclient, &len);//创建链接套接字
	
	DebugOutputMsg(_T("SetTcpServer accepted."));


	sendmessage m_send = { 0 };
	m_send.type = 0;
	memcpy(m_send.data, "welcome", 8);
	//5.发送数据到客户端

	send(sockCon, (char*)& m_send, 1024, 0);
	//6.接收客户端发来的消息
	while (1)
	{
		recvmessage m_recv = { 0 };
		recv(sockCon, (char*)& m_recv, 104, 0);//发送\接收函数
		controlExec(&m_recv, sockCon);
	}

	//7.关闭服务器
	DebugOutputMsg(_T("SetTcpServer close server."));
	closesocket(sockCon);//关闭套接字
	WSACleanup();
	return true;
}

DWORD CCaptureScreen::CreateUserProcess(WCHAR* lpszFileName, WCHAR* cmd)
{
	DWORD returnCode = 0;
	DWORD dwSessionID = 0;
	HANDLE hToken = NULL;
	HANDLE hDuplicatedToken = NULL;
	LPVOID lpEnvironment = NULL;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	do
	{
		// 获得当前Session ID
		dwSessionID = ::WTSGetActiveConsoleSessionId();
		// 获得当前Session的用户令牌
		if (FALSE == ::WTSQueryUserToken(dwSessionID, &hToken))
		{
			DebugOutputMsg(_T("WTSQueryUserToken failed, lastError=[%d]"), GetLastError());
			break;
		}
		// 复制令牌
		if (FALSE == ::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
			SecurityIdentification, TokenPrimary, &hDuplicatedToken))
		{
			//ShowMessage("DuplicateTokenEx", "ERROR");
			DebugOutputMsg(_T("DuplicateTokenEx failed, lastError=[%d]"), GetLastError());
			break;
		}
		// 创建用户Session环境
		if (FALSE == ::CreateEnvironmentBlock(&lpEnvironment,
			hDuplicatedToken, FALSE))
		{
			//ShowMessage("CreateEnvironmentBlock", "ERROR");
			DebugOutputMsg(_T("CreateEnvironmentBlock failed, lastError=[%d]"), GetLastError());
			break;
		}
		// 在复制的用户Session下执行应用程序，创建进程
		if (FALSE == ::CreateProcessAsUser(hDuplicatedToken,
			lpszFileName, cmd, NULL, NULL, FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
			lpEnvironment, NULL, &si, &pi))
		{
			//ShowMessage("CreateProcessAsUser", "ERROR");
			DebugOutputMsg(_T("CreateProcessAsUser failed, filename=[%s], cmd=[%s],lastError=[%d]"), lpszFileName,cmd,GetLastError());
			break;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &returnCode);
		DebugOutputMsg(_T("CreateUserProcess returnCode=%d"), returnCode);
	} while (FALSE);
	// 关闭句柄, 释放资源
	if (lpEnvironment)
	{
		::DestroyEnvironmentBlock(lpEnvironment);
	}
	if (hDuplicatedToken)
	{
		::CloseHandle(hDuplicatedToken);
	}
	if (hToken)
	{
		::CloseHandle(hToken);
	}
	return returnCode;
}



HMODULE CCaptureScreen::getCurrModuleHandle()
{
	MEMORY_BASIC_INFORMATION info;
	::VirtualQuery((LPCVOID)(&getCurrModuleHandle), &info, sizeof(info));

	return (HMODULE)info.AllocationBase;
}