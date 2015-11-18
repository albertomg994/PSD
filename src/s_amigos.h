#ifndef S_AMIGOS_H
#define S_AMIGOS_H

#include "stdio.h"
#include "ims.h"
#include "s_usuarios.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------

struct peticion_amistad {
	char emisor[IMS_MAX_NAME_SIZE];
	char destinatario[IMS_MAX_NAME_SIZE];
};

/* Estructura del servidor que almacena todas las peticiones de amistad
   pendientes de procesar. */
struct amistades_pendientes {
	int nPeticiones;
	struct peticion_amistad amistades_pendientes[MAX_AMISTADES_PENDIENTES];
};

struct amigos_usuario {
	int nAmigos;
	char usuario[IMS_MAX_NAME_SIZE];
	char amigos[IMS_MAX_AMIGOS][IMS_MAX_NAME_SIZE];
};

/* Estructura del servidor que almacena la lista de amigos de cada usuario. */
struct listas_amigos {
	int nUsuarios;
	struct amigos_usuario listas [MAX_USERS];
};

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
int addFriendRequest(struct amistades_pendientes* ap, struct datos_usuarios* du, char* emisor, char* destinatario);
void delFriendRequest(struct amistades_pendientes* ap, char* emisor, char* receptor);
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaPeticiones *lista);
void createFriendListEntry(char* username, struct listas_amigos* la);

int loadFriendsData(struct listas_amigos* la);
int saveFriendsData(struct listas_amigos* la);
void printFriendsData(struct listas_amigos* la);

int getFriendList(char* username, struct listas_amigos* la, char* lista);

#endif
