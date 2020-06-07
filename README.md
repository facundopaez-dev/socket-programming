# Programación de sockets
### Objetivos
- Especificar y desarrollar un protocolo a nivel de aplicación.
- Utilizar el modelo cliente-servidor en el desarrollo de aplicaciones.
- Utilizar los servicios provistos por la capa de transporte.

### Enunciado
Un edificio de departamentos cuenta con un portero eléctrico en el acceso que permite transmitir tanto audio como vídeo y un sistema de control de luminarias y de riego automático en base al horario y humedad del suelo. Implementar un sistema que simule este escenario mediante un esquema cliente servidor. La programación de cada evento y su respuesta se establece mediante un archivo de configuración.

**Desarrollar un protocolo de comunicación que permita esta comunicación. Los servicios provistos por el servidor serán:**  
- Encender/apagar las luces.  
- Activar/desactivar el riego automático.  
- Enviar un aviso si alguien llama al portero eléctrico.  
- Enviar una imagen si alguien llama al portero eléctrico.  
- Transmitir/recibir audio para contestar el portero eléctrico.

Los clientes podrán:  
- Encender o apagar las luces.  
- Activar o desactivar el riego automático.  
- Recibir un mensaje si alguien llama al portero eléctrico.  
- Solicitar una imagen al portero eléctrico.  
- Contestar una llamada del portero eléctrico.

Documentar el protocolo desarrollado teniendo en cuenta la estructura de una propuesta
de RFC.

**Programar en lenguaje C el protocolo de comunicación definido. Implementar clientes y el servidor.**

**Documentar las decisiones de diseño tomadas para llegar a la implementación que se
entrega.**
