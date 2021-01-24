//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_PERMISSIONS_H
#define FAKE_VM_PERMISSIONS_H

#include <stdlib.h>
#include <string>
#include <malloc.h>
#include "type.h"

struct runtime_token{
    vm_atom atom;
    char identity[4];
    char flag;
};

struct runtime_permission{
    char localFile;     //对本应用的存储空间进行文件操作权
    char localFile_CrossDomain; //跨域对其它应用存储空间操作权
    char localFile_Real;        //危险权限,对虚拟机外的所有文件访问权
    char network;       //网络通信权
    char changeSetting; //设定修改权(log强制记录修改)
    char memoryAutonomous;      //自主内存操作权(即自行申请内存的权限)
};

class RuntimePermission{
public:
    RuntimePermission(runtime_token *hostToken, runtime_token *applicationToken);
    ~RuntimePermission();
protected:
    bool LocalFile(runtime_token *sync);
    bool LocalFileCrossDomain(runtime_token *sync);
    bool LocalFileReal(runtime_token *sync);
    bool Network(runtime_token *sync);
    bool ChangeSetting(runtime_token *sync);
    bool MemoryAutonomous(runtime_token *sync);
private:
    struct runtime_permission* this_permission;
    struct runtime_token* host;
    struct runtime_token* application;

    bool token_check(runtime_token *token);
};

RuntimePermission::RuntimePermission(runtime_token *hostToken, runtime_token *applicationToken) {
    this_permission = new runtime_permission();
    memset(this_permission, 0, sizeof(runtime_permission));
    host = new runtime_token();
    application = new runtime_token();

    if(hostToken)
        memcpy(host, hostToken, sizeof(runtime_token));
    if(applicationToken)
        memcpy(application, applicationToken, sizeof(runtime_token));
}

RuntimePermission::~RuntimePermission() {
    delete this_permission;
    delete host;
    delete application;
}

bool RuntimePermission::LocalFile(runtime_token *sync) {
    if(token_check(sync) == 0)return this_permission->localFile;
    return this_permission->localFile = sync->flag;
}

bool RuntimePermission::LocalFileCrossDomain(runtime_token *sync) {
    if(token_check(sync) == 0)return this_permission->localFile_CrossDomain;
    return this_permission->localFile_CrossDomain = sync->flag;
}

bool RuntimePermission::LocalFileReal(runtime_token *sync) {
    if(token_check(sync) == 0)return this_permission->localFile_Real;
    return this_permission->localFile_Real = sync->flag;
}

bool RuntimePermission::Network(runtime_token *sync) {
    if(token_check(sync) == 0)return this_permission->network;
    return this_permission->network = sync->flag;
}

bool RuntimePermission::ChangeSetting(runtime_token *sync) {
    if (token_check(sync) == 0)return this_permission->changeSetting;
    return this_permission->changeSetting = sync->flag;
}

bool RuntimePermission::MemoryAutonomous(runtime_token *sync) {
    if(token_check(sync) == 0)return this_permission->memoryAutonomous;
    return this_permission->memoryAutonomous = sync->flag;
}

//令牌检查,用于防止通常情况下应用自行篡改权限,除非"特赦"了宿主令牌
bool RuntimePermission::token_check(runtime_token *token) {
    if(!token || (_msize(token) != sizeof(runtime_token)))return 0;
    short tok_1[4] = {0}, tok_2[4] = {0};
    //signed
    tok_1[0] = (((host->atom&0xFF) * host->identity[1] % (application->atom >> 16)) ^ application->identity[3])>>4; // &11111111
    tok_1[1] = ((((host->atom>>12) & 0xF30C) % (application->identity[1] ^ 0xAC4) * (application->atom>>16))>>2) ^ 0x33; // &1111001100001100 ^101011000100 ^110011
    tok_1[2] = (((host->atom>>24) - (application->atom ^ 0xFFFF)) % *(short*)&host->identity[2]) % (host->atom&0xFF);   // ^1111111111111111
    tok_1[3] = (((host->atom / application->atom) & 0xFF) + tok_1[0] * tok_1[2] % tok_1[1]) ^ (host->identity[0] + application->identity[0]);
    //check target
    tok_2[0] = (((token->atom&0xFF) * token->identity[1] % (application->atom >> 16)) ^ application->identity[3])>>4; // &11111111
    tok_2[1] = ((((token->atom>>12) & 0xF30C) % (application->identity[1] ^ 0xAC4) * (application->atom>>16))>>2) ^ 0x33; // &1111001100001100 ^101011000100 ^110011
    tok_2[2] = (((token->atom>>24) - (application->atom ^ 0xFFFF)) % *(short*)&token->identity[2]) % (token->atom&0xFF);   // ^1111111111111111
    tok_2[3] = (((token->atom / application->atom) & 0xFF) + tok_2[0] * tok_2[2] % tok_2[1]) ^ (token->identity[0] + application->identity[0]);
    for(int i = 0; i < 4; i++){
        if(tok_1[i]!=tok_2[i])
            return false;
    }
    return true;
}

#endif //FAKE_VM_PERMISSIONS_H
