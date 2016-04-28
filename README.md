# Servicio de Mensajería Instantánea - IMS

Este es uno de los proyectos elaborados para la asignatura de Programación de Sistemas Distribuidos.

Somos conscientes de lo rudimentarias e ineficientes que son las estructuras de datos utilizadas, así como el uso de memoria estática en vez de dinámica en muchos casos. Debido al poco tiempo disponible para llevar a cabo la práctica y por alejarse del objetivo principal de ésta, consideramos adecuadas estas decisiones de implementación.

Esta código ha sido escrito por:

* [Alberto Miedes Garcés](http://github.com/albertomg994)
* [Denys Sypko](http://github.com/densypko)

## Preparación del entorno (Ubuntu, Linux)
<ol>
  <li>
    <p>Descargar la version 2.8.24 de <a href="http://sourceforge.net/projects/gsoap2/files/">gsoap</a></p>
  </li>
  <li>
    <p>Copiar el fichero gsoap_2.8.24.zip en $HOME</p>
    <pre><code> > cp gsoap_2.8.24.zip $HOME </code></pre>
  </li>
  <li>
    <p>Descomprimir con:</p>
    <pre><code> > unzip gsoap_2.8.24.zip </code></pre>
    <p>Se generará un directorio gsoap-2.8 - Ya podemos borrar el .zip</p>
  </li>
  <li><p>Instalar los siguientes paquetes:</p>
      <pre><code> > sudo apt-get install bison flex byacc openssl g++ libssl-dev </code></pre>
  </li>
  <li>
    <p>Configurar la instalación (dentro del directorio gsoap-2.8):</p>
    <pre><code> > ./configure </code></pre>
  </li>
  <li>
    <p>Compilamos (dentro del directorio gsoap-2.8):</p>
    <pre><code> > make </code></pre>
  </li>
  <li>
    <p>Creamos el directorio para la instalación:</p>
    <pre><code> > mkdir $HOME/gsoap-linux_2.8.24 </code></pre>
  </li>
  <li>
    <p>Instalamos:</p>
    <pre><code> > make install exec_prefix=$HOME/gsoap-linux_2.8.24 </code></pre>
    <p>Si hay probelmas de permisos:</p>
    <pre><code> > sudo make install exec_prefix=$HOME/gsoap-linux_2.8.24 </code></pre>
  </li>
  
  <li>
    <p>Incluimos en $HOME/.bashrc</p>
    <pre><code>
      > export GSOAP_HOME=$HOME/gsoap-linux_2.8.24
      > export GSOAP_LIB=${GSOAP_HOME}/lib
      > export GSOAP_INCLUDE=${GSOAP_HOME}/include
      > PATH=$PATH:$GSOAP_HOME/bin
    </code></pre>
  </li>
  
</ol>

## Compilación y ejecución del servidor
  <ol>
    <li>
      <p>Generamos los stubs a partir de la interfaz remota (fichero: ims.h):</p>
      <pre><code> $ soapcpp2 -c -S ims.h </code></pre>
    </li>
    <li>
      <p>Compilamos</p>
      <pre><code> > make server </code></pre>
      <p> Generará un ejecutable llamado 'servidor' </p>
    </li>
    <li>
      <p>Ejecución:</p>
      <pre><code> > ./server [puerto] </code></pre>
      <p> Por ejemplo: </p>
      <pre><code> > ./server 5000 </code></pre>
    </li>
  </ol>
## Compilación y ejecución del cliente
  <ol>
    <li>
      <p>Generamos los stubs a partir de la interfaz remota (fichero: ims.h):</p>
      <pre><code> $ soapcpp2 -c -C ims.h </code></pre>
    </li>
    <li>
      <p>Compilamos</p>
      <pre><code> > make client </code></pre>
      <p> Generará un ejecutable llamado 'cliente' </p>
    </li>
    <li>
      <p>Ejecución:</p>
      <pre><code> > ./client [URL + puerto] </code></pre>
      <p> Por ejemplo: </p>
      <pre><code> > ./client http://192.168.0.35:5000 </code></pre>
    </li>
  </ol>
