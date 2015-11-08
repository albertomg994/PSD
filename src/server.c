// Alberto Miedes Garcés y Denys Sypko

#include "soapH.h"
#include "imsService.nsmap"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras propias del servidor
// -----------------------------------------------------------------------------
#define DEBUG_MODE 1
#define MAX_USERS 100

struct reg_usuario {
	xsd__string username;
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

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char **argv){

	int m, s;				// sockets
	struct soap soap;

	if (argc < 2) {
		printf("Usage: %s <port>\n",argv[0]);
		exit(-1);
	}

	// Init environment
	soap_init(&soap);

	// Bind to the specified port. Devuelve el socket primario del servidor.
	m = soap_bind(&soap, NULL, atoi(argv[1]), 100);

	// Check result of binding
	if (m < 0) {
  		soap_print_fault(&soap, stderr);
		exit(-1);
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

int loadUsersData(struct datos_usuarios * t) {
	if (DEBUG_MODE) printf("loadUsersData()");

	return 0;
}

/**
 * Persiste en un fichero los datos de los usuarios (actualmente en memoria)
 * dinámica.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int saveUsersData(struct datos_usuarios * t) {
	return 0;
}

/*
 * Imprime el contenido de la estructura que almacena los datos de los usuarios.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int printUsersData(struct datos_usuarios * t) {

	/*if (DEBUG_MODE) printf("Hay %d usuarios por imprimir.\n", t->nUsers);

	for (int i = 0; i < t->nUsers; i++) {
		printf("Usuario %d: %c", i, t->usuarios[i].username);
	}*/

	return 0;
}

/**
 * Añade un usuario al sistema IMS.
 * @param username Nombre del usuario a dar de alta.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error, -2 si el usuario ya existe.
 */
int addUser(struct datos_usuarios * t, xsd__string username) {
	return 0;
}

/**
 * Elimina un usuario del sistema IMS.
 * @param username Nombre del usuario a dar de baja.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error, -2 si el usuario no existía.
 */
int deleteUser(struct datos_usuarios * t, xsd__string username) {
	return 0;
}
