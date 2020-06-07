## Nota
Este documento contiene los mensajes del protocolo de comunicación (el cual es un protocolo de la capa de aplicación) que podrán ser utilizados por el cliente y el servidor para comunicarse entre ellos.

## Mensajes del protocolo de comunicación
Un cliente puede realizar las siguientes solicitudes a un servidor:  
- Encender o apagar las luces.  
- Activar o desactivar el riego automático.
- Solicitar una imagen al portero eléctrico.
- Contestar una llamada del portero eléctrico.

Para esto se dispondrá de los siguientes comandos:  
- turnon para encender las luces.
- turnoff para apagar las luces.
- ienable para activar el riego automático.
- idisable para desactivar el riego automático.
- rimage para solicitar una imagen al portero eléctrico.
- takecall para contestar una llamada del portero eléctrico.

**turnon** retorna el mensaje "light on".  
**turnoff** retorna el mensaje "light off".  
**ienable** retorna el mensaje "irrigation enabled".  
**idisable** retorna el mensaje "irrigation disabled".  
**rimage** retorna el mensaje "image sended".  
**takecall** retorna el mensaje "taken call".  
**ping** retorna el mensaje "I'm listening".
