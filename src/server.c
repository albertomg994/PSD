// Alberto Miedes Garcés y Denys Sypko

#include "soapH.h"
#include "imsService.nsmap"
#include "stdio.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras propias del servidor
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1
#define MAX_USERS 100

struct reg_usuario {
	char username[IMS_MAX_USR_SIZE];
	int connected;
};

struct datos_usuarios {
	int nUsers;
	struct reg_usuario usuarios[MAX_USERS];
};

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
int loadUsersData(struct datos_usuarios * t);
int saveUsersData(struct datos_usuarios * t);
int printUsersData(struct datos_usuarios * t);
int addUser(struct datos_usuarios * t, xsd__string username);
int deleteUser(struct datos_usuarios * t, xsd__string username);

void clean_stdin(void) {
	int c;
	do {
		c = getchar();
	} while (c != '\n' && c != EOF);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char **argv){

	int m, s;				// sockets
	struct soap soap;
	struct datos_usuarios db;	// en mem. estática (todo)

	if (argc < 2) {
		printf("Usage: %s <port>\n",argv[0]);
		exit(-1);
	}

	// Init environment
	soap_init(&soap);

	// Cargamos la información de usuarios
	if (loadUsersData(&db) == -1) exit(-1);

	// Bind to the specified port. Devuelve el socket primario del servidor.
	m = soap_bind(&soap, NULL, atoi(argv[1]), 100);

	// Check result of binding
	if (m < 0) {
  		soap_print_fault(&soap, stderr);
		exit(-1);
	}

	char opcion = -1;
	while (opcion != '4') {
		printf("\n\ngSOAP server menu\n");
		printf("=================\n");
		printf("1.- Mostrar datos de usuarios\n");
		printf("2.- Dar de alta un usuario\n");
		printf("3.- Dar de baja un usuario\n");
		printf("4.- Ponerse a la escucha (se perderá el control)\n");
		printf("5.- Salir\n");

		opcion = getchar();
		clean_stdin();

		if (opcion == '1') {
			printUsersData(&db);
		}
		else if (opcion == '2') {
			printf("Nombre de usuario:");
			char name[IMS_MAX_USR_SIZE];
			scanf("%s", name);
			name[strlen(name)] = '\0';
			clean_stdin();

			if(addUser(&db, name) == -1)
				printf("Error añadiendo a %s\n", name);
		}
		else if (opcion == '3') {
			printf("Todavía no implementado...\n");
			/*printf("Nombre de usuario:");
			char* name;
			scanf("%s", name);
			fflush(stdin);
			if(addUser(&db, name) == -1)
				printf("Error eliminando a %s\n", name);*/
		}
		else if (opcion == '5') {
			saveUsersData(&db);
			exit(0);
		}
	}

	// Listen to next connection
	while (1) {

		// accept
		s = soap_accept(&soap);

	  if (s < 0) {
			soap_print_fault(&soap, stderr);
			exit(-1);
	  }

		// Execute invoked operation
	  soap_serve(&soap);

		// Clean up!
		soap_end(&soap);
	}

	// Escribimos en el fichero los cambios
	saveUsersData(&db);

	return 0;
}

int ims__sendMessage (struct soap *soap, struct Message myMessage, int *result){

	printf ("Received by server: \n\tusername:%s \n\tmsg:%s\n", myMessage.name, myMessage.msg);
	return SOAP_OK;
}

int ims__receiveMessage (struct soap *soap, struct Message *myMessage){

	// Allocate space for the message field of the myMessage struct then copy it
	myMessage->msg = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMessage->msg, 0, IMS_MAX_MSG_SIZE);
	strcpy (myMessage->msg, "Invoking the remote function receiveMessage simply retrieves this standard message from the server"); // always same msg

	// Allocate space for the name field of the myMessage struct then copy it
	myMessage->name = (xsd__string) malloc (IMS_MAX_NAME_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMessage->name, 0, IMS_MAX_NAME_SIZE);
	strcpy(myMessage->name, "aServer");

	return SOAP_OK;
}

int ims__darAlta (struct soap *soap, struct MensajeAlta msg, int *result) {

	printf("Recibido nombre de usuario: %s\n", msg.username);

	return SOAP_OK;
}

/**
 * Carga los datos de los usuarios desde un fichero.
 * @param t Estructura de datos donde cargar la información.
 * @return 0 si éxito, -1 si error
 */
int loadUsersData(struct datos_usuarios * t) {

	if (DEBUG_MODE) printf("loadUsersData()\n");
	FILE *fichero;
	char line[IMS_MAX_USR_SIZE];
	int nUsr = 0;

	// Abrir el fichero
	fichero = fopen("usuarios.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	} else
		printf("Fichero abierto correctamente.\n");

	// Leer los usuarios hasta fin de fichero
	while (fgets(line, IMS_MAX_USR_SIZE, fichero) != NULL) {
		//printf("Se ha leido %s\n", line);
		line[strlen(line)-1] = '\0'; // quitamos el '\n' del fichero
		strncpy((t->usuarios[nUsr]).username, line, IMS_MAX_USR_SIZE);
		(t->usuarios[nUsr]).connected = 0;
		//printf("(t->usuarios[%d]).username -> %s\n", nUsr, (t->usuarios[nUsr]).username);
		nUsr++;
	}
	t->nUsers = nUsr;
	printUsersData(t);

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/**
 * Persiste en un fichero los datos de los usuarios (actualmente en memoria)
 * dinámica.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int saveUsersData(struct datos_usuarios * t) {

	FILE* fichero;

	// Abrir el fichero (sobrescribe)
	fichero = fopen("usuarios.txt", "wt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	} else
		printf("Fichero abierto correctamente.\n");

	// Escribir los datos
	int i;
	for (i = 0; i < t->nUsers; i++) {
		fwrite(t->usuarios[i].username, strlen(t->usuarios[i].username), 1, fichero);
		fputc('\n',fichero);
		printf("Escribimos en el fichero --> %s\n", t->usuarios[i].username);
	}

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/*
 * Imprime el contenido de la estructura que almacena los datos de los usuarios.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int printUsersData(struct datos_usuarios * t) {

	printf("==============================\n");
	printf("Hay %d usuarios.\n", t->nUsers);
	printf("------------------------------\n");
	int i;
	for (i = 0; i < t->nUsers; i++) {
		printf("Usuario %d: %s\n", i, t->usuarios[i].username);
	}
	printf("==============================\n");
	return 0;
}

/**
 * Añade un usuario al sistema IMS.
 * @param username Nombre del usuario a dar de alta.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error, -2 si el usuario ya existe.
 */
int addUser(struct datos_usuarios * t, xsd__string username) {

	int existe = 0, i = 0;

	// Buscar si existe un usuario con el mismo nombre
	while (existe == 0 && i < t->nUsers) {
		if (strcmp(t->usuarios[i].username, username) == 0)
			existe = 1;
		i++;
	}
	if (existe == 1) return -1;

	// Copiar el nuevo usuario en la estructura
	strcpy(t->usuarios[i].username, username);
	t->nUsers++;

	return 0;
}

/**
 * Elimina un usuario del sistema IMS.
 * @param username Nombre del usuario a dar de baja.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error, -2 si el usuario no existía.
 */
int deleteUser(struct datos_usuarios * t, xsd__string username) {

	int existe = 0, i = 0;

	// Buscar si existe un usuario con el mismo nombre
	while (existe == 0 && i < t->nUsers) {
		if (strcmp(t->usuarios[i].username, username) == 0)
			existe = 1;
		else
			i++;
	}

	// Si no existía, salimos
	if (existe == 0) return -2;

	// Eliminar el usuario de la estructura (está en el i) (i.e. desplazar el resto)
	t->nUsers--;
	for (i; i < t->nUsers; i++) {
		strcpy(t->usuarios[i].username, t->usuarios[i+1].username); // destino, origen
		t->usuarios[i].connected = t->usuarios[i+1].connected;
	}

	return 0;
}
