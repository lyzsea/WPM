#pragma once
#include <string>

class WPMService;

class CServiceInstaller
{
public:
	CServiceInstaller(WPMService* pService);
	~CServiceInstaller(void);

	void PreSelfInstall(const std::wstring& strCmdLine);
	void SelfUninstall();

private:
	WPMService* m_pService;
	std::wstring m_strAppDir;
	std::wstring m_strCurModule;

	bool WriteUninstallInfo();
	bool ClearRegInfo();

	void CheckOldVersion();
};