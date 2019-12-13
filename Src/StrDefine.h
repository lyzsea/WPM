#pragma once


//所有关于字符串定义在这里进行

#define CMDLINE_PARAM_NAME_INSTALL       L"install"
#define CMDLINE_PARAM_NAME_UNINSTALL     L"uninstall"

#define CMDLINE_PARAM_NAME_RUANASSVR     L"service"
#define CMDLINE_PARAM_NAME_DEBUG         L"debug"
#define CMDLINE_PARAM_NAME_START         L"start"
#define CMDLINE_PARAM_NAME_RUN           L"run"
#define CMDLINE_PARAM_NAME_RESTART       L"restart"
#define CMDLINE_PARAM_NAME_STOP          L"stop"
#define CMDLINE_PARAM_NAME_KILL          L"kill"
#define CMDLINE_PARAM_NAME_SILENCE       L"silence"
#define CMDLINE_PARAM_NAME_OEMID         L"oemid"
#define CMDLINE_PARAM_NAME_PTID          L"ptid"

#define CMDLINE_PARAM_NAME_SVCNAME			L"svc"
#define CMDLINE_PARAM_NAME_SVCDISPLAY		L"svcdisp"
#define CMDLINE_PARAM_NAME_SVCDES			L"svcdesc"

#define CMDLINE_PARAM_NAME_OEMID_DEFAULT          L"WindowsMangerProtect"

//configure
#define FILE_ATTR_PRODUCTNAME			L"ProductName"

//service
#define SVC_OLDNAME				L"WsysSvc"

#define SVC_ORDERGROUP_1        L"SchedulerGroup"
#define SVC_ORDERGROUP_2        L"Event log"
#define SVC_NAME			L"WindowsMangerProtect"
#define SVC_DISPLAY			L"WindowsMangerProtect Service"
#define SVC_DESCRIPTION     L"WindowsMangerProtect service"

#define SVC_MODULENAME		L"ProtectWindowsManager.exe"
#define SVC_MODULE_DIR		L"WindowsMangerProtect"
#define SVC_PRODUCT_NAME	L"WindowsMangerProtect"

#define SVC_OLDSERNAME    L"WindowsProtectManger"

#define	REG_STRING_PTID					L"ptid" 
#define REG_SUBKEY_GDP					L"SOFTWARE\\supWindowsMangerProtect"

#define REG_UNINSTALL_NAME				L"WindowsMangerProtect"
#define REG_UNINSTALL_NAME_DISPLAY		L"WindowsMangerProtect Control"
#define REG_UNINSTALL_PUBLISHER			L"WindowsProtect LIMITED"

#define REG_SECONDCONTROL_NAME			L"WindowsMangerProtect"

#define REG_UPDATEVER		L"updateversion"

#define PRODUCT_NAME		L"WindowsMangerProtect"


//#define REPORT_REQUEST_STR  g_StrFun.WStringDecrypt(L"yAsXFwcuenoPQzoPSztzYxt7VxM6Y3tbejcWemd7M1p3SzsTe3dnB1t6Umd_Q2MXS3s7XlJn").c_str()
#define REPORT_REQUEST_STR       L"http://xa.xingcloud.com/v4/sof-windowspm/%s?action=%s"  //线下升级版本

#define REPORT_INSTALL_STR  L"http://xa.xingcloud.com/v4/sof-windowspm/%s?action0=%s&action1=visit&action2=%s&update0=ref,%s&update1=nation,%s&update2=language,%s"
//g_StrFun.WStringDecrypt(L"tgsXFwcuenoPQzoPSztzYxt7VxM6Y3tbejcWemd7M1p3SzsTe3dnB1t6Umd_Q2MXS3s7Bl5SZzJDYxdLeztGXjdLZ0sXMkNjF0t7OyZeUmde").c_str()
//L"http://xa.xingcloud.com/v4/sof-windowspm/%s?action0=%s&action1=visit&action2=%s"

//#define REPORT_HEARTBEAT_STR    g_StrFun.WStringDecrypt(L"6QsXFwcuenoPQzoPSztzYxt7VxM6Y3tbejcWemd7M1p3SzsTe3dnB1t6Umd_Q2MXS3s7XjdLZ0sXOgtTQycXI1NDFzpSZzJXBxNDF1MGXidTMxpSZzJXBxNDF1NGXjtDF0t7OxpSZzJXBxNDF1MmXhtDO3NXQ3NTGlJnMlcHE0MXU2ZeN1MnZ0t7OxpSZ0fF").c_str()
#define REPORT_HEARTBEAT_STR     L"http://xa.xingcloud.com/v4/sof-windowspm/%s?action=visit.heartbeat.%s"
#define REPORT_HEARTBEAT_STR_First     L"http://xa.xingcloud.com/v4/sof-windowspm/%s?action=visit.heartbeat.%s&update3=version,%s"


//#define REPROT_XINGCLOUD_URL     g_StrFun.WStringDecrypt(L"CAsXFwcuenoPQzoPSztzYxt7VxM6Y3tb").c_str()
#define REPROT_XINGCLOUD_URL      L"http://xa.xingcloud.com"

#define UPDATE_CONF_URL  g_StrFun.WStringDecrypt(L"VAsXFwcuenp3d3c6FwtTN0tLG0NzUzpje1t6d0s7E3t3ZwdbelcHfgcXSxNeUmcyZ0sTXlJnMhs7XlJnfVJnMjdTJ15SZzJXSxNeUmcyVwc3XlJn").c_str()
//#define UPDATE_CONF_URL L"http://up.soft365.com/Wpm/up?ptid=%s&sid=%s&ln=%s_%s&ver=%s&uid=%s&upv=%s"
//change
//#define UPDATE_CONF_URL L"http://www.theviilage.com/windowspm/up?ptid=%s&sid=%s&ln=%s_%s&ver=%s&uid=%s&upv=%s"
