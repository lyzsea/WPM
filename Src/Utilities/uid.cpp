
#include "uid.h"
#include <tchar.h>
#include <winioctl.h>
#include <string>


#ifndef MAX_PATHEX
#define MAX_PATHEX MAX_PATH*2
#endif

using namespace std;

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

namespace UID{
	namespace help{

BOOL IsWow64_ID()
{
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"),"IsWow64Process");

	BOOL bIsWow64 = false;

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// Error handle
			return false;
		}
	}

	return  bIsWow64;
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

BOOL GetCPUID(LPTSTR lpstrCpuID,size_t leng)   
{ 
	unsigned   long   s1 = 0,s2 = 0; 

	__asm{   
		mov eax,01h   
			xor edx,edx   
			cpuid   
			mov s1,edx   
			mov s2,eax   
	}     

	if (s1 == 0 || s2 == 0)
		return FALSE;
	_stprintf_s(lpstrCpuID,leng,_T("%u_%u"),s1,s2);
	return TRUE;
}
BOOL ReadKeyValue(HKEY hKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPTSTR lpValue,DWORD dwLength)
{
	BOOL bRet=FALSE;

	HKEY hKEY=NULL;
	DWORD dwValueLength=dwLength;
	DWORD dwType=REG_SZ; 

	DWORD dwVersion = 0;
	DWORD dwBuilder = 0;
	
	//ELEX_GetOSVersion(&dwVersion,&dwBuilder); 

	if (IsWow64_ID())
	{

		if(RegOpenKeyEx(hKey,lpSubKey,0,KEY_READ|KEY_WOW64_64KEY,&hKEY)!=ERROR_SUCCESS)
			goto ENDPOS;
	}
	else
	{
		if(RegOpenKeyEx(hKey,lpSubKey,0,KEY_READ,&hKEY)!=ERROR_SUCCESS)
			goto ENDPOS;
	}


	if(RegQueryValueEx(hKEY,lpValueName,NULL,&dwType,(LPBYTE)lpValue,&dwValueLength)!=ERROR_SUCCESS)
		goto ENDPOS;

	bRet=TRUE;

ENDPOS:

	if(hKEY!=NULL)
		RegCloseKey(hKEY);

	return bRet;
}
void GetOSGUID(LPTSTR lpstrOSID,DWORD leng)
{

	ReadKeyValue(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Cryptography"),_T("MachineGuid"),lpstrOSID,leng);
}

#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.    
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.    
#define  IOCTL_GET_DRIVE_INFO   0x0007c088    
#define  IOCTL_GET_VERSION          0x00074080    

typedef struct _GETVERSIONOUTPARAMS    
{    
	BYTE bVersion;      // Binary driver version.    
	BYTE bRevision;     // Binary driver revision.    
	BYTE bReserved;     // Not used.    
	BYTE bIDEDeviceMap; // Bit map of IDE devices.    
	DWORD fCapabilities; // Bit mask of driver capabilities.    
	DWORD dwReserved[4]; // For future use.    
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;  


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

 
DWORD getGUID(wchar_t * wcsRand, unsigned int len)
{ 

	IDINFO dinfo;
	if (GetPhysicalDriveInfoInNT(0,&dinfo) /*|| GetIdeDriveAsScsiInfoInNT(0,&dinfo)*/)
	{
		string sn = dinfo.sSerialNumber;
		sn = ReplaceString(sn," ","_");
		string model = dinfo.sModelNumber;
		model = ReplaceString(model," ","_");
		//printf_s("%s_%s\r\n",model.c_str(),sn.c_str());

		sn = ConvertName(sn.c_str());
		model = ConvertName(model.c_str());

		int nNewSize = ::MultiByteToWideChar(CP_UTF8, 0, sn.c_str(), -1, NULL, 0);
		wchar_t* pwszSerialBuf = new wchar_t[ nNewSize + 1 ];
		memset( pwszSerialBuf, 0, nNewSize + 1 );
		::MultiByteToWideChar(CP_UTF8, 0, sn.c_str(), nNewSize, pwszSerialBuf, nNewSize);

		nNewSize = ::MultiByteToWideChar(CP_UTF8, 0, model.c_str(), -1, NULL, 0);
		wchar_t* pwszModelBuf = new wchar_t[ nNewSize + 1 ];
		memset( pwszModelBuf, 0, nNewSize + 1 );
		::MultiByteToWideChar(CP_UTF8, 0, model.c_str(), nNewSize, pwszModelBuf, nNewSize);

		ZeroMemory( wcsRand, len * sizeof(wchar_t) );
		swprintf_s(wcsRand,len,L"%s_%s",pwszModelBuf,pwszSerialBuf);

		delete []pwszModelBuf;
		delete []pwszSerialBuf;

	}
	else
	{	
		/*
		wchar_t Volumelabel[20] = {0};

		DWORD SerialNumber;

		DWORD MaxCLength;

		DWORD FileSysFlag;

		wchar_t FileSysName[20] = {0};

		

		GetVolumeInformation( NULL,Volumelabel,255,&SerialNumber,&MaxCLength,&FileSysFlag,FileSysName,255);

		//std::wstring strMac;
		//GetMacAddress(strMac);
		
		
		
		TCHAR strCpuID[MAX_PATHEX] = {0};

		if (!GetCPUID(strCpuID,MAX_PATHEX))
		{
			DWORD dwlen = MAX_PATHEX;
			GetOSGUID(strCpuID,dwlen);
		}

		ZeroMemory( wcsRand, len * sizeof(wchar_t) );
		//swprintf_s( wcsRand, len, L"_%s_%08X_%s", strMac.c_str(),SerialNumber, strCpuID);

		swprintf_s( wcsRand, len, L"%s_%08X", strCpuID,SerialNumber);
		*/
	}

	return 0;
}

// 打开设备
// filename: 设备的“文件名”(设备路径)
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

// 向驱动发“IDENTIFY DEVICE”命令，获得设备信息
// hDevice: 设备句柄
// pIdInfo:  设备信息结构指针
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

// 向SCSI MINI-PORT驱动发“IDENTIFY DEVICE”命令，获得设备信息
// hDevice: 设备句柄
// pIdInfo:  设备信息结构指针
BOOL IdentifyDeviceAsScsi(HANDLE hDevice, int nDrive, PIDINFO pIdInfo)
{
	PSENDCMDINPARAMS pSCIP;     // 输入数据结构指针
	PSENDCMDOUTPARAMS pSCOP;    // 输出数据结构指针
	PSRB_IO_CONTROL pSRBIO;     // SCSI输入输出数据结构指针
	DWORD dwOutBytes;           // IOCTL输出数据长度
	BOOL bResult;               // IOCTL返回值

	// 申请输入/输出数据结构空间
	pSRBIO = (PSRB_IO_CONTROL)::GlobalAlloc(LMEM_ZEROINIT,
		sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1);
	pSCIP = (PSENDCMDINPARAMS)((char *)pSRBIO + sizeof(SRB_IO_CONTROL));
	pSCOP = (PSENDCMDOUTPARAMS)((char *)pSRBIO + sizeof(SRB_IO_CONTROL));

	// 填充输入/输出数据
	pSRBIO->HeaderLength = sizeof(SRB_IO_CONTROL);
	pSRBIO->Timeout = 10000;
	pSRBIO->Length = sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1;
	pSRBIO->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
	::strncpy_s((char *)pSRBIO->Signature,8, "SCSIDISK", 8);

	// 指定ATA/ATAPI命令的寄存器值
	//    pSCIP->irDriveRegs.bFeaturesReg = 0;
	//    pSCIP->irDriveRegs.bSectorCountReg = 0;
	//    pSCIP->irDriveRegs.bSectorNumberReg = 0;
	//    pSCIP->irDriveRegs.bCylLowReg = 0;
	//    pSCIP->irDriveRegs.bCylHighReg = 0;
	//    pSCIP->irDriveRegs.bDriveHeadReg = 0;
	pSCIP->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
	pSCIP->bDriveNumber = nDrive;

	// IDENTIFY DEVICE
	bResult = ::DeviceIoControl(hDevice,    // 设备句柄
		IOCTL_SCSI_MINIPORT,                // 指定IOCTL
		pSRBIO, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,    // 输入数据缓冲区
		pSRBIO, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1,    // 输出数据缓冲区
		&dwOutBytes,            // 输出数据长度
		(LPOVERLAPPED)NULL);    // 用同步I/O

	// 复制设备参数结构
	::memcpy(pIdInfo, pSCOP->bBuffer, sizeof(IDINFO));

	// 释放输入/输出数据空间
	::GlobalFree(pSRBIO);

	return bResult;
}

// 将串中的字符两两颠倒
// 原因是ATA/ATAPI中的WORD，与Windows采用的字节顺序相反
// 驱动程序中已经将收到的数据全部反过来，我们来个负负得正
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

// 读取IDE硬盘的设备信息，必须有足够权限
// nDrive: 驱动器号(0=第一个硬盘，1=0=第二个硬盘，......)
// pIdInfo: 设备信息结构指针
BOOL GetPhysicalDriveInfoInNT(int nDrive, PIDINFO pIdInfo)
{
	std::wstring nameold= L"\\\\.\\Phys";
	HANDLE hDevice;         // 设备句柄
	BOOL bResult;           // 返回结果
	wchar_t szFileName[20];    // 文件名

	//

	nameold += L"icalDrive";
	nameold += L"%d";

	::wsprintf(szFileName,nameold.c_str(), nDrive);

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

// 用SCSI驱动读取IDE硬盘的设备信息，不受权限制约
// nDrive: 驱动器号(0=Primary Master, 1=Promary Slave, 2=Secondary master, 3=Secondary slave)
// pIdInfo: 设备信息结构指针
BOOL GetIdeDriveAsScsiInfoInNT(int nDrive, PIDINFO pIdInfo)
{
	HANDLE hDevice;         // 设备句柄
	BOOL bResult;           // 返回结果
	wchar_t szFileName[20];    // 文件名

	::wsprintf(szFileName,L"\\\\.\\Scsi%d:", nDrive/2);

	hDevice = OpenDevice(szFileName);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// IDENTIFY DEVICE
	bResult = IdentifyDeviceAsScsi(hDevice, nDrive%2, pIdInfo);

	// 检查是不是空串
	if (pIdInfo->sModelNumber[0] == '\0')
	{
		bResult = FALSE;
	}

	if (bResult)
	{
		// 调整字符串
		AdjustString(pIdInfo->sSerialNumber, 20);
		AdjustString(pIdInfo->sModelNumber, 40);
		AdjustString(pIdInfo->sFirmwareRev, 8);
	}

	return bResult;
}

}}