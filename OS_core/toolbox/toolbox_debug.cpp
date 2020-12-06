#include "private/print_format.h"
#include <windows.h>

int main(){
    _core_print_progress_bar_init();
    for(int i = 1; i <= 100; i++){
        _core_print_progress_bar_update((int)i);
        Sleep(60);
    }
    putchar('\n');
    getchar();
    return 0;
}
