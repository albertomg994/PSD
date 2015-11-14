#ifndef S_AMIGOS_H
#define S_AMIGOS_H

#include "stdio.h"
#include "ims.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------
struct peticion_amistad {
	char emisor[IMS_MAX_NAME_SIZE];
	char destinatario[IMS_MAX_NAME_SIZE];
};

struct amistades_pendientes {
	int nPeticiones;
	struct peticion_amistad amistades_pendientes[MAX_AMISTADES_PENDIENTES];
};

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
int addFriendRequest(struct amistades_pendientes* ap, char* emisor, char* destinatario);
void delFriendRequest(struct amistades_pendientes* ap, char* emisor, char* receptor);
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaAmigos *lista);

#endif
