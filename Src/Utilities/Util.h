#ifndef  _COMMON_UTIL_HELPER_H
#define _COMMON_UTIL_HELPER_H

#include <string>
#include <windows.h>
#include <vector>
namespace Util{ namespace help
{
	// 软件加密文件的头
	typedef struct tagSD_FILEHEAD
	{
		DWORD		m_dwVersion[4];				// 版本
		DWORD		m_dwLength;					// 长度
		DWORD		m_dwCRC32;					// CRC32校验
		DWORD		m_dwSeed;					// 加密种子
	}SD_FILEHEADEX, *PSD_FILEHEADEX;

	std::wstring GetProgramRunDir(HMODULE hDllModule = NULL);  

	BOOL MyCopyFile(LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName,BOOL bFailIfExists);
	bool ExecuteWait(LPCWSTR lpszProcessName, LPCWSTR lpszCmdLine, LPCWSTR lpCurrentDirectory, bool bForceAdmin, bool bWaitProcess,bool bIsshow,DWORD dwTimeOut = INFINITE);
	bool ReadKeyValueW(HKEY hKey,LPCWSTR lpSubKey,LPCWSTR lpValueName,DWORD dwType,LPBYTE lpValue,DWORD& dwLength,DWORD dwDesired);
	void DelSelf(std::wstring &strOutFileDir);
	void DelUninstallInfo(const std::wstring& keyName);

	void DeleteInstallFiles(std::wstring strDeleteDir);

	bool DeleteDirectory(std::wstring srcDir);

	int CompareVersion(const std::wstring strOld,const std::wstring strNew);

	void GetFilesFullnameInDir(const std::wstring &strSourDir, 
		std::wstring strExtenal,
		std::vector<std::wstring> &filesPathVector,
		BOOL isIncludeSubDir) ;

	void DeleteFilesInDirectory(const std::wstring &strSourDir, BOOL isIncludeSubDir) ;
	
	bool ReadSoftDictFileW(wchar_t *ptcPath, PVOID *ppBuf, DWORD *pLength, DWORD *pdwVersion, DWORD dwExtendLength);
	bool WriteSoftDictFileW(wchar_t *ptcPath, PVOID pBuf, DWORD dwLength, DWORD *pdwVersion);

	bool MyGetFileSize(std::wstring sFilePath, int* nSize);

}}



#endif