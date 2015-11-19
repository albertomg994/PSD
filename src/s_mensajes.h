#ifndef S_MENSAJES_H
#define S_MENSAJES_H



void s_mensajes();
int sendMessage (struct Message2 myMessage);
int receiveMessage(char* username,struct ListaMensajes* result);
#endif
