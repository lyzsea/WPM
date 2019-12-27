#pragma once

namespace Utilities
{
	class MD5
	{
	public:
		static bool md5(const char *buf, char *md5val, int bufSize = 0);
		static bool md5(const char *buf, wchar_t *md5val, int bufSize = 0);

		static bool md5File(const char *path, char *md5val);
		static bool md5File(const wchar_t *path, wchar_t *md5val);
	private:
		//MD5摘要值结构体
		typedef struct MD5VAL_STRUCT
		{
			unsigned int a;
			unsigned int b;
			unsigned int c;
			unsigned int d;
		} MD5VAL;

		//计算字符串的MD5值(若不指定长度则由函数计算)
		static MD5VAL md5sum(char * str, unsigned int size=0);

		//MD5文件摘要
		static MD5VAL md5sumFile(FILE * fpin);
		static unsigned int MD5::conv(unsigned int a);
	};
}