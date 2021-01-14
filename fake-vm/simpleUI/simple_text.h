//
// Created by shinsya on 2021/1/14.
//

#ifndef FAKE_VM_SIMPLE_TEXT_H
#define FAKE_VM_SIMPLE_TEXT_H

#include "simple_window.h"
#include "simple_activity.h"
#include "simple_unicode.h"
#include "simple_style.h"
#include "simple_view.h"

#define CreateLabel(text, x, y, width, height, parent, id) CreateWindowExW(WS_EX_TRANSPARENT, L"STATIC", text, _text_style_default, x, y, width, height, parent, (HMENU)id, simple_global_default_instance?simple_global_default_instance:GetModuleHandle(0), NULL)

class simple_text : public simple_view{
public:
    simple_text(HWND parent);
    simple_text(HWND parent, const char *text);
    simple_text(HWND parent, const wchar_t *text);
    simple_text(HWND parent, int id);
    simple_text(HWND parent, const char *text, int id);
    simple_text(HWND parent, const wchar_t *text, int id);
    simple_text(HWND parent, int x, int y, int width, int height);
    simple_text(HWND parent, const wchar_t *text, int x, int y, int width, int height);
    simple_text(HWND parent, const char *text, int x, int y, int width, int height);
};

simple_text::simple_text(HWND parent) {
    this_parent = parent;
    this_id = _simple_default_id++;
    this_hwnd = CreateLabel(L"", 0, 0, _text_width_default, _text_height_default, parent, this_id);
    setFont(SimpleFont_Text);
}

simple_text::simple_text(HWND parent, int id) {
    this_parent = parent;
    this_id = id;
    this_hwnd = CreateLabel(L"", 0, 0, _text_width_default, _text_height_default, parent, this_id);
    setFont(SimpleFont_Text);
}

simple_text::simple_text(HWND parent, const char *text) {
    this_parent = parent;
    this_id = _simple_default_id++;
    this_hwnd = CreateLabel(L"", 0, 0, _text_width_default, _text_height_default, parent, this_id);
    setFont(SimpleFont_Text);
    setText(text);
}

simple_text::simple_text(HWND parent, const wchar_t *text) {
    this_parent = parent;
    this_id = _simple_default_id++;
    this_hwnd = CreateLabel(text, 0, 0, _text_width_default, _text_height_default, parent, this_id);
    setFont(SimpleFont_Text);
}

simple_text::simple_text(HWND parent, const char *text, int id) {
    this_parent = parent;
    this_id = id;
    this_hwnd = CreateLabel(L"", 0, 0, _text_width_default, _text_height_default, parent, this_id);
    setFont(SimpleFont_Text);
    setText(text);
}

simple_text::simple_text(HWND parent, const wchar_t *text, int id) {
    this_parent = parent;
    this_id = id;
    this_hwnd = CreateLabel(text, 0, 0, _text_width_default, _text_height_default, parent, this_id);
    setFont(SimpleFont_Text);
}

simple_text::simple_text(HWND parent, int x, int y, int width, int height) {
    this_parent = parent;
    this_id = _simple_default_id++;
    this_hwnd = CreateLabel(L"", x, y, width, height, parent, this_id);
    setFont(SimpleFont_Text);
}

simple_text::simple_text(HWND parent, const char *text, int x, int y, int width, int height) {
    this_parent = parent;
    this_id = _simple_default_id++;
    this_hwnd = CreateLabel(L"", x, y, width, height, parent, this_id);
    setFont(SimpleFont_Text);
    setText(text);
}

simple_text::simple_text(HWND parent, const wchar_t *text, int x, int y, int width, int height) {
    this_parent = parent;
    this_id = _simple_default_id++;
    this_hwnd = CreateLabel(text, x, y, width, height, parent, this_id);
    setFont(SimpleFont_Text);
}

#endif //FAKE_VM_SIMPLE_TEXT_H
