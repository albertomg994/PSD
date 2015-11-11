# PSD
Preparación del entorno:
  1. Descargar la version 2.8.24 de gsoap
  2. Descomprimir en el $HOME del usuario que va a utilizarlo.
  3. Instalar los siguientes paquetes:
        $ sudo apt-get install bison flex byacc openssl g++ libssl-dev

Pasos de ejecución:
  1. Generamos los stubs a partir de la interfaz remota (fichero: interfaz.h):
    - $ soapcpp2 -c calc.h.
  2. Añadir a nuestro .bashrc
    - export GSOAP_HOME=$HOME/gsoap-linux_2.8.24
    - export GSOAP_LIB=${GSOAP_HOME}/lib
    - export GSOAP_INCLUDE=${GSOAP_HOME}/include
    -  PATH=$PATH:$GSOAP_HOME/bin
  3. Compilamos
    - make --> genera a ./cliente y al ./server
  4. Comunicacion con las maquinas del laboratorio
    - http://Pto<num>.fdi.ucm.es:"PUERTO"
