/************************************************************************/
/* 
    Date : 10/15/2012
	File : ZBASE64.H
    11
       
	Version : 1.0
	Copyright
*/
/************************************************************************/  

#pragma once

#include <string>

using namespace std;

namespace Utilities{

class CeBase64
{
public:
	CeBase64();
	~CeBase64();

	/********************************************************* 
	* Function 	:	将输入数据进行base64编码
	* Parameter	:	[in]pIn		需要进行编码的数据
					[in]uInLen  输入参数的字节数
					[out]strOut 输出的进行base64编码之后的字符串
	* Return	: 	bool
	* Author	:	Eadwin  (lidi@elex-tech.com)
	* Date		:	10/16/2012
	**********************************************************/ 
	bool static Encode(const unsigned char *pIn, unsigned long uInLen, string& strOut);

	/********************************************************* 
	* Function 	:	将输入数据进行base64编码
	* Parameter	:	[in]pIn		需要进行编码的数据
					[in]uInLen  输入参数的字节数
					[out]pOut		输出的进行base64编码之后的字符串
					[out]strOut 输出的进行base64编码之后的字符串
	* Return	: 	bool
	* Author	:	Eadwin  (lidi@elex-tech.com)
	* Date		:	10/16/2012
	**********************************************************/ 
	bool static Encode(const unsigned char *pIn, unsigned long uInLen, unsigned char *pOut, unsigned long *uOutLen);
	
	/********************************************************* 
	* Function 	:	将输入数据进行base64解码
	* Parameter	:	[in]strIn		需要进行解码的数据
					[out]pOut		输出解码之后的节数数据
					[out]uOutLen	输出的解码之后的字节数长度
	* Return	: 	bool
	* Author	:	Eadwin  (lidi@elex-tech.com)
	* Date		:	10/16/2012
	**********************************************************/ 
	bool static Decode(const string& strIn, unsigned char *pOut, unsigned long *uOutLen);

	/********************************************************* 
	* Function 	:	将输入数据进行base64解码
	* Parameter	:	[in]pIn			需要进行解码的数据
					[in]uInLen		输入参数的字节数
					[out]pOut		输出解码之后的节数数据
					[out]uOutLen	输出的解码之后的字节数长度
	* Return	: 	bool
	* Author	:	Eadwin  (lidi@elex-tech.com)
	* Date		:	10/16/2012
	**********************************************************/ 
	bool static Decode(const unsigned char *pIn, unsigned long uInLen, unsigned char *pOut, unsigned long *uOutLen);
	
};

}