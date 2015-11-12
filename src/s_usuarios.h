#ifndef S_USUARIOS_H
#define S_USUARIOS_H

#include "stdio.h"
#include "ims.h"

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------
#define MAX_USERS 100

struct reg_usuario {
	char username[IMS_MAX_NAME_SIZE];
	char amigos[IMS_MAX_AMIGOS][IMS_MAX_NAME_SIZE];
	int numAmigos;
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
void s_usuarios();

int loadUsersData(struct datos_usuarios * t);
int saveUsersData(struct datos_usuarios * t);
int printUsersData(struct datos_usuarios * t);

int addUser(struct datos_usuarios * t, xsd__string username);
int deleteUser(struct datos_usuarios * t, xsd__string username);

#endif
