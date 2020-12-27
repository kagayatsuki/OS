//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_PERMISSION_MANAGER_H
#define FAKE_VM_PERMISSION_MANAGER_H


#include "permissions.h"
#include "time.h"


class FakeVM_PermissionManager{
protected:
    FakeVM_PermissionManager(runtime_permission* permission, runtime_token* app_token);
    ~FakeVM_PermissionManager();
    runtime_token* makeToken();
    runtime_token* host_token;
    RuntimePermission* permission;
};

FakeVM_PermissionManager::FakeVM_PermissionManager(runtime_permission *permission, runtime_token* app_token) {
    host_token = makeToken();
    permission = new RuntimePermission(host_token, app_token);
}

FakeVM_PermissionManager::~FakeVM_PermissionManager() {
    delete permission;
    delete host_token;
}

runtime_token *FakeVM_PermissionManager::makeToken() {
    runtime_token* token = new runtime_token();
    time_t time_tmp = time(NULL);
    time_tmp = (time_tmp<<4) + time(NULL);
    srand(time_tmp);
    token->atom = rand() + 87653242;
    *(int *)token->identity = rand();
    return token;
}

#endif //FAKE_VM_PERMISSION_MANAGER_H
