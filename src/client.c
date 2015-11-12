#include "soapH.h"
#include "imsService.nsmap"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras propias del cliente
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1

// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------
struct soap soap;
char* serverURL;
char username_global[IMS_MAX_NAME_SIZE]; // para logout()

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
void registrarse();
void iniciarSesion();
void cerrarSesion();
void menuAvanzado();
void enviarMensaje();
void sendFriendRequest();

void clean_stdin(void) {
	int c;
	do {
		c = getchar();
	} while (c != '\n' && c != EOF);
}

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

	/*
	// Receive a Message struct from the server into myMsgB
	soap_call_ims__receiveMessage (&soap, serverURL, "", &myMsgB);

	// Check for errors...
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}
	else
		printf ("Received from server: \n\tusername: %s \n\tmsg: %s\n", myMsgB.name, myMsgB.msg);


	// Probamos a mandar una petición de alta
	// int ims__darAlta (char username [IMS_MAX_NAME_SIZE]);
	// int ims__sendMessage (struct Message myMessage, int *result);
	struct MensajeAlta peticion;
	peticion.username = (xsd__string) malloc(IMS_MAX_NAME_SIZE);
	strcpy(peticion.username, "amiedes");
   soap_call_ims__darAlta (&soap, serverURL, "", peticion, &res);

	// Check for errors...
	if (soap.error) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}*/


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
	scanf("%255s", name);
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
 * Pide el nombre de usuario e inicia sesión en la cuenta con dicho nombre.
 * También establece el valor de la variable global 'username_global'.
 * En caso de éxito muestra un menu para enviar mensajes.
 */
void iniciarSesion() {

	int res;

	// 1. Pedir datos del login
	printf("Nombre de usuario:");
	//char name[IMS_MAX_NAME_SIZE];
	scanf("%255s", username_global);
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
	else
		menuAvanzado();
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
		printf("4.- Cerrar sesión\n");

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
				printf("Not yet implemented (3)...\n");
				break;
			case '4':
				cerrarSesion();
				break;
			default:
				break;
		}
	} while (opcion != '4');
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
