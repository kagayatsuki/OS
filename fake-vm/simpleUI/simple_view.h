//
// Created by shinsya on 2021/1/14.
//

#ifndef FAKE_VM_SIMPLE_VIEW_H
#define FAKE_VM_SIMPLE_VIEW_H

#include "simple_unicode.h"
#include "simple_window.h"
#include "simple_activity.h"

int _simple_default_id = 3000;

class simple_view{
private:
    wchar_t *buff_text;
protected:
    HWND this_parent;
    HWND this_hwnd;
    int this_id;
public:
    simple_view();
    ~simple_view();

    virtual void setText(const wchar_t *text);
    virtual void setText(const char *text);
    virtual void setSize(int width, int height);
    virtual void setPosition(int x, int y);
    virtual void setCallback(ActCall callback);
    virtual void setFont(HFONT font);
    virtual int getId(){return this_id;}
    virtual HWND getHwnd(){return this_hwnd;}
    virtual HWND getParent(){return this_parent;}
};

void simple_view::setFont(HFONT font) {
    SendMessageW(this_hwnd, WM_SETFONT, (WPARAM)font, 0);
}

void simple_view::setCallback(ActCall callback) {
    _simple_callback_set(_simple_activity_find(this_parent), this_id, callback);
}

simple_view::~simple_view() {
    if(buff_text)
        delete[] buff_text;
}

simple_view::simple_view() {
    buff_text = 0;
    this_parent = this_hwnd = 0;
    this_id = 0;
}

void simple_view::setPosition(int x, int y) {
    tagRECT rect;
    GetWindowRect(this_hwnd, &rect);
    MoveWindow(this_hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top, true);
}

void simple_view::setSize(int width, int height) {
    tagRECT rect;
    GetClientRect(this_hwnd, &rect);
    MoveWindow(this_hwnd, rect.left, rect.top, width, height, true);
}


void simple_view::setText(const char *text) {
    if(buff_text)
        delete[] buff_text;
    buff_text = AnsiToUnicode(text);
    SendMessageW(this_hwnd, WM_SETTEXT, 0, (LPARAM)buff_text);
}

void simple_view::setText(const wchar_t *text) {
    SendMessageW(this_hwnd, WM_SETTEXT, 0, (LPARAM)text);
}

#endif //FAKE_VM_SIMPLE_VIEW_H
