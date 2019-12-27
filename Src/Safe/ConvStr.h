#pragma once
_STD_BEGIN

//! UNICODE串转换为ANSI串, std::w2a 
inline std::string w2a(LPCWSTR s) 
{ 
	std::string str; 
	int wlen = (NULL == s) ? 0 : (int)wcslen(s); 
	if (wlen > 0) 
	{ 
		long len = WideCharToMultiByte(CP_ACP, 0, s, wlen, NULL, 0, NULL, NULL); 
		str.resize(len); 
		WideCharToMultiByte(CP_ACP, 0, s, wlen, 
			const_cast<char*>(str.data()), len, NULL, NULL); 
	}

	return str; 
}

//! UNICODE串转换为ANSI串, std::w2a 
/*! 
    \ingroup _GROUP_UTILFUNC 
*/ 
inline std::string w2a(const std::wstring& s) 
{ 
    return w2a(s.c_str()); 
}

//! ANSI串转换为UNICODE串, std::a2w 
/*! 
    \ingroup _GROUP_UTILFUNC 
*/ 
inline std::wstring a2w(LPCSTR s) 
{ 
    std::wstring wstr; 
    int len = (NULL == s) ? 0 : (int)strlen(s); 
    if (len > 0) 
    { 
        int wlen = MultiByteToWideChar(CP_ACP, 0, s, len, NULL, 0); 
        wstr.resize(wlen); 
        MultiByteToWideChar(CP_ACP, 0, s, len, 
            const_cast<LPWSTR>(wstr.data()), wlen); 
    }

    return wstr; 
}

//! ANSI串转换为UNICODE串, std::a2w 
/*! 
    \ingroup _GROUP_UTILFUNC 
*/ 
inline std::wstring a2w(const std::string& s) 
{ 
    return a2w(s.c_str()); 
}

#ifdef _UNICODE 
inline std::wstring w2t(LPCWSTR s) { return s; } 
inline std::wstring w2t(const std::wstring& s) { return s; } 
inline std::wstring t2w(LPCTSTR s) { return s; } 
#else 
inline std::string w2t(LPCWSTR s) { return w2a(s); } 
inline std::string w2t(const std::wstring& s) { return w2a(s); } 
inline std::wstring t2w(LPCTSTR s) { return a2w(s); } 
#endif
_STD_END