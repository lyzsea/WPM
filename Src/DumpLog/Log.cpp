// Log.cpp : Defines the exported functions for the DLL application.
//

// include
#include "Log.h"
//#include <MsXml2.h>
#include <Psapi.h>
#include <xstring>


#include <shlobj.h>
#include <sys/timeb.h>
#include <time.h>
#include <Shlwapi.h>
#include "strings/StringsHelper.h"
#include "tinyXml/tinystr.h"
#include "tinyXml/tinyxml.h"


#include <strsafe.h>

#define FIRST_FILE                  (1)
#define THREAD_WAIT_TIME            (200)
#define MAX_WRITE_SIZE              (1000)      /**<  */
//#define DEFAULT_FILE_SIZE           (0x100000)  /**< 1M. the the max size of log file. */
#define FILE_SIZE_OFFSET            (100)       /**< the offset of the max size of log file. */
#define LOG_FOLDER_CAPACITY         (10000000)  /**< 10M. the capacity of folder stores log files. */
#define CLEAN_SCALE                 (0.4)       /**< the scale of clean up the log folder. */
#define DEFAULT_PLUS_DIRECTORY      (L"\\log\\")
#define XML_NODE_PATH_LOG           (L"//LogConfig")
#define FORMAT_FILE_NAME_TEXT       (L"%s%s%.3d.log")
#define FORMAT_WRITE_TIME_YMD_TEXT  (L"%.4d-%.2d-%.2d")
#define FORMAT_WRITE_TIME_HMS_TEXT  (L"%.2d-%.2d-%.2d-%.3d")
#define FORMAT_WRITE_INFO_TEXT      (L"%ld %s [%s : %d] %d %s\r\n")
#define FORMAT_HEADER_INFO_IN_LOG   (L"RecordDate: %s , ProcessName: %s\r\n\
the style of log information in the log is: \r\n\
-------------------------------------------------------------------\r\n\
HH-MM-SS-MS | ModuleName | [FileName, ErrLine] | ErrLevel | ErrInfo\r\n\
-------------------------------------------------------------------\r\n\
LogLevel : \r\n\
1 --- serious error, \r\n\
2 --- common error, \r\n\
3 --- warning, \r\n\
4 --- information,\r\n\
5 --- debug information.\r\n\r\n")

// log config file about .exe directory.
#define CONFIG_FILE_NAME_TO_CURRENT_DIR             (L"LogConfig.xml")

// the definition of the xml nodes.
#define XML_NODE_ATTRIBUTE_PROCESS_SECURITY         ("ProcessSecurity")
#define XML_NODE_ATTRIBUTE_LOG_FOLDER_DIR           ("LogFolderDir")
#define XML_NODE_ATTRIBUTE_LOG_FOLDER_SIZE          ("LogFolderSize")
#define XML_NODE_ATTRIBUTE_LOG_FOLDER_CLEAN_SCALE   ("LogFolderCleanScale")
#define XML_NODE_ATTRIBUTE_LOG_LEVEL                ("LogLevel")
#define XML_NODE_ATTRIBUTE_LOG_ENCRYPT              ("LogEncrypt")

// the config information of log setting.
struct LogConfigInfo
{
    DOUBLE      cleanUpFolderScale;         /**< the scale of clean up the log folder. */
    int        nFolderSize;                // the size of folder stores log files.
    BOOL        bProcessSafe;               // whether safe in different process.
    LOG_LEVEL   enumCurLogLevel;            // current log's level.
    WCHAR       szFileStoreDir[MAX_PATH];   // file store directory.
    //UINT        nFileSize;                  // file's size.
	BOOL        bIsEncrypt;
	
	LogConfigInfo()
	{
		cleanUpFolderScale = 0.2;
		nFolderSize = 10000000;
		bProcessSafe = FALSE;
		enumCurLogLevel = LOG_LEVEL::LEVEL_INFO;
		bIsEncrypt    =  FALSE;
	}
};

/**
 * An empty name space.
 * In the space, define some local variable.
 */
namespace 
{
 HANDLE             g_hLogFile = NULL;                 // pointer to file.
 HANDLE             g_hMutex = NULL;                   // handle of mutex.
 LPCWSTR            g_mutexName = L"Local\\bnd_installer_Log_Mutex";// the mutex's name.
 LPWSTR             g_pszReadBuffer = NULL;            // store file data.
 CRITICAL_SECTION   g_criSection = {0};                // pointer to critical section.
 BOOL               g_bInitialized = FALSE;            // whether initialized this module.
 WCHAR              curLogFileName[MAX_PATH] = {0};// current log file name.

 LogConfigInfo      g_logConfigInfo;
};

/**
 * These local function used in this module.
 */
HRESULT    Log_CreateDir( __in LPCWSTR lpszDir );
HRESULT    Log_CreateDefaultDir();
HRESULT    Log_CreateNamedMutex(__in HANDLE &hMutex, __in LPCWSTR lpszMutexName);
HRESULT    Log_CreateCriSec(CRITICAL_SECTION &cricSec);
HRESULT    Log_CreateNewFile();
HRESULT    Log_WriteDataToFile();
HRESULT    Log_IsDirExist( __in LPCWSTR lpszPath );
HRESULT    Log_GetDefaultDir( __in LPWSTR lpszDir );
HRESULT    Log_AllocBuffer( __in DWORD dwSize );
HRESULT    Log_GetCurrentProcessName(__inout LPWSTR pszProcessName);
HRESULT    Log_CleanUpFolder(__in LPCWSTR lpszPath, __in DWORD dwFolderCapacity, __in DOUBLE cleanScale);
HRESULT    Log_GetFolderSize(__in LPCWSTR lpszPath, __inout DWORD &dwFolderSize);
HRESULT    Log_ReadParamFromFile(__in LPCWSTR filePath , LogConfigInfo &configInfo);
//HRESULT    Log_ReadLogParam(__in IXMLDOMDocument2* pXMLDocument , LogConfigInfo &configInfo);
HRESULT    Log_GetFileFullPath(__in LPCWSTR pszPlusName,__inout LPWSTR filePath);

#define LEAVE() \
    { \
        if(g_logConfigInfo.bProcessSafe) \
        {\
            ReleaseMutex(g_hMutex); \
        }\
        else \
        {\
            LeaveCriticalSection(&g_criSection); \
        }\
    }

#define HR_CHECK(err) \
    { \
        if (FAILED(err)) \
        { \
            goto log_exit; \
        } \
    } 

#define PTR_CHECK(ptr) \
    { \
        if (ptr == NULL) \
        { \
            goto log_exit; \
        } \
    }

#define ARG_CHECK(arg) \
    { \
        if (arg == NULL) \
        { \
            hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS); \
            goto log_exit; \
        } \
    }

#define RELEASE(ptr) \
{ \
    if ((ptr) != NULL) \
    { \
        ptr->Release(); \
        (ptr) = NULL; \
    } \
}

#define SYSFREESTRING(ptr) \
{ \
    if ((ptr) != NULL) \
    { \
        SysFreeString((ptr)); \
        (ptr) = NULL; \
    } \
}



/**
 * Init the log module.
 * @param [in] nFileSize: set the capability of log file, provide default value.
 *        [in] pszLogFileDir: set the store directory of log, provide default value.
 *        [in] enumLogLevel: set the log's level.
 *        [in] bProcessSafe: whether safe in different process.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT InitLog()
{
    HRESULT     hr = S_OK;
    WCHAR       configFileFullName[MAX_PATH] = {0};

    if (g_bInitialized)
    {
        // Initialized already, skip.
        goto log_exit;
    }

    // read the parameters from the xml config file.
    Log_GetFileFullPath(CONFIG_FILE_NAME_TO_CURRENT_DIR, configFileFullName);

    hr = Log_ReadParamFromFile(configFileFullName, g_logConfigInfo);
    HR_CHECK(hr);

    // Create directory.
    // if "pLogFileDir" is not a directory,return FALSE.
    // if "pLogFileDir" is not a full directory,create it.
    // otherwise do nothing.
    if (wcscmp(g_logConfigInfo.szFileStoreDir , L"") != 0)
    {
        hr = Log_IsDirExist(g_logConfigInfo.szFileStoreDir);
        if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
        {
            hr = Log_CreateDir(g_logConfigInfo.szFileStoreDir);
        }

        HR_CHECK(hr);    
    }    
    else
    {
        hr = Log_CreateDefaultDir();
        HR_CHECK(hr);
    }

    // create mutex or critical section.
    if (g_logConfigInfo.bProcessSafe)
    {
        hr = Log_CreateNamedMutex(g_hMutex, g_mutexName);
    }
    else
    {
        hr = Log_CreateCriSec(g_criSection);
    }
    HR_CHECK(hr);

    // alloc writing buffer.
    hr = Log_AllocBuffer(MAX_WRITE_SIZE);
    HR_CHECK(hr);

    // Set current log's level.
    hr = SetLogLevel(g_logConfigInfo.enumCurLogLevel);
    HR_CHECK(hr);

    // in this function, find the latest file to write.
    // if do not found log file ,create a new file.
    // if the file that found is full, create a new file, 
    // otherwise open it to wait for writing.
    hr = Log_CreateNewFile();
    HR_CHECK(hr);

    g_bInitialized = TRUE;

log_exit:

    return hr;
}

/**
 * Close log module and release the resource.
 * @param none.
 * @return void.
 */
void UninitLog()
{
    if (g_bInitialized)
    {
        // release resource and close file
        if (g_pszReadBuffer != NULL)
        {
            delete [] g_pszReadBuffer;
            g_pszReadBuffer = NULL;
        }

        if (g_logConfigInfo.bProcessSafe)
        {
            ::CloseHandle(g_hMutex);
            g_hMutex = NULL;
        }
        else
        {
            ::DeleteCriticalSection(&g_criSection);
        }

        if (g_hLogFile != NULL)
        {
            ::CloseHandle(g_hLogFile);
            g_hLogFile = NULL;
        }

        // clean up the folder stores log files.
        Log_CleanUpFolder(g_logConfigInfo.szFileStoreDir, 
            g_logConfigInfo.nFolderSize, 
            g_logConfigInfo.cleanUpFolderScale);

        g_bInitialized = FALSE;
    }
}

/**
 * Set log's level.
 * @param [in] enumLogLevel: this parameter set log's current level.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT SetLogLevel(__in LOG_LEVEL enumLogLevel )
{
    HRESULT hr = S_OK;

    if ((enumLogLevel < LEVEL_NONE) || (enumLogLevel > LEVEL_ALL))
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
        goto log_exit;
    }

    g_logConfigInfo.enumCurLogLevel = enumLogLevel;

log_exit:
    return hr;
}

HRESULT SetModuleName(__in LPCWSTR strModuleName)
{
	HRESULT hr = S_OK;

	size_t len1 = 0;
	hr = ::StringCchLength(strModuleName , MAX_PATH, &len1);
	HR_CHECK(hr);
	len1++;

	hr = ::StringCchCopy(MODULE_NAME, len1, strModuleName);
	HR_CHECK(hr);
log_exit:

	return hr;

}

/**
 * Get current log's level.
 * @param none.
 * @return LOG_LEVEL: log's current level.
 */
LOG_LEVEL GetCurLogLevel()
{
    return g_logConfigInfo.enumCurLogLevel;
}

HRESULT GetLogFileName( __inout LPWSTR lpszFileName )
{
    HRESULT hr = S_OK;

    if (lpszFileName == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

    size_t len1 = 0;
    hr = ::StringCchLength(g_logConfigInfo.szFileStoreDir , MAX_PATH, &len1);
    HR_CHECK(hr);
    len1++;

    hr = ::StringCchCopy(lpszFileName, len1, g_logConfigInfo.szFileStoreDir);
    HR_CHECK(hr);

    if (lpszFileName[len1 - 2] != L'\\')
    {
        lpszFileName[len1 - 1] = L'\\';
        lpszFileName[len1] = L'\0';
        len1 += 1;
    }

    size_t len2 = 0;
    hr = ::StringCchLength(curLogFileName, MAX_PATH, &len2);
    HR_CHECK(hr);
    len2 += len1;

    hr = ::StringCchCat(lpszFileName, len2, curLogFileName);
    HR_CHECK(hr);

log_exit:
    return hr;
}


/**
 * Invokes this function to write data to log file.
 * @param [in] pszSrc: the error file name.
 *        [in] uLine: the error line number.
 *        [in] level: log's level.
 *        [in] pszFormat: the formated string.
 *        [in] ... : the param list.
 * @return void.
 */
void TraceToFile(
            __in LPCWSTR pszModuleName,
            __in LPCWSTR pszFileName,
            __in ULONG uLine, 
            __in LONG level, 
            __in LPCWSTR pszFormat, 
            ...
            )
{
    HRESULT hr = S_OK;
    LPWSTR lpDest = NULL;
    //WCHAR szTime[128];
	//std::wstring strCppFileName = pszFileName;

    //memset(szTime, 0, sizeof(szTime));
	
    if (!g_bInitialized)
    {
        // If the log module hasn't been initialized, 
        // let's initialize it with default settings.
        hr = InitLog();
        HR_CHECK(hr);
    }

    //
    // inter mutex or critical section
    //
    BOOL bWait = TRUE;
    if (g_logConfigInfo.bProcessSafe)
    {
        PTR_CHECK(g_hMutex);

        while ((g_hMutex != NULL) && bWait)
        {
            DWORD nRet = ::WaitForSingleObject(g_hMutex, 200);
            switch (nRet)
            {
            case WAIT_OBJECT_0: bWait = FALSE; break;
            case WAIT_FAILED: ::ReleaseMutex(g_hMutex); goto log_exit;
            default: break;
            }
        }

        PTR_CHECK(g_hMutex);
    }
    else
    {
        ::EnterCriticalSection(&g_criSection);
    }

    if (level > g_logConfigInfo.enumCurLogLevel)
    {
        goto log_exit;
    }

    //
    // format string.
    //
    va_list ptr;
    va_start(ptr, pszFormat);
    int nLen = _vscwprintf(pszFormat, ptr) + 1;
    
    lpDest = new WCHAR[nLen];
    PTR_CHECK(lpDest);

    vswprintf_s(lpDest, nLen, pszFormat, ptr);    
    va_end(ptr);

    // Display operating system-style date and time. 
    //_wstrtime_s(szTime, sizeof(szTime)/sizeof(WCHAR));
	//struct _timeb timebuffer;
	//_ftime( &timebuffer ); 


	/*size_t nLastIndex = strCppFileName.find_last_of(L"\\");
	size_t nLastSecondIndex = strCppFileName.substr(0,nLastIndex).find_last_of(L"\\");

	if (nLastSecondIndex != std::wstring::npos)
	{
	strCppFileName =L".." + strCppFileName.substr(nLastSecondIndex);
	}*/
	long dwTimer = _time32(0);
    hr = ::StringCchPrintf(g_pszReadBuffer, MAX_WRITE_SIZE, 
                FORMAT_WRITE_INFO_TEXT,
                dwTimer,  pszModuleName, pszFileName, uLine, level, lpDest);
    HR_CHECK(hr);

    // write data to file
    hr = Log_WriteDataToFile();
    HR_CHECK(hr);


log_exit:
    // release resource.
    if (lpDest != NULL)
    {
        delete []lpDest;
        lpDest = NULL;
    }

    // leave mutex or critical section
    if (g_bInitialized)
    {
        // if log module has been initialized 
        // leave mutex or critical section, 
        // to avoid dead lock.
        LEAVE();
    }
}

void TraceErrorReport( __in DWORD Status, __in LPCWSTR ModuleName,__in LPCWSTR FileName, __in DWORD LineNumber )
{
    TraceToFile(
        ModuleName, 
        FileName, 
        LineNumber, 
        LEVEL_ERR,
        L"Error 0x%08X.",
        Status
        );
}

/**
 * Alloc buffer for store some information used in writing.
 * @param [in] dwSize: the buffer's size.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_AllocBuffer( __in DWORD dwSize )
{
    HRESULT hr = S_OK;

    g_pszReadBuffer = new WCHAR[dwSize];
    if (g_pszReadBuffer == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto log_exit;
    }
    ::ZeroMemory(g_pszReadBuffer, sizeof(WCHAR)*dwSize);
    

log_exit:
    return hr;
}

/**
 * Create more level directory.
 * @param [in] lpszDir: the directory that will be created.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_CreateDir(__in LPCWSTR lpszDir )
{
    HRESULT hr = S_OK;
    int nRet = 0;

    if (lpszDir == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
        goto log_exit;
    }

    nRet = ::SHCreateDirectoryEx(NULL, lpszDir, NULL);
    if (ERROR_SUCCESS != nRet)
    {
        hr = nRet;
        goto log_exit;
    }

log_exit:
    return hr;
}

/**
 * Create default directory.
 * @param none.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_CreateDefaultDir()
{
    HRESULT hr = S_OK;
    WCHAR tempDir[MAX_PATH] = {0};
    size_t len = 0;

    hr = Log_GetDefaultDir(tempDir);
    HR_CHECK(hr);

    hr = Log_IsDirExist(tempDir);
    if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
    {
        hr = Log_CreateDir(tempDir);
    }

    HR_CHECK(hr);

    hr = ::StringCchLength(tempDir , MAX_PATH, &len);
    HR_CHECK(hr);
    len++;

    hr = ::StringCchCopy(g_logConfigInfo.szFileStoreDir, len, tempDir);
    HR_CHECK(hr);


log_exit:
    return hr;
}

/**
 * Get default directory, used in Log_CreateDefaultDir().
 * @param [in] lpszDir: it is a pointer stored returned value.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_GetDefaultDir(__in LPWSTR lpszDir)
{
    HRESULT hr = S_OK;
    size_t len = 0;
    int nIndex = 0;
	WCHAR wszDefaultDir[MAX_PATH] = {0};

	GetModuleFileName(NULL,wszDefaultDir,MAX_PATH);

    /*hr = ::SHGetSpecialFolderPath(NULL, 
                wszDefaultDir, CSIDL_FAVORITES , FALSE);
    if (FAILED(hr))
    {
        hr = ::StringCchLength(wszDefaultDir, MAX_PATH, &len);
        HR_CHECK(hr);
        len++;

        hr = ::StringCchCopy(lpszDir, len, wszDefaultDir);
        HR_CHECK(hr);
    }
    else*/
    {
        hr = ::StringCchLength( wszDefaultDir, MAX_PATH, &len);
        HR_CHECK(hr);

        for (int i=0; i<(int)len; i++)
        {
            WCHAR wcChar = wszDefaultDir[i];
            if (wcChar == L'\\')
                nIndex = i;
        }

        wszDefaultDir[nIndex] = L'\0';
        
        size_t tempLen = 0;
        hr = ::StringCchLength(DEFAULT_PLUS_DIRECTORY, MAX_PATH, &tempLen);
        HR_CHECK(hr);
        tempLen++;

        hr = ::StringCchLength(wszDefaultDir, MAX_PATH, &len);
        HR_CHECK(hr);

        hr = ::StringCchCat(wszDefaultDir, (len + tempLen) , DEFAULT_PLUS_DIRECTORY);
        HR_CHECK(hr);

        hr = ::StringCchLength(wszDefaultDir, MAX_PATH, &len);
        HR_CHECK(hr);
        len++;

        hr = ::StringCchCopy(lpszDir, len, wszDefaultDir);
        HR_CHECK(hr);
    }


log_exit:
    return hr;
}

/**
 * Create a named mutex for the safe of different process.
 * @param none.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_CreateNamedMutex( __in HANDLE &hMutex,  __in LPCWSTR lpszMutexName )
{
    HRESULT hr = S_OK;
    
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    hMutex = ::CreateMutex(&sa, FALSE, lpszMutexName);
    if (hMutex == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

log_exit:

    return hr;
}

/**
 * Create a critical section used in a process.
 * @param none.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_CreateCriSec(CRITICAL_SECTION &cricSec)
{
    HRESULT hr = S_OK;

    ::InitializeCriticalSection(&cricSec);

    return hr;
}

/**
 * Create new file used in Log_WriteDataToFile().
 * @param none.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_CreateNewFile()
{
    HRESULT hr = S_OK;
    WCHAR filePath[MAX_PATH] = {0};
    WCHAR processName[MAX_PATH] = {0};

    SYSTEMTIME sysTime;
    const int timeLength = 40;
    WCHAR szTime[timeLength] = {0};

    ::GetLocalTime(&sysTime);

    hr = ::StringCchPrintf(szTime, timeLength, 
        L"%.4d-%.2d-%.2d[%.2d-%.2d-%.2d-%.3d]", 
        sysTime.wYear,sysTime.wMonth,sysTime.wDay,
        sysTime.wHour, sysTime.wMinute, sysTime.wSecond, 
        sysTime.wMilliseconds);
    HR_CHECK(hr);

    hr = Log_GetCurrentProcessName(processName);
    HR_CHECK(hr);

    hr = ::StringCchPrintf(curLogFileName, MAX_PATH, L"%s_%s.log", processName, szTime);
    HR_CHECK(hr);

    hr = ::StringCchPrintf(filePath, MAX_PATH, L"%s%s", 
                    g_logConfigInfo.szFileStoreDir, curLogFileName);
    HR_CHECK(hr);

    g_hLogFile = ::CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE, 
                        NULL, CREATE_NEW, NULL, NULL);
    if (g_hLogFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        g_hLogFile = NULL;
        goto log_exit;
    }

    // 
    // create file head for unicode text.
    // 
    UCHAR fileHead[2] = {0};
    fileHead[0] = 0xff;
    fileHead[1] = 0xfe;
    DWORD dwWritenSize = 0;
    BOOL bSuccess = FALSE;
    
    bSuccess = ::WriteFile(g_hLogFile, fileHead, _countof(fileHead), &dwWritenSize, NULL);
    if (!bSuccess)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

    // 
    // write header information at start of log file.
    // 
    bSuccess = ::SetEndOfFile(g_hLogFile);
    if (!bSuccess)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

    // format the time as YYMMDD.
    ::GetSystemTime(&sysTime);

    hr = ::StringCchPrintf(szTime, timeLength, 
        FORMAT_WRITE_TIME_YMD_TEXT, 
        sysTime.wYear,sysTime.wMonth,sysTime.wDay);
    HR_CHECK(hr);

    // format the header information.
    const int headerLength = 1024;
    WCHAR headerInfo[headerLength] = {0};
    hr = ::StringCchPrintf(headerInfo, headerLength, 
        FORMAT_HEADER_INFO_IN_LOG, szTime, processName);
    HR_CHECK(hr);

    size_t nLen = 0;
    hr = ::StringCchLength(headerInfo, MAX_WRITE_SIZE, &nLen);
    HR_CHECK(hr);

    nLen *= 2;
	if (g_logConfigInfo.bIsEncrypt)
	{
		LPBYTE p=(LPBYTE)headerInfo;
		BYTE bSeed=0x5A;

		for(DWORD i=0;i< nLen;i++)
		{
			if(p[i]!=0 && p[i]!=bSeed)
				p[i]^=bSeed;
		}

		DWORD dwWritenSize = 0;
		bSuccess = ::WriteFile(g_hLogFile, 
			p, (DWORD)nLen, &dwWritenSize, NULL);
	}
	else
	{
		bSuccess = ::WriteFile(g_hLogFile, headerInfo, (DWORD)nLen, &dwWritenSize, NULL);
	}
  
    if (!bSuccess)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

log_exit:
    return hr;
}

/**
 * Write data into specific file.
 * @param none.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_WriteDataToFile()
{
    HRESULT hr = S_OK;
    size_t nLen = 0;
    BOOL bSuccess = FALSE;
    
    DWORD g_dwFileCurSize = ::GetFileSize(g_hLogFile, NULL);
    DWORD dwRet = ::SetFilePointer(g_hLogFile, g_dwFileCurSize, 0, FILE_BEGIN);
    if (dwRet == INVALID_SET_FILE_POINTER)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

    hr = ::StringCchLength(g_pszReadBuffer, MAX_WRITE_SIZE, &nLen);
    HR_CHECK(hr);

    nLen = nLen * 2;

	if (g_logConfigInfo.bIsEncrypt)
	{
		LPBYTE p=(LPBYTE)g_pszReadBuffer;
		BYTE bSeed=0x5A;

		for(DWORD i=0;i<nLen;i++)
		{
			if(p[i]!=0 && p[i]!=bSeed)
				p[i]^=bSeed;
		}

		DWORD dwWritenSize = 0;
		bSuccess = ::WriteFile(g_hLogFile, 
			p, (DWORD)nLen, &dwWritenSize, NULL);
	}
	else
	{
		DWORD dwWritenSize = 0;
		bSuccess = ::WriteFile(g_hLogFile, 
			g_pszReadBuffer, (DWORD)nLen, &dwWritenSize, NULL);
	}
   
    if (!bSuccess)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

log_exit:
    
    return hr;
}

/**
 * Whether the specific path exists.
 * @param [in] lpszPath, the pointer to file's path.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_IsDirExist( __in LPCWSTR lpszPath )
{
    HRESULT hr = S_OK;
    size_t len = 0;
    WCHAR szTemp[MAX_PATH] = {0};

    hr = ::StringCchLength(lpszPath, MAX_PATH, &len);
    HR_CHECK(hr);
    len++;

    hr = ::StringCchCopy(szTemp, len, lpszPath);
    HR_CHECK(hr);

    hr = ::StringCchLength( lpszPath, MAX_PATH, &len);
    HR_CHECK(hr);

    if (szTemp[len-1] == L'\\')
    {
        szTemp[len-1] = L'\0';
    }

    WIN32_FIND_DATA wfd ;
    HANDLE hFind = FindFirstFile(szTemp, &wfd); 
    if(hFind == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
        goto log_exit;
    }
    else
    {
        if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = S_OK;
        }
        else 
        {
            hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
        }

        ::FindClose(hFind);
    }

log_exit:
    return hr;
}

/**
 * Get current process name.
 * @param [in][out] pszProcessName: get current process name and return it.
 * @return HRESULT: return S_OK if successful.
 */
HRESULT Log_GetCurrentProcessName(__inout LPWSTR pszProcessName )
{
    HRESULT     hr = S_OK;
    WCHAR       filePath[MAX_PATH] = {0};
    DWORD       dwRet = 0;

    if (pszProcessName == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
        goto log_exit;
    }

    dwRet = ::GetModuleFileName(NULL, filePath, MAX_PATH);
    if (dwRet == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

    size_t length = 0;
    size_t pos = 0;
    hr = ::StringCchLength(filePath, MAX_PATH, &length);
    HR_CHECK(hr);

    for (size_t i = 0; i < length; i++)
    {
        if (filePath[i] == L'\\')
        {
            pos = i;
        }
    }

    size_t j = 0;
    for (size_t i = pos + 1; i < length -4; i++, j++)
    {
        pszProcessName[j] = filePath[i];
    }

    pszProcessName[j] = L'\0';


log_exit:
    return hr;
}

HRESULT Log_GetFolderSize( __in LPCWSTR lpszPath,  __inout DWORD &dwFolderSize )
{
    WCHAR szFileFilter[MAX_PATH] = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fileInfo;

    dwFolderSize = 0;
    HRESULT hr = S_OK;

    hr = ::StringCchPrintf(szFileFilter, MAX_PATH, L"%s\\*.*", lpszPath);
    HR_CHECK(hr);

    hFind = ::FindFirstFile(szFileFilter,&fileInfo);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto log_exit;
    }

    BOOL bFound = TRUE;
    do
    {
        if(!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            dwFolderSize += fileInfo.nFileSizeLow;
        }

        bFound = ::FindNextFile(hFind, &fileInfo);
    }
    while(bFound);

    if (!bFound)
    {
        DWORD dwErr = ::GetLastError();
        if (dwErr != ERROR_NO_MORE_FILES)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto log_exit;
        }
    }

log_exit:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        ::FindClose(hFind);
    }

    return hr;
}

HRESULT Log_CleanUpFolder( __in LPCWSTR lpszPath,  __in DWORD dwFolderCapacity,   __in DOUBLE cleanScale)
{
    HRESULT hr = S_OK;

    WCHAR szFileFilter[MAX_PATH] = {0};
    WCHAR szFileFullName[MAX_PATH] = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fileInfo;

    if (lpszPath == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
        goto log_exit;
    }

    DWORD dwFolderSize = 0;
    hr = Log_GetFolderSize(lpszPath, dwFolderSize);
    HR_CHECK(hr);

    if (dwFolderSize > dwFolderCapacity)
    {
        hr = ::StringCchPrintf(szFileFilter, MAX_PATH, L"%s\\*.*", lpszPath);
        HR_CHECK(hr);

        hFind = ::FindFirstFile(szFileFilter,&fileInfo);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            goto log_exit;
        }

        DWORD fileLeftSize = (DWORD)(dwFolderCapacity * (1 - cleanScale));

        BOOL bFound = TRUE;
        do
        {
            if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // format the file full name.
                hr = ::StringCchPrintf(szFileFullName, MAX_PATH, L"%s\\%s", lpszPath, fileInfo.cFileName);
                HR_CHECK(hr);

                BOOL bRet = ::DeleteFile(szFileFullName);
                if (!bRet)
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    break;
                }

                hr = Log_GetFolderSize(lpszPath, dwFolderSize);
                HR_CHECK(hr);

                if (dwFolderSize < fileLeftSize)
                {
                    break;
                }
            }

            bFound = ::FindNextFile(hFind, &fileInfo);
        }
        while(bFound);

        if (!bFound)
        {
            DWORD dwErr = ::GetLastError();
            if (dwErr != ERROR_NO_MORE_FILES)
            {
                hr = HRESULT_FROM_WIN32(dwErr);
                goto log_exit;
            }
        }
    }

log_exit:
    if (hFind == INVALID_HANDLE_VALUE)
    {
        ::FindClose(hFind);
    }

    return hr;
}

HRESULT Log_ReadParamFromFile( __in LPCWSTR filePath, __in LogConfigInfo &configInfo )
{
    HRESULT             hr = S_OK;
	TiXmlDocument* pXmlDoc  = NULL;
	TiXmlElement*pRootElement = NULL;
	if (PathFileExists(filePath))
	{
		pXmlDoc = new TiXmlDocument(strings::ws2s(filePath).c_str());
		bool bLoadOk = pXmlDoc->LoadFile();
		pRootElement = pXmlDoc->RootElement();

		int ct =0;
		for (TiXmlNode *pItem = pRootElement->FirstChild(); pItem; pItem = pItem->NextSibling())
		{
			ct = pItem->Type();
			switch(ct)
			{
			case TiXmlNode::TINYXML_TEXT:
				break;
			case TiXmlNode::TINYXML_ELEMENT:
				{
					if (strcmp( pItem->Value(),XML_NODE_ATTRIBUTE_LOG_LEVEL) == 0)
					{
						configInfo.enumCurLogLevel = (LOG_LEVEL)atoi(pItem->ToElement()->GetText());
					}
					else if (strcmp(pItem->Value(), XML_NODE_ATTRIBUTE_PROCESS_SECURITY) == 0)
					{
						int tempInt = atoi(pItem->ToElement()->GetText());

						if (tempInt != 0)
						{
							configInfo.bProcessSafe = TRUE;
						}
						else
						{
							configInfo.bProcessSafe = FALSE;
						}
					}
					else if (strcmp(pItem->Value(), XML_NODE_ATTRIBUTE_LOG_FOLDER_DIR) == 0)
					{
						size_t len = 0;
						if (pItem->ToElement()->GetText() != NULL)
						{
							std::wstring strLogDir = strings::s2ws(pItem->ToElement()->GetText());
							hr = ::StringCchLength(strLogDir.c_str(), MAX_PATH, &len);
							HR_CHECK(hr);

							len += 1;
							hr = ::StringCchCopy(configInfo.szFileStoreDir, len, strLogDir.c_str());
							HR_CHECK(hr);
						}
					}
					else if (strcmp(pItem->Value(), XML_NODE_ATTRIBUTE_LOG_FOLDER_SIZE) == 0)
					{
						configInfo.nFolderSize = atoi(pItem->ToElement()->GetText());
					}
					else if (strcmp(pItem->Value(), XML_NODE_ATTRIBUTE_LOG_FOLDER_CLEAN_SCALE) == 0)
					{
						configInfo.cleanUpFolderScale = (DOUBLE)atof(pItem->ToElement()->GetText());
					}
					else if (strcmp(pItem->Value(),XML_NODE_ATTRIBUTE_LOG_ENCRYPT) == 0 )
					{
						int tempInt = atoi(pItem->ToElement()->GetText());

						if (tempInt != 0)
						{
							configInfo.bIsEncrypt = TRUE;
						}
						else
						{
							configInfo.bIsEncrypt = FALSE;
						}
					}
				}
				break;
			}
		}
	}
	else
	{
		WCHAR szFileStoreDir[MAX_PATH] = {0};
		::GetModuleFileName(NULL, szFileStoreDir, MAX_PATH);

		std::wstring strTemp = szFileStoreDir;
		size_t loopIndex = strTemp.find_last_of(L'\\');
		if (loopIndex != std::wstring::npos)
		{
			strTemp = strTemp.substr(0,loopIndex);
			strTemp.append(L"\\log\\");
			::StringCchCopy(configInfo.szFileStoreDir, strTemp.length() + 1,  strTemp.c_str());
		}
	}

log_exit:

    return hr;
}

//HRESULT Log_ReadLogParam(
//                         __in IXMLDOMDocument2* pXMLDocument, 
//                         __in LogConfigInfo &configInfo
//                         )
//{
//    HRESULT                 hr = S_OK;
//    IXMLDOMNodeList*        pNodeList = NULL;
//    IXMLDOMNode*            pNode = NULL;
//    IXMLDOMNodeList*        pChildList = NULL;
//    IXMLDOMNode*            pChildNode = NULL;
//    BSTR                    childNodeName = NULL;
//    BSTR                    childNodeText = NULL;
//
//    ARG_CHECK(pXMLDocument);
//
//    // get the parameters in log config file.
//    hr = pXMLDocument->selectNodes(XML_NODE_PATH_LOG, &pNodeList);
//    HR_CHECK(hr);
//
//    hr = pNodeList->get_item(0, &pNode);
//    HR_CHECK(hr);
//
//    // find child node.
//    hr = pNode->get_childNodes(&pChildList);
//    HR_CHECK(hr);
//
//    long length = 0;
//    hr = pChildList->get_length(&length);
//    HR_CHECK(hr);
//
//    for(long i = 0; i < length; i++)
//    {
//        hr = pChildList->get_item(i, &pChildNode);
//        HR_CHECK(hr);
//
//        hr = pChildNode->get_nodeName(&childNodeName);
//        HR_CHECK(hr);
//
//        if (wcscmp(childNodeName, XML_NODE_ATTRIBUTE_PROCESS_SECURITY) == 0)
//        {
//            hr = pChildNode->get_text(&childNodeText);
//            HR_CHECK(hr);
//
//            std::wstring tempStr(childNodeText);
//            UINT tempInt = (UINT)_wtoi(tempStr.c_str());
//            
//            if (tempInt != 0)
//            {
//                configInfo.bProcessSafe = TRUE;
//            }
//            else
//            {
//                configInfo.bProcessSafe = FALSE;
//            }
//        }
//        else if (wcscmp(childNodeName, XML_NODE_ATTRIBUTE_LOG_FOLDER_DIR) == 0)
//        {
//            hr = pChildNode->get_text(&childNodeText);
//            HR_CHECK(hr);
//
//            size_t len = 0;
//            hr = ::StringCchLength(childNodeText, MAX_PATH, &len);
//            HR_CHECK(hr);
//
//            len += 1;
//            hr = ::StringCchCopy(configInfo.szFileStoreDir, len, childNodeText);
//            HR_CHECK(hr);
//        }
//        else if (wcscmp(childNodeName, XML_NODE_ATTRIBUTE_LOG_FOLDER_SIZE) == 0)
//        {
//            hr = pChildNode->get_text(&childNodeText);
//            HR_CHECK(hr);
//
//            std::wstring tempStr(childNodeText);
//            configInfo.nFolderSize = (UINT)_wtoi(tempStr.c_str());
//        }
//        else if (wcscmp(childNodeName, XML_NODE_ATTRIBUTE_LOG_FOLDER_CLEAN_SCALE) == 0)
//        {
//            hr = pChildNode->get_text(&childNodeText);
//            HR_CHECK(hr);
//
//            std::wstring tempStr(childNodeText);
//            configInfo.cleanUpFolderScale = (DOUBLE)_wtof(tempStr.c_str());
//        }
//        else if (wcscmp(childNodeName, XML_NODE_ATTRIBUTE_LOG_LEVEL) == 0)
//        {
//            hr = pChildNode->get_text(&childNodeText);
//            HR_CHECK(hr);
//
//            std::wstring tempStr(childNodeText);
//            configInfo.enumCurLogLevel = (LOG_LEVEL)_wtoi(tempStr.c_str());
//        }
//    }
//
//log_exit:
//    SYSFREESTRING(childNodeText);
//    SYSFREESTRING(childNodeName);
//
//    RELEASE(pChildNode);
//    RELEASE(pChildList);
//    RELEASE(pNode);
//    RELEASE(pNodeList);
//
//    return hr;
//}

//HRESULT 
//Log_GetNodeAttributeValue(
//                      __in IXMLDOMNode* pNode,
//                      __in BSTR attributeName, 
//                      __inout BSTR& attributeValue
//                      )
//{
//    HRESULT                 hr = S_OK;
//    IXMLDOMNamedNodeMap*    pNodeMap = NULL;
//    IXMLDOMNode*            pNodeAttribute = NULL;
//
//    ARG_CHECK(pNode);
//    ARG_CHECK(attributeName);
//
//    hr = pNode->get_attributes(&pNodeMap);
//    HR_CHECK(hr);
//
//    hr = pNodeMap->getNamedItem(attributeName, &pNodeAttribute);
//    HR_CHECK(hr);
//
//    hr = pNodeAttribute->get_text(&attributeValue);
//    HR_CHECK(hr);    
//
//log_exit:
//    RELEASE(pNodeMap);
//    RELEASE(pNodeAttribute);
//
//    return hr;
//}

HRESULT Log_GetFileFullPath( __in LPCWSTR pszPlusName, __inout LPWSTR filePath )
{
    HRESULT     hr = S_OK;
    DWORD       dwErr = 0;
    size_t      nLength1 = 0;
    size_t      nLength2 = 0;
    WCHAR       moduleFullName[MAX_PATH] = {0};
    std::wstring wstrPathValue;

    // check the pointer.
    ARG_CHECK(pszPlusName);
    ARG_CHECK(filePath);

    dwErr = ::GetModuleFileName(NULL, moduleFullName, MAX_PATH);
    if (dwErr == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto log_exit;
    }

    wstrPathValue = moduleFullName;
    size_t loopIndex = wstrPathValue.find_last_of(L'\\');
    if (loopIndex > 0)
    {
        wstrPathValue = wstrPathValue.substr(0,loopIndex);
        ::StringCchCopy(filePath, wstrPathValue.length() + 1,  wstrPathValue.c_str());
        //wcscpy_s(filePath, wstrPathValue.length(), wstrPathValue.c_str());
        //filePath = _bstr_t(wstrPathValue.c_str());
    }

    hr = StringCchLength(pszPlusName, MAX_PATH, &nLength1);
    HR_CHECK(hr);
    nLength1 += 1;


    hr = StringCchLength(filePath, MAX_PATH, &nLength2);
    HR_CHECK(hr);

    if (filePath[nLength2 - 1] != L'\\')
    {
        filePath[nLength2] = L'\\';
    }

    hr = StringCchLength(filePath, MAX_PATH, &nLength2);
    HR_CHECK(hr);
    nLength2 += nLength1;

    hr = StringCchCat(filePath, nLength2, pszPlusName);
    HR_CHECK(hr);

log_exit:

    return hr;
}

