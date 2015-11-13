// Alberto Miedes Garcés y Denys Sypko

#include "soapH.h"
#include "imsService.nsmap"
#include "stdio.h"
#include "externo.h"
#include "s_usuarios.h"
#include "s_mensajes.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras propias del servidor
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1

struct datos_usuarios db;	// en mem. estática (todo)
struct amistades_pendientes ap;

struct peticion_amistad {
	char emisor[IMS_MAX_NAME_SIZE];
	char destinatario[IMS_MAX_NAME_SIZE];
};

struct amistades_pendientes {
	int nPeticiones;
	struct peticion_amistad amistades_pendientes[MAX_AMISTADES_PENDIENTES];
};

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
int addFriendRequest(struct amistades_pendientes* ap, char* emisor, char* destinatario);
void delFriendRequest(struct amistades_pendientes* ap, struct peticion_amistad* old);
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaAmigos *lista);

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char **argv){

	s_usuarios();
	s_mensajes();

	int m, s;				// sockets
	struct soap soap;

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
			char name[IMS_MAX_NAME_SIZE];
			scanf("%255s", name);
			name[strlen(name)] = '\0';
			clean_stdin();

			if(addUser(&db, name) == -1)
				printf("Error añadiendo a %s\n", name);
		}
		else if (opcion == '3') {
			printf("Nombre de usuario:");
			char name2[IMS_MAX_NAME_SIZE];
			scanf("%255s", name2);
			name2[strlen(name2)] = '\0';
			clean_stdin();

			if(deleteUser(&db, name2) < 0)
				printf("Error añadiendo a %s\n", name2);
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

int ims__sendMessage (struct soap *soap, struct Message2 myMessage, int *result) {
	printf("Received by server:\n");
	printf("\temisor: %s\n", myMessage.emisor);
	printf("\treceptor: %s\n", myMessage.receptor);
	printf("\ttexto: %s\n", myMessage.msg);
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

int ims__darAlta (struct soap *soap, char* username, int *result) {

	if (DEBUG_MODE) printf("Recibido nombre de usuario: %s\n", username);
	*result = addUser(&db, username); //-1 err, -2 no existe

	if (*result >= 0)
		saveUsersData(&db);

	return SOAP_OK;
}

int ims__darBaja(struct soap *soap, char* username, int *result){

	if (DEBUG_MODE) printf("Recibido nombre de usuario: %s\n", username);
	*result= deleteUser(&db,username);


	if (*result >= 0)
		saveUsersData(&db);

	return SOAP_OK;
}

/**
 * Marca la sesión de un usuario como iniciada ('connected = 1').
 * @soap Contexto gSOAP
 * @param username Nombre del usuario que hace login.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__login (struct soap *soap, char* username, int *result) {

	int existe = 0, i = 0;
	if (DEBUG_MODE) printf("Recibido nombre de usuario: %s\n", username);

	// Buscar si existe un usuario con el mismo nombre
	while (existe == 0 && i < db.nUsers) {
		if (strcmp(db.usuarios[i].username, username) == 0) {
			existe = 1;
			db.usuarios[i].connected = 1;
		}
		i++;
	}

	if(existe == 0)
		*result = -1;

	if(DEBUG_MODE) printUsersData(&db);

	return SOAP_OK;
}

/**
 * Marca la sesión de un usuario como apagada ('connected = 0').
 * @soap Contexto gSOAP
 * @param username Nombre del usuario que hace logout.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__logout (struct soap *soap, char* username, int *result) {

	int existe = 0, i = 0;
	if (DEBUG_MODE) printf("Recibido nombre de usuario: %s\n", username);

	// Buscar si existe un usuario con el mismo nombre
	while (existe == 0 && i < db.nUsers) {
		if (strcmp(db.usuarios[i].username, username) == 0) {
			existe = 1;
			db.usuarios[i].connected = 0;
		}
		i++;
	}

	if(existe == 0)
		*result = -1;

	if(DEBUG_MODE) printUsersData(&db);

	return SOAP_OK;
}

/**
 * Servicio gSOAP para enviar peticiones de amistad.
 */
int ims__sendFriendRequest (struct soap *soap, struct PeticionAmistad p, int *result) {
	printf("Received by server:\n");
	printf("\temisor pet.amistad: %s\n", p.emisor);
	printf("\treceptor pet.amistad: %s\n", p.receptor);

	if(addFriendRequest(&ap, p.emisor, p.receptor) == 0)
		printf("Petición añadida con éxito.\n");
	else
		printf("Error al añadir la petición.\n");

	return SOAP_OK;
}

/**
 * Servicio gSOAP para pedir las peticiones de amistad pendientes.
 * @param soap Contexto gSOAP.
 * @param username Nombre de usuario que invoca la llamada.
 * @param lista Estructura donde se devuelve la lista de peticiones pendientes.
 */
int ims__getAllFriendRequests (struct soap* soap, char* username, struct ListaAmigos *lista) {

	// Variable para la respuesta
	//lista = malloc(sizeof(struct RespuestaPeticionesAmistad)); CUIDADO!!
	lista->nPeticiones = 0;
	lista->nombres = (xsd__string) malloc(MAX_AMISTADES_PENDIENTES*IMS_MAX_NAME_SIZE + 1);

	// Rellenar la estructura
	searchPendingFriendRequests(username, &ap, lista);

	// Mostrar la estructura
	printf("Contenido de la estructura:\n");
	printf("---------------------------\n");
	printf("result->nPeticiones = %d\n", lista->nPeticiones);
	printf("Nombres: %s\n", lista->nombres);

	return SOAP_OK;
}

/**
 * Añade una petición de amistad al array de peticiones pendientes en el servidor.
 * @param ap Estructura que almacena las peticiones
 * @param emisor Emisor de la petición de amistad
 * @param destinatario Destinatario de la petición de amistad
 * @return 0 si éxito, -1 si la lista está llena
 */
int addFriendRequest(struct amistades_pendientes* ap, char* emisor, char* destinatario) {

	if (ap->nPeticiones >= MAX_AMISTADES_PENDIENTES)
		return -1;

	/* FALTARÍA CONTROLAR QUE EXISTE EL USUARIO AL QUE ENVIAMOS LA PETICIÓN */

	// Añadir al array
	strcpy(ap->amistades_pendientes[ap->nPeticiones].emisor, emisor);
	strcpy(ap->amistades_pendientes[ap->nPeticiones].destinatario, destinatario);

	printf("peticion[%d].emisor = %s\n", ap->nPeticiones, ap->amistades_pendientes[ap->nPeticiones].emisor);
	printf("peticion[%d].destinatario = %s\n", ap->nPeticiones, ap->amistades_pendientes[ap->nPeticiones].destinatario);

	ap->nPeticiones++;

	return 0;
}

/**
 * Borra una petición de amistad de la estructura del servidor.
 */
void delFriendRequest(struct amistades_pendientes* ap, struct peticion_amistad* old) {
	// Se invocará desde el ser. gsoap
	// borra una petición del array
}

/**
 * Busca las peticiones de un usuario y las mete en la estructura.
 * @param username Nombre del usuario que solicita sus peticiones pendientes.
 * @param ap Puntero a la estructura del servidor con todas las peticiones.
 * @param lista Puntero a la estructura que devolverá la llamada gSOAP.
 */
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaAmigos *lista) {

	int i;
	for (i = 0; i < ap->nPeticiones; i++) {
		// Si el usuario coincide, añadirlo a la respuesta
		if(strcmp(username, ap->amistades_pendientes[i].destinatario) == 0) {

			strcat(lista->nombres, ap->amistades_pendientes[i].emisor);	// Añadir a la lista

			if (i < ap->nPeticiones -1)
				strcat(lista->nombres, " \0"); // Add space

			lista->nPeticiones++;
		}
	}
}
