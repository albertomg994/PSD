//gsoap ims service name: imsService
//gsoap ims service style: rpc
//gsoap ims service encoding: encoded
//gsoap ims service namespace: urn:imsService

// -----------------------------------------------------------------------------
// Tipos y constantes comunes a cliente y servidor
// -----------------------------------------------------------------------------
#define IMS_MAX_MSG_SIZE 256
#define IMS_MAX_NAME_SIZE 256
#define IMS_MAX_USR_SIZE 32		// tam. máximo de nombre de usuario

typedef char *xsd__string;

// -----------------------------------------------------------------------------
// Estructuras comunes a cliente y servidor
// -----------------------------------------------------------------------------
struct Message {
	xsd__string name;
	xsd__string msg;
};

struct Message2 {
	char* receptor;
	char* emisor;
	char* msg;
};

struct PeticionAmistad {
	char* emisor;
	char* receptor;
};

struct RespuestaPeticionAmistad {
	char* emisor;
	char* receptor;
	int aceptada;
};

// -----------------------------------------------------------------------------
// Servicios
// -----------------------------------------------------------------------------

// Enviar un mensaje a un usuario
int ims__sendMessage (struct Message2 myMessage, int *result);

// Recibir un mensaje de prueba
int ims__receiveMessage (struct Message *myMessage);

// Alta en el sistema
int ims__darAlta (char* username, int *result);

// Baja en el sistema
int ims__darBaja(char* username, int *result);

// Login en el sistema
int ims__login (char* username, int *result);

// Logout del sistema
int ims__logout (char* username, int *result);

// Una sola invocación que revise mensajes, peticiones de amistad, avisos de entrega

// Enviar una petición de amistad
int ims__sendFriendRequest (struct PeticionAmistad p, int *result);

// Enviar una petición de amistad
//int ims__answerFriendRequest (int* result);

// Recibir todos los mensajes
//int ims__getAllMessages (int* result);

// Recibir todas las peticiones de amistad pendientes
//int ims__getAllFriendRequests (int* result);
