#ifndef S_AMIGOS_H
#define S_AMIGOS_H

#include "stdio.h"
#include "ims.h"
#include "s_usuarios.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------

/* Estructura que representa una petición de amistad */
struct PeticionAmistad {
	char emisor[IMS_MAX_NAME_SIZE];
	char destinatario[IMS_MAX_NAME_SIZE];
};

/* Estructura del servidor que almacena todas las peticiones de amistad
   pendientes de procesar. */
struct ListaAmistadesPend {
	int size;
	struct PeticionAmistad peticiones[MAX_AMISTADES_PENDIENTES];
};

/* Estructura del servidor que almacena la lista de amigos de un usuario */
struct AmigosUsuario {
	int size;													// Nº amigos del usuario
	char usuario[IMS_MAX_NAME_SIZE];						// Nombre del usuario
	char amigos[IMS_MAX_AMIGOS][IMS_MAX_NAME_SIZE];	// Lista de amigos
};

/* Estructura del servidor que almacena la lista de amigos de cada usuario. */
struct ListasAmigos {
	int size;											// Nº usuarios del sistema
	struct AmigosUsuario listas [MAX_USERS];	// Lista de amigos de cada usuario
};

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
// Funciones relacionadas con peticiones de amistad
int  frq__loadPeticiones  (struct ListaAmistadesPend* ap);
int  frq__savePeticiones  (struct ListaAmistadesPend* ap);
void frq__printPeticiones (struct ListaAmistadesPend* ap);

int  frq__addFriendRequest  (struct ListaAmistadesPend* ap, struct ListaUsuarios* lu, struct ListasAmigos* la, char* emisor, char* destinatario);
void frq__delFriendRequest  (struct ListaAmistadesPend* ap, char* emisor, char* receptor);
int  frq__findFriendRequest (struct ListaAmistadesPend* ap, xsd__string emisor, xsd__string destinatario);
void frq__copyFriendRequest (struct PeticionAmistad* dst, struct PeticionAmistad* src);

void frq__retrievePendingFriendRequests (char username[IMS_MAX_NAME_SIZE], struct ListaAmistadesPend* ap, struct ListaPeticiones *lista);
void frq__delUserRelatedFriendRequests  (struct ListaAmistadesPend* ap, xsd__string username);

// Funciones relacionadas 'Listas de amigos'
int  frd__loadFriendsData (struct ListasAmigos* la);
int  frd__saveFriendsData (struct ListasAmigos* la);
void frd__printFriendsData(struct ListasAmigos* la);

void frd__createFriendListEntry (struct ListasAmigos* la, char* username);
int  frd__deleteFriendListEntry (struct ListasAmigos* la, xsd__string username);

int  frd__getFriendList  (struct ListasAmigos* la, char* username, char* lista);
int  frd__isFriendInList (struct ListasAmigos* la, char* emisor, char* destinatario);

int  frd__addFriendRelationship (struct ListasAmigos* la, char* persona1, char* persona2);
void frd__deleteUserFromEverybodyFriendList (struct ListasAmigos* la, xsd__string username);

// Funciones relacionadas con 'AmigosUsuario'
int  frd__findAmigosUsuario (struct ListasAmigos* la, xsd__string username);
void frd__copyAmigosUsuario (struct AmigosUsuario* dest, struct AmigosUsuario* src);

#endif
