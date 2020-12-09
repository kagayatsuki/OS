/*********************************************
 * OS loader
 * This program must be run as Administrator
 * :shinsya
 ********************************************/

#if       _WIN32_WINNT < 0x0500
#undef  _WIN32_WINNT
#define _WIN32_WINNT   0x0500
#endif

#include <stdio.h>
#include <conio.h>
#include <dos.h>
//#include <bios.h>
//#include <ShlObj.h>
//#include <ShlObj_core.h>
#include <Windows.h>


#define _INIT_LOADER_ARG "-loader"


bool shutdown_flag = false;


bool _init_permission_check(int argc, char* argv[]){
    int i = 0;
    char buffer[512];
    bool byLoader = false;
    for(;i<argc;i++){
        if(strncmp(argv[i], _INIT_LOADER_ARG, strlen(argv[i])) == 0){
            byLoader = true;
            break;
        }
    }
    if(!byLoader)return false;
    if(true)
        return true;
    return false;
}

void _init_load_self(LPCSTR self_exe, LPCSTR param){
    SHELLEXECUTEINFO ShExecInfo;
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS ;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = "runas";
    ShExecInfo.lpFile = self_exe;
    ShExecInfo.lpParameters = param;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    BOOL ret = ShellExecuteEx(&ShExecInfo);
}

void _init_config(int argc, char* argv[]){
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);  //Set pc to never sleep
}

int _init_loader(){
    printf("============================ OS ============================\n");
    getchar();

    //debug setting
    shutdown_flag = true;

    return 0;
}

int main(int argc, char* argv[]){
    if(argc <= 1){  //administrator permission check
        _init_load_self(argv[0], _INIT_LOADER_ARG);
        return 0;
    }

    _init_config(argc, argv);   //init config

    int cr = _init_loader();    //loader
    if(cr != 0){    //loader exception
        printf("There were some exceptions. Code: %d\n", cr);
        getchar();
        return 0;
    }
    ShowWindow(GetConsoleWindow(), SW_HIDE);    //hide console window when inited OS
    while(true){
        if(shutdown_flag)   //OS running in multi-thread, check exit signal here (This signal set by OS working thread)
            break;
    }

    return 0;
}
