#include "s_usuarios.h"
#include "externo.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// -----------------------------------------------------------------------------
// Implementación de funciones
// -----------------------------------------------------------------------------
/**
 * Carga los datos de los usuarios desde un fichero.
 * @param t Estructura de datos donde cargar la información.
 * @return 0 si éxito, -1 si error
 */
int loadUsersData(struct datos_usuarios * t) {

	printf("loadUsersData()\n");
	FILE *fichero;
	char line[IMS_MAX_NAME_SIZE];
	int nUsr = 0;

	// Abrir el fichero
	fichero = fopen("usuarios.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	} else
		printf("Fichero abierto correctamente.\n");

	// Leer los usuarios hasta fin de fichero
	char c = fgetc(fichero);
	while (c != EOF) {
		// Read till space
		//strcat(t->usuarios[nUsr]).username, &c);
		appendChar(t->usuarios[nUsr].username, c);
		c = fgetc(fichero);
		while (c != ' ') {
			//strcat(t->usuarios[nUsr]).username, &c); // Add to name
			appendChar(t->usuarios[nUsr].username, c);
			c = fgetc(fichero);
		}
		// Read (baja = 0 or baja = 1)
		c = fgetc(fichero);
		if (c == '0')
			t->usuarios[nUsr].baja = 0;
		else
			t->usuarios[nUsr].baja = 1;

		(t->usuarios[nUsr]).connected = 0;

		// Read '\n'
		c = fgetc(fichero);
		nUsr++;

		// Read next user (or EOF)
		c = fgetc(fichero);
	}

	t->nUsers = nUsr;
	printUsersData(t);

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/**
 * Persiste en un fichero los datos de los usuarios (actualmente en memoria)
 * dinámica.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int saveUsersData(struct datos_usuarios * t) {

	FILE* fichero;

	// Abrir el fichero (sobrescribe)
	fichero = fopen("usuarios.txt", "wt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"usuarios.txt\"\n");
		return -1;
	} else
		printf("Fichero abierto correctamente.\n");

	// Escribir los datos
	int i;
	for (i = 0; i < t->nUsers; i++) {
		fwrite(t->usuarios[i].username, strlen(t->usuarios[i].username), 1, fichero);
		fputc(' ', fichero);
		if (t->usuarios[i].baja == 1)
			fputc('1', fichero);
		else
			fputc('0', fichero);
		fputc('\n',fichero);
	}

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/*
 * Imprime el contenido de la estructura que almacena los datos de los usuarios.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int printUsersData(struct datos_usuarios * t) {

	printf("==============================\n");
	printf("Hay %d usuarios.\n", t->nUsers);
	printf("------------------------------\n");
	int i;
	for (i = 0; i < t->nUsers; i++) {
		printf("Usuario %d: %s\t\tConnected = %d \tBaja: %d\n", i, t->usuarios[i].username, t->usuarios[i].connected, t->usuarios[i].baja);
	}
	printf("==============================\n");
	return 0;
}

/**
 * Añade un usuario al sistema IMS.
 * @param username Nombre del usuario a dar de alta.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error, -2 si el usuario ya existe.
 */
int addUser(struct datos_usuarios * t, xsd__string username) {

	int existe = 0, i = 0,dir;
  FILE *fichero;
	// Buscar si existe un usuario con el mismo nombre
	while (existe == 0 && i < t->nUsers) {
		if (strcmp(t->usuarios[i].username, username) == 0)
			existe = 1;
		i++;
	}

  if (existe == 1) return -2;

  /*Lo hago despues de comprobacion para asegurar que el usuario no existe */
  dir=mkdir(username,0777);

  if(dir!=0){
    perror("Error al crear la carpeta\n");
    return -1;
  }

  chdir(username);
  fichero = fopen("mensajes_pendientes.txt", "wt");
  if (fichero == NULL) {
    printf("No se encuentra el fichero \"usuarios.txt\"\n");
    return -1;
   } else
      printf("Fichero abierto correctamente.\n");
   // Cerrar el fichero
   if(fclose(fichero) != 0) {
      printf("Error cerrando el fichero.\n");
    	return -1;
   }

    chdir("..");
    printf("Sea ha creado la carpeta %s\n",username);




	// Copiar el nuevo usuario en la estructura
	strcpy(t->usuarios[i].username, username);
	t->nUsers++;

	return 0;
}

/**
 * Elimina un usuario del sistema IMS.
 * @param username Nombre del usuario a dar de baja.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error si el usuario no existía.
 */
int deleteUser(struct datos_usuarios * t, xsd__string username) {

	// Buscar si existe un usuario con el mismo nombre
	int pos = searchUserInUserList(t, username);

	// Si no existía, salimos
	if (pos < 0) return -1;

	// Eliminar el usuario de la estructura (i.e. desplazar el resto)
	t->nUsers--;
	for (pos; pos < t->nUsers; pos++) {
		strcpy(t->usuarios[pos].username, t->usuarios[pos+1].username); // destino, origen
		t->usuarios[pos].connected = t->usuarios[pos+1].connected;
	}

	return 0;
}

/**
 * Busca el nombre de un usuario en la estructura del servidor.
 * @param t Punteor a la estructura
 * @param username Nombre del usuario a Buscar
 * @return -1 si no existe, la posición donde está si existe.
 */
int searchUserInUserList(struct datos_usuarios * t, xsd__string username) {

	int i = 0, pos = -1;

	while (pos == -1 && i < t->nUsers) {
		if (strcmp(t->usuarios[i].username, username) == 0)
			pos = i;
		else
			i++;
	}

	return pos;
}
