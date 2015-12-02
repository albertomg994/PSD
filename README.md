# Servicio de Mensajería Instantánea - PSD

## Preparación del entorno (Ubuntu, Linux)
<ol>
  <li>
    <p>Descargar la version 2.8.24 de <a href="http://sourceforge.net/projects/gsoap2/files/">gsoap</a></p>
  </li>
  <li>
    <p>Copiar el fichero <code>gsoap_2.8.24.zip</code> en <code>$HOME</code></p>
  </li>
  <li>
    <p>Descomprimir con:</p>
    <p><pre><code> > unzipgsoap_2.8.24.zip</code></pre></p>
    <p>Se generará un directorio gsoap-2.8</p>
  </li>
  <li><p>Instalar los siguientes paquetes:</p>
      <pre><code> > sudo apt-get install bison flex byacc openssl g++ libssl-dev </code></pre>
  </li>
  <li>
    <p>Configurar la instalación:</p>
    <code>./configure</code>
  </li>
  <li>
    <p>Compilamos:</p>
    <p><code>make</code></p>
  </li>
  <li>
    <p>Creamos el directorio para la instalación:</p>
    <p><code> > mkdir $HOME/gsoap-linux_2.8.24 </code></p>
  </li>
  <li>
    <p>Instalamos:</p>
    <p><code>make install exec_prefix=$HOME/gsoap-linux_2.8.24</code></p>
    <p>Si hay probelmas de permisos:</p>
    <p><code> > sudo make install exec_prefix=$HOME/gsoap-linux_2.8.24 </code></p>
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
