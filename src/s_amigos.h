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
int addFriendRequest(struct amistades_pendientes* ap, struct datos_usuarios* du, struct listas_amigos* la, char* emisor, char* destinatario);
void delFriendRequest(struct amistades_pendientes* ap, char* emisor, char* receptor);
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaPeticiones *lista);
void delUserRelatedFriendRequests(struct amistades_pendientes* ap, xsd__string username);

void createFriendListEntry(char* username, struct listas_amigos* la);
int deleteFriendListEntry(struct listas_amigos * la, xsd__string username);

void deleteUserFromEverybodyFriendList(struct listas_amigos* la, xsd__string username);

int loadFriendsData(struct listas_amigos* la);
int saveFriendsData(struct listas_amigos* la);
void printFriendsData(struct listas_amigos* la);

int loadPeticionesData(struct amistades_pendientes* ap);
int savePeticionesData(struct amistades_pendientes* ap);
void printPeticionesData(struct amistades_pendientes* ap);

int getFriendList(char* username, struct listas_amigos* la, char* lista);
int isFriendInList(struct listas_amigos* la, char* emisor, char* destinatario);

void copy(struct amigos_usuario* dest, struct amigos_usuario* src);
int searchUserInFriendList(struct listas_amigos * la, xsd__string username);

#endif
