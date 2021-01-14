//
// Created by shinsya on 2021/1/3.
//

#ifndef FAKE_VM_SIMPLE_WINDOW_H
#define FAKE_VM_SIMPLE_WINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <wchar.h>
#include "simple_unicode.h"
#include "simple_activity.h"

typedef LRESULT(*LRESULT_W)(HWND__*,UINT,WPARAM,LPARAM);
//class simple_window;

int wCount = 0;     //已创建的窗口数量

LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);   //大回调


class simple_window{
public:
    simple_window();
    ~simple_window();
    HWND create(DWORD exStyle, const WCHAR* wTitle, int x, int y, int width, int height, HWND wParent, HMENU wMenu);
    void show();
    void hide();
    void destroy();
    HWND hwnd(){return this_hwnd;};

    void setCallback(int actId, ActCall callback);
    void setTitle(const char *text);
    void setTitle(const wchar_t *text);
    void setSize(int width, int height);
    void setPosition(int x, int y);
protected:
    HWND this_hwnd;
private:

    WCHAR *wClassName;
    WNDCLASSEXW wndclass;
    HINSTANCE hInstance;
    bool cRegister;
    char cNameA[32];

    wchar_t *buff_title;
};

void simple_window::setSize(int width, int height) {
    tagRECT rect;
    GetClientRect(this_hwnd, &rect);
    MoveWindow(this_hwnd, rect.left, rect.top, width, height, true);
}

void simple_window::setPosition(int x, int y) {
    tagRECT rect;
    GetWindowRect(this_hwnd, &rect);
    MoveWindow(this_hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top, true);
}

void simple_window::setTitle(const char *text) {
    if(buff_title)
        delete[] buff_title;
    buff_title = AnsiToUnicode(text);
    SendMessageW(this_hwnd, WM_SETTEXT, 0, (LPARAM)buff_title);
}

void simple_window::setTitle(const wchar_t *text) {
    SendMessageW(this_hwnd, WM_SETTEXT, 0, (LPARAM)text);
}

void simple_window::setCallback(int actId, ActCall callback) {
    if(this_hwnd){
        _simple_activity_call_set(_simple_activity_find(this), actId, callback);
    }
}

simple_window::simple_window(){
    memset(cNameA, 0, 32);
    this_hwnd = 0;
    buff_title = 0;

    sprintf(cNameA, "simpleW%d", wCount++);
    wClassName = AnsiToUnicode(cNameA);
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    if(simple_global_default_instance)
        wndclass.hInstance = hInstance = simple_global_default_instance;
    else
        wndclass.hInstance = hInstance = GetModuleHandle(0);
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hIcon = NULL;
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = wClassName;
    cRegister = false;

    _simple_activity_new(this);
}

void simple_window::show() {
    MSG msg;
    if(this_hwnd){
        ShowWindow(this_hwnd, SW_SHOW);
        UpdateWindow(this_hwnd);
        while(GetMessage(&msg, NULL, 0, 0)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

HWND simple_window::create(DWORD wStyle, const WCHAR* wTitle, int x, int y, int width, int height, HWND wParent, HMENU wMenu){
    if(this_hwnd == 0){
        if(!cRegister){
            RegisterClassExW(&wndclass);
            cRegister = true;
        }
        this_hwnd = CreateWindowW(wClassName, wTitle, wStyle, x, y, width, height, wParent, wMenu, hInstance, NULL);
        return this_hwnd;
    }
    return 0;
}


void simple_window::destroy() {
    if(this_hwnd)
        DestroyWindow(this_hwnd);
}

simple_window::~simple_window() {
    if(wClassName)
        delete []wClassName;
    _simple_activity_delete(this);
    destroy();
}

/** 获取活动信息重调 **/
SimpleActivity *_simple_activity_find(HWND activity){
    ActivityInfo *tmp = activityList;
    simple_window* tmp2;
    while(tmp){
        tmp2 = (simple_window*)(tmp->activity);
        if(tmp2){
            if(tmp2->hwnd() == activity)
                return tmp;
        }
        tmp = tmp->next;
    }
    return 0;
}


LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    SimpleActivity *tActivity = _simple_activity_find(hWnd);
    SimpleCall *tCall = 0;

    static HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));    //默认白色控件背景： 对于Static 等

    tCall = _simple_callback_find(tActivity, (int)message, 1);
    switch(message)
    {
        case WM_COMMAND:    //此处接收窗口发生的事件，包括组件事件
        {
            int wmId = LOWORD(wParam);
            tCall = _simple_callback_find(tActivity, wmId);
            if(tCall){
                if (tCall->callback)
                    tCall->callback(GetDlgItem(hWnd, wmId));    //回调传参为控件句柄
            }
            break;
        }
        case WM_PAINT:
        {
            if(tCall){          //如果调用者自己定义了绘制函数，则使用指定的函数
                if(tCall->callback){
                    tCall->callback(hWnd);
                }
            }else{          //否则用默认绘制方法
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);;
                EndPaint(hWnd,&ps);
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
            return (LRESULT)hBrush;
        case WM_DESTROY:
            if(tCall){          //调用者定义窗口将关闭时的过程函数
                if(tCall->callback){
                    tCall->callback(hWnd);
                    return 0;
                }
            }else{          //默认过程
                PostQuitMessage(0);
            }
            break;
        default:
            if(tCall){      //其它窗口事件
                if(tCall->callback){
                    tCall->callback(hWnd);
                    return 0;
                }
            }
            break;
    }
    return DefWindowProcW(hWnd,message,wParam,lParam);
}

#endif //FAKE_VM_SIMPLE_WINDOW_H
