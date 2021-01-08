//
// Created by shinsya on 2021/1/3.
//

#ifndef FAKE_VM_SIMPLE_UNICODE_H
#define FAKE_VM_SIMPLE_UNICODE_H

#include <stdio.h>
#include <string>
#include <windows.h>

/** Ansi转Unicode编码 **/
wchar_t *AnsiToUnicode(const char *szStr){
    int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
    if (nLen == 0) {
        return NULL;
    }
    wchar_t* pResult = new wchar_t[nLen + 1];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
    return pResult;
}

/** Unicode转Ansi编码 **/
char *UnicodeToAnsi(const wchar_t *szStr){
    int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
    if (nLen == 0) { return NULL; }
    char* pResult = new char[nLen];
    WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
    return pResult;
}

#endif //FAKE_VM_SIMPLE_UNICODE_H
