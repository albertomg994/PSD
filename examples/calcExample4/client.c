#define DEBUG_MODE 1

#include "soapH.h"
#include "calcExample.nsmap"

int main(int argc, char **argv){
  struct soap soap;
  char *serverURL;
  int numA, numB, res;
  char operation;

  	if (argc != 5) {
    	   printf("Usage: %s http://server:port numA [+|-|m|/] numB\n",argv[0]);
    	   exit(0);
  	}

	// Init gSOAP environment
  	soap_init(&soap);
  	
	// Obtain server address
	serverURL = argv[1];
	
	// Obtain operation and numbers
	operation =(char) argv[3][0];
	numA = atoi (argv[2]);
	numB = atoi (argv[4]);
	
	// Debug?
	if (DEBUG_MODE){
		printf ("Server: %s\n", serverURL);
		printf ("Operation: %d %c %d\n", numA, operation, numB);
	}

	// What is the operation to be executed?
	switch (operation) {
  		case '+':
    		soap_call_calcns__add(&soap,serverURL, "", numA, numB, &res);
    	break;

		case '-':
    		soap_call_calcns__subs(&soap,serverURL, "", numA, numB, &res);
    	break;

		case 'm':
    		soap_call_calcns__mult(&soap,serverURL, "", numA, numB, &res);
    	break;

		case '/':
    		soap_call_calcns__div(&soap,serverURL, "", numA, numB, &res);
    	break;
	}
	  		
 	// Check for errors...
  	if (soap.error) {
      		soap_print_fault(&soap, stderr); exit(1);
  	}


  	printf("Result is = %d \n", res);
  	
	// Clean the environment
  	soap_end(&soap);
  	soap_done(&soap);

  return 0;
}
