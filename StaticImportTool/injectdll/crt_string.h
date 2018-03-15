#ifndef CRT_STRING_H_
#define CRT_STRING_H_

#include <windows.h>

BOOL __ISUPPER__(__in CHAR c);
CHAR __TOLOWER__(__in CHAR c);
UINT __STRLEN__(__in LPSTR lpStr1);
UINT __STRLENW__(__in LPWSTR lpStr1);
LPWSTR __STRSTRIW__(__in LPWSTR lpStr1, __in LPWSTR lpStr2);
INT __STRCMPI__(__in LPSTR lpStr1, __in LPSTR lpStr2);
INT __STRNCMPI__(__in LPSTR lpStr1, __in LPSTR lpStr2, __in DWORD dwLen);
INT __STRNCMPIW__(__in LPWSTR lpStr1, __in LPWSTR lpStr2, __in DWORD dwLen);
LPWSTR __STRCATW__(__in LPWSTR	strDest, __in LPWSTR strSource);
LPVOID __MEMCPY__(__in LPVOID lpDst,__in LPVOID lpSrc,__in DWORD dwCount);
VOID __MEMSET__(__in LPVOID p, __in CHAR cValue, __in DWORD dwSize);

#endif