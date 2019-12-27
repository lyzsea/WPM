/************************************************************************/
/* 
    Date : 10/22/2012
	File : ECMDLINE.H
    Author : Eadwin  (lidi@elex-tech.com)
    Funtion :  
	Version : 1.0
	Copyright (c) 2012 ELEX CDD
*/
/************************************************************************/  
#pragma once

#include <Shlwapi.h>
#include <windows.h>
#include <string>
#include <tchar.h>
#include <map>

#ifdef UNICODE
#define TString std::wstring
#else
#define TString std::string
#endif // UNICODE


namespace Utilities{namespace String{


class CCmdLine
{
public:
    CCmdLine(LPCTSTR lpszCmdLine = NULL)
    {
        Analyze(lpszCmdLine);
    }

    ~CCmdLine()
    {
    }

    class CParam
    {
    public:
        CParam(LPCTSTR lpszValue)
        {
            m_strValue = lpszValue;
        }

        ~CParam()
        {
        }

        TString String()
        {
            return m_strValue;
        }

        operator TString()
        {
            return m_strValue;
        }

        operator int()
        {
            return ::StrToInt(m_strValue.c_str());
        }

		operator bool()
		{
			return ::StrToInt(m_strValue.c_str()) ? true : false;
		}

    private:
        TString m_strValue;
    };

    BOOL Analyze(LPCTSTR lpszCmdLine)
    {
        BOOL bResult = FALSE;
        int nPos = 0, nParamNamePos = 0, nParamValuePos = 0;
        int nStatus = 0;
        TString strParamName;
        BOOL bInComma = FALSE;
        TCHAR chPos = _T('\0');

        if (!lpszCmdLine)
            goto Exit0;

        m_strCmdLine = lpszCmdLine;

        for (int nPos = 0; 0 == nPos || _T('\0') != lpszCmdLine[nPos - 1]; nPos ++)
        {
            chPos = lpszCmdLine[nPos];

            switch (nStatus)
            {
            case 0:

                if (bInComma)
                {
                    if (_T('\"') == chPos)
                        bInComma = FALSE;
                }
                else if (_T('-') == chPos || _T('/') == chPos)
                    nStatus = 1;
                else if (_T('\"') == chPos)
                    bInComma = TRUE;

                break;

            case 1:
                nParamNamePos = nPos;

                nStatus = 2;
                break;

            case 2:
                if (_T(' ') == chPos || _T('\0') == chPos)
                {
                    strParamName = m_strCmdLine.substr(nParamNamePos, nPos - nParamNamePos);

                    SetParam(strParamName.c_str(), TRUE);

                    nStatus = 0;
                }
                else if (_T('=') == chPos)
                {
                    strParamName = m_strCmdLine.substr(nParamNamePos, nPos - nParamNamePos);
                    nStatus = 3;
                }

                break;

            case 3:
                if (_T('\"') == chPos)
                {
                    nPos ++;
                    bInComma = TRUE;
                }
                else if (_T(' ') == chPos || _T('\0') == chPos)
                {
                    SetParam(strParamName.c_str(), _T(""));
                    nStatus = 0;

                    break;
                }

                nParamValuePos = nPos;
                nStatus = 4;

                break;

            case 4:
                if ((_T(' ') == chPos && !bInComma) || _T('\0') == chPos || (_T('\"') == chPos && bInComma))
                {
					TString szNewline = m_strCmdLine.substr(nParamValuePos, nPos - nParamValuePos);
                    SetParam(strParamName.c_str(), szNewline.c_str());
                    nStatus = 0;
                    bInComma = FALSE;
                }

                break;
            }
        }

        bResult = TRUE;

    Exit0:

        return bResult;
    }

    BOOL HasParam(LPCTSTR lpszParamName)
    {
        if (m_mapParams.find(lpszParamName) != m_mapParams.end( ))
            return TRUE;

        return FALSE;
    }

    BOOL SetParam(LPCTSTR lpszParamName, LPCTSTR lpszParamValue)
    {
        m_mapParams[lpszParamName] = lpszParamValue;

        return TRUE;
    }

    BOOL SetParam(LPCTSTR lpszParamName, int nValue)
    {
		TCHAR strNumber[1024] = {0};
		_itow_s(nValue,strNumber,10);
        m_mapParams[lpszParamName] = strNumber;

        return TRUE;
    }

    void GetCmdLine(TString& strCmdLine)
    {
        std::map<TString,TString>::iterator pos = m_mapParams.begin();
        TString strParam;

        strCmdLine = _T("");

        while (pos != m_mapParams.end())
        {

            if (pos->second == _T("1"))
			{
				strParam.append(_T(" -"));
                strParam.append(pos->first);
			}
            else
			{
				strParam.append(_T(" -"));
				strParam.append(pos->first);
				strParam.append(_T("="));
				strParam.append(pos->second);
			}

            strCmdLine += strParam;
        }
    }

    CParam operator[](LPCTSTR lpszParamName)
    {
        TString strValue;

       strValue = m_mapParams.find(lpszParamName)->second;

        return CParam(strValue.c_str());
    }


protected:

    TString m_strCmdLine;

    template < typename T >
    class CECharTraits
    {
    };

    template <>
    class CECharTraits<char>
    {
    public:
        static char CharToUpper(char c)
        {
            if (c >= 'a' && c <= 'z')
                return c - 'a' + 'A';
            else
                return c;
        }

        static char CharToLower(char c)
        {
            if (c >= 'A' && c <= 'Z')
                return c - 'A' + 'a';
            else
                return c;
        }
    };

    template <>
    class CECharTraits<wchar_t>
    {
    public:
        static wchar_t CharToUpper(wchar_t c)
        {
            if (c >= 'a' && c <= 'z')
                return c - 'a' + 'A';
            else
                return c;
        }

        static wchar_t CharToLower(wchar_t c)
        {
            if (c >= 'A' && c <= 'Z')
                return c - 'A' + 'a';
            else
                return c;
        }
    };

    typedef std::map<TString, TString> _CmdParamMap;

    _CmdParamMap m_mapParams;

private:
};

}}