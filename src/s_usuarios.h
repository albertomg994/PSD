#ifndef S_USUARIOS_H
#define S_USUARIOS_H

#include "stdio.h"
#include "ims.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------
#define MAX_USERS 100 // Nº máximo de usuarios dados de alta en el sistema.

struct reg_usuario {
	char username[IMS_MAX_NAME_SIZE];
	int connected;
	int baja;
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
int searchUserInUserList(struct datos_usuarios * t, xsd__string username);

#endif
