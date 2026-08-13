#pragma once

#include "windows.h"


#define CHINESE_SIMPLIFIED 1
#define   CHINESE_TRADITIONAL 2

class CChineseConvertor 
{
public:
	CChineseConvertor(void);
	~CChineseConvertor(void);
	LPSTR Big52GBKSimplified(char * szText);
	LPSTR Big52GBKTraditional(char * szText);
	LPSTR GBK2Big5(char * szText);
	LPSTR GBKSimplified2GBKTraditional(char * szSimplified);
	LPSTR GBKTraditional2GBKSimplified(char * szTraditional);
	LPWSTR UTF82UNICODE(char*   utf8str);
	LPSTR UNICODE2UTF8(LPCWSTR  strText);

	char *m_pszUnknown;
	// ×ª»»µ½Unicode
	LPWSTR ToUnicode(char * szSource, int nEncoding);
	LPSTR ToMultiByte(LPCWSTR szSource, int nEncoding);
};
