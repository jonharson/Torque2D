//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLATFORM_STRING_H_
#define _PLATFORM_STRING_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

//------------------------------------------------------------------------------

extern DLLEXPORTS dsize_t dStrlen(const char *str);

extern DLLEXPORTS char* dStrcat(char *dst, const char *src);
extern DLLEXPORTS UTF8* dStrcat(UTF8 *dst, const UTF8 *src);

extern DLLEXPORTS char* dStrncat(char* dst, const char* src, dsize_t len);
extern DLLEXPORTS char* dStrcatl(char *dst, dsize_t dstSize, ...);

extern DLLEXPORTS int dStrcmp(const char *str1, const char *str2);
extern DLLEXPORTS int dStrcmp(const UTF16 *str1, const UTF16 *str2);
extern DLLEXPORTS int dStrcmp(const UTF8 *str1, const UTF8 *str2);

extern DLLEXPORTS int dStricmp(const char *str1, const char *str2);
extern DLLEXPORTS int dStrncmp(const char *str1, const char *str2, dsize_t len);
extern DLLEXPORTS int dStrnicmp(const char *str1, const char *str2, dsize_t len);

extern DLLEXPORTS char* dStrcpy(char *dst, const char *src);
extern DLLEXPORTS char* dStrcpyl(char *dst, dsize_t dstSize, ...);
extern DLLEXPORTS char* dStrncpy(char *dst, const char *src, dsize_t len);
extern DLLEXPORTS char* dStrncpy(UTF8 *dst, const UTF8 *src, dsize_t len);

extern DLLEXPORTS char* dStrupr(char *str);
extern DLLEXPORTS char* dStrlwr(char *str);
inline DLLEXPORTS char dToupper(const char c) { if (c >= char('a') && c <= char('z')) return char(c + 'A' - 'a'); else return c; }
inline DLLEXPORTS char dTolower(const char c) { if (c >= char('A') && c <= char('Z')) return char(c - 'A' + 'a'); else return c; }

extern DLLEXPORTS char* dStrchr(char *str, int c);
extern DLLEXPORTS const char* dStrchr(const char *str, int c);
extern DLLEXPORTS char* dStrrchr(char *str, int c);
extern DLLEXPORTS const char* dStrrchr(const char *str, int c);
extern DLLEXPORTS dsize_t dStrspn(const char *str, const char *set);
extern DLLEXPORTS dsize_t dStrcspn(const char *str, const char *set);
extern DLLEXPORTS char* dStrstr(char *str1, char *str2);
extern DLLEXPORTS char* dStrstr(const char *str1, const char *str2);

extern DLLEXPORTS char* dStrtok(char *str, const char *sep);

extern DLLEXPORTS int dStrrev(char* str);

extern DLLEXPORTS int dAtoi(const char *str);
extern DLLEXPORTS float dAtof(const char *str);
extern DLLEXPORTS bool dAtob(const char *str);
extern DLLEXPORTS int dItoa(int n, char s[]);

extern DLLEXPORTS bool dIsalnum(const char c);
extern DLLEXPORTS bool dIsalpha(const char c);
extern DLLEXPORTS bool dIsdigit(const char c);
extern DLLEXPORTS bool dIsspace(const char c);

extern DLLEXPORTS int dSscanf(const char *buffer, const char *format, ...);
extern DLLEXPORTS int dFflushStdout();
extern DLLEXPORTS int dFflushStderr();

extern DLLEXPORTS void dPrintf(const char *format, ...);
extern DLLEXPORTS int dVprintf(const char *format, void *arglist);
extern DLLEXPORTS int dSprintf(char *buffer, dsize_t bufferSize, const char *format, ...);
extern DLLEXPORTS int dVsprintf(char *buffer, dsize_t bufferSize, const char *format, void *arglist);

#define QSORT_CALLBACK FN_CDECL
extern DLLEXPORTS void dQsort(void *base, U32 nelem, U32 width, int (QSORT_CALLBACK *fcmp)(const void *, const void *));

// UNICODE is a windows platform API switching flag. Don't define it on other platforms.
#ifdef UNICODE
#define dT(s)    L##s
#else
#define dT(s)    s
#endif

#define dStrdup(x) dStrdup_r(x, __FILE__, __LINE__)

extern DLLEXPORTS char* dStrdup_r(const char *src, const char*, dsize_t);

#endif  // _PLATFORM_STRING_H_
