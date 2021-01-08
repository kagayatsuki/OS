//
// Created by shinsya on 2021/1/8.
//

#ifndef FAKE_VM_SIMPLE_ACTIVITY_H
#define FAKE_VM_SIMPLE_ACTIVITY_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "simple_unicode.h"

typedef void(*ActCall)(HWND view);  //标准控件回调函数指针

typedef struct callback_list{   //控件对应的回调表
    int call_id;
    ActCall callback;

    callback_list *next;
}CallbackList;

typedef struct view_list{   //记录控件信息
    HWND handle;
    //int id;
    CallbackList *callList;

    view_list *next;
}ViewList;

typedef struct activity_info{   //记录窗体信息
    void* activity;
    ViewList *viewList;
    //CallbackList *windowActList;
    CallbackList *viewActList;

    activity_info *next;
}ActivityInfo;

activity_info* activityList = 0;    //活动记录表

HINSTANCE simple_global_default_instance = 0;   //另指定的实例
void Simple_SethInstance(HINSTANCE instance){simple_global_default_instance = instance;}    //辅助函数，用于另指定实例

/** 增加新活动记录节点 **/
void _simple_activity_new(void *activity){
    ActivityInfo *tmp = new ActivityInfo();
    tmp->activity = activity;
    //debug:
    char tmpS[64] = {0};
    wchar_t *tmpS2;
    sprintf(tmpS, "New Activity: %p", activity);
    tmpS2 = AnsiToUnicode(tmpS);
    //MessageBoxW(0, tmpS2, L"SimpleUI Debug", MB_OK);
    delete []tmpS2;
    if(activityList == NULL){
        activityList = tmp;
    }else{
        ActivityInfo *nextNode = activityList;
        while(nextNode->next){
            nextNode = nextNode->next;
        }
        nextNode->next = tmp;
    }
}

/** 获取活动信息 **/
ActivityInfo *_simple_activity_find(void *activity){
    ActivityInfo *tmp = activityList;
    while(tmp){
        if(tmp->activity == activity)
            return tmp;
        tmp = tmp->next;
    }
    return 0;
}

/** 获取控件信息 **/
ViewList *_simple_view_find(ActivityInfo *activity, HWND viewHandle){
    if(activity == NULL)
        return 0;
    if(activity->viewList == NULL)
        return 0;
    ViewList *tmp = activity->viewList;
    while(tmp){
        if(tmp->handle == viewHandle)
            return tmp;
        tmp = tmp->next;
    }
    return 0;
}

/** 获取回调信息 **/
CallbackList *_simple_callback_find(ViewList *view, int callId){
    if(view == NULL)
        return 0;
    if(view->callList == NULL)
        return 0;
    CallbackList *tmp = view->callList;
    while(tmp){
        if(tmp->call_id == callId)
            return tmp;
        tmp = tmp->next;
    }
    return 0;
}

CallbackList *_simple_callback_find(ActivityInfo *activity, int callId){
    if(activity == NULL)
        return 0;
    CallbackList *tmp = activity->viewActList;
    while(tmp){
        if(tmp->call_id == callId)
            return tmp;
        tmp = tmp->next;
    }
    return 0;
}

/** 追加控件信息 **/
void _simple_view_new(ActivityInfo *activity, HWND handle){
    if(activity == NULL)
        return;
    if(activity->viewList){
        ViewList *tmp = activity->viewList;
        while(tmp->next){tmp = tmp->next;}
        tmp->next = new ViewList();
        tmp->next->handle = handle;
    }else{
        activity->viewList = new ViewList();
        activity->viewList->handle = handle;
    }
}

/** 设定控件回调 **/
void _simple_callback_set(ViewList *view, int actId, ActCall callback){
    if(view == NULL)
        return;
    CallbackList *tmp = view->callList;
    if(tmp){
        bool find = false;
        if(tmp->next) {
            while (tmp->next) {
                if(tmp->call_id == actId)   //对于已有的动作定义，直接修改而不新建节点
                    break;
                tmp = tmp->next;
            }
        }
        if(tmp->call_id == actId)   //对于仅1个记录时的兼容处理
            find = true;
        if(!find){      //没有动作定义，新建节点
            tmp->next = new CallbackList();
            tmp = tmp->next;    //前置
        }
    }else{  //一个定义都没有
        view->callList = new CallbackList();
        tmp = view->callList;
    }
    tmp->callback = callback;
    tmp->call_id = actId;
}

void _simple_callback_set(ActivityInfo *activity, int actId, ActCall callback){
    if(activity == NULL)
        return;
    CallbackList *tmp = activity->viewActList;
    if(tmp){
        bool find = false;
        if(tmp->next) {
            while (tmp->next) {
                if(tmp->call_id == actId)   //对于已有的动作定义，直接修改而不新建节点
                    break;
                tmp = tmp->next;
            }
        }
        if(tmp->call_id == actId)   //对于仅1个记录时的兼容处理
            find = true;
        if(!find){      //没有动作定义，新建节点
            tmp->next = new CallbackList();
            tmp = tmp->next;    //前置
        }
    }else{  //一个定义都没有
        activity->viewActList = new CallbackList();
        tmp = activity->viewActList;
    }
    tmp->callback = callback;
    tmp->call_id = actId;
}

/** 清除回调记录表 **/
void _simple_callback_clean(CallbackList* list){
    if(list->next)
        _simple_callback_clean(list->next);
    delete list;
}

/** 清除控件记录表 **/
void _simple_view_clean(ViewList* list){
    if(list->next)
        _simple_view_clean(list->next);
    if(list->callList)
        _simple_callback_clean(list->callList);
    delete list;
}

/** 删除活动记录表 **/
void _simple_activity_delete(void *activity){
    ActivityInfo *tmp = activityList, *nextNode;
    //debug:
    char tmpS[64] = {0};
    wchar_t *tmpS2;
    sprintf(tmpS, "Delete Activity: %p", activity);
    tmpS2 = AnsiToUnicode(tmpS);
    //MessageBoxW(0, tmpS2, L"SimpleUI Debug", MB_OK);
    delete []tmpS2;
    while(tmp){
        if(tmp->next){
            if(tmp->next->activity == activity){
                nextNode = tmp->next->next;
                if(tmp->next->viewList)
                    _simple_view_clean(tmp->next->viewList);    //删除活动信息前先清理控件和回调表
                /*if(tmp->next->windowActList)
                    _simple_callback_clean(tmp->next->windowActList);
                */
                if(tmp->viewActList)
                    _simple_callback_clean(tmp->viewActList);
                delete tmp->next;
                tmp->next = nextNode;
                break;
            }
        }else{
            if(tmp->activity == activity){
                if(tmp->viewList)
                    _simple_view_clean(tmp->viewList);
                /*if(tmp->windowActList)
                    _simple_callback_clean(tmp->windowActList);
                */
                if(tmp->viewActList)
                    _simple_callback_clean(tmp->viewActList);
                delete tmp;
                activityList = 0;
                break;
            }
        }
        tmp = tmp->next;
    }
}

#endif //FAKE_VM_SIMPLE_ACTIVITY_H
