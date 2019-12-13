#ifndef WPMV2_NEWGDP_SERVICE_APISERVICE_H_
#define WPMV2_NEWGDP_SERVICE_APISERVICE_H_
#pragma once

#include <tchar.h> 
#include <windows.h>

namespace APIService {
  BOOL Uninstall();
  BOOL IsInstalled();
  BOOL UninstallService();
  BOOL StopService();
  BOOL StartService();
  BOOL UninstAndStopService();
  BOOL InstAndStartService(BOOL bAfterXP, bool bDelayStartup);
  BOOL InstallService(BOOL bAfterXP, bool bDelayStartup);
  BOOL Install(BOOL bAfterXP, bool bDelayStartup);
}
#endif  // WPMV2_NEWGDP_SERVICE_APISERVICE_H_


