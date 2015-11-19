#include "stdio.h"
#include <stdlib.h>
#include <string.h>

void clean_stdin(void) {
	int c;
	do {
		c = getchar();
	} while (c != '\n' && c != EOF);
}

void  appendChar(char*s, char c) {

	char* aux = malloc(2);
	//char aux[2];
	aux[0] = c;
	aux[1] = '\0';

	strcat(s, aux);

	free(aux);
}
