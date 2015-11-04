#include "soapH.h"
#include "imsService.nsmap"

#define DEBUG_MODE 1

int main(int argc, char **argv){

  struct soap soap;
  struct Message myMsgA, myMsgB;
  char *serverURL;
  char *msg;
  int res;

	// Usage
  	if (argc != 3) {
    	   printf("Usage: %s http://server:port message\n",argv[0]);
    	   exit(0);
  	}

	// Init gSOAP environment
  	soap_init(&soap);
  	
	// Obtain server address
	serverURL = argv[1];
	
	// Obtain message
	msg = argv[2];
	
	// Debug?
	if (DEBUG_MODE){
		printf ("Server to be used by client: %s\n", serverURL);
		printf ("Message to be sent by client: %s\n", msg);
	}

	// Allocate space for the message field of myMsgA then copy into it
	myMsgA.msg = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMsgA.msg, 0, IMS_MAX_MSG_SIZE); 
	strcpy (myMsgA.msg, msg);

	// Allocate space for the name field of myMsgA then copy into it
	myMsgA.name = (xsd__string) malloc (IMS_MAX_NAME_SIZE);
	// Not necessary with strcpy since uses null-terminated strings
	// memset(myMsgA.name, 0, IMS_MAX_NAME_SIZE);
	strcpy (myMsgA.name, "aClient");

	// Send the contents of myMsgA to the server
    	soap_call_ims__sendMessage (&soap, serverURL, "", myMsgA, &res);
	  		
 	// Check for errors...
  	if (soap.error) {
      	soap_print_fault(&soap, stderr); 
		exit(1);
  	}


	// Receive a Message struct from the server into myMsgB
    	soap_call_ims__receiveMessage (&soap, serverURL, "", &myMsgB);

	// Check for errors...
  	if (soap.error) {
      	soap_print_fault(&soap, stderr); 
		exit(1);
  	}
	else
		printf ("Received from server: \n\tusername: %s \n\tmsg: %s\n", myMsgB.name, myMsgB.msg);    

  	
	// Probamos a mandar una petición de alta
	// int ims__darAlta (char username [IMS_MAX_USR_SIZE]);
	// int ims__sendMessage (struct Message myMessage, int *result);
	struct MensajeAlta peticion;
	peticion.username = (xsd__string) malloc(IMS_MAX_USR_SIZE);
	strcpy(peticion.username, "amiedes");
    	soap_call_ims__darAlta (&soap, serverURL, "", peticion, &res);
	  		
 	// Check for errors...
  	if (soap.error) {
      	soap_print_fault(&soap, stderr); 
		exit(1);
  	}


	// Clean the environment
  	soap_end(&soap);
  	soap_done(&soap);

  return 0;
}
