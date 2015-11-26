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
int usr__loadListaUsuarios(struct ListaUsuarios* lu) {

	FILE *fichero;
	char line[IMS_MAX_NAME_SIZE];
	int nUsr = 0;

	// Abrir el fichero
	fichero = fopen("Server/usuarios.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/usuarios.txt\"\n");
		return -1;
	} /*else
		printf("Fichero \"Server/usuarios.txt\" abierto correctamente.\n");*/

	// Leer los usuarios hasta fin de fichero
	char c = fgetc(fichero);
	while (c != EOF) {
		// Read till space
		appendChar(lu->usuarios[nUsr].username, c);
		c = fgetc(fichero);
		while (c != ' ') {
			appendChar(lu->usuarios[nUsr].username, c);
			c = fgetc(fichero);
		}
		// Read (baja = 0 or baja = 1)
		c = fgetc(fichero);
		if (c == '0')
			lu->usuarios[nUsr].baja = 0;
		else
			lu->usuarios[nUsr].baja = 1;

		(lu->usuarios[nUsr]).connected = 0;

		// Read '\n'
		c = fgetc(fichero);
		nUsr++;

		// Read next user (or EOF)
		c = fgetc(fichero);
	}

	lu->size = nUsr;

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero \"Server/usuarios.txt\".\n");
		return -1;
	}

	return 0;
}

/**
 * Persiste en un fichero una 'ListaUsuarios' alojada en memoria.
 * @param lu Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int usr__saveListaUsuarios(struct ListaUsuarios* lu) {

	FILE* fichero;

	// Abrir el fichero (sobrescribe)
	fichero = fopen("Server/usuarios.txt", "wt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/usuarios.txt\"\n");
		return -1;
	} /*else
		printf("Fichero \"Server/usuarios.txt\" abierto correctamente.\n");*/

	// Escribir los datos
	int i;
	for (i = 0; i < lu->size; i++) {
		fwrite(lu->usuarios[i].username, strlen(lu->usuarios[i].username), 1, fichero);
		fputc(' ', fichero);
		if (lu->usuarios[i].baja == 1)
			fputc('1', fichero);
		else
			fputc('0', fichero);
		fputc('\n',fichero);
	}

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero \"Server/usuarios.txt\".\n");
		return -1;
	}

	return 0;
}

/*
 * Imprime el contenido de la estructura que almacena los datos de los usuarios.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error.
 */
int usr__printListaUsuarios(struct ListaUsuarios* lu) {

	printf("==============================\n");
	printf("Hay %d usuarios.\n", lu->size);
	printf("------------------------------\n");
	int i;
	for (i = 0; i < lu->size; i++) {
		printf("Usuario %d: %s\t\tConnected = %d \tBaja: %d\n", i, lu->usuarios[i].username, lu->usuarios[i].connected, lu->usuarios[i].baja);
	}
	printf("==============================\n");
	return 0;
}

/**
 * Dado su nombre, añade un usuario a una 'ListaUsuarios'.
 * @param username Nombre del usuario a dar de alta.
 * @param lu Puntero a la 'ListaUsuarios'.
 * @return 0 si éxito, -1 si error, -2 si el usuario ya existe.
 */
int usr__addUsuario(struct ListaUsuarios* lu, xsd__string username) {

	int dir, pos;
	FILE *fichero;

	// Comprobar si no se ha alcanzado el límite de usuarios del sistema
	if (lu->size >= MAX_USERS) return -1;

	// Buscar si existe un usuario con el mismo nombre
	pos = usr__findUsuario(lu, username, NULL);
	if (pos != -1) return -2;

	// Crear una estructura e inicializarla con el nuevo 'Usuario'
	struct Usuario new;
	strcpy(new.username, username); new.connected = 0; new.baja = 0;

	chdir("Server");
	/*Lo hago despues de comprobacion para asegurar que el usuario no existe */
	dir = mkdir(username,0777);
	//chdir("Server");
	if(dir!=0){
		perror("Error al crear la carpeta\n");
		return -1;
	}

	chdir(username);
	fichero = fopen("mensajes_pendientes.txt", "wt");
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"mensajes_pendientes.txt\"\n");
		return -1;
	} /*else
		printf("Fichero \"mensajes_pendientes.txt\" abierto correctamente.\n");*/
	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero \"mensajes_pendientes.txt\".\n");
		return -1;
	}
	chdir("..");
	printf("Sea ha creado la carpeta %s\n",username);
	chdir("..");

	// Copiar el nuevo usuario en la estructura
	usr__copyUsuario(&lu->usuarios[lu->size], &new);
	lu->size++;

	return 0;
}

/**
 * Elimina un 'Usuario' de una 'ListaUsuarios'.
 * @param username Nombre del usuario a dar de baja.
 * @param lu Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error si el usuario no existía.
 */
int usr__delUsuario(struct ListaUsuarios* lu, xsd__string username) {

	// Buscar si existe un usuario el nombre indicado
	int pos = usr__findUsuario(lu, username, NULL);

	// Si no existía, salimos
	if (pos < 0) return -1;

	// Eliminar el usuario de la estructura (i.e. desplazar el resto)
	lu->size--;
	for (pos; pos < lu->size; pos++) {
		usr__copyUsuario(&lu->usuarios[pos], &lu->usuarios[pos+1]);
	}

	return 0;
}

/**
 * Busca un 'Usuario', mediante su nombre, en una 'ListaUsuarios'.
 * @param lu Puntero a la estructura del servidor.
 * @param username Nombre del usuario a buscar
 * @param copy SALIDA. Dirección donde guarda una copia del elemento buscado (si lo encuentra).
 * @return -1 si no existe, la posición donde está si existe.
 */
int usr__findUsuario(struct ListaUsuarios* lu, xsd__string username, struct Usuario* copy) {

	int i = 0, pos = -1;

	while (pos == -1 && i < lu->size) {
		// Si encontramos el usuario buscado, nos quedamos con una copia y su posición.
		if (strcmp(lu->usuarios[i].username, username) == 0 && lu->usuarios[i].baja == 0) {
			if (copy != NULL)
				usr__copyUsuario(copy, &lu->usuarios[i]);
			pos = i;
		}
		else
			i++;
	}

	return pos;
}

/**
 * Copia el un 'Usuario' de una variable a otra.
 * @param dst Puntero a donde copiaremos el usuario.
 * @param src Puntero a la variable a copiar.
 */
void usr__copyUsuario(struct Usuario* dst, struct Usuario* src) {
	strcpy(dst->username, src->username);
	dst->connected = src->connected;
	dst->baja = src->baja;
}
