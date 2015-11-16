#include "soapH.h"
#include "imsService.nsmap"
#include "externo.h"
#include <string.h>

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras propias del cliente
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1

struct MisAmigos {
	int nElems;
	char amigos[IMS_MAX_AMIGOS][IMS_MAX_NAME_SIZE];
};

// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------
struct soap soap;
char* serverURL;
char username_global[IMS_MAX_NAME_SIZE]; // para logout()
struct MisAmigos mis_amigos;

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
void registrarse();
void darBaja();
void iniciarSesion();
void cerrarSesion();
void menuAvanzado();
void enviarMensaje();
void sendFriendRequest();
void receiveFriendRequests();
void showFriends();
int getFriendList();

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char **argv) {

  struct Message myMsgB;
  char *msg;

	// Usage
	if (argc != 3) {
		printf("Usage: %s http://server:port message\n",argv[0]);
		exit(0);
	}

	// 1. Init gSOAP environment
  	soap_init(&soap);

	// 2. Obtain server address
	serverURL = argv[1];

	// 3. Obtain message to be sent
	msg = argv[2];

	// Debug?
	if (DEBUG_MODE){
		printf ("Server to be used by client: %s\n", serverURL);
		printf ("Message to be sent by client: %s\n", msg);
	}

	char opcion;
	do {
		printf("1.- Registrarse\n");
		printf("2.- Iniciar sesión\n");
		printf("3.- Salir\n");
		opcion = getchar();
		clean_stdin();
		switch(opcion) {
			case '1':
				registrarse();
				break;
			case '2':
				iniciarSesion();
				break;
			case '3':
				break;
			default:
				break;
		}
	} while (opcion != '3');

	// Clean the environment
	soap_end(&soap);
	soap_done(&soap);

	return 0;
}

/**
 * Pide un nombre al usuario para registrarse en el sistema. Si ya existe,
 * muestra un mensaje de error.
 */
void registrarse() {

	int res;

	// 1. Pedir datos para registrarse
	printf("Nombre de usuario:");
	char name[IMS_MAX_NAME_SIZE];
	scanf("%31s", name);
	name[strlen(name)] = '\0';
	clean_stdin();

	// 2. Llamar a gSOAP
   soap_call_ims__darAlta (&soap, serverURL, "", name, &res);

	// 3. Control de errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	if (res == 0)
		printf("El usuario %s ha sido registrado correctamente.\n", name);
	else if (res == -2)
		printf("Es posible que el nombre de usuario ya exista.\n");
	else
		printf("Error del servidor.\n");
}

/**
 * Da de baja al usuario.
 */
void darBaja() {

	int res;

	// 1. Llamar a gSOAP
   soap_call_ims__darBaja (&soap, serverURL, "", username_global, &res);

	// 2. Control de errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	if (res == 0)
		printf("El usuario %s ha sido dado de baja correctamente.\n", username_global);
	else if (res == -2)
		printf("El usuario no existe.\n");
	else
		printf("Error del servidor.\n");
}

/**
 * Pide el nombre de usuario e inicia sesión en la cuenta con dicho nombre.
 * También establece el valor de la variable global 'username_global'.
 * En caso de éxito muestra un menu para enviar mensajes.
 */
void iniciarSesion() {

	int res;

	// 1. Pedir datos del login
	printf("Nombre de usuario:");
	//char name[IMS_MAX_NAME_SIZE];
	scanf("%31s", username_global);
	username_global[strlen(username_global)] = '\0';
	clean_stdin();

	// 2. Llamar a gSOAP
	soap_call_ims__login (&soap, serverURL, "", username_global, &res);

	// 3. Control de errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	if (res < 0)
		printf("El usuario %s no existe.\n", username_global);
	else {

		if (getFriendList() < 0)
			printf("Error obteniendo tu lista de amigos del servidor.\n");
		else
			menuAvanzado();
	}
}

/**
 * Menu con las opciones para el cliente cuando ya ha iniciado sesión.
 */
void menuAvanzado() {

	char opcion;

	do {
		printf("1.- Enviar mensaje a otro usuario\n");
		printf("2.- Enviar petición de amistad\n");
		printf("3.- Consultar peticiones de amistad\n");
		printf("4.- Ver amigos\n");
		printf("5.- Dar de baja\n");
		printf("6.- Cerrar sesión\n");

		opcion = getchar();
		clean_stdin();

		switch(opcion) {
			case '1':
				enviarMensaje();
				break;
			case '2':
				sendFriendRequest();
				break;
			case '3':
				receiveFriendRequests();
				break;
			case '4':
				showFriends();
				break;
			case '5':
				darBaja();
				break;
			case '6':
				cerrarSesion();
				break;
			default:
				break;
		}
	} while (opcion != '5'  && opcion != '6');
}

/**
 * Cierra la sesión del usuario actual. Para obtener el nombre de usuario,
 * consultamos el valor de la variable global 'username_global'.
 */
void cerrarSesion() {

	int res;

	// 1. Llamar a gSOAP
	soap_call_ims__logout (&soap, serverURL, "", username_global, &res);

	// 2. Control de errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	if (res < 0)
		printf("El usuario %s no existe.\n", username_global);
	else
		printf("Has hecho logout.\n");
}

/**
 * Envia un mensaje a otro usuario.
 */
void enviarMensaje() {

	struct Message2 mensaje;
	char text[IMS_MAX_MSG_SIZE];
	char receptor[IMS_MAX_NAME_SIZE];
	int res;

	// 1. Poner el mensaje
	printf("Introduce el texto:");
	scanf("%255s", text);
	text[strlen(text)] = '\0';
	clean_stdin();
	mensaje.msg = malloc (IMS_MAX_MSG_SIZE);
	strcpy (mensaje.msg, text);

	// 2. Poner el emisor
	mensaje.emisor = malloc (IMS_MAX_NAME_SIZE);
	strcpy (mensaje.emisor, username_global);

	// 3. Poner el receptor
	printf("Destinatario:");
	scanf("%255s", receptor);
	receptor[strlen(receptor)] = '\0';
	clean_stdin();
	mensaje.receptor = malloc (IMS_MAX_NAME_SIZE);
	strcpy(mensaje.receptor, receptor);

	// 4. Llamada gSOAP
	soap_call_ims__sendMessage (&soap, serverURL, "", mensaje, &res);

	// 5. Comprobar errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}
}

/**
 * Envía una petición de amistad al usuario que indiquemos.
 */
void sendFriendRequest() {

	struct PeticionAmistad pet;
	char receptor[IMS_MAX_NAME_SIZE];
	int res;

	// 1. Poner el emisor
	pet.emisor = malloc (IMS_MAX_NAME_SIZE);
	strcpy (pet.emisor, username_global);

	// 2. Poner el receptor
	printf("Destinatario:");
	scanf("%255s", receptor);
	receptor[strlen(receptor)] = '\0';
	clean_stdin();
	pet.receptor = malloc (IMS_MAX_NAME_SIZE);
	strcpy(pet.receptor, receptor);

	// 3. Llamada gSOAP
	soap_call_ims__sendFriendRequest(&soap, serverURL, "", pet, &res);

	// 4. Comprobar errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}
}

/**
 * Pide al servidor las peticiones de amistad pendientes.
 */
void receiveFriendRequests() {

	struct ListaPeticiones lista_peticiones;
	char aux[MAX_AMISTADES_PENDIENTES][IMS_MAX_NAME_SIZE];
	struct RespuestaPeticionAmistad rp;
	int res;

	// 1. Llamada gSOAP
	soap_call_ims__getAllFriendRequests (&soap, serverURL, "", username_global, &lista_peticiones);

	// 2. Comprobar errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	// 3. Interpretar los resultados
	if (lista_peticiones.nElems == 0)
		printf("No tienes ninguna petición de amistad pendiente.\n");
	else {
		printf("Tienes %d peticiones de amistad pendientes:\n", lista_peticiones.nElems);
		printf("------------------------------------------\n");
	  	char* palabra = strtok (lista_peticiones.peticiones," ");
		int i = 0;
		while (palabra != NULL) {
	   	printf ("\t * %s\n",palabra);
			strcpy(aux[i], palabra);
	    	palabra = strtok (NULL, " ");
			i++;
	  	}
		printf("------------------------------------------\n");

		char c;
		for (i = 0; i < lista_peticiones.nElems; i++) {

			// Preguntamos si acepta o declina cada una de las peticiones
			printf("¿Quieres aceptar la petición de amistad de %s? (s/n)\n", aux[i]);
			c = getchar();
			clean_stdin();

			// Rellenamos la estructura que se envía en la llamada gSOAP
			if (c == 's')
				rp.aceptada = 1;
			else
				rp.aceptada = 0;

			rp.emisor = malloc(IMS_MAX_NAME_SIZE);
			strcpy(rp.emisor, aux[i]);
			rp.receptor = malloc(IMS_MAX_NAME_SIZE);
			strcpy(rp.receptor, username_global);

			// Llamada gSOAP
			soap_call_ims__answerFriendRequest (&soap, serverURL, "", rp, &res);

			if (res < 0)
				printf("Ocurrió un error al aceptar la petición e %s.\n", rp.emisor);

			// Comprobar errores
			if (soap.error) {
				soap_print_fault(&soap, stderr);
				exit(1);
			}
		}

	}
}

/**
 * Imprime la lista de amigos del cliente.
 */
void showFriends() {

	printf("Tus amigos son:\n");
	printf("---------------\n");
	if (mis_amigos.nElems == 0)
		printf ("   < No tienes amigos :( > ");
	else {
		int i;
		for (i = 0; i < mis_amigos.nElems; i++)
			printf ("  - %s\n", mis_amigos.amigos[i]);
	}
	printf("---------------\n");
}

/**
 * Obtiene la lista de amigos del servidor, y la almacena en la estructura que
 * el cliente tiene habilitada para ello.
 * @return 0 si éxito, -1 si error.
 */
int getFriendList() {

	struct ListaAmigos lista;

	// Obtener mi lista de amigos
	soap_call_ims__getFriendList(&soap, serverURL, "", username_global, &lista);

	// Comprobar errores
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		return -1; //exit(1);
	}

	// Procesar y meter en la estructura
	char* amigo = strtok (lista.amigos," ");
	int i = 0;
 	while (amigo != NULL) {
		strcpy(mis_amigos.amigos[i], amigo);
	 	amigo = strtok (NULL, " ");
	 	i++;
 	}

	mis_amigos.nElems = i;
	return 0;
}
