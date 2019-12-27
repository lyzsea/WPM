#pragma once

//Dll列表
/*
USERENV.dll
{
   CreateEnvironmentBlock
   DestroyEnvironmentBlock
}

PSAPI.dll
{
  EnumProcesses
  GetModuleFileNameExW
  EnumProcessModules
}

SHELL32.dll
{
 ShellExecuteExW
 SHGetFolderPathW
 SHChangeNotify

}



*/

//初始化加载函数
void InitFunctionLoad();



//函数类型声明
typedef BOOL  (WINAPI *TypeCreateEnvironmentBlock)(
	_Out_     LPVOID *lpEnvironment,
	_In_opt_  HANDLE hToken,
	_In_      BOOL bInherit
	);

typedef BOOL (WINAPI *TypeDestroyEnvironmentBlock)(
	_In_  LPVOID lpEnvironment
	);





//扩展定义

extern TypeCreateEnvironmentBlock g_CreateEnvironmentBlock;
extern TypeDestroyEnvironmentBlock g_DestroyEnvironmentBlock;


