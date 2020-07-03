## Nota
Este documento contiene los mensajes del protocolo de comunicación (el cual es un protocolo de la capa de aplicación) que podrán ser utilizados por el cliente y el servidor para comunicarse entre ellos.

## Mensajes del protocolo de comunicación
Un cliente puede realizar las siguientes solicitudes al servidor:  
- Encender las luces.
- Apagar las luces.  
- Activar el riego automático.
- Desactivar el riego automático.
- Solicitar una imagen al portero eléctrico.
- Hacer una llamada al portero eléctrico.  
- Contestar una llamada del portero eléctrico.
- Enviar audio al portero eléctrico.  
- Recibir audio del portero eléctrico.  
- Solicitar ID (identificador).  
- Desconectarse del servidor.
- Comprobar si el servidor está en funcionamiento.

Para esto se dispondrá de los siguientes comandos:  
- turnon para encender las luces.
- turnoff para apagar las luces.
- ienable para activar el riego automático.
- idisable para desactivar el riego automático.
- rimage para solicitar una imagen al portero eléctrico.
- callto para llamar al portero eléctrico haciendo uso de un número de departamento.
- takecall para contestar una llamada del portero eléctrico.
- sendaudio para enviar audio al portero eléctrico haciendo uso de un número de departamento.
- recaudio para recibir audio del portero eléctrico.
- id para saber el ID propio.
- exit para desconectarse del servidor.
- ping para comprobar si el servidor está en funcionamiento.

**turnon** retorna al emisor el mensaje "light on".  
**turnoff** retorna al emisor el mensaje "light off".  
**ienable** retorna al emisor el mensaje "irrigation enabled".  
**idisable** retorna al emisor el mensaje "irrigation disabled".  
**exit** retorna el mensaje "successful desconnection".  
**ping** retorna el mensaje "I'm listening".  
**id** retorna el ID del cliente.

**rimage** permite a un cliente recibir una imagen de parte del servidor. En caso de que el servidor tenga una imagen para transmitir, la misma se creará en el directorio del programa cliente y en la pantalla del cliente deberá verse el mensaje "File transfer complete" al terminarse la transferencia de dicha imagen. En caso contrario, no se creará una imagen en el directorio del programa cliente y en la pantalla del cliente deberá verse el mensaje "nonexisting_file".

**callto** utiliza como argumento el número de departamento del cliente al que se quiere llamar. En el caso en el cual existe el número de departamento y el cliente solicitado esté conectado retorna al emisor el mensaje "call sent", mientras que al receptor se le envía un **aviso** con el siguiente mensaje "the client of the department (number) is calling you".  

**takecall** retorna al receptor el mensaje "call (number) taken" en caso de que haya un cliente llamando al portero eléctrico, en caso contrario retorna el mensaje "no incoming calls".  

**sendaudio** utiliza como argumento el número de departamento del cliente al que se quiere enviar audio. En el caso en el cual existe el número de departamento y el cliente solicitado esté conectado retorna al emisor el mensaje "audio sent", mientras que al receptor se le envía un **aviso** con el siguiente mensaje "the client of the department (number) sent you an audio".  

**recaudio** retorna al receptor el mensaje "audio (number) received" en caso de que haya un audio disponible para escuchar, en caso contrario retorna el mensaje "there are no audios to listen".  

Para los comandos callto y sendaudio:
- En el caso en el cual no existe el número de departamento se devuelve el mensaje "department number does not exist".
- En el caso de que sí existe el número de departamento y el cliente solicitado no esté conectado se devuelve el mensaje "the requested client is not connected".
- En el caso en el cual se utiliza como argumento el número de departamento del emisor se devuelve el mensaje "Can't send a message to the same department".
