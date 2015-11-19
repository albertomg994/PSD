// Alberto Miedes Garcés y Denys Sypko
#include "soapH.h"
#include "imsService.nsmap"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "externo.h"
#include "s_usuarios.h"
#include "s_mensajes.h"
#include "s_amigos.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y variables globales
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1
#define DEBUG_SIGINT 0

struct datos_usuarios db;	// en mem. estática (todo)
struct amistades_pendientes ap;
struct listas_amigos la;

/* Flag para activar el guardado. */
volatile sig_atomic_t save_data = 0;
// -----------------------------------------------------------------------------
// Estructuras propias del servidor
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char **argv){

	int m, s;				// sockets
	struct soap soap;
	sigset_t grupo;		// grupo para enmascarar SIGINT

	if (argc < 2) {
		printf("Usage: %s <port>\n",argv[0]);
		exit(-1);
	}

	// Init environment
	soap_init(&soap);

	// Cargamos la información de usuarios
	if (loadUsersData(&db) == -1) exit(-1);

	// Cargamos la información de los amigos
	if (loadFriendsData(&la) == -1) exit(-1);

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
			saveFriendsData(&la);
			exit(0);
		}
	}

	// Listen to next connection
	while (1) {

		// accept
		s = soap_accept(&soap);

		// Enmascarar SIGINT
		if(DEBUG_SIGINT) printf("Enmascaro SIGINT...\n");
		sigemptyset(&grupo);
		sigaddset(&grupo, SIGINT);
		sigprocmask(SIG_BLOCK, &grupo, NULL);

		if (s < 0) {
			soap_print_fault(&soap, stderr);
			exit(-1);
		}

		// Execute invoked operation
		soap_serve(&soap);

		// Clean up!
		soap_end(&soap);

		// Guardar los posibles cambios en fichero.
		saveUsersData(&db);
		saveFriendsData(&la);

		// Depurar la captura de CTRL+C
		if(DEBUG_SIGINT) {
			sigset_t pendientes;
			sigpending(&pendientes);
			if (sigismember(&pendientes, SIGINT))
				printf("SIGINT está en pendientes...\n");
			printf("Desenmascaro SIGINT\n");
		}

		// Desenmascarar SIGINIT
		sigprocmask(SIG_UNBLOCK, &grupo, NULL);
	}

	return 0;
}

int ims__sendMessage (struct soap *soap, struct Message2 myMessage, int *result) {
	printf("Received by server:\n");
	printf("\temisor: %s\n", myMessage.emisor);
	printf("\treceptor: %s\n", myMessage.receptor);
	printf("\ttexto: %s\n", myMessage.msg);

	*result = sendMessage (myMessage);

	return SOAP_OK;
}

int ims__receiveMessage (struct soap* soap, char* username, struct ListaMensajes* result){

	// Allocate space for the message field of the myMessage struct then copy it
	result->mensajes =  malloc( (IMS_MAX_NAME_SIZE+IMS_MAX_MSG_SIZE)*MAX_MENSAJES);
	FILE * fichero;
	char caracter;
	chdir(username);
	fichero = fopen("mensajes_pendientes.txt", "rt");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	} else
		printf("Fichero abierto correctamente.\n");

	int i=0;
	caracter = fgetc(fichero);
	while (feof(fichero) == 0){
		result->mensajes[i] = caracter;
		caracter = fgetc(fichero);
		i++;
	}
	result->mensajes[i]='\0';

	// ?BORRAR EL CONTENIDO del FICHERO¿
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}
	chdir("..");


	//myMessage->msg = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMessage->msg, 0, IMS_MAX_MSG_SIZE);
	//strcpy (myMessage->msg, "Invoking the remote function receiveMessage simply retrieves this standard message from the server"); // always same msg

	// Allocate space for the name field of the myMessage struct then copy it
	//myMessage->name = (xsd__string) malloc (IMS_MAX_NAME_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMessage->name, 0, IMS_MAX_NAME_SIZE);
	//strcpy(myMessage->name, "aServer");

	return SOAP_OK;
}

int ims__darAlta (struct soap *soap, char* username, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	result->code = addUser(&db, username); //-1 err, -2 ya existe

	if (result->code == 0) {
		strcpy(result->msg, "Usuario registrado correctamente.");
		createFriendListEntry(username, &la);
	}
	else if (result->code == -1)
		strcpy(result->msg, "ERROR (-1): Error interno del servidor.");
	else if (result->code == -2)
		strcpy(result->msg, "ERROR (-2): Este nombre no está disponible.");

	return SOAP_OK;
}

int ims__darBaja(struct soap *soap, char* username, struct ResultMsg* result){

	result->msg = malloc(IMS_MAX_MSG_SIZE);
	int res;

	// 1. Borrar de la BD de usuarios --> Marcar baja=0
	res = deleteUser(&db, username); // 0 éxito, -1 err.

	// 2. Borrar de la estructura de amistades
	if (res == 0) res = deleteFriendListEntry(&la, username); // 0 éxito, -1 err.

	// 3. Borrar también de las listas de amigos de otras personas.
	deleteUserFromEverybodyFriendList(&la, username);

	// 3. Borrar peticiones de amistad (en cualquier dirección) pendientes
	if (res == 0) delUserRelatedFriendRequests(&ap, username);

	// 4. TODO: Borrar mensajes y conversaciones

	result->code = res;

	return SOAP_OK;
}

/**
 * Marca la sesión de un usuario como iniciada ('connected = 1').
 * @soap Contexto gSOAP
 * @param username Nombre del usuario que hace login.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__login (struct soap *soap, char* username, struct ResultMsg *result) {

	int existe = 0, i = 0;

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Buscar si existe un usuario con este nombre o si ya tiene una sesión iniciada.
	while (existe == 0 && i < db.nUsers) {
		if (strcmp(db.usuarios[i].username, username) == 0) { // Existe el usuario
			existe = 1;
			if (db.usuarios[i].connected == 0) {					// No tiene sesión iniciada
				db.usuarios[i].connected = 1;
				result->code = 0;
			} else															// Tiene sesión iniciada
				result->code = -2;
		}
		i++;
	}

	// Si no existía el usuario:
	if (existe == 0)
		result->code = -1;

	// Rellenamos la estructura que se devuelve al cliente
	if (result->code == 0)
		strcpy(result->msg, "Login correcto.");
	else if (result->code == -1)
		strcpy(result->msg, "ERROR (-1): El usuario indicado no existe.");
	else if (result->code == -2)
		strcpy(result->msg, "ERROR (-2): El usuario indicado ya tiene una sesión iniciada.");

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
int ims__sendFriendRequest (struct soap *soap, struct PeticionAmistad p, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	result->code = addFriendRequest(&ap, &db, &la, p.emisor, p.receptor);

	if (result->code == 0)
		strcpy(result->msg, "Petición enviada correctamente.");
	else if (result->code == -1)
		strcpy(result->msg, "ERROR (-1): La lista de peticiones de amistad del servidor está llena. Inténtalo de nuevo más tarde.");
	else if (result->code == -2)
		strcpy(result->msg, "ERROR (-2): No puedes enviarte una petición a ti mismo.");
	else if (result->code == -3)
		strcpy(result->msg, "ERROR (-3): Este usuario ya es tu amigo.");
	else if (result->code == -4)
		strcpy(result->msg, "ERROR (-4): Este usuario no existe.");

	return SOAP_OK;
}

/**
 * Servicio gSOAP para pedir las peticiones de amistad pendientes.
 * @param soap Contexto gSOAP.
 * @param username Nombre de usuario que invoca la llamada.
 * @param lista Estructura donde se devuelve la lista de peticiones pendientes.
 */
int ims__getAllFriendRequests (struct soap* soap, char* username, struct ListaPeticiones *result) {

	// Variable para la respuesta
	result->nElems = 0;
	result->peticiones = (xsd__string) malloc(MAX_AMISTADES_PENDIENTES*IMS_MAX_NAME_SIZE + 1);
	result->peticiones[0] = '\0'; // Si no lo ponemos, riesgo de violación de segmento.

	// Rellenar la estructura
	searchPendingFriendRequests(username, &ap, result);

	// Mostrar la estructura
	printf("Contenido de la estructura:\n");
	printf("---------------------------\n");
	printf("result->nPeticiones = %d\n", result->nElems);
	printf("Nombres: %s\n", result->peticiones);

	return SOAP_OK;
}

/**
 * Servicio gSOAP para aceptar o denegar una petición de amistad de un usuario
 * hacia otro.
 * @param soap Contexto gSOAP.
 * @param rp Estructura con la respuesta a la petición de amistad.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__answerFriendRequest (struct soap* soap, struct RespuestaPeticionAmistad rp, int* result) {

	/* TODO: cuando esté implementada la parte de mensajería, deberá colocar
	 un mensaje para emisor y receptor informando del resultado de la petición. */

	if(rp.aceptada == 1) {
		printf("%s ha aceptado la petición de amistad de %s.\n", rp.receptor, rp.emisor);
		*result = addFriendToList(&la, rp.emisor, rp.receptor);
		saveFriendsData(&la); /* TODO: esto habrá que quitarlo */
	}
	else
		printf("%s ha denegado la petición de amistad de %s.\n", rp.receptor, rp.emisor);

	// Borramos la petición de amistad de la estructura en memoria
	delFriendRequest(&ap, rp.emisor, rp.receptor);

	return SOAP_OK;
}

/**
 * Devuelve la lista de amigos de un usuario.
 * @param soap Contexto gSOAP.
 * @param username Nombre del usuario.
 * @result Estructura con la lista de amigos.
 */
int ims__getFriendList(struct soap* soap, char* username,  struct ListaAmigos* result) {

	result->amigos = malloc(IMS_MAX_NAME_SIZE*IMS_MAX_AMIGOS + 1);
	result->amigos[0] = '\0'; // Si no lo ponemos, violación de segmento al strcat()

	getFriendList(username, &la, result->amigos);

	return SOAP_OK;
}
