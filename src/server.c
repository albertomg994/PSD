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
#include "pthread.h"
#include "signal.h"
#include <sys/types.h>
#include <unistd.h>

// -----------------------------------------------------------------------------
// Tipos, constantes y variables globales
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1
#define DEBUG_SIGINT 0
#define ALARMA_TIRAR_SESIONES 0
#define ALARMA_CHECK_SESIONES 1
#define SESSION_DURATION 30

struct ListaUsuarios lu;
struct ListaAmistadesPend ap;
struct ListasAmigos la;
struct ListasMensajes lmsg;

/* Para indicar las sesiones que podrían expirar */
volatile int sesion_expira [MAX_USERS];
volatile sig_atomic_t check_sessions;
volatile sig_atomic_t TIPO_ALARMA;
volatile sig_atomic_t exit_th_check_sessions;

// Signal Handlers
void alarm_handler(int sig) {

	if (TIPO_ALARMA == ALARMA_TIRAR_SESIONES) {

		printf("(!!!) TIRAR SESIONES.\n");
		int i;
		for (i = 0; i < MAX_USERS; i++)
			sesion_expira[i] = 1;

		// Poner la siguiente alarma, alternando.
		TIPO_ALARMA = ALARMA_CHECK_SESIONES;
		alarm(SESSION_DURATION/2);
	}
	else if (TIPO_ALARMA == ALARMA_CHECK_SESIONES) {

		printf("(!!!) CHECK SESIONES.\n");
		check_sessions = 1;
		// Poner la siguiente alarma, alternando.
		TIPO_ALARMA = ALARMA_TIRAR_SESIONES;
		alarm(SESSION_DURATION/2);
	}
}
void sigint_handler(int sig) {
	exit_th_check_sessions = 1;
	signal(SIGINT, SIG_DFL);		// Reset to default handler
	signal(getpid(), SIGINT);		// Launch SIGINT again
}

// -----------------------------------------------------------------------------
// Estructuras propias del servidor
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
void reactivarSesion(char* username);
int sesionExpirada(char* username);
void* th_check_sessions(void* arg);
// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char **argv){

	int m, s;				// sockets
	struct soap soap;
	sigset_t grupo;		// grupo para enmascarar SIGINT
	pthread_t th_comprueba_sesiones;

	if (argc < 2) {
		printf("Usage: %s <port>\n",argv[0]);
		exit(-1);
	}

	/* Establish a handler for SIGALRM signals. */
	TIPO_ALARMA = ALARMA_TIRAR_SESIONES;
	signal(SIGALRM, alarm_handler);

	/* Set an alarm to go off in a little while. */
	alarm (SESSION_DURATION/2);

	/* Handler para SIGINT */
	signal(SIGINT, sigint_handler);

	// Lanzamos la hebra
	int res = pthread_create(&th_comprueba_sesiones, NULL, th_check_sessions, NULL);
	if (res != 0) {
		printf("Error creando la hebra.\n");
		exit(EXIT_FAILURE);
	}

	// Init environment
	soap_init(&soap);

	// Cargamos la información de usuarios
	if (usr__loadListaUsuarios(&lu) == -1) exit(-1);

	// Cargamos la información de los amigos
	if (frd__loadFriendsData(&la) == -1) exit(-1);

	// Cargamos las peticiones de amistad pendientes
	if (frq__loadPeticiones(&ap) == -1) exit(-1);

	// Cargamos los mensjaes no chequeados
	if (msg__loadMensajesEnviados(&lmsg) == -1) exit(-1);

	// Inicializar el contenido del array de expiración de sesiones
	int i;
	for (i = 0; i < MAX_USERS; i++)
		sesion_expira[i] = 0;

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
			msg__saveMensajesEnviados(&lmsg);
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
		msg__saveMensajesEnviados(&lmsg);

		// Ver si llego CTRL+C mientras atendíamos una petición.
		/*sigset_t pendientes;
		sigpending(&pendientes);
		if (sigismember(&pendientes, SIGINT)) {
			printf("SIGINT está en pendientes...\n");
			printf("Intento cerrar el thread de las comprobaciones...\n");
			exit_th_check_sessions = 1;
		}*/

		// Desenmascarar SIGINIT
		sigprocmask(SIG_UNBLOCK, &grupo, NULL);

	}

	return 0;
}

int ims__sendMessage (struct soap *soap, struct Message2 myMessage, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(myMessage.emisor) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(myMessage.emisor);

	int i = 0, j, salir = 0;
	// Comprobar si el emisor es el amigo del receptor.
	while (i < la.size && salir == 0) {
		if (strcmp(myMessage.emisor, la.listas[i].usuario) == 0) {
			for (j = 0; j < la.listas[i].size; j++) {
				if(strcmp(myMessage.receptor,la.listas[i].amigos[j]) == 0)
					salir = 1;
			}
		}
		i++;
	}

	// Si el usuario al que queremos enviar es nuestro amigo
	if(salir == 1){
		sendCheck(&lmsg, &myMessage);
		if (sendMessage (myMessage) < 0) {
			result->code = -500;
			strcpy(result->msg, "Error del servidor.");
		} else
			result->code = 0;
	}
	// Si no...
	else{
		result->code = -300;
		strcpy(result->msg, "El usuario no es tu amigo.");
	}

	return SOAP_OK;
}

int ims__receiveMessage (struct soap* soap, char* username, struct ListaMensajes* result){

	int error;
	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(username);

	error = checkMessage(username,&lmsg);
	if (error!=0) {
		printf("ERROR: al chequearMensajesª\n");
	}
	error = receiveMessage(username,result);
	if (error!=0) {
		printf("ERROR: al recibirMensajesª\n");
	}

	return SOAP_OK;
}

int ims__consultarEntrega(struct soap *soap, char* username, struct ListaMensajes* result){

	int error;
	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(username);

	error = consultEntrega(username,&lmsg,result);

	if(error!=0){
		printf("ERROR: al consultarMensajesª\n");
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

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(username);

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

	// 4. Borrar mensajes enviados y la carpte del usuario.
	if (res == 0) msg_delUserMessage(&lmsg, username);

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
	else {										// Login correcto
		result->code = 0;
		lu.usuarios[pos].connected = 1;
	}

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
int ims__logout (struct soap *soap, char* username, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Buscar si existe un usuario con el mismo nombre
	int pos = usr__findUsuario(&lu, username, NULL);

	if (pos >= 0) {
		lu.usuarios[pos].connected = 0;
		result->code = 0;
		strcpy(result->msg, "Éxito.");
	} else {
		result->code = -201;
		strcpy(result->msg, "El usuario no existe.");
	}

	return SOAP_OK;
}

/**
 * Servicio gSOAP para enviar peticiones de amistad.
 */
int ims__sendFriendRequest (struct soap *soap, struct IMS_PeticionAmistad p, struct ResultMsg* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(p.emisor) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(p.emisor);

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
int ims__getAllFriendRequests (struct soap* soap, char* username, struct ListaPeticiones* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(username);

	// Variable para la respuesta
	result->size = 0;
	result->peticiones = (xsd__string) malloc(MAX_AMISTADES_PENDIENTES*IMS_MAX_NAME_SIZE + 1);
	result->peticiones[0] = '\0'; // Si no lo ponemos, riesgo de violación de segmento.

	// Rellenar la estructura
	frq__retrievePendingFriendRequests(username, &ap, result);

	// Resultado de la llamada
	result->code = 0;
	strcpy(result->msg, "Éxito.");

	return SOAP_OK;
}

/**
 * Servicio gSOAP para aceptar o denegar una petición de amistad de un usuario
 * hacia otro.
 * @param soap Contexto gSOAP.
 * @param rp Estructura con la respuesta a la petición de amistad.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__answerFriendRequest (struct soap* soap, struct RespuestaPeticionAmistad rp, struct ResultMsg* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(rp.receptor) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(rp.receptor);

	/* TODO: cuando esté implementada la parte de mensajería, deberá colocar
	 un mensaje para emisor y receptor informando del resultado de la petición. */

	if(rp.aceptada == 1) {

		result->code = frd__addFriendRelationship(&la, rp.emisor, rp.receptor);

		// Rellenar resultado de la llamada
		if (result->code < 0)
			strcpy(result->msg, "Error.");
		else
			strcpy(result->msg, "Éxito.");
	}

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

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	printf("Llega una petición de %s. Comprobamos si su sesión ha expirado.\n", username);

	// Comprobamos si la sesión ha expirado
	if (sesionExpirada(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	reactivarSesion(username);

	result->amigos = malloc(IMS_MAX_NAME_SIZE*IMS_MAX_AMIGOS + 1);
	result->amigos[0] = '\0'; // Si no lo ponemos, violación de segmento al strcat()

	frd__getFriendList(&la, username, result->amigos);

	return SOAP_OK;
}

/**
 * Comprueba si la sesión de un usuario ha expirado.
 * @return 1 si ha expirado, 0 si todavía es válida.
 */
int sesionExpirada(char* username) {

	printf("Comprobando si ha expirado la sesión de %s...\n", username);

	int pos = usr__findUsuario(&lu, username, NULL);
	if (pos >= 0) {
		if (lu.usuarios[pos].connected == 0) {
			printf("El usuario figura como NO conectado.\n");
			return 1;
		}
		else {
			printf("El usuario figura como conectado.\n");
			return 0;
		}
	}
	return 1;
}

/**
 * Quita el flag de 'expirar sesión de un usuario' del servidor, que se activa
 * cada X tiempo mediante un temporizador.
 */
void reactivarSesion(char* username) {
	int pos = usr__findUsuario(&lu, username, NULL);
	if (pos >= 0)
		sesion_expira[pos] = 0;
	printf("Reactivando sesión de %s\n", username);
	printf("sesion_expira[%d] = %d\n", pos, sesion_expira[pos]);
}

/**
 * Hebra encargada de desconectar a los usuarios si sesion_expira[i] = 1
 */
void* th_check_sessions(void* arg) {

	while (!exit_th_check_sessions) {
		if (check_sessions == 1) {
			printf("La hebra comprueba las sesiones inactivas...\n");
			int i;
			for (i = 0; i < MAX_USERS; i++) {
				if (sesion_expira[i] == 1)
					lu.usuarios[i].connected = 0;
			}
			check_sessions = 0;
		}
	}
	printf("th_check_sessions makes exit...\n");
	pthread_exit(0);
}
