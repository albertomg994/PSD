#include "s_amigos.h"
#include "externo.h"
#include <string.h>

/**
 * Añade una petición de amistad al array de peticiones pendientes en el servidor.
 * @param ap Estructura que almacena las peticiones
 * @param emisor Emisor de la petición de amistad
 * @param destinatario Destinatario de la petición de amistad
 * @return 0 si éxito, -1 si la lista está llena
 */
int addFriendRequest(struct amistades_pendientes* ap, char* emisor, char* destinatario) {

	if (ap->nPeticiones >= MAX_AMISTADES_PENDIENTES)
		return -1;

	/* TODO: Faltaría controlar que existe el usuario al que enviamos la petición */

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
void searchPendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct amistades_pendientes* ap, struct ListaAmigos *lista) {

	int i;
	for (i = 0; i < ap->nPeticiones; i++) {
		// Si el usuario coincide, añadirlo a la respuesta
		if(strcmp(username, ap->amistades_pendientes[i].destinatario) == 0) {

			strcat(lista->nombres, ap->amistades_pendientes[i].emisor);	// Añadir a la lista

			if (i < ap->nPeticiones -1)
				strcat(lista->nombres, " \0"); // Add space

			lista->nPeticiones++;
		}
	}
}

/*
		struct amigos_usuario {
			char usuario[IMS_MAX_NAME_SIZE];
			char amigos[IMS_MAX_AMIGOS];
		}

		struct listas_amigos {
			int nUsuarios;
			struct amigos_usuario[MAX_USERS];
		}
*/

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

		la->listas[nUsr].nAmigos = 0;						// Set num. friends
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
 *
 */
int saveFriendsData(struct listas_amigos* la) {
	printf("saveFriendsData()\n");
	return 0;
}

/**
 *
 */
int printFriendsData(struct listas_amigos* la) {
	printf("printFriendsData()\n");
	return 0;
}
