#ifndef S_USUARIOS_H
#define S_USUARIOS_H

#include "stdio.h"
#include "ims.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------
#define MAX_USERS 100 // Nº máximo de usuarios dados de alta en el sistema.

struct Usuario {
	int baja;
	int connected;
	char username[IMS_MAX_NAME_SIZE];
};

struct ListaUsuarios {
	int size;										// Nº de usuarios
	struct Usuario usuarios[MAX_USERS];		// Array de usuarios
};

// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
int usr__loadListaUsuarios(struct ListaUsuarios* lu);
int usr__saveListaUsuarios(struct ListaUsuarios* lu);
int usr__printListaUsuarios(struct ListaUsuarios* lu);

int usr__addUsuario(struct ListaUsuarios* lu, xsd__string username);
int usr__delUsuario(struct ListaUsuarios* lu, xsd__string username);
int usr__findUsuario(struct ListaUsuarios* lu, xsd__string username, struct Usuario* copy);
int usr__isUsernameAvailable(struct ListaUsuarios* lu, xsd__string username);

void usr__copyUsuario(struct Usuario* dst, struct Usuario* src);

#endif
