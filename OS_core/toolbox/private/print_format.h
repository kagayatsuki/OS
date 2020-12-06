/********************************************
* :shinsya
********************************************/

#ifndef _CORE_TOOLBOX_PRIVATE_PRT_FM
#define _CORE_TOOLBOX_PRIVATE_PRT_FM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _core_print_progress_bar_init(){
	printf("[                                                                                           ] 00.00%%");
	fflush(stdout);
}

void _core_print_progress_bar_update(double d){
	int i = 0, c = 0;
	char tmp_b[120] = {0};
	tmp_b[109] = '\0';
	memset(tmp_b, '\b', 109);
	printf(tmp_b);
	fflush(stdout);
	tmp_b[0] = '[';
	for(;i<(int)d;i++)tmp_b[i+1] = '=';
	for(; c < 100 - (int)d;c++)tmp_b[i+1+c]=' ';
	tmp_b[i+1+c+1] = ']';
	tmp_b[i+1+c+2] = ' ';
	tmp_b[i+1+c+3] = '\0';
	printf(tmp_b);
	printf("%5.2f%%",d);
	fflush(stdout);
}
#endif
