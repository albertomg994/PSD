# Servicio de Mensajería Instantánea - PSD

## Preparación del entorno (Ubuntu, Linux)
<ol>
  <li><p>Descargar la version 2.8.24 de gsoap</p></li>
  <li><p>Descomprimir en el $HOME del usuario que va a utilizarlo.</p></li>
  <li><p>Instalar los siguientes paquetes:</p>
      <pre><code> $ sudo apt-get install bison flex byacc openssl g++ libssl-dev </code></pre>
  </li>
</ol>
## Compilación
<ol>
  <li>
    <p>Generamos los stubs a partir de la interfaz remota (fichero: interfaz.h):</p>
    <pre><code> $ soapcpp2 -c calc.h. </code></pre>
  </li>
  <li>Añadir a nuestro .bashrc
    - export GSOAP_HOME=$HOME/gsoap-linux_2.8.24
    - export GSOAP_LIB=${GSOAP_HOME}/lib
    - export GSOAP_INCLUDE=${GSOAP_HOME}/include
    -  PATH=$PATH:$GSOAP_HOME/bin
  </li>
  <li>
    <p>Compilamos</p>
    <pre><code> make --> genera a ./cliente y al ./server </code></pre>
  </li>
  <li>
    <p>Comunicacion con las maquinas del laboratorio</p>
    <p> - http://Pto<num>.fdi.ucm.es:"PUERTO"</p>
  </li>
</ol>
## Ejecución
