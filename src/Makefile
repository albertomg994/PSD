SSL_LIBS=-lssl -lcrypto
SSL_FLAGS=-DWITH_OPENSSL

#SSL_LIBS=
#SSL_FLAGS=

all:	client server

client:	client.c externo.o externo.h
	gcc $(SSL_FLAGS) -o client client.c soapC.c soapClient.c externo.o -I$(GSOAP_INCLUDE) -lgsoap $(SSL_LIBS) -L$(GSOAP_LIB)

server:	server.c externo.o externo.h s_usuarios.o s_usuarios.h s_mensajes.o s_mensajes.h
	gcc $(SSL_FLAGS) -o server server.c soapC.c soapServer.c externo.o s_usuarios.o s_mensajes.o -I$(GSOAP_INCLUDE) -lgsoap $(SSL_LIBS) -L$(GSOAP_LIB)

clean:	
	rm client server *.o *.xml *.nsmap *.wsdl *.xsd soapStub.h soapServerLib.c soapH.h soapServer.c soapClientLib.c soapClient.c soapC.c
