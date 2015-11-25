#include "s_amigos.h"
#include "externo.h"
#include <string.h>

/**
 * Lee de fichero el contenido de la estructura del servidor que almacena
 * las peticiones de amistad pendientes.
 * @param ap Puntero a la estructura.
 * @return 0 si éxito, -1 si error
 */
int frq__loadPeticiones(struct ListaAmistadesPend* ap) {

	FILE *fichero;
	char line[IMS_MAX_NAME_SIZE + 1];

	// Abrir el fichero
	fichero = fopen("Server/peticiones_pendientes.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/peticiones_pendientes.txt\"\n");
		return -1;
	} else
		printf("Server/peticiones_pendientes.txt abierto correctamente.\n");

	// Leer los usuarios y sus amigos hasta fin de fichero
	int nPeticiones = 0;
	while (fgets(line, IMS_MAX_NAME_SIZE + 1, fichero) != NULL) {

		// Set sender
		line[strlen(line)-1] = '\0';	// quitamos el '\n' del fichero
		strcpy(ap->peticiones[nPeticiones].emisor, line);

		// Read and set receiver
		fgets(line, IMS_MAX_NAME_SIZE + 1, fichero);
		line[strlen(line)-1] = '\0';
		strcpy(ap->peticiones[nPeticiones].destinatario, line);

		nPeticiones++;
	}
	ap->size = nPeticiones;

	frq__printPeticiones(ap);

	// Cerrar el fichero
	if(fclose(fichero) != 0) {
		printf("Error cerrando el fichero.\n");
		return -1;
	}

	return 0;
}

/**
 * Guarda en fichero el contenido de la estructura del servidor que almacena
 * las peticiones de amistad pendientes.
 * @param ap Puntero a la estructura.
 * @return 0 si éxito, -1 si error
 */
int frq__savePeticiones(struct ListaAmistadesPend* ap) {

	// Abrir el fichero (sobrescribe)
	FILE* fichero = fopen("Server/peticiones_pendientes.txt", "wt");

	// Check errors while opening
	if (fichero == NULL) {
		printf("No se encuentra el fichero \"peticiones_pendientes.txt\"\n");
		return -1;
	} else
		printf("peticones_pendientes.txt abierto correctamente.\n");

	// Escribir los datos
	int i;
	for (i = 0; i < ap->size; i++) {
		// write sender
		fwrite(ap->peticiones[i].emisor, strlen(ap->peticiones[i].emisor), 1, fichero);
		fputc('\n', fichero);

		// write receiver
		fwrite(ap->peticiones[i].destinatario, strlen(ap->peticiones[i].destinatario), 1, fichero);
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
 * Imprime el contenido de la estructura del servidor que almacena las peticiones
 * de amistad pendientes.
 * @param ap Puntero a la estructura.
 */
void frq__printPeticiones(struct ListaAmistadesPend* ap) {
	printf("===================================\n");
	printf(" Peticiones de amistad pendientes  \n");
	printf("===================================\n");
	if(ap->size == 0)
		printf("            < vacía >           \n");
	else {
		int i;
		for (i = 0; i < ap->size; i++) {
			printf("-----------------------------------\n");
			printf(" * emisor: %s\n", ap->peticiones[i].emisor);
			printf(" * destinatario: %s\n", ap->peticiones[i].destinatario);
		}
	}
	printf("-----------------------------------\n");
}

/**
 * Añade una petición de amistad al array de peticiones pendientes en el servidor.
 * @param ap Estructura que almacena las peticiones
 * @param emisor Emisor de la petición de amistad
 * @param destinatario Destinatario de la petición de amistad
 * @return 0 si éxito, -1 si la lista está llena, -2 petición a si mismo, -3 ya es tu amigo, -4 no existe
 */
int frq__addFriendRequest(struct ListaAmistadesPend* ap, struct ListaUsuarios* lu, struct ListasAmigos* la, char* emisor, char* destinatario) {

	printf("frq__addFriendRequest()\n");

	// Lista de amigos llena
	if (ap->size >= MAX_AMISTADES_PENDIENTES)
		return -1;

	// Cliente se manda petición a si mismo
	if (strcmp(emisor, destinatario) == 0)
		return -2;

	// El destinatario no existe
	int pos = usr__findUsuario(lu, destinatario, NULL);
	if (pos == -1)
		return -4;

	// Mandar petición a alguien que ya es nuestro amigo
	if (frd__isFriendInList(la, emisor, destinatario) == 1) // TODO: PROBABLEMENTE HAY UN BUG AQÚI!!
		return -3;

	// Ya ha mandado esta misma petición con anterioridad.
	if (frq__findFriendRequest(ap, emisor, destinatario) >= 0) {
		return -5;
	}

	// Ya hay una petición de amistad en sentido inverso
	if (frq__findFriendRequest(ap, destinatario, emisor) >= 0) {
		return -6;
	}

	// Añadir a la lista la nueva petición
	strcpy(ap->peticiones[ap->size].emisor, emisor);
	strcpy(ap->peticiones[ap->size].destinatario, destinatario);
	ap->size++;

	return 0;
}

/**
 * Borra una petición de amistad de la estructura del servidor. Se invoca desde
 * el servicio gSOAP. Si no existe dicha petición, la operación no tiene efecto.
 * @param ap Puntero a la estructura del servidor.
 * @param old Petición de amistad a borrar.
 */
void frq__delFriendRequest(struct ListaAmistadesPend* ap, char* emisor, char* receptor) {

	int i;

	// Buscamos la petición que queremos borrar
	int pos = frq__findFriendRequest(ap, emisor, receptor);

	// Si existía, la borramos (i.e. desplazar las de la derecha)
	if (pos >= 0) {
		for (i = pos; i < ap->size - 1; i++)
			frq__copyFriendRequest(&ap->peticiones[i], &ap->peticiones[i+1]);
		ap->size--;
	}
}

/**
 * Busca una petición de amistad en la estructura del servidor.
 * @param ap Puntero a la estructura
 * @param emisor Emisor de la petición
 * @param destinatario Destinatario de la petición de amistad.
 * @return El índice dentro de ap (si existe), o -1 si no existe.
 */
int frq__findFriendRequest(struct ListaAmistadesPend* ap, xsd__string emisor, xsd__string destinatario) {

	int i = 0, pos = -1;

	while (pos == -1 && i < ap->size) {
		// Si encontramos la petición en cuestión.
		if (strcmp(emisor, ap->peticiones[i].emisor) == 0 &&
			 strcmp(destinatario, ap->peticiones[i].destinatario) == 0) {
				 pos = i;
		}
		else
			i++;
	}
	return pos;
}

/**
 * Copia una petición de amistad en otra.
 * @param dst Puntero a la variable destino.
 * @param src Puntero a la variable origen.
 */
void frq__copyFriendRequest (struct PeticionAmistad* dst, struct PeticionAmistad* src) {
	strcpy(dst->emisor, src->emisor);
	strcpy(dst->destinatario, src->destinatario);
}

/**
 * Busca las peticiones de un usuario y las mete en la estructura.
 * @param username Nombre del usuario que solicita sus peticiones pendientes.
 * @param ap Puntero a la estructura del servidor con todas las peticiones.
 * @param lista Puntero a la estructura que devolverá la llamada gSOAP.
 */
void frq__retrievePendingFriendRequests(char username[IMS_MAX_NAME_SIZE], struct ListaAmistadesPend* ap, struct ListaPeticiones* lista) {

	int i;
	for (i = 0; i < ap->size; i++) {
		// Si el usuario coincide, añadirlo a la respuesta
		if(strcmp(username, ap->peticiones[i].destinatario) == 0) {

			strcat(lista->peticiones, ap->peticiones[i].emisor);	// Añadir a la lista

			// Añado siempre el espacio (en el malloc de server.c ya contemplé esto)
			strcat(lista->peticiones, " ");

			lista->size++;
		}
	}
}

/**
 * Elimina todas las peticiones de amistad pendientes relacionadas con un
 * usuario (para cuando se da de baja).
 * @param ap Puntero a la estructura del servidor con las peticiones pendientes.
 * @param username Nombre del usuario que se da de baja.
 */
void frq__delUserRelatedFriendRequests(struct ListaAmistadesPend* ap, xsd__string username) {

	int i, j;

	for (i = 0; i < ap->size; i++) {
		if (strcmp(username, ap->peticiones[i].emisor) == 0 || strcmp(username, ap->peticiones[i].destinatario) == 0) {
			ap->size--;
			// Desplazar todas las de la derecha.
			for (j = i; j < ap->size; j++)
				frq__copyFriendRequest(&ap->peticiones[j], &ap->peticiones[j+1]);
		}
	}
}

/**
 * Carga la "listas_amigos.txt" a la estructura del servidor.
 * @param la Puntero a la estructura del servidor.
 * @return 0 si éxito, -1 si error
 */
int frd__loadFriendsData(struct ListasAmigos* la) {

	FILE *fichero;
	char line[IMS_MAX_NAME_SIZE*MAX_USERS + 1];

	// Abrir el fichero
	fichero = fopen("Server/listas_amigos.txt", "rt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/listas_amigos.txt\"\n");
		return -1;
	} else
		printf("Server/listas_amigos.txt abierto correctamente.\n");

	// Leer los usuarios y sus amigos hasta fin de fichero
	int nUsr = 0;
	while (fgets(line, IMS_MAX_NAME_SIZE*MAX_USERS + 1, fichero) != NULL) {

		line[strlen(line)-1] = '\0';	// quitamos el '\n' del fichero

		// Split user & friends in the line
		int nAmigos = 0;
		char* palabra = strtok (line, " ");

		strcpy(la->listas[nUsr].usuario, palabra);		// Read username
		palabra = strtok (NULL, " ");							// Read 1st. friend

		while (palabra != NULL) {
			strcpy(la->listas[nUsr].amigos[nAmigos], palabra);	// Copy friend
			nAmigos++;
			palabra = strtok (NULL, " ");								// Read friend
		}

		la->listas[nUsr].size = nAmigos;						// Set num. friends
		nUsr++;
	}
	la->size = nUsr;

	frd__printFriendsData(la);

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
int frd__saveFriendsData(struct ListasAmigos* la) {

	printf("saveFriendsData()\n");

	// Abrir el fichero (sobrescribe)
	FILE* fichero = fopen("Server/listas_amigos.txt", "wt");

	if (fichero == NULL) {
		printf("No se encuentra el fichero \"Server/listas_amigos.txt\"\n");
		return -1;
	} else
		printf("Server/listas_amigos.txt abierto correctamente.\n");

	// Escribir los datos
	int i, j, numAmigos, numUsuarios = la->size;
	for (i = 0; i < numUsuarios; i++) {
		// write user
		fwrite(la->listas[i].usuario, strlen(la->listas[i].usuario), 1, fichero);
		numAmigos = la->listas[i].size;
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
void frd__printFriendsData(struct ListasAmigos* la) {

	printf("======================\n");
	printf(" * LISTAS DE AMIGOS * \n");
	printf("======================\n");

	if (la->size == 0)
		printf("\t < vacía > \n");
	else {
		int nUsr;
		for (nUsr = 0; nUsr < la->size; nUsr++) {
			printf(" * %s:\n", la->listas[nUsr].usuario);
			if (la->listas[nUsr].size == 0)
				printf("\t < vacía > \n");
			else {
				int nAmigos;
				for (nAmigos = 0; nAmigos < la->listas[nUsr].size; nAmigos++)
					printf("\t- %s\n", la->listas[nUsr].amigos[nAmigos]);
			}
		}
	}
	printf("----------------------\n");
}

/**
 * Da de alta a un usuario en la lista de amigos (si acaba de registrarse)
 * @param username Nombre de usuario a dar de alta
 * @param la Puntero a la estructura de datos del servidor.
 */
void frd__createFriendListEntry(struct ListasAmigos* la, char* username) {
	la->listas[la->size].size = 0;
	strcpy(la->listas[la->size].usuario, username);
	la->size++;
}

/**
 * Elimina un usuario del la lista de amistades.
 * @param username Nombre del usuario a dar de baja.
 * @param t Puntero a la estructura de datos.
 * @return 0 si éxito, -1 si error si el usuario no existía.
 */
int frd__deleteFriendListEntry(struct ListasAmigos* la, xsd__string username) {

	// Buscar si existe un usuario con el mismo nombre
	int pos = frd__findAmigosUsuario(la, username);

	// Si no existía, salimos
	if (pos < 0) return -1;

	// Eliminar el usuario de la estructura (i.e. desplazar el resto)
	la->size--;
	for (pos; pos < la->size; pos++)
		frd__copyAmigosUsuario(&la->listas[pos], &la->listas[pos+1]);

	return 0;
}

/**
 * Devuelve la lista de amigos de un usuario.
 * @param la Puntero a la estructura del servidor con todas las listas de amigos.
 * @param username Nombre del usuario de quien buscamos los amigos.
 * @param lista Parámetro de salida con la lista que nos piden.
 */
int frd__getFriendList(struct ListasAmigos* la, char* username, char* lista) {

	int i = 0, j, salir = 0;
	// Buscamos el usuario que nos piden
	while (i < la->size && salir == 0) {
		if (strcmp(username, la->listas[i].usuario) == 0) {
			// Copiamos todos los amigos del usuario en la lista
			for (j = 0; j < la->listas[i].size; j++) {
				strcat(lista, la->listas[i].amigos[j]);
				if (j < la->listas[i].size - 1)
					strcat(lista, " ");
			}
			salir = 1;
		}
		i++;
	}
	return 0;
}

/**
 * Comprueba si 'destinatario' ya está en la lista de amigos de 'emisor'
 * @param pos Posición del emisor en la lista de amigos.
 * @param destinatario Receptor de la petición de amistad.
 * @return 0 si no está, 1 si ya existe, -1 error
 */
int frd__isFriendInList(struct ListasAmigos* la, char* emisor, char* destinatario) {

	printf("frd__isFriendInList()\n");

	// Obtenemos el índice de la lista de amigos de 'emisor'
	int pos = frd__findAmigosUsuario(la, emisor);
	if (pos < 0)
		return -1;

	printf("%s tiene %d amigos.\n", emisor, la->listas[pos].size);

	// Buscamos si 'destinatario' se encuentra dentro de dicha lista
	int i = 0, existe = 0;
	while (existe == 0 && i < la->listas[pos].size) {
		if (strcmp(la->listas[pos].amigos[i], destinatario) == 0) {// Encontramos el amigo
			existe = 1;
			printf("%s es amigo de %s\n", emisor, destinatario);
		}
		else
			i++;
	}

	return existe;
}

/**
 * Añade una nueva relación de amistad a la estructura que las almacena.
 * @param persona1 Primer componente de la relación de amistad.
 * @param persona2 Segundo componente de la relación de amistad.
 * @return 0 si éxito, -1 si error
 */
int frd__addFriendRelationship(struct ListasAmigos* la, char* persona1, char* persona2) {

	int i = 0, nAmigos, salir = 0;
	while (i < la->size && salir < 2) {
		if(strcmp(persona1, la->listas[i].usuario) == 0 || strcmp(persona2, la->listas[i].usuario) == 0) {
			// Añadir al final
			nAmigos = la->listas[i].size;
			if (nAmigos == IMS_MAX_AMIGOS)
				return -1;

			// Hay que comprobar en que usuario estamos para añadir al otro
			if (strcmp(persona1, la->listas[i].usuario) == 0)
				strcpy(la->listas[i].amigos[nAmigos], persona2);
			else
				strcpy(la->listas[i].amigos[nAmigos], persona1);

			la->listas[i].size++;
			salir++;
		}
		i++;
	}
	return 0;
}

/**
 * Borra a un usuario de las listas de amigos de todo el mundo.
 * @param la Puntero a la estructura del servidor.
 * @param username Nombre del usuario a eliminar.
 */
void frd__deleteUserFromEverybodyFriendList(struct ListasAmigos* la, xsd__string username) {
	int i, j, k;
	// Para cada usuario del sistema
	for (i = 0; i < la->size; i++) {
		// Para la lista de cada usuario, buscar al que Borramos
		for (j = 0; j < la->listas[i].size; j++) {
			if (strcmp(la->listas[i].amigos[j], username) == 0) {
				// Borrar username y desplazar el resto de amigos
				la->listas[i].size--;
				for (k = j; k < la->listas[i].size; k++)
					strcpy(la->listas[i].amigos[k], la->listas[i].amigos[k+1]);
			}
		}
	}
}

/**
 * Busca la lista de amigos de un usuario concreto dentro de la estructura del
 * servidor.
 * @param la Puntero a la estructura
 * @param username Nombre del usuario a Buscar
 * @return -1 si no existe, la posición donde está si existe.
 */
int frd__findAmigosUsuario(struct ListasAmigos* la, xsd__string username) {

	int i = 0, pos = -1;

	while (pos == -1 && i < la->size) {
		if (strcmp(la->listas[i].usuario, username) == 0)
			pos = i;
		else
			i++;
	}

	return pos;
}

/**
 * Copia la estructura AmigosUsuario src en la dest.
 * @param dest Puntero a la estructura destino.
 * @param src Puntero a la estructura origen.
 */
void frd__copyAmigosUsuario (struct AmigosUsuario* dest, struct AmigosUsuario* src) {
	dest->size = src->size;
	strcpy(dest->usuario, src->usuario);
	int i;
	for (i = 0; i < src->size; i++) {
		strcpy(dest->amigos[i], src->amigos[i]);
	}
}
