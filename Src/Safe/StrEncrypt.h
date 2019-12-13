#pragma once

class CStrEncrypt
{
public:
	CStrEncrypt(void);
	~CStrEncrypt(void);


	std::string StringEncrypt(const char* srcstr, int srclen = -1);   //×Ö·û´®¼ÓÃÜ
	std::string StringDecrypt(const char* srcstr);           //×Ö·û´®½âÃÜ

	std::wstring WStringEncrypt(std::wstring srwstr, int srclen = -1);   //Unicode×Ö·û´®¼ÓÃÜ 
	std::wstring WStringDecrypt(std::wstring srwstr);                   //Unicode×Ö·û´®½âÃÜ



private:
	char UnIndex64(BYTE nIndex);
	BYTE Index64(char ch);
	void ToBase64(const char* instr, int len, char* outstr);
	void UnBase64(const char* instr, int len, char* outstr);
	char EncryptChar(char c) ;
	char randchar(); 
	 char  HexToASCII(unsigned char  data_hex);
	 bool HexGroupToString(char *OutStrBuffer, char *InHexBuffer, int HexLength);
	 bool StringToHexGroup(char *OutHexBuffer, char *InStrBuffer, int strLength);
	 
	 unsigned long GetTime(void);
};
