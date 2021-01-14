//
// Created by shinsya on 2021/1/8.
//

#ifndef FAKE_VM_SIMPLE_STYLE_H
#define FAKE_VM_SIMPLE_STYLE_H

#define _window_style_default WS_OVERLAPPEDWINDOW
#define _button_style_default WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_FLAT
#define _text_style_default WS_CHILD | WS_VISIBLE | SS_LEFT

#define _text_width_default 60
#define _text_height_default 24

#define SimpleFont_Button CreateFont(18, 0, 0, 0, 10, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FW_NORMAL, "Microsoft YaHei")
#define SimpleFont_Text CreateFont(18, 0, 0, 0, 10, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FW_NORMAL, "Microsoft YaHei")

#endif //FAKE_VM_SIMPLE_STYLE_H
