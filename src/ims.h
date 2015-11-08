//gsoap ims service name: imsService
//gsoap ims service style: rpc
//gsoap ims service encoding: encoded
//gsoap ims service namespace: urn:imsService

// -----------------------------------------------------------------------------
// Tipos y constantes comunes a cliente y servidor
// -----------------------------------------------------------------------------
#define IMS_MAX_MSG_SIZE 256
#define IMS_MAX_NAME_SIZE 256
#define IMS_MAX_USR_SIZE 32

typedef char *xsd__string;

// -----------------------------------------------------------------------------
// Estructuras comunes a cliente y servidor
// -----------------------------------------------------------------------------
struct Message{
	xsd__string name;
	xsd__string msg;
	// nombre del emisor?
};

struct MensajeAlta {
	xsd__string username;
};

// -----------------------------------------------------------------------------
// Servicios
// -----------------------------------------------------------------------------

// Enviar un mensaje de prueba
int ims__sendMessage (struct Message myMessage, int *result);

// Recibir un mensaje de prueba
int ims__receiveMessage (struct Message *myMessage);

// Alta en el sistema
int ims__darAlta (struct MensajeAlta msg, int *result);

// Una sola invocación que revise mensajes, peticiones de amistad, avisos de entrega
