//
// Created by shinsya on 2021/1/3.
//

#include "simple_window.h"
#include "simple_unicode.h"
#include "simple_activity.h"
#include "simple_button.h"
#include <windows.h>

HWND mWin;

void buttonTest(HWND hwnd){
    MessageBoxW(mWin, L"click", L"提示", MB_OK);
}

void testCall(HWND hwnd){
    MessageBoxW(mWin, L"exit", L"提示", MB_OK);
    PostQuitMessage(0);
}

int main(){
    char* wTitle = "test window";
    WCHAR *wTitle_t = AnsiToUnicode(wTitle);
    simple_window* mainWin = new simple_window();

    if((mWin = mainWin->create(WS_OVERLAPPEDWINDOW,wTitle_t,CW_USEDEFAULT, 0, 400, 280, 0, 0)) == 0)
        MessageBoxW(0, L"无法创建主窗口,请尝试以命令行启动程序", L"提示", MB_OK);
    else{
        _simple_callback_set(_simple_activity_find(mWin), (int)WM_DESTROY, testCall);   //绑定窗口关闭时的函数
        simple_button button1(mWin,3,buttonTest);
        button1.setText("test");
        button1.setSize(100, 50);
        button1.setPosition(50, 6);
        mainWin->show();
    }
    delete mainWin;
    delete[] wTitle_t;
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPWSTR    lpCmdLine,
        int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    Simple_SethInstance(hInstance);
    char* wTitle = "test window";
    WCHAR *wTitle_t = AnsiToUnicode(wTitle);
    simple_window* mainWin = new simple_window();
    HWND mWin;
    if((mWin = mainWin->create(WS_OVERLAPPEDWINDOW,wTitle_t,CW_USEDEFAULT, 0, 400, 280, 0, 0)) == 0)
        printf("Can't create window\n");
    else
        mainWin->show();
    delete mainWin;
    delete[] wTitle_t;
    return 0;
}
