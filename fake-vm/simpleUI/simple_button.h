//
// Created by shinsya on 2021/1/8.
//

#ifndef FAKE_VM_SIMPLE_BUTTON_H
#define FAKE_VM_SIMPLE_BUTTON_H

#include <windows.h>
#include "simple_unicode.h"
#include "simple_activity.h"
#include "simple_window.h"
#include "simple_style.h"

int default_id = 3000;


class simple_button{
public:
    /** inits **/
    simple_button(HWND parent);
    simple_button(HWND parent, ActCall callback);
    simple_button(HWND parent, int id);
    simple_button(HWND parent, int id, ActCall callback);
    simple_button(HWND parent, const wchar_t *text);
    simple_button(HWND parent, const wchar_t *text, ActCall callback);
    simple_button(HWND parent, const wchar_t *text, int id);
    simple_button(HWND parent, const wchar_t *text, int id, ActCall callback);

    /** funcs **/
    void setText(const wchar_t *text);
    void setText(const char *text);
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setDefaultFont();
    void setFont(HFONT font);

    void setCallback(ActCall callback);
protected:
    HWND this_parent;
    HWND this_hwnd;
    int this_id;
};

void simple_button::setCallback(ActCall callback) {
    _simple_callback_set(_simple_activity_find(this_parent), this_id, callback);
}

void simple_button::setFont(HFONT font) {
    SendMessageW(this_hwnd, WM_SETFONT, (WPARAM)font, 0);
}

void simple_button::setPosition(int x, int y) {
    tagRECT rect;
    GetWindowRect(this_hwnd, &rect);
    MoveWindow(this_hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top, true);
}

void simple_button::setSize(int width, int height) {
    tagRECT rect;
    GetClientRect(this_hwnd, &rect);
    MoveWindow(this_hwnd, rect.left, rect.top, width, height, true);
}

void simple_button::setDefaultFont() {
    HFONT t_font = CreateFont(18, 0, 0, 0, 10, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FW_NORMAL, "微软雅黑");
    SendMessageW(this_hwnd, WM_SETFONT, (WPARAM)t_font, 0);
}

void simple_button::setText(const char *text) {
    wchar_t *tmp = AnsiToUnicode(text);
    SendMessageW(this_hwnd, WM_SETTEXT, 0, (LPARAM)tmp);
    delete[] tmp;
}

void simple_button::setText(const wchar_t *text) {
    SendMessageW(this_hwnd, WM_SETTEXT, 0, (LPARAM)text);
}

simple_button::simple_button(HWND parent) {
    this_id = default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, int id) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, ActCall callback) {
    this_id = default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, const wchar_t *text) {
    this_id = default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, int id, ActCall callback) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, const wchar_t *text, int id) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, const wchar_t *text, ActCall callback) {
    this_id = default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setDefaultFont();
}

simple_button::simple_button(HWND parent, const wchar_t *text, int id, ActCall callback) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setDefaultFont();
}

#endif //FAKE_VM_SIMPLE_BUTTON_H
