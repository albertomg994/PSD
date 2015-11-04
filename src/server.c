
#include "soapH.h"
#include "imsService.nsmap"

int main(int argc, char **argv){ 

  int m, s;
  struct soap soap;

  	if (argc < 2) {
    	printf("Usage: %s <port>\n",argv[0]); 
		exit(-1); 
  	}

	// Init environment
  	soap_init(&soap);

	// Bind to the specified port	
	m = soap_bind(&soap, NULL, atoi(argv[1]), 100);

	// Check result of binding		
	if (m < 0) {
  		soap_print_fault(&soap, stderr); exit(-1); 
	}

	// Listen to next connection
	while (1) { 

		// accept
	  	s = soap_accept(&soap);    

	  	if (s < 0) {
			soap_print_fault(&soap, stderr); exit(-1);
	  	}

		// Execute invoked operation
	  	soap_serve(&soap);

		// Clean up!
	  	soap_end(&soap);
	}

  return 0;
}

int ims__sendMessage (struct soap *soap, struct Message myMessage, int *result){

	printf ("Received by server: \n\tusername:%s \n\tmsg:%s\n", myMessage.name, myMessage.msg);
	return SOAP_OK;
}


int ims__receiveMessage (struct soap *soap, struct Message *myMessage){

	// Allocate space for the message field of the myMessage struct then copy it
	myMessage->msg = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMessage->msg, 0, IMS_MAX_MSG_SIZE);
	strcpy (myMessage->msg, "Invoking the remote function receiveMessage simply retrieves this standard message from the server"); // always same msg

	// Allocate space for the name field of the myMessage struct then copy it
	myMessage->name = (xsd__string) malloc (IMS_MAX_NAME_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMessage->name, 0, IMS_MAX_NAME_SIZE);  
	strcpy(myMessage->name, "aServer");	

	return SOAP_OK;
}


int ims__darAlta (struct soap *soap, struct MensajeAlta msg, int *result) {

	printf("Recibido nombre de usuario: %s\n", msg.username);

	return SOAP_OK;
}


