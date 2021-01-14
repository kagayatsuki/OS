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
#include "simple_view.h"


class simple_button : public simple_view{
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
};

simple_button::simple_button(HWND parent) {
    this_id = _simple_default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)_simple_default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, int id) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, ActCall callback) {
    this_id = _simple_default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)_simple_default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, const wchar_t *text) {
    this_id = _simple_default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)_simple_default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, int id, ActCall callback) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", L"", _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, const wchar_t *text, int id) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, const wchar_t *text, ActCall callback) {
    this_id = _simple_default_id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)_simple_default_id++, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setFont(SimpleFont_Button);
}

simple_button::simple_button(HWND parent, const wchar_t *text, int id, ActCall callback) {
    this_id = id;
    this_parent = parent;
    this_hwnd = CreateWindowExW(WS_EX_TRANSPARENT, L"BUTTON", text, _button_style_default, 0, 0, 75, 24, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL);
    _simple_callback_set(_simple_activity_find(parent), this_id, callback);
    setFont(SimpleFont_Button);
}

#endif //FAKE_VM_SIMPLE_BUTTON_H
