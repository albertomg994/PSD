#ifndef S_MENSAJES_H
#define S_MENSAJES_H

// -----------------------------------------------------------------------------
// Tipos, constantes y estructuras
// -----------------------------------------------------------------------------
struct Mensaje {
   char emisor [IMS_MAX_NAME_SIZE];
   char receptor [IMS_MAX_NAME_SIZE];
   char msg [IMS_MAX_MSG_SIZE];
};

struct ListasMensajes{
   int size;
   struct Mensaje lista [MAX_MENSAJES];
};
// -----------------------------------------------------------------------------
// Cabeceras de funciones
// -----------------------------------------------------------------------------
void s_mensajes();
int msg__loadMensajesEnviados(struct ListasMensajes* lmsg);
int msg__saveMensajesEnviados(struct ListasMensajes* lmsg);

int sendMessage (struct Message2 myMessage);
int receiveMessage(char* username,struct ListaMensajes* result);
int checkMessage(char* username,struct ListasMensajes* lmsg);
int consultEntrega(char* username,struct ListasMensajes* lmsg,struct ListaMensajes* result);
int sendCheck(struct ListasMensajes* lmsg,struct Message2* myMessage);
#endif
