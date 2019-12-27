#pragma once

#define MEM_BUFFER_SIZE 10

/* HTTPRequest: Structure that returns the HTTP headers and message
                from the request*/
typedef struct
{ 
	LPSTR headers;/* Pointer to HTTP headers */
	LPSTR message;/* Pointer to the HTTP message */
	long messageLength;/* Length of the message */
} HTTPRequest;

/* MemBuffer: Structure used to implement a memory buffer, which is a
              buffer of memory that will grow to hold variable sized
              parts of the HTTP message. */
typedef struct
{
	unsigned char *buffer;
	unsigned char *position;
	size_t size;
} MemBuffer;




class CCallMeUpLoadByUninstall
{
public: 
	  static CCallMeUpLoadByUninstall*  GetCallMeUploadObject();

	  int SendHTTP(LPCSTR url,LPCSTR headers,BYTE *post,DWORD postLength,HTTPRequest *req);

	  ~CCallMeUpLoadByUninstall(void);


private:
	CCallMeUpLoadByUninstall(void);

	BOOL ValidHostChar(char ch);
	void MemBufferCreate(MemBuffer *b);
	void MemBufferGrow(MemBuffer *b);
	void MemBufferAddByte(MemBuffer *b,unsigned char byt);
	void MemBufferAddBuffer(MemBuffer *b,unsigned char *buffer,size_t size);
	DWORD GetHostAddress(LPCSTR host);
	void SendString(SOCKET sock,LPCSTR str);
	void ParseURL(LPCSTR url,LPSTR protocol,int lprotocol,LPSTR host,int lhost,LPSTR request,int lrequest,int *port);


};
