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

int msg__loadMensajesEnviados(struct ListasMensajes* lmsg){

   FILE *fichero;
	char emisor[IMS_MAX_NAME_SIZE];
   char receptor[IMS_MAX_NAME_SIZE];
   char msg[IMS_MAX_MSG_SIZE];
	int nMsg = 0;

	// Abrir el fichero
	fichero = fopen("Server/mensajes_enviados.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/mensajes_enviados.txt\"\n");
		return -1;
	}

	// Leer los usuarios hasta fin de fichero
	char c = fgetc(fichero);
	while (c != EOF) {
		// Read till space
		appendChar(lmsg->lista[nMsg].emisor, c);
      // leer a emisor
		c = fgetc(fichero);
		while (c != ' ') {
			appendChar(lmsg->lista[nMsg].emisor, c);
			c = fgetc(fichero);
		}
      // leer a receptor
      c = fgetc(fichero);
      while (c != ' ') {
			appendChar(lmsg->lista[nMsg].receptor, c);
			c = fgetc(fichero);
		}

      // leer a mensaje
      c = fgetc(fichero);
      while (c != '\n') {
			appendChar(lmsg->lista[nMsg].msg, c);
			c = fgetc(fichero);
		}
      //Concateno el '\n'
      appendChar(lmsg->lista[nMsg].msg, '\n');
		nMsg++;
		// Read next msg (or EOF)
		c = fgetc(fichero);
	}

	lmsg->size = nMsg;

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero \"Server/mensajes_enviados.txt\".\n");
		return -1;
	}

   return 0;
}

int msg__saveMensajesEnviados(struct ListasMensajes* lmsg){
   FILE *fichero;
	char line[IMS_MAX_NAME_SIZE];
	int nUsr = 0;

	// Abrir el fichero
	fichero = fopen("Server/mensajes_enviados.txt", "wt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/mensajes_enviados.txt\"\n");
		return -1;
	}

   // Escribir los datos
   int i;
   for (i = 0; i < lmsg->size; i++) {
   	fwrite(lmsg->lista[i].emisor, strlen(lmsg->lista[i].emisor), 1, fichero);
   	fputc(' ', fichero);
      fwrite(lmsg->lista[i].receptor, strlen(lmsg->lista[i].receptor), 1, fichero);
      fputc(' ', fichero);
      fwrite(lmsg->lista[i].msg, strlen(lmsg->lista[i].msg), 1, fichero);
   }

   // Cerrar el fichero
   if(fclose(fichero) != 0) {
   	printf("Error cerrando el fichero \"Server/mensajes_enviados.txt\".\n");
   	return -1;
   }

   return 0;
}

int msg_delUserMessage(struct ListasMensajes* lmsg, char* username){
   int j;
   // 1º Eliminar la carpeta del usuario.
   chdir("Server");

   chdir(username);
   remove("mensajes_pendientes.txt");
   chdir("..");

   if(rmdir(username) != 0){
      perror("ERROR: no se puede eliminar el directorio.\n");
      return -1;
   }

   // 2º Eliminar los mensajes_enviados por usuario
   int pos = msg_findMessage(lmsg,username);
   while(pos >= 0){
         for(j=pos; j < lmsg->size-1; j++)
            msg_copyMessage(&lmsg->lista[j],&lmsg->lista[j+1]);
         lmsg->size--;
      pos = msg_findMessage(lmsg,username);
   }

   chdir("..");
   return 0;
}

void msg_copyMessage(struct Mensaje* dst, struct Mensaje* src){
   strcpy(dst->emisor,src->emisor);
   strcpy(dst->receptor,src->receptor);
   strcpy(dst->msg,src->msg);
}

int msg_findMessage(struct ListasMensajes* lmsg,char* emisor){
   int i = 0, pos = -1;
   while(pos == -1 && i < lmsg->size){
      // Si encontramos la petición en cuestión.
		if (strcmp(emisor, lmsg->lista[i].emisor) == 0 ) {
				 pos = i;
		}
		else
			i++;
   }
   return pos;
}

int sendMessage (struct Message2 myMessage){
   FILE * fichero;

   chdir("Server");
	chdir(myMessage.receptor);

	fichero = fopen("mensajes_pendientes.txt", "a");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"mensajes_pendientes.txt.txt\"\n");
		return -1;
	}

	fwrite(myMessage.emisor, strlen(myMessage.emisor), 1, fichero);
	fputc(' ',fichero);
	fwrite(myMessage.msg, strlen(myMessage.msg), 1, fichero);

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
      printf("Error cerrando el fichero \"mensajes_pendientes.txt\".\n");
		return -1;
	}

	chdir("..");
   chdir("..");

   return 0;
}

int receiveMessage(char* username, struct ListaMensajes* result){

   int i=0;
   // Allocate space for the message field of the myMessage struct then copy it
	result->mensajes =  malloc( (IMS_MAX_NAME_SIZE+IMS_MAX_MSG_SIZE)*MAX_MENSAJES);
	FILE * fichero;
	char caracter;
   chdir("Server");
	chdir(username);
	fichero = fopen("mensajes_pendientes.txt", "rt");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"mensajes_pendientes.txt\"\n");
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
		printf("Error cerrando el fichero \"mensajes_pendientes.txt.txt\".\n");
		return -1;
	}
   /*BORRAR EL CONTENIDO del FICHERO*/
   fichero = fopen("mensajes_pendientes.txt", "wt");
   if(fclose(fichero) != 0) {
      printf("Error cerrando el fichero \"mensajes_pendientes.txt\".\n");
      return -1;
   }
   chdir("..");
   chdir("..");
   return 0 ;
}

int checkMessage(char* username,struct ListasMensajes* lmsg){

   FILE * fichero;
	char caracter;
	char emisor[256];
	char linea[256];
	int i;
   chdir("Server");
	chdir(username);
	fichero = fopen("mensajes_pendientes.txt", "rt");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"mensajes_pendientes.txt\"\n");
		return -1;
	}
	while ( fgets(linea,256,fichero) != NULL ){
		sscanf(linea,"%s ",emisor);
		for(i=0;i < lmsg->size;i++){
			if (strcmp( emisor, lmsg->lista[i].emisor) == 0 && strcmp( username,lmsg->lista[i].receptor) == 0 ) {
				if(lmsg->lista[i].msg[strlen(lmsg->lista[i].msg)-2] != '*' ){
					lmsg->lista[i].msg[strlen(lmsg->lista[i].msg)-1] = '*';
					lmsg->lista[i].msg[strlen(lmsg->lista[i].msg)] = '\n';
					lmsg->lista[i].msg[strlen(lmsg->lista[i].msg)+1] = '\0';
				}
			}
		}
	}

	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero \"mensajes_pendientes.txt\".\n");
		return -1;
	}
   chdir("..");
   chdir("..");
   return 0;
}

int consultEntrega(char* username,struct ListasMensajes* lmsg,struct ListaMensajes* result){
   int i;
	// Allocate space for the message field of the myMessage struct then copy it
	result->mensajes =  malloc( (IMS_MAX_NAME_SIZE+IMS_MAX_MSG_SIZE)*MAX_MENSAJES);
	result->mensajes[0]='\0';

	for(i=0;i < lmsg->size;i++){
		if (strcmp(username,lmsg->lista[i].emisor) == 0) {
			strcat(result->mensajes,lmsg->lista[i].receptor);
			result->mensajes[strlen(result->mensajes)] = ' ';
			result->mensajes[strlen(result->mensajes)+1] = '\0';
			strcat(result->mensajes,lmsg->lista[i].msg);
		}
	}
   return 0;
}
int sendCheck(struct ListasMensajes* lmsg,struct Message2* myMessage){
   //Meter el mensaje en la Estructora para el chequeo.
   strcpy(lmsg->lista[lmsg->size].emisor,myMessage->emisor);
   strcpy(lmsg->lista[lmsg->size].receptor,myMessage->receptor);
   strcpy(lmsg->lista[lmsg->size].msg,myMessage->msg);
   lmsg->size++;
   return 0;
}
