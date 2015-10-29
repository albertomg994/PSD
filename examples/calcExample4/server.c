
#include "soapH.h"
#include "calcExample.nsmap"

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

int calcns__add (struct soap *soap, int a, int b, int *res){  
  *res = a + b;
  return SOAP_OK;
}

int calcns__subs (struct soap *soap, int a, int b, int *res){  
  *res = a - b;
  return SOAP_OK;
}

int calcns__mult (struct soap *soap, int a, int b, int *res){  
  *res = a * b;
  return SOAP_OK;
}

int calcns__div (struct soap *soap, int a, int b, int *res){  
  *res = a / b;
  return SOAP_OK;
}

