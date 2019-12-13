// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include "Safe/StrEncrypt.h"


#include "FunDefine.h"
#include "StrDefine.h"
#include "AntiVm/VMDetect.h"

extern CStrEncrypt g_StrFun;
extern  std::string g_strGuid;
// TODO: reference additional headers your program requires here

#define CHECK_ROOT 0 //IsCheckBackdoor

#include "../src/DumpLog/Log.h"

#define RANDOM_SHIT (((((__COUNTER__) * 1103515245) + 12345) >> 24)&0xff)

#define EMIT_SHIT __asm _emit RANDOM_SHIT

// 8 bytes
#define SMALL_SHIT_BLOCK EMIT_SHIT EMIT_SHIT EMIT_SHIT EMIT_SHIT EMIT_SHIT EMIT_SHIT EMIT_SHIT EMIT_SHIT
// 32 bytes
#define MEDIUM_SHIT_BLOCK SMALL_SHIT_BLOCK SMALL_SHIT_BLOCK SMALL_SHIT_BLOCK SMALL_SHIT_BLOCK
// 128 bytes
#define LARGE_SHIT_BLOCK MEDIUM_SHIT_BLOCK MEDIUM_SHIT_BLOCK MEDIUM_SHIT_BLOCK MEDIUM_SHIT_BLOCK
// 512 bytes
#define HUGE_SHIT_BLOCK LARGE_SHIT_BLOCK LARGE_SHIT_BLOCK LARGE_SHIT_BLOCK LARGE_SHIT_BLOCK


extern wchar_t g_szGuid[MAX_PATH];