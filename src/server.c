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
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct RespuestaPeticionesAmistad* respuesta);

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
 */
int ims__getAllFriendRequests (struct soap* soap, char* username, struct RespuestaPeticionesAmistad* result) {

	perror("Antes del malloc");
	//struct RespuestaPeticionesAmistad respuesta;
	//respuesta.peticiones = malloc(IMS_MAX_NAME_SIZE*MAX_AMISTADES_PENDIENTES);

	// Primer malloc para la estructura
	result = malloc(sizeof(struct RespuestaPeticionesAmistad));
	result->peticiones = malloc(IMS_MAX_NAME_SIZE*MAX_AMISTADES_PENDIENTES);

	perror("Después del malloc");
	result->nPeticiones = 0;

	//perror("Debug 1");
	//printf("DEBUG: %s\n", respuesta.peticiones[0]);
	//perror("Debug 2");
	searchPendingFriendRequests(username, &ap, result);

	//result = &respuesta;
	return SOAP_OK;


	/*struct RespuestaPeticionesAmistad {
		int nPeticiones;
		char** peticiones;
	};*/

	/////////
	// Allocate space for the message field of the myMessage struct then copy it
	/*myMessage->msg = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
	strcpy (myMessage->msg, "Invoking the remote function receiveMessage simply retrieves this standard message from the server"); // always same msg

	// Allocate space for the name field of the myMessage struct then copy it
	myMessage->name = (xsd__string) malloc (IMS_MAX_NAME_SIZE);
	strcpy(myMessage->name, "aServer");*/
	/////////
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
 */
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct RespuestaPeticionesAmistad* respuesta) {

	perror("searchPendingFriendRequests()");

	int i;
	for (i = 0; i < ap->nPeticiones; i++) {
		perror("bucle...");
		// Si el usuario coincide, añadirlo a la respuesta
		if(strcmp(username, ap->amistades_pendientes[i].destinatario) == 0) {
			perror("Hay una con mi nombre.");
			printf("%s quiere ser mi amigo.\n", ap->amistades_pendientes[i].emisor);
			strcpy(respuesta->peticiones[respuesta->nPeticiones], ap->amistades_pendientes[i].emisor);
			respuesta->nPeticiones++;
		}
	}

}
