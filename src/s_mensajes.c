#include "stdio.h"
#include "ims.h"
#include "s_mensajes.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
void s_mensajes()
{
    printf("Se ha ejecutado la función s_mensajes!");
}

int sendMessage (struct Message2 myMessage){
   FILE * fichero;
   //chdir("Server");
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
   //chdir("..");
   return 0;
}
int receiveMessage(char* username, struct ListaMensajes* result){

   int i=0;
   // Allocate space for the message field of the myMessage struct then copy it
	result->mensajes =  malloc( (IMS_MAX_NAME_SIZE+IMS_MAX_MSG_SIZE)*MAX_MENSAJES);
	FILE * fichero;
	char caracter;
   //chdir("Server");
	chdir(username);
	fichero = fopen("mensajes_pendientes.txt", "rt");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	}
	caracter = fgetc(fichero);
	while (feof(fichero) == 0){
		result->mensajes[i] = caracter;
		caracter = fgetc(fichero);
		i++;
	}
	result->mensajes[i]='\0';

	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}
   /*BORRAR EL CONTENIDO del FICHERO*/
   fichero = fopen("mensajes_pendientes.txt", "wt");
   if(fclose(fichero) != 0) {
      printf("Error cerrando el fichero.\n");
      return -1;
   }
   chdir("..");
   //chdir("..");
   return 0 ;
}
