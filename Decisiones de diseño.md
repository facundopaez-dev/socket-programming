### Decisiones de diseño
- Elegí los comandos escritos en el archivo "Protocolo de comunicación" como mecanismos de interacción entre los clientes y el servidor.
- Elegí el **protocolo modo texto** para la comunicación entre los clientes y el servidor.
- Elegí un límite para la cantidad de conexiones. Este limite establece el tamaño de un arreglo que almacena el descriptor de archivo de cada Socket creado para la conexión entre un cliente y el servidor. Por lo tanto, este arreglo contiene la cantidad de conexiones que se pueden hacer al servidor. A este arreglo lo denomino como **arreglo de conexiones**.  
- Al arreglo de conexiones, lo veo como un arreglo de **departamentos**. Al verlo de esta forma, cada departamento (representado por una celda del arreglo de conexiones) tiene un identificador (ID) univoco. Dentro de cada departamento hay un cliente, y la forma en la cual se registra que un cliente está en un departamento es almacenando el descriptor de archivo (del Socket utilizado para la conexión entre un cliente y el servidor) en una posición del arreglo de conexiones.  
- Elegí utilizar hilos en lugar de procesos ya que con hilos la implementación del servidor concurrente se hace más sencilla.
- Elegí que el establecimiento de la conexión entre los clientes y el servidor sea hecho mediante el protocolo TCP.
