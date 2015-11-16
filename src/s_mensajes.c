#include "stdio.h"
#include "ims.h"
#include "s_mensajes.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
void s_mensajes()
{
    printf("Se ha ejecutado la función s_mensajes!");
}

int sendMessage (struct Message2 myMessage){
   FILE * fichero;
	chdir(myMessage.receptor);
   
	fichero = fopen("mensajes_pendientes.txt", "a");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	} else
		printf("Fichero abierto correctamente.\n");

	fwrite(myMessage.emisor, strlen(myMessage.emisor), 1, fichero);
	fputc(' ',fichero);
	fwrite(myMessage.msg, strlen(myMessage.msg), 1, fichero);
	fputc('\n',fichero);

	printf("Escribimos en el fichero.\n");

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}
	printf("Fichero cerrado.\n");
	chdir("..");
   return 0;
}
