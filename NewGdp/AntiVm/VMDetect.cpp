#include "../stdafx.h"
#include "VMDetect.h"
#include "StringMatch.h"
#include <Windows.h>
#include <winioctl.h>
#include <string>
#include <stdio.h>
#include "../src/Safe/ConvStr.h"


static UINT GetSystemDir(char *szOutStr, UINT nSize);
static bool DirectoryExists(const char *szDirName);
static bool CheckDir(const char *szRootDir, const char *szChildDir);
static bool CheckReg(const char *szSubKey, const char *szValue,
                     char *szOutBuf, int nBufSize);
static bool _strstr(char *substr, char *str);



using namespace std;
string ReplaceString(const string &srcStr, const string &oldStr, const string &newStr);
string ConvertName(char const * name);

bool DetectVM()
{
	#define BUF_SIZE 1024
	static char szBuf[BUF_SIZE] = {'\0'};
//		if (IsVMWare()) return true;
/*	if (IsVPC()) return true;


	DWORD dwUsl = BUF_SIZE;
	if (!GetUserNameA(szBuf, &dwUsl)) return true;
	if (0 == lstrcmpA(szBuf, "CurrentUser")) return true;
	if (0 == lstrcmpA(szBuf, "Sandbox")) return true;

	dwUsl = BUF_SIZE;
	if (!GetComputerNameA(szBuf, &dwUsl)) return true;
	if (0 == lstrcmpA(szBuf, "SANDBOX")) return true;

	if (!CheckReg("HARDWARE\\DESCRIPTION\\System",
								"SystemBiosVersion", szBuf, BUF_SIZE)) return true;
	if (_strstr("VBOX", szBuf)) return true;

	if (!CheckReg("HARDWARE\\DESCRIPTION\\System",
								"VideoBiosVersion", szBuf, BUF_SIZE)) return true;
	if (_strstr("VirtualBox", szBuf)) return true;
	
	if (IsVMHD())  return true;
	*/
	//VIRTUALIZER1_END
	if (IsVMHD())  return true;

	return false;
}

bool IsVMHD()  //加入判断流程,检测当前虚拟主机
{
	bool res = false;
	std::string struid = w2a(g_szGuid);
	//char uid[MAX_PATH] = {0};
	//getGUID(uid, MAX_PATH);

	return  IsFindHdString((char*)struid.c_str());
}

bool IsFindHdString(char* csUid)
{
	bool res = false;

	if (csUid != NULL)
	{
		res = MatchingString(csUid, "*Virtual*", false);
	}

	return res;
}

//*********************************************************************
bool IsVMWare()
{
	bool res = true;

	__try {
		__asm
		{
			push   edx
			push   ecx
			push   ebx

			mov    eax, 'VMXh'
			mov    ebx, 0      // any value but not the MAGIC VALUE
			mov    ecx, 10     // get VMWare version
			mov    edx, 'VX'   // port number

			in     eax, dx     // read port
                         // on return EAX returns the VERSION
			cmp    ebx, 'VMXh' // is it a reply from VMWare?
			setz   [res]       // set return value

			pop    ebx
			pop    ecx
			pop    edx
		}
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		res = false;
	}

	return res;
}

//*********************************************************************
// IsInsideVPC's exception filter
DWORD __forceinline IsInsideVPC_exceptionFilter(LPEXCEPTION_POINTERS ep)
{
  PCONTEXT ctx = ep->ContextRecord;

  ctx->Ebx = -1; // Not running VPC

  ctx->Eip += 4; // skip past the "call VPC" opcodes

  return EXCEPTION_CONTINUE_EXECUTION;
  // we can safely resume execution since we skipped faulty instruction
}

//*********************************************************************
bool IsVPC()
{
  bool res = false;

  __try
  {
    _asm push ebx
    _asm mov  ebx, 0 // It will stay ZERO if VPC is running

    _asm mov  eax, 1 // VPC function number


    // call VPC 

    _asm __emit 0Fh
    _asm __emit 3Fh
    _asm __emit 07h
    _asm __emit 0Bh

    _asm test ebx, ebx
    _asm setz [res]
    _asm pop ebx
  }
  // The except block shouldn't get triggered if VPC is running!!

  __except(IsInsideVPC_exceptionFilter(GetExceptionInformation()))
  {
  }

  return res;
}

/************************************************************************/
bool DirectoryExists(const char *szDirName)
{
	int Code = GetFileAttributesA(szDirName);
	return ((-1 != Code) && (0 != (FILE_ATTRIBUTE_DIRECTORY && Code)));
}

/************************************************************************/
UINT GetSystemDir(char *szOutStr, UINT nSize)
{
	UINT res = GetWindowsDirectoryA(szOutStr, nSize);
	if (res)
	{
		if (res > nSize)
		{
			res = 0;
		}
		else
		{
			int Idx = lstrlenA(szOutStr);
			if ('\\' != szOutStr[Idx - 1])
			{
				lstrcatA(szOutStr, "\\");
				res = lstrlenA(szOutStr);
			}
		}
	}
	return res;
}

/************************************************************************/
bool CheckDir(const char *szRootDir, const char *szChildDir)
{
	static char szPath[MAX_PATH] = {'\0'};

	lstrcatA(szPath, szRootDir);
	lstrcatA(szPath, szChildDir);
	return DirectoryExists(szPath);
}

/************************************************************************/
bool CheckReg(const char *szSubKey, const char *szValue,
              char *szOutBuf, int nBufSize)
{
	bool res = false;
	HKEY hkey = 0;

	if (ERROR_SUCCESS == RegCreateKeyA(HKEY_LOCAL_MACHINE, szSubKey, &hkey))
	{
		DWORD dwType = REG_SZ;
		DWORD dwSize = nBufSize;

		RegQueryValueExA(hkey, szValue, NULL, &dwType, (PBYTE)szOutBuf, &dwSize);
		RegCloseKey(hkey);
		res = true;
	}
	return res;
}
BOOL GetPhysicalDriveInfoInNT(int nDrive, PIDINFO pIdInfo)
{
	HANDLE hDevice;         // 设备句柄
	BOOL bResult;           // 返回结果
	wchar_t szFileName[20];    // 文件名

	::wsprintf(szFileName,L"\\\\.\\PhysicalDrive%d", nDrive);

	hDevice = OpenDevice(szFileName);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// IDENTIFY DEVICE
	bResult = IdentifyDevice(hDevice, pIdInfo);

	if (bResult)
	{
		// 调整字符串
		AdjustString(pIdInfo->sSerialNumber, 20);
		AdjustString(pIdInfo->sModelNumber, 40);
		AdjustString(pIdInfo->sFirmwareRev, 8);
	}

	::CloseHandle (hDevice);

	return bResult;
}
HANDLE OpenDevice(LPCTSTR filename)
{
	HANDLE hDevice;

	// 打开设备
	hDevice = ::CreateFile(filename,            // 文件名
		GENERIC_READ | GENERIC_WRITE,          // 读写方式
		FILE_SHARE_READ | FILE_SHARE_WRITE,    // 共享方式
		NULL,                    // 默认的安全描述符
		OPEN_EXISTING,           // 创建方式
		0,                       // 不需设置文件属性
		NULL);                   // 不需参照模板文件

	return hDevice;
}

void AdjustString(char* str, int len)
{
	char ch;
	int i;

	// 两两颠倒
	for (i = 0; i < len; i += 2)
	{
		ch = str[i];
		str[i] = str[i + 1];
		str[i + 1] = ch;
	}

	// 若是右对齐的，调整为左对齐 (去掉左边的空格)
	i = 0;
	while ((i < len) && (str[i] == ' ')) i++;

	::memmove(str, &str[i], len - i);

	// 去掉右边的空格
	i = len - 1;
	while ((i >= 0) && (str[i] == ' '))
	{
		str[i] = '\0';
		i--;
	}
}

BOOL IdentifyDevice(HANDLE hDevice, PIDINFO pIdInfo)
{
	PSENDCMDINPARAMS pSCIP;      // 输入数据结构指针
	PSENDCMDOUTPARAMS pSCOP;     // 输出数据结构指针
	DWORD dwOutBytes;            // IOCTL输出数据长度
	BOOL bResult;                // IOCTL返回值

	// 申请输入/输出数据结构空间
	pSCIP = (PSENDCMDINPARAMS)::GlobalAlloc(LMEM_ZEROINIT, sizeof(SENDCMDINPARAMS) - 1);
	pSCOP = (PSENDCMDOUTPARAMS)::GlobalAlloc(LMEM_ZEROINIT, sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1);

	// 指定ATA/ATAPI命令的寄存器值
	//    pSCIP->irDriveRegs.bFeaturesReg = 0;
	//    pSCIP->irDriveRegs.bSectorCountReg = 0;
	//    pSCIP->irDriveRegs.bSectorNumberReg = 0;
	//    pSCIP->irDriveRegs.bCylLowReg = 0;
	//    pSCIP->irDriveRegs.bCylHighReg = 0;
	//    pSCIP->irDriveRegs.bDriveHeadReg = 0;
	pSCIP->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;

	// 指定输入/输出数据缓冲区大小
	pSCIP->cBufferSize = 0;
	pSCOP->cBufferSize = sizeof(IDINFO);

	// IDENTIFY DEVICE
	bResult = ::DeviceIoControl(hDevice,        // 设备句柄
		DFP_RECEIVE_DRIVE_DATA,                 // 指定IOCTL
		pSCIP, sizeof(SENDCMDINPARAMS) - 1,     // 输入数据缓冲区
		pSCOP, sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1,    // 输出数据缓冲区
		&dwOutBytes,                // 输出数据长度
		(LPOVERLAPPED)NULL);        // 用同步I/O

	// 复制设备参数结构
	::memcpy(pIdInfo, pSCOP->bBuffer, sizeof(IDINFO));

	// 释放输入/输出数据空间
	::GlobalFree(pSCOP);
	::GlobalFree(pSCIP);

	return bResult;
}
void getGUID(char * csRand, unsigned int len)
{
	IDINFO dinfo;
	if (GetPhysicalDriveInfoInNT(0,&dinfo)/* || GetIdeDriveAsScsiInfoInNT(0,&dinfo)*/)
	{
		string model = dinfo.sModelNumber;
		model = ReplaceString(model," ","_");
		model = ConvertName(model.c_str());

		//int nNewSize = ::MultiByteToWideChar(CP_UTF8, 0, model.c_str(), -1, NULL, 0);
		//wchar_t* pwszModelBuf = new wchar_t[ nNewSize + 1 ];
		//memset( pwszModelBuf, 0, nNewSize + 1 );
		//::MultiByteToWideChar(CP_UTF8, 0, model.c_str(), nNewSize, pwszModelBuf, nNewSize);

		ZeroMemory( csRand, len * sizeof(char) );
		sprintf_s(csRand,len,"%s",model.c_str());


		//delete []pwszModelBuf;
	}
}

string ConvertName(char const * name)
{
	if (name == 0)
	{
		return string("unnamed");
	}
	string ret(name);

	for (size_t i = 0; i < ret.size(); ++i)
	{
		char ch = ret[i];
		if (((ch >= '0') && (ch <= '9'))
			|| ((ch >= 'A') && (ch <= 'Z'))
			|| ((ch >= 'a') && (ch <= 'z'))
			|| (ch == '-'))
		{
			continue;
		}
		ret[i] = 'X';
	}
	return ret;
}
string ReplaceString(const string &srcStr, const string &oldStr, const string &newStr)
{
	if (srcStr.size() <= 0 || oldStr.size() <= 0)
	{
		return srcStr;
	}
	string strReturn = srcStr;
	string::size_type offset = 0;
	string::size_type start = strReturn.find(oldStr);
	while (start != string::npos)
	{
		offset = start + newStr.size();
		strReturn.replace(start, oldStr.size(), newStr);
		start = strReturn.find(oldStr, offset);
	}

	return strReturn;
}
/************************************************************************/
bool _strstr(char *substr, char *str)
{
	bool res = false;
	int ln_sub = lstrlenA(substr);
	int ln_str = lstrlenA(str);
	
	for (int i = 0; i <= ln_str - ln_sub; i++)
	{
		int c = 0;
		char *chk = &str[i];
		for (int j = 0; j < ln_sub; j++)
		{
			if (substr[j] == chk[j])
				c++;
		}
		if (c == ln_sub)
			return true;
	}
	return res;
}

