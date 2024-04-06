#include <stdio.h>

int main(void){
	int i = 0x80000000;
	int SF = (i >> 31) & 1;
	i = (-SF) ^ i;
	printf("%x\n", i);
	return 0;


}
