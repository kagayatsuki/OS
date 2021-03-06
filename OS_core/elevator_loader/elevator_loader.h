//
// Created by guoku on 2021/3/1.
//

#ifndef ELEVATOR_LOADER_ELEVATOR_LOADER_H
#define ELEVATOR_LOADER_ELEVATOR_LOADER_H

#include <windows.h>
#include "args_struct.h"
#include "evr_thread.h"

typedef struct elevator_names{

}evr_names;

class evr_loader{
    HANDLE module;
    evrcall func_entry;
    evrthread func_exit;
    bool inited;
public:
    evr_loader();
    bool init(char *path);
    bool start(evr_arg *arg);
    bool uninstall();
};

evr_loader::evr_loader()
:module(0)
{
    func_entry = 0;
    func_exit = 0;
    inited = false;
    //TODO: 未来可能的其它初始化操作
}

bool evr_loader::init(char *path) {
    if(module)
        return false;
    if(inited)
        return true;
    module = LoadLibrary(path);
    if(module == 0)
        return false;
    func_entry = (evrcall)GetProcAddress((HINSTANCE__*)module, func_name_entry);
    func_exit = (evrthread)GetProcAddress((HINSTANCE__*)module, func_name_exit);
    if(!(func_entry && func_exit))
        return false;
    inited = true;
}

bool evr_loader::start(evr_arg *arg) {
    if(!inited)
        return false;
    func_entry(arg);
    return true;
}

bool evr_loader::uninstall() {
    if(!inited)
        return false;
    puts("Uninstalling elevator. Waiting for 30s.\n");
    WaitingForExit(func_exit, 30000);
    FreeLibrary((HINSTANCE__*)module);
    module = 0;
    inited = false;
    return true;
}

#endif //ELEVATOR_LOADER_ELEVATOR_LOADER_H
