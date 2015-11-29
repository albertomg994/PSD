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
#include "s_services.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y variables globales
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1
#define DEBUG_SIGINT 0
#define ALARMA_TIRAR_SESIONES 0	/* SIGALARM handler, type 1 */
#define ALARMA_CHECK_SESIONES 1	/* SIGALARM handler, type 2 */
#define SESSION_DURATION 30		/* Client session duration  */

struct ListaUsuarios lu;
struct ListaAmistadesPend ap;
struct ListasAmigos la;
struct ListasMensajes lmsg;

/* Para indicar las sesiones que podrían expirar */
volatile int sesiones_expiradas [MAX_USERS];
volatile sig_atomic_t check_sessions;
volatile sig_atomic_t TIPO_ALARMA;
volatile sig_atomic_t exit_thread;

// -----------------------------------------------------------------------------
// Signal Handlers
// -----------------------------------------------------------------------------

/* SIGALARM handler. */
void alarm_handler(int sig) {

	if (TIPO_ALARMA == ALARMA_TIRAR_SESIONES) {

		printf("(!!!) TIRAR SESIONES.\n");
		int i;
		for (i = 0; i < MAX_USERS; i++)
			sesiones_expiradas[i] = 1;

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

/* SIGINT handler */
void sigint_handler(int sig) {
	exit_thread = 1;
	signal(SIGINT, SIG_DFL);		// Reset to default handler
	kill(getpid(), SIGINT);		// Launch SIGINT again
}

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
void renewSession (char* username);
int  isSessionExpired (char* username);
void* th_check_sessions (void* arg);

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
		sesiones_expiradas[i] = 0;

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


/**
 * Comprueba si la sesión de un usuario ha expirado.
 * @return 1 si ha expirado, 0 si todavía es válida.
 */
int isSessionExpired (char* username) {

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
 * Renueva la sesión de un usuario.
 * @param username Nombre del usuario cuya sesión queremos renovar.
 */
void renewSession(char* username) {
	int pos = usr__findUsuario(&lu, username, NULL);
	if (pos >= 0)
		sesiones_expiradas[pos] = 0;
	printf("Reactivando sesión de %s\n", username);
	printf("sesiones_expiradas[%d] = %d\n", pos, sesiones_expiradas[pos]);
}

/**
 * Hebra encargada de desconectar a los usuarios si sesiones_expiradas[i] = 1
 */
void* th_check_sessions(void* arg) {

	while (!exit_thread) {
		if (check_sessions == 1) {
			printf("La hebra comprueba las sesiones inactivas...\n");
			int i;
			for (i = 0; i < MAX_USERS; i++) {
				if (sesiones_expiradas[i] == 1)
					lu.usuarios[i].connected = 0;
			}
			check_sessions = 0;
		}
	}
	printf("th_check_sessions makes exit...\n");
	pthread_exit(0);
}
