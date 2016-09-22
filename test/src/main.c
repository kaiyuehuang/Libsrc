#include <stdio.h>

#include "../../output/aes_demo.h"
int main(void)
{
	char msg[100];
	char aecout[200];
	aes_de_char((const char *)msg,aecout,100);
	return 0;
}
