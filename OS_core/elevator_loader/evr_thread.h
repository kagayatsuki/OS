//
// Created by guoku on 2021/3/6.
//

#ifndef ELEVATOR_LOADER_EVR_THREAD_H
#define ELEVATOR_LOADER_EVR_THREAD_H

#include "args_struct.h"

#include <Windows.h>

DWORD __stdcall _thread_wait_exit(LPVOID lpParam){
    evrthread exit_t = (evrthread)lpParam;
    exit_t();
    return (DWORD)0;
}

void WaitingForExit(evrthread exit_t, unsigned int MilliSec){
    HANDLE thread = CreateThread(NULL, 0, _thread_wait_exit, (LPVOID)exit_t, 0, NULL);
    WaitForSingleObject(thread, MilliSec);
}

#endif //ELEVATOR_LOADER_EVR_THREAD_H
