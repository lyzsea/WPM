#ifndef __UTL_STR_NUM_H_
#define __UTL_STR_NUM_H_
inline BYTE CharToBin(TCHAR ch)
{
	 if(ch>='0' && ch<='9')
		 return ch - '0';

	 if(ch>='a' && ch<='f')
		 return ch - 'a' + 10;

	 if(ch>='A' && ch<='F')
		 return ch - 'A' + 10;
	 return 0xff;
}
inline bool Hex2Integer(LPCTSTR lpHex, int nHexLen, unsigned int* pOut)
{
	UINT uNum = 0;
	for (int i = 0; i < nHexLen && i < 8; i++)
	{
		if (lpHex[i] == 0)
			break;

		BYTE nHalfByte = CharToBin(lpHex[i]);
		if (nHalfByte == 0xFF)
			return false;

		uNum = (uNum<<4)|nHalfByte;
		
	}

	*pOut = uNum;
	return true;
}
#endif//__UTL_STR_NUM_H_