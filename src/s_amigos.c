#include "s_amigos.h"
#include "externo.h"
#include <string.h>

/**
 * Añade una petición de amistad al array de peticiones pendientes en el servidor.
 * @param ap Estructura que almacena las peticiones
 * @param emisor Emisor de la petición de amistad
 * @param destinatario Destinatario de la petición de amistad
 * @return 0 si éxito, -1 si la lista está llena, -2 petición a si mismo, -3 ya es tu amigo, -4 no existe
 */
int addFriendRequest(struct amistades_pendientes* ap, struct datos_usuarios* du, struct listas_amigos* la, char* emisor, char* destinatario) {

	// Lista de amigos llena
	if (ap->nPeticiones >= MAX_AMISTADES_PENDIENTES)
		return -1;

	// Cliente se manda petición a si mismo
	if (strcmp(emisor, destinatario) == 0)
		return -2;

	// El destinatario no existe
	int pos = searchUserInUserList(du, destinatario);
	if (pos == -1)
		return -4;

	// Mandar petición a alguien que ya es nuestro amigo
	if (isFriendInList(la, pos, destinatario) == 1)
		return -3;



	// Añadir al array
	strcpy(ap->amistades_pendientes[ap->nPeticiones].emisor, emisor);
	strcpy(ap->amistades_pendientes[ap->nPeticiones].destinatario, destinatario);

	printf("peticion[%d].emisor = %s\n", ap->nPeticiones, ap->amistades_pendientes[ap->nPeticiones].emisor);
	printf("peticion[%d].destinatario = %s\n", ap->nPeticiones, ap->amistades_pendientes[ap->nPeticiones].destinatario);

	ap->nPeticiones++;

	return 0;
}

/**
 * Borra una petición de amistad de la estructura del servidor. Se invoca desde
 * el servicio gSOAP.
 * @param ap Puntero a la estructura del servidor.
 * @param old Petición de amistad a borrar.
 */
void delFriendRequest(struct amistades_pendientes* ap, char* emisor, char* receptor) {

	int i = 0, salir = 0, j;

	// Buscamos la petición que queremos borrar
	while (salir == 0 && ap->nPeticiones) {
		if (strcmp(emisor, ap->amistades_pendientes[i].emisor) == 0 &&
			 strcmp(receptor, ap->amistades_pendientes[i].destinatario) == 0) {
				 // Desplazar desde i hasta el nPeticiones-2
				 for (j = i; j < ap->nPeticiones - 1; j++) {
					 strcpy(ap->amistades_pendientes[j].emisor, ap->amistades_pendientes[j+1].emisor);
					 strcpy(ap->amistades_pendientes[j].destinatario, ap->amistades_pendientes[j+1].destinatario);
				 }
				 ap->nPeticiones--;
				 salir = 1;
		}
		i++;
	}
}

/**
 * Busca las peticiones de un usuario y las mete en la estructura.
 * @param username Nombre del usuario que solicita sus peticiones pendientes.
 * @param ap Puntero a la estructura del servidor con todas las peticiones.
 * @param lista Puntero a la estructura que devolverá la llamada gSOAP.
 */
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaPeticiones *lista) {

	int i;
	for (i = 0; i < ap->nPeticiones; i++) {
		// Si el usuario coincide, añadirlo a la respuesta
		if(strcmp(username, ap->amistades_pendientes[i].destinatario) == 0) {

			strcat(lista->peticiones, ap->amistades_pendientes[i].emisor);	// Añadir a la lista

			if (i < ap->nPeticiones -1)
				strcat(lista->peticiones, " \0"); // Add space

			lista->nElems++;
		}
	}
}

/**
 * Elimina todas las peticiones de amistad pendientes relacionadas con un
 * usuario (para cuando se da de baja).
 * @param ap Puntero a la estructura del servidor con las peticiones pendientes.
 * @param username Nombre del usuario que se da de baja.
 */
void delUserRelatedFriendRequests(struct amistades_pendientes* ap, xsd__string username) {
	int i;
	for (i = 0; i < ap->nPeticiones; i++) {
		if (strcmp(username, ap->amistades_pendientes[i].emisor) == 0 || strcmp(username, ap->amistades_pendientes[i].destinatario) == 0) {
			// Desplazar todas las de la derecha.
			ap->nPeticiones--;
			int j;
			for (j = i; j < ap->nPeticiones; j++) {
				strcpy(ap->amistades_pendientes[i].emisor, ap->amistades_pendientes[i+1].emisor);
				strcpy(ap->amistades_pendientes[i].destinatario, ap->amistades_pendientes[i+1].destinatario);
			}
		}
	}
}

/**
 * Carga la "listas_amigos.txt" a la estructura del servidor.
 * @param la Puntero a la estructura del servidor.
 * @return 0 si éxito, -1 si error
 */
int loadFriendsData(struct listas_amigos* la) {

	FILE *fichero;
	char line[IMS_MAX_NAME_SIZE*MAX_USERS + 1];

	// Abrir el fichero
	fichero = fopen("listas_amigos.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"listas_amigos.txt\"\n");
		return -1;
	} else
		printf("listas_amigos.txt abierto correctamente.\n");

	// Leer los usuarios y sus amigos hasta fin de fichero
	int nUsr = 0;
	while (fgets(line, IMS_MAX_NAME_SIZE*MAX_USERS + 1, fichero) != NULL) {

		line[strlen(line)-1] = '\0';	// quitamos el '\n' del fichero
		printf("Se ha leido: %s\n", line);

		// Split user & friends in the line
		int nAmigos = 0;
		char* palabra = strtok (line, " ");

		strcpy(la->listas[nUsr].usuario, palabra);		// Read username
		palabra = strtok (NULL, " ");							// Read 1st. friend

		while (palabra != NULL) {
			printf ("\tLeido amigo: %s\n",palabra);
			strcpy(la->listas[nUsr].amigos[nAmigos], palabra);	// Copy friend
			nAmigos++;
			palabra = strtok (NULL, " ");								// Read friend
		}

		la->listas[nUsr].nAmigos = nAmigos;						// Set num. friends
		nUsr++;
	}
	la->nUsuarios = nUsr;

	printFriendsData(la);

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/**
 * Guarda en un fichero las listas de amigos.
 * @param la Puntero a la estructura del servidor.
 * @return 0 si éxito, -1 si error
 */
int saveFriendsData(struct listas_amigos* la) {

	printf("saveFriendsData()\n");

	// Abrir el fichero (sobrescribe)
	FILE* fichero = fopen("listas_amigos.txt", "wt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"listas_amigos.txt\"\n");
		return -1;
	} else
		printf("listas_amigos.txt abierto correctamente.\n");

	// Escribir los datos
	int i, j, numAmigos, numUsuarios = la->nUsuarios;
	for (i = 0; i < numUsuarios; i++) {
		// write user
		fwrite(la->listas[i].usuario, strlen(la->listas[i].usuario), 1, fichero);
		numAmigos = la->listas[i].nAmigos;
		if (numAmigos > 0)
			fputc(' ', fichero);

		// write friends
		for (j = 0; j < numAmigos; j++) {
			fwrite(la->listas[i].amigos[j], strlen(la->listas[i].amigos[j]), 1, fichero);
			if (j < numAmigos -1)
				fputc(' ', fichero);
		}
		fputc('\n',fichero);
	}

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/**
 * Imprime el contenido de la estructura del servidor que almacena para cada
 * usuario, su lista de amigos.
 * @param la Puntero a la estructura del servidor.
 */
void printFriendsData(struct listas_amigos* la) {

	printf("======================\n");
	printf(" * LISTAS DE AMIGOS * \n");
	printf("======================\n");

	if (la->nUsuarios == 0)
		printf("\t < vacía > \n");
	else {
		int nUsr;
		for (nUsr = 0; nUsr < la->nUsuarios; nUsr++) {
			printf(" * %s:\n", la->listas[nUsr].usuario);
			if (la->listas[nUsr].nAmigos == 0)
				printf("\t < vacía > \n");
			else {
				int nAmigos;
				for (nAmigos = 0; nAmigos < la->listas[nUsr].nAmigos; nAmigos++)
					printf("\t- %s\n", la->listas[nUsr].amigos[nAmigos]);
			}
		}
	}
	printf("----------------------\n");
}

/**
 * Añade una nueva relación de amistad a la estructura que las almacena.
 * @param persona1 Primer componente de la relación de amistad.
 * @param persona2 Segundo componente de la relación de amistad.
 * @return 0 si éxito, -1 si error
 */
int addFriendToList(struct listas_amigos* la, char* persona1, char* persona2) {

	int i = 0, nAmigos, salir = 0;
	while (i < la->nUsuarios && salir < 2) {
		if(strcmp(persona1, la->listas[i].usuario) == 0 || strcmp(persona2, la->listas[i].usuario) == 0) {
			// Añadir al final
			nAmigos = la->listas[i].nAmigos;
			if (nAmigos == IMS_MAX_AMIGOS)
				return -1;

			// Hay que comprobar en que usuario estamos para añadir al otro
			if (strcmp(persona1, la->listas[i].usuario) == 0)
				strcpy(la->listas[i].amigos[nAmigos], persona2);
			else
				strcpy(la->listas[i].amigos[nAmigos], persona1);

			la->listas[i].nAmigos++;
			salir++;
		}
		i++;
	}
	return 0;
}

/**
 * Devuelve la lista de amigos de un usuario.
 * @param username Nombre del usuario de quien buscamos los amigos.
 * @param la Puntero a la estructura del servidor con todas las listas de amigos.
 * @param lista Parámetro de salida con la lista que nos piden.
 */
int getFriendList(char* username, struct listas_amigos* la, char* lista) {

	int i = 0, j, salir = 0;
	// Buscamos el usuario que nos piden
	while (i < la->nUsuarios && salir == 0) {
		if (strcmp(username, la->listas[i].usuario) == 0) {
			// Copiamos todos los amigos del usuario en la lista
			for (j = 0; j < la->listas[i].nAmigos; j++) {
				strcat(lista, la->listas[i].amigos[j]);
				if (j < la->listas[i].nAmigos - 1)
					strcat(lista, " ");
			}
			salir = 1;
		}
		i++;
	}
	return 0;
}

/**
 * Da de alta a un usuario en la lista de amigos (si acaba de registrarse)
 * @param username Nombre de usuario a dar de alta
 * @param la Puntero a la estructura de datos del servidor.
 */
void createFriendListEntry(char* username, struct listas_amigos* la) {
	la->listas[la->nUsuarios].nAmigos = 0;
	strcpy(la->listas[la->nUsuarios].usuario, username);
	la->nUsuarios++;
}

/**
 * Comprueba si 'destinatario' ya está en la lista de amigos de 'emisor'
 * @param emisor Emisor de la petición de amistad.
 * @param destinatario Receptro de la petición de amistad.
 * @return 0 si no está, 1 si ya existe.
 */
int isFriendInList(struct listas_amigos* la, int pos, char* destinatario) {

	// NOTA IMPORTANTE: doy por hecho que la posición de un usuario en la estructura
	// de los usuarios registrados y la estructra de los amigos es la misma.
	// Si no, esto petará.

	int i = 0, existe = 0;
	while (existe == 0 && i < la->listas[pos].nAmigos) {
		if (strcmp(la->listas[pos].amigos[i], destinatario)) // Encontramos el amigo
			existe = 1;
		else
			i++;
	}

	return existe;
}

/**
 * Busca el nombre de un usuario en la estructura del servidor.
 * @param t Puntero a la estructura
 * @param username Nombre del usuario a Buscar
 * @return -1 si no existe, la posición donde está si existe. v
 */
int searchUserInFriendList(struct listas_amigos * la, xsd__string username) {

	int i = 0, pos = -1;

	while (pos == -1 && i < la->nUsuarios) {
		if (strcmp(la->listas[i].usuario, username) == 0)
			pos = i;
		else
			i++;
	}

	return pos;
}

/**
 * Elimina un usuario del la lista de amistades.
 * @param username Nombre del usuario a dar de baja.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error si el usuario no existía.
 */
int deleteFriendListEntry(struct listas_amigos * la, xsd__string username) {

	// Buscar si existe un usuario con el mismo nombre
	int pos = searchUserInFriendList(la, username);

	// Si no existía, salimos
	if (pos < 0) return -1;

	// Eliminar el usuario de la estructura (i.e. desplazar el resto)
	la->nUsuarios--;
	for (pos; pos < la->nUsuarios; pos++) {
		strcpy(la->listas[pos].usuario, la->listas[pos+1].usuario); // destino, origen
		copy(&la->listas[pos], &la->listas[pos+1]);
	}

	return 0;
}

void copy(struct amigos_usuario* dest, struct amigos_usuario* src) {
	dest->nAmigos = src->nAmigos;
	strcpy(dest->usuario, src->usuario);
	int i;
	for (i = 0; i < src->nAmigos; i++) {
		strcpy(dest->amigos[i], src->amigos[i]);
	}
}

void deleteUserFromEverybodyFriendList(struct listas_amigos* la, xsd__string username) {
	int i, j, k;
	// Para cada usuario del sistema
	for (i = 0; i < la->nUsuarios; i++) {
		// Para la lista de cada usuario, buscar al que Borramos
		for (j = 0; j < la->listas[i].nAmigos; j++) {
			if (strcmp(la->listas[i].amigos[j], username) == 0) {
				// Borrar username y desplazar el resto de amigos
				la->listas[i].nAmigos--;
				for (k = j; k < la->listas[i].nAmigos; k++)
					strcpy(la->listas[i].amigos[k], la->listas[i].amigos[k+1]);
			}
		}
	}
}
