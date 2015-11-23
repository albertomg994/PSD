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

struct ListaUsuarios lu;
struct ListaAmistadesPend ap;
struct ListasAmigos la;

struct ListasMensajes lmsg;
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
	if (usr__loadListaUsuarios(&lu) == -1) exit(-1);

	// Cargamos la información de los amigos
	if (frd__loadFriendsData(&la) == -1) exit(-1);

	// Cargamos las peticiones de amistad pendientes
	if (frq__loadPeticiones(&ap) == -1) exit(-1);

	lmsg.size=0;

	// Bind to the specified port. Devuelve el socket primario del servidor.
	m = soap_bind(&soap, NULL, atoi(argv[1]), 100);

	// Check result of binding
	if (m < 0) {
  		soap_print_fault(&soap, stderr);
		exit(-1);
	}

	char opcion = -1;
	while (opcion != '5') {
		printf("\n\ngSOAP server menu\n");
		printf("=================\n");
		printf("1.- Mostrar datos de usuarios\n");
		printf("2.- Dar de alta un usuario\n");
		printf("3.- Dar de baja un usuario\n");
		printf("4.- Mostrar peticiones de amistad pendientes\n");
		printf("5.- Ponerse a la escucha (se perderá el control)\n");
		printf("6.- Salir\n");

		opcion = getchar();
		clean_stdin();

		if (opcion == '1') {
			usr__printListaUsuarios(&lu);
		}
		else if (opcion == '2') {
			printf("Nombre de usuario:");
			char name[IMS_MAX_NAME_SIZE];
			scanf("%255s", name);
			name[strlen(name)] = '\0';
			clean_stdin();

			if(usr__addUsuario(&lu, name) == -1)
				printf("Error añadiendo a %s\n", name);
		}
		else if (opcion == '3') {
			printf("Nombre de usuario:");
			char name2[IMS_MAX_NAME_SIZE];
			scanf("%255s", name2);
			name2[strlen(name2)] = '\0';
			clean_stdin();

			if(usr__delUsuario(&lu, name2) < 0)
				printf("Error añadiendo a %s\n", name2);
		}
		else if (opcion == '4')
			frq__printPeticiones(&ap);
		else if (opcion == '6') {
			usr__saveListaUsuarios(&lu);
			frd__saveFriendsData(&la);
			frq__savePeticiones(&ap);
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
		usr__saveListaUsuarios(&lu);
		frd__saveFriendsData(&la);
		frq__savePeticiones(&ap);

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

	int i = 0, j, salir = 0;
	// Comprobar si el emisor es el amigo del reseptor.
	while (i < la.size && salir == 0) {
		if (strcmp(myMessage.emisor, la.listas[i].usuario) == 0) {
			for (j = 0; j < la.listas[i].size; j++) {
				if(strcmp(myMessage.receptor,la.listas[i].amigos[j]) == 0)
					salir = 1;
			}
		}
		i++;
	}

	if(salir == 1){

		strcpy(lmsg.lista[lmsg.size].emisor,myMessage.emisor);
		strcpy(lmsg.lista[lmsg.size].receptor,myMessage.receptor);
		strcpy(lmsg.lista[lmsg.size].msg,myMessage.msg);
		lmsg.size++;

		printf("-----Server: es tu amigo.\n");
		*result = sendMessage (myMessage);

	}else{
		printf("-----Server: no es tu amigo el mensaje ignorado.\n");
		*result = -1;
	}


	return SOAP_OK;
}

int ims__receiveMessage (struct soap* soap, char* username, struct ListaMensajes* result){
	FILE * fichero;
	char caracter;
	char emisor[256];
	char linea[256];
	int i;
	chdir(username);
	fichero = fopen("mensajes_pendientes.txt", "rt");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	}
	while ( fgets(linea,256,fichero) != NULL ){
		sscanf(linea,"%s ",emisor);
		for(i=0;i < lmsg.size;i++){
			if (strcmp( emisor, lmsg.lista[i].emisor) == 0 && strcmp( username,lmsg.lista[i].receptor) == 0 ) {
				if(lmsg.lista[i].msg[strlen(lmsg.lista[i].msg)-2] != '*' ){
					lmsg.lista[i].msg[strlen(lmsg.lista[i].msg)-1] = '*';
					lmsg.lista[i].msg[strlen(lmsg.lista[i].msg)] = '\n';
					lmsg.lista[i].msg[strlen(lmsg.lista[i].msg)+1] = '\0';
				}
				printf("msg----------------->%s\n",lmsg.lista[i].msg);
			}
		}
	}

	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	int error= receiveMessage(username,result);
	if(error!=0){
		printf("ERROR: al recibirMensajesª\n");
	}
	return SOAP_OK;
}

int ims__consultarEntrega(struct soap *soap, char* username, struct ListaMensajes* result){
	int i;
	// Allocate space for the message field of the myMessage struct then copy it
	result->mensajes =  malloc( (IMS_MAX_NAME_SIZE+IMS_MAX_MSG_SIZE)*MAX_MENSAJES);
	result->mensajes[0]='\0';

	for(i=0;i < lmsg.size;i++){
		if (strcmp(username,lmsg.lista[i].emisor) == 0) {
			strcat(result->mensajes,lmsg.lista[i].receptor);
			result->mensajes[strlen(result->mensajes)] = ' ';
			result->mensajes[strlen(result->mensajes)+1] = '\0';
			strcat(result->mensajes,lmsg.lista[i].msg);
		}
	}

	return SOAP_OK;

}


int ims__darAlta (struct soap *soap, char* username, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	result->code = usr__addUsuario(&lu, username); //-1 err, -2 ya existe

	if (result->code == 0) {
		strcpy(result->msg, "Usuario registrado correctamente.");
		frd__createFriendListEntry(&la, username);
	}
	else if (result->code == -1)
		strcpy(result->msg, "ERROR (-1): Error interno del servidor.");
	else if (result->code == -2)
		strcpy(result->msg, "ERROR (-2): Este nombre no está disponible.");

	return SOAP_OK;
}

int ims__darBaja(struct soap *soap, char* username, struct ResultMsg* result){

	result->msg = malloc(IMS_MAX_MSG_SIZE);
	int res = 0;

	// 1. Borrar de la BD de usuarios
	int pos = usr__findUsuario(&lu, username, NULL);
	if (pos < 0) res = -1;
	else lu.usuarios[pos].baja = 1;

	// 2. Borrar de la estructura de amistades
	if (res == 0) res = frd__deleteFriendListEntry(&la, username); // 0 éxito, -1 err.

	// 3. Borrar también de las listas de amigos de otras personas.
	frd__deleteUserFromEverybodyFriendList(&la, username);

	// 3. Borrar peticiones de amistad (en cualquier dirección) pendientes
	if (res == 0) frq__delUserRelatedFriendRequests(&ap, username);

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
	struct Usuario aux;
	int pos = usr__findUsuario(&lu, username, &aux);

	if (pos < 0)							// No existe el usr__findUsuario
		result->code = -1;
	else if (aux.baja == 1)				// Existía pero se dio de baja
		result->code = -1;
	else if (aux.connected == 1)		// Ya tiene una sesión iniciada
		result->code = -2;
	else										// Login correcto
		result->code = 0;

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

	// Buscar si existe un usuario con el mismo nombre
	int pos = usr__findUsuario(&lu, username, NULL);

	if (pos >= 0) {
		lu.usuarios[pos].connected = 0;
		*result = 0;
	} else {
		*result = -1;
	}

	if(DEBUG_MODE) usr__printListaUsuarios(&lu);

	return SOAP_OK;
}

/**
 * Servicio gSOAP para enviar peticiones de amistad.
 */
int ims__sendFriendRequest (struct soap *soap, struct IMS_PeticionAmistad p, struct ResultMsg *result) {

	printf("ims__sendFriendRequest()\n");

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	result->code = frq__addFriendRequest(&ap, &lu, &la, p.emisor, p.receptor);

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
	else if (result->code == -5)
		strcpy(result->msg, "ERROR (-5): Ya has mandado una petición de amistad a este usuario.");
	else if (result->code == -6)
		strcpy(result->msg, "ERROR (-6): Existe una petición equivalente en tu bandeja de entrada.");

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
	result->size = 0;
	result->peticiones = (xsd__string) malloc(MAX_AMISTADES_PENDIENTES*IMS_MAX_NAME_SIZE + 1);
	result->peticiones[0] = '\0'; // Si no lo ponemos, riesgo de violación de segmento.

	// Rellenar la estructura
	frq__retrievePendingFriendRequests(username, &ap, result);

	// Mostrar la estructura
	printf("Contenido de la estructura:\n");
	printf("---------------------------\n");
	printf("result->nPeticiones = %d\n", result->size);
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
		*result = frd__addFriendRelationship(&la, rp.emisor, rp.receptor);
		frd__saveFriendsData(&la); /* TODO: esto habrá que quitarlo */
	}
	else
		printf("%s ha denegado la petición de amistad de %s.\n", rp.receptor, rp.emisor);

	// Borramos la petición de amistad de la estructura en memoria
	frq__delFriendRequest(&ap, rp.emisor, rp.receptor);

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

	frd__getFriendList(&la, username, result->amigos);

	return SOAP_OK;
}
