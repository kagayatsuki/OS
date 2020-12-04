/********************************************
* :shinsya
********************************************/

#ifndef _CORE_TOOLBOX_PRIVATE_PRT_FM
#define _CORE_TOOLBOX_PRIVATE_PRT_FM
#include <stdio.h>

void _core_print_progress_bar_init(){
	printf("[                                                                                           ] 00.00%%");
}

void _core_print_progress_bar_update(double d){
	int i = 0;
	for(;i<109;i++)putchar('\b');
	i = 0;
	putchar('[');
	for(;i<(int)d;i++)putchar('=');
	i = 0;
	for(;i<100 - (int)d;i++)putchar(' ');
	putchar(']');
	putchar(' ');
	printf("%5.2f%%",d);
}
#endif
