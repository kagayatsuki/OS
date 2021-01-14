//
// Created by shinsya on 2021/1/3.
//

#include "simple_window.h"
#include "simple_unicode.h"
#include "simple_activity.h"
#include "simple_button.h"
#include "simple_style.h"
#include "simple_text.h"
#include <windows.h>

HWND mWin;

void buttonTest(HWND hwnd){
    MessageBoxW(mWin, L"Button was clicked", L"Notice", MB_OK);
}

void testCall(HWND hwnd){
    if(MessageBoxW(mWin, L"Would you like to exit?", L"Notice", MB_OKCANCEL) == IDOK)
        PostQuitMessage(0);
}

int main(){
    WCHAR *wTitle_t = L"test window";
    simple_window* mainWin = new simple_window();

    if((mWin = mainWin->create(_window_style_default,wTitle_t,CW_USEDEFAULT, 0, 400, 280, 0, 0)) == 0)
        MessageBoxW(0, L"Can not create main window, Please try to launch by cmd", L"Notice", MB_OK);
    else{
        mainWin->setCallback((int)WM_CLOSE, testCall);   //绑定窗口关闭时的函数
        simple_button button1(mWin,3);
        simple_text label1(mWin, "Test Text", 10, 60, 100, 24);
        button1.setText("Test text");
        button1.setSize(100, 28);
        button1.setPosition(20, 16);
        button1.setCallback(buttonTest);
        mainWin->setTitle("Test Window");
        mainWin->show();
    }
    delete mainWin;
    return 0;
}



