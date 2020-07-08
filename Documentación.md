## Documentación
### Programa cliente
El programa cliente en su ejecución crea tres hilos. Uno de ellos se encarga de hacer las peticiones al servidor con el cual está conectado el cliente, un segundo hilo se encarga de recibir, haciendo uso del protocolo TCP, las respuestas de parte del servidor y el último se encarga de recibir, haciendo uso del protocolo UDP, las respuestas del servidor.

El hilo que hace uso del protocolo UDP no se utiliza debido a que no se logro implementar la funcionalidad del chat (envío y recepción de audio), la cual es la única funcionalidad que está pensada para que haga uso de UDP.

Es importante destacar que se usa el protocolo TCP para el encendido y apagado de las luces, la activación y desactivación del riego automático, la transmisión de una imagen, la desconexión, el envío y recepción de llamadas, la verificación de si el servidor está en funcionamiento y la consulta por el ID del departamento.

En la ejecución del programa cliente se crean dos sockets, uno de ellos es TCP y el otro es UDP. El descriptor de archivo del socket TCP se pasa al hilo que espera respuestas, de parte del servidor, en TCP, mientras que el descriptor de archivo del socket UDP se pasa al hilo que espera respuestas, también de parte del servidor, en UDP. Se tiene que recordar que éste último no se lo utiliza porque no se pudo implementar el chat (envío y recepción de audio), siendo esta funcionalidad la única que está pensada para hacer uso del protocolo UDP.

Un detalle importante a tener en cuenta sobre el programa cliente es que éste antepone para cada comando el texto "tcp " o "udp " dependiendo del comando que ingrese el usuario por el teclado. Luego, el servidor cuando recibe la petición del cliente toma el modo de uso (TCP o UDP), el nombre del comando y el argumento del comando.

Por ejemplo, si el usuario ingresa el comando turnon, el programa cliente crea la siguiente cadena de caracteres "tcp turnon" y la envía al servidor, el cual almacena "tcp" en buffer y "turnon" en otro buffer, es decir, descompone la cadena enviada (petición) por el programa cliente. En este caso, se antepuso "tcp " al comando turnon porque éste usa el protocolo TCP.

Si el comando sendaudio (chat) estuviese implementado, se le antepondría "udp " debido a que sendaudio está pensado para que haga uso del protocolo UDP.

En el caso de ejecutar los comandos turnon, turnoff, ienable, idisable, exit, ping, id, rimage y takecall, seguidos de uno o varios espacios y cualquier palabra, tal ejecución deberá ser satisfactoria (es decir, el servidor dará la respuesta expresada en el protocolo de comunicación) ya que el servidor ignora lo que venga después del nombre cada uno de estos comandos que no utilizan argumentos para su ejecución en el servidor.

#### Dependencias
El programa cliente depende de los siguientes archivos:
- util.c, el cual está dentro de la carpeta **client**. Este archivo a su vez depende del archivo utildefinitions.h. El programa cliente tiene su propio archivo util.c, el cual no lo comparte con el programa servidor.
- utildefinitions.h, el cual está en la ruta **client/headers**.
- answers.h, el cual está en la ruta **concurrent servers/headers**.
- confirmations.h, el cual está en la ruta **concurrent servers/headers**.
- modes.h, el cual está en la ruta **concurrent servers/headers**.
- namecommands.h, el cual está en la ruta **concurrent servers/headers**
- notices.h, el cual está en la ruta **concurrent servers/headers**.

### Programa servidor
El programa servidor en su ejecución crea un socket TCP y uno UDP para cada cliente que se conecta a él, y crea un hilo para cada cliente a través del cual maneja las peticiones de ese cliente.

Éste hilo envía las respuestas a su cliente haciendo uso del protocolo TCP, porque como bien se dijo anteriormente, al no haberse podido implementar el chat (envío y recepción de audio) no hay funcionalidad que haga uso del protocolo UDP, siendo el chat la única funcionalidad que está pensada para que haga uso de dicho protocolo.

El descriptor de archivo del socket TCP y del socket UDP se pasa al hilo creado para cada cliente, y como se dijo anteriormente, dicho hilo es el encargado de manejar las peticiones de su cliente.

A grandes rasgos, el programa servidor se puede dividir en dos partes: Una de ellas consiste en un bucle infinito que tiene un socket TCP a través del cual el servidor espera a la conexión de los clientes (esto sucede en el hilo principal) y la otra consiste en un bucle infinito dentro de cada hilo creado para cada cliente que se conecta al servidor, siendo cada hilo el encargado de responder las peticiones de su cliente.

#### Dependencias
El programa servidor depende de los siguientes archivos:
- commands.c, el cual está en la carpeta **concurrent servers**. Este archivo a su vez depende del archivo commandsdefinitions.h.
- util.c, el cual está en la carpeta **concurrent servers**. Este archivo a su vez depende del archivo utildefinitions.h. El programa servidor tiene su propio archivo util.c, el cual no lo comparte con el programa cliente.
- answers.h, el cual está en la ruta **concurrent servers/headers**.
- commandsdefinitions.h, el cual está en la ruta **concurrent servers/headers**.
- confirmations.h, el cual está en la ruta **concurrent servers/headers**.
- modes.h, el cual está en la ruta **concurrent servers/headers**.
- namecommands.h, el cual está en la ruta **concurrent servers/headers**.
- notices.h, el cual está en la ruta **concurrent servers/headers**.
- utildefinitions.h, el cual está en la ruta **concurrent servers/headers**.

### Archivos auxiliares
#### Programa cliente
El archivo utildefinitions.h contiene la firma de las funciones que están implementadas en el archivo util.c.

#### Programa servidor
El archivo utildefinitions.h contiene la firma de las funciones que están implementadas en el archivo util.c.

El archivo commandsdefinitions.h contiene la firma de las funciones que están implementadas en el archivo commands.c.

El archivo answer.h contiene las respuestas del servidor a los clientes. Respuestas como "light on" en el caso de que un cliente ejecute el comando turnon, por ejemplo.

El archivo confirmations.h contiene las respuestas del servidor a los clientes para el caso en el que se pide una imagen al servidor, esto se hace con el comando rimage.

El archivo modes.h contiene las cadenas de caracteres que le permiten tanto al programa cliente como al programa servidor indicar qué protocolo van a utilizar para comunicarse.

El archivo namecommands.h contiene las cadenas de caracteres que representan a cada comando expresado en el protocolo de comunicación. Es con estas cadenas que el programa cliente y el programa servidor pueden identificar qué comando enviar como petición y qué comando ejecutar respectivamente.

El archivo notices.h contiene las cadenas de caracteres que se usan para notificar sobre un suceso a los clientes receptores, esto es en el caso de que se ejecute un comando que requiere, como argumento, el ID del departamento de un cliente conectado al servidor. Un ejemplo de este tipo de comandos es el comando callto que va seguido de un espacio y del ID del departamento del cliente con el que se quiere establecer la llamada.

Las cadenas de caracteres que están destinadas para el chat, AUDIO_NOTICE_TO_RECEIVER_FIRST_PART y AUDIO_NOTICE_TO_RECEIVER_SECOND_PART, no se usan debido a que no se pudo implementar el chat.

Este archivo notices.h también tiene una cadena de caracteres que se llama INVALID_COMMAND_NOTICE, la cual es utilizada por un cliente, cuando el usuario ingresa un comando invalido, para avisarle al servidor de dicho comando invalido.

### Mejoras
- Implementar la función de chat (envío y recepción de audio).
- Extraer del programa servidor la lógica de intercambio de datos (IP y puerto) entre el programa servidor y el programa cliente, la cual se lleva a cabo para que ambos programas puedan comunicarse mediante UDP.
- Eliminar código fuente repetido. Hay código fuente repetido de las instrucciones de selección que comprueban los errores de ciertas llamadas al sistema, como write(), por ejemplo.
