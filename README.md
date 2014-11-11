Tarea 3 Programación Sistemas 2014-2
================================
Emulador de retardos y pérdidas de paquetes en transferencias UDP.
Instrucciones de compilación
--
 Para compilar todos los programas necesarios a utilizar:

    $ make all

Instrucciones de ejecución
-
 Para este caso se probará el emulador con el cliente y servidor "echo".
 
1. En un terminal montar el servidor 

    $ ./server puerto-escucha

2. En otro terminal correr el cliente

    $ ./client ip-proxy puerto-proxy

3. En un tercer terminal ejecutar

   $ ./erp_udp retardo_promedio variación_retardo porcentaje_pérdida puerto_local [host_remoto] puerto_remoto 
