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
}CallbackList, SimpleCall;

typedef struct activity_info{   //记录窗体信息
    void* activity;     //窗口类指针
    CallbackList *windowActList;    //窗体事件回调
    CallbackList *viewActList;      //控件事件回调

    activity_info *next;
}ActivityInfo, SimpleActivity;

activity_info* activityList = 0;    //活动记录表

HINSTANCE simple_global_default_instance = 0;   //另指定的实例
void Simple_SethInstance(HINSTANCE instance){simple_global_default_instance = instance;}    //辅助函数，用于另指定实例

/** 增加新活动记录节点 **/
void _simple_activity_new(void *activity){
    ActivityInfo *tmp = new ActivityInfo();
    tmp->activity = activity;
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
SimpleActivity *_simple_activity_find(void *activity){
    ActivityInfo *tmp = activityList;
    while(tmp){
        if(tmp->activity == activity)
            return tmp;
        tmp = tmp->next;
    }
    return 0;
}

/** 获取回调信息 **/
SimpleCall *_simple_callback_find(SimpleActivity *activity, int callId, char winAct){
    if(activity == NULL)
        return 0;
    CallbackList *tmp;
    if(winAct)  //窗口还是控件回调
        tmp = activity->windowActList;
    else
        tmp = activity->viewActList;

    while(tmp){
        if(tmp->call_id == callId)
            return tmp;
        tmp = tmp->next;
    }
    return 0;
}

SimpleCall *_simple_callback_find(SimpleActivity *activity, int callId){
    return _simple_callback_find(activity, callId, 0);
}

/** 设定回调 **/
void _simple_callbackList_callback_set(SimpleActivity *activity, int actId, ActCall callback, char type){
    if(activity == NULL)
        return;
    CallbackList *tmp;
    if(type)
        tmp = activity->viewActList;
    else
        tmp = activity->windowActList;
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
        if(type){
            activity->viewActList = new CallbackList();
            tmp = activity->viewActList;
        }else{
            activity->windowActList = new CallbackList();
            tmp = activity->windowActList;
        }
    }
    tmp->callback = callback;
    tmp->call_id = actId;
}

/** 主窗体子控件的回调设定 **/
void _simple_callback_set(ActivityInfo *activity, int actId, ActCall callback){
    _simple_callbackList_callback_set(activity, actId, callback, 1);
}

/** 对主窗体事件的分类处理 **/
void _simple_activity_call_set(ActivityInfo *activity, int actId, ActCall callback){
    _simple_callbackList_callback_set(activity, actId, callback, 0);
}

/** 清除回调记录表 **/
void _simple_callback_clean(CallbackList* list){
    if(list->next)
        _simple_callback_clean(list->next);
    delete list;
}

/** 删除活动记录表 **/
void _simple_activity_delete(void *activity){
    ActivityInfo *tmp = activityList, *nextNode;
    while(tmp){
        if(tmp->next){
            if(tmp->next->activity == activity){
                nextNode = tmp->next->next;
                if(tmp->next->windowActList)            //删除活动信息前先清理回调表
                    _simple_callback_clean(tmp->next->windowActList);
                if(tmp->viewActList)
                    _simple_callback_clean(tmp->viewActList);
                delete tmp->next;
                tmp->next = nextNode;
                break;
            }
        }else{
            if(tmp->activity == activity){
                if(tmp->windowActList)
                    _simple_callback_clean(tmp->windowActList);
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
