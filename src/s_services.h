// Variables definidas en server.c
extern struct ListaUsuarios lu;
extern struct ListaAmistadesPend ap;
extern struct ListasAmigos la;
extern struct ListasMensajes lmsg;

// Funciones definidas en server.c
extern void renewSession(char* username);

/**
 * Servicio gSOAP para enviar un mensaje a un usuario.
 * @param soap Contexto gSOAP.
 * @param myMessage Estructura con el mensaje a enviar.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__sendMessage (struct soap *soap, struct Message2 myMessage, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(myMessage.emisor) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(myMessage.emisor);

	int i = 0, j, salir = 0;
	// Comprobar si el emisor es el amigo del receptor.
	while (i < la.size && salir == 0) {
		if (strcmp(myMessage.emisor, la.listas[i].usuario) == 0) {
			for (j = 0; j < la.listas[i].size; j++) {
				if(strcmp(myMessage.receptor,la.listas[i].amigos[j]) == 0)
					salir = 1;
			}
		}
		i++;
	}

	// Si el usuario al que queremos enviar es nuestro amigo
	if(salir == 1){
		sendCheck(&lmsg, &myMessage);
		if (sendMessage (myMessage) < 0) {
			result->code = -500;
			strcpy(result->msg, "Error del servidor.");
		} else
			result->code = 0;
	}
	// Si no...
	else{
		result->code = -300;
		strcpy(result->msg, "El usuario no es tu amigo.");
	}

	return SOAP_OK;
}

/**
 * Servicio gSOAP para obtener la lista de mensajes de un usuario.
 * @param soap Contexto gSOAP.
 * @param username Nombre del usuario que desea obtener sus mensajes.
 * @param result Lista de mensajes del usuario.
 */
int ims__receiveMessage (struct soap* soap, char* username, struct ListaMensajes* result){

	int error;
	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(username);

	error = checkMessage(username,&lmsg);
	if (error!=0) {
		printf("ERROR: al chequearMensajesª\n");
	}
	error = receiveMessage(username,result);
	if (error!=0) {
		printf("ERROR: al recibirMensajesª\n");
	}

	return SOAP_OK;
}

/**
 * Servicio gSOAP para obtener checkear la recepción de los mensajes enviados.
 * @param soap Contexto gSOAP.
 * @param username Nombre del usuario que desea obtener sus mensajes.
 * @param result ?
 */
int ims__consultarEntrega(struct soap *soap, char* username, struct ListaMensajes* result){

	int error;
	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(username);

	error = consultEntrega(username,&lmsg,result);

	if(error!=0){
		printf("ERROR: al consultarMensajesª\n");
	}

	return SOAP_OK;
}

/**
 * Servicio gSOAP para dar de alta a un nuevo usuario.
 * @param soap Contexto gSOAP.
 * @param username Nombre del usuario a dar de alta.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__darAlta (struct soap *soap, char* username, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	int disponible = usr__isUsernameAvailable(&lu, username);

	if (disponible) {
		result->code = 0;
		strcpy(result->msg, "Usuario registrado correctamente.");
		frd__createFriendListEntry(&la, username);
	}
	else {
		result->code = -2;
		strcpy(result->msg, "ERROR (-2): Este nombre no está disponible.");
	}

	return SOAP_OK;
}

/**
 * Servicio gSOAP para dar de baja a un nuevo usuario.
 * @param soap Contexto gSOAP.
 * @param username Nombre del usuario a dar de baja.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__darBaja(struct soap *soap, char* username, struct ResultMsg* result){

	result->msg = malloc(IMS_MAX_MSG_SIZE);
	int res = 0;

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(username);

	// 1. Borrar de la BD de usuarios
	int pos = usr__findUsuario(&lu, username, NULL);
	if (pos < 0) res = -1;
	else lu.usuarios[pos].baja = 1;

	// 2. Borrar de la estructura de amistades
	if (res == 0) res = frd__deleteFriendListEntry(&la, username); // 0 éxito, -1 err.

	// 3. Borrar también de las listas de amigos de otras personas.
	frd__deleteUserFromEverybodyFriendList(&la, username);

	// 3. Borrar peticiones de amistad (en cualquier dirección) pendientes
	if (res == 0) frq__delUserRelatedFriendRequests(&ap, username);

	// 4. Borrar mensajes enviados y la carpte del usuario.
	if (res == 0) msg_delUserMessage(&lmsg, username);

	result->code = res;

	return SOAP_OK;
}

/**
 * Marca la sesión de un usuario como iniciada ('connected = 1').
 * @param soap Contexto gSOAP
 * @param username Nombre del usuario que hace login.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__login (struct soap *soap, char* username, struct ResultMsg *result) {

	int existe = 0, i = 0;
	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Buscar si existe un usuario con este nombre o si ya tiene una sesión iniciada.
	struct Usuario aux;
	int pos = usr__findUsuario(&lu, username, &aux);

	if (pos < 0)							// No existe el usr__findUsuario
		result->code = -1;
	else if (aux.baja == 1)				// Existía pero se dio de baja
		result->code = -1;
	else if (aux.connected == 1)		// Ya tiene una sesión iniciada
		result->code = -2;
	else {										// Login correcto
		result->code = 0;
		lu.usuarios[pos].connected = 1;
	}

	// Rellenamos la estructura que se devuelve al cliente
	if (result->code == 0)
		strcpy(result->msg, "Login correcto.");
	else if (result->code == -1)
		strcpy(result->msg, "ERROR (-1): El usuario indicado no existe.");
	else if (result->code == -2)
		strcpy(result->msg, "ERROR (-2): El usuario indicado ya tiene una sesión iniciada.");

	return SOAP_OK;
}

/**
 * Marca la sesión de un usuario como apagada ('connected = 0').
 * @param soap Contexto gSOAP
 * @param username Nombre del usuario que hace logout.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__logout (struct soap *soap, char* username, struct ResultMsg *result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Buscar si existe un usuario con el mismo nombre
	int pos = usr__findUsuario(&lu, username, NULL);

	if (pos >= 0) {
		lu.usuarios[pos].connected = 0;
		result->code = 0;
		strcpy(result->msg, "Éxito.");
	} else {
		result->code = -201;
		strcpy(result->msg, "El usuario no existe.");
	}

	return SOAP_OK;
}

/**
 * Servicio gSOAP para enviar peticiones de amistad.
 */
int ims__sendFriendRequest (struct soap *soap, struct IMS_PeticionAmistad p, struct ResultMsg* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(p.emisor) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(p.emisor);

	result->code = frq__addFriendRequest(&ap, &lu, &la, p.emisor, p.receptor);

	if (result->code == 0)
		strcpy(result->msg, "Petición enviada correctamente.");
	else if (result->code == -1)
		strcpy(result->msg, "ERROR (-1): La lista de peticiones de amistad del servidor está llena. Inténtalo de nuevo más tarde.");
	else if (result->code == -2)
		strcpy(result->msg, "ERROR (-2): No puedes enviarte una petición a ti mismo.");
	else if (result->code == -3)
		strcpy(result->msg, "ERROR (-3): Este usuario ya es tu amigo.");
	else if (result->code == -4)
		strcpy(result->msg, "ERROR (-4): Este usuario no existe.");
	else if (result->code == -5)
		strcpy(result->msg, "ERROR (-5): Ya has mandado una petición de amistad a este usuario.");
	else if (result->code == -6)
		strcpy(result->msg, "ERROR (-6): Existe una petición equivalente en tu bandeja de entrada.");

	return SOAP_OK;
}

/**
 * Servicio gSOAP para pedir las peticiones de amistad pendientes.
 * @param soap Contexto gSOAP.
 * @param username Nombre de usuario que invoca la llamada.
 * @param lista Estructura donde se devuelve la lista de peticiones pendientes.
 */
int ims__getAllFriendRequests (struct soap* soap, char* username, struct ListaPeticiones* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(username);

	// Variable para la respuesta
	result->size = 0;
	result->peticiones = (xsd__string) malloc(MAX_AMISTADES_PENDIENTES*IMS_MAX_NAME_SIZE + 1);
	result->peticiones[0] = '\0'; // Si no lo ponemos, riesgo de violación de segmento.

	// Rellenar la estructura
	frq__retrievePendingFriendRequests(username, &ap, result);

	// Resultado de la llamada
	result->code = 0;
	strcpy(result->msg, "Éxito.");

	return SOAP_OK;
}

/**
 * Servicio gSOAP para aceptar o denegar una petición de amistad de un usuario
 * hacia otro.
 * @param soap Contexto gSOAP.
 * @param rp Estructura con la respuesta a la petición de amistad.
 * @param result Resultado de la llamada al servicio gSOAP.
 */
int ims__answerFriendRequest (struct soap* soap, struct RespuestaPeticionAmistad rp, struct ResultMsg* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(rp.receptor) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(rp.receptor);

	/* TODO: cuando esté implementada la parte de mensajería, deberá colocar
	 un mensaje para emisor y receptor informando del resultado de la petición. */

	if(rp.aceptada == 1) {

		result->code = frd__addFriendRelationship(&la, rp.emisor, rp.receptor);

		// Rellenar resultado de la llamada
		if (result->code < 0)
			strcpy(result->msg, "Error.");
		else
			strcpy(result->msg, "Éxito.");
	}

	// Borramos la petición de amistad de la estructura en memoria
	frq__delFriendRequest(&ap, rp.emisor, rp.receptor);

	return SOAP_OK;
}

/**
 * Devuelve la lista de amigos de un usuario.
 * @param soap Contexto gSOAP.
 * @param username Nombre del usuario.
 * @result Estructura con la lista de amigos.
 */
int ims__getFriendList(struct soap* soap, char* username,  struct ListaAmigos* result) {

	result->msg = malloc(IMS_MAX_MSG_SIZE);

	printf("Llega una petición de %s. Comprobamos si su sesión ha expirado.\n", username);

	// Comprobamos si la sesión ha expirado
	if (isSessionExpired(username) == 1) {
		result->code = -200;
		strcpy(result->msg, "ERROR (-200): No tienes sesión abierta.");
		return SOAP_OK;
	}

	// Reactivar la sesión del usuario
	renewSession(username);

	result->amigos = malloc(IMS_MAX_NAME_SIZE*IMS_MAX_AMIGOS + 1);
	result->amigos[0] = '\0'; // Si no lo ponemos, violación de segmento al strcat()

	frd__getFriendList(&la, username, result->amigos);

	return SOAP_OK;
}
