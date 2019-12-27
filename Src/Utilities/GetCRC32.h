namespace utils
{
	namespace CRC
	{

#ifdef __cplusplus
		extern "C"
		{
#endif

			// 这个函数只能在计算多块数据的CRC32时使用，初始值为-1，最后需要取反
			void 
				CRC32(  unsigned long crc,
				unsigned long cbBuffer,
				void * pvBuffer,
				unsigned long * pNewCrc);

			// 正确的函数
			unsigned long 
				GetCRC32(unsigned long cbBuffer,void * pvBuffer);

#ifdef __cplusplus
		}
#endif
	}
}

