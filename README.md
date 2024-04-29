# Servidor Cliente distribuido Dados 100

Este proyecto consiste en un servidor y un cliente simple que se comunican a través de sockets. El servidor escucha en el puerto 20001 de la dirección IP local, mientras que el cliente busca el servidor en la ruta especificada en un archivo `.env`. 

## Archivos

- `server.o`: El archivo ejecutable del servidor.
- `client.o`: El archivo ejecutable del cliente.
- `.env`: Archivo de configuración que especifica la ruta donde el cliente buscará el servidor.
## Compilación
- `server` : g++ ./server/server.cpp -o server.o
- `cliente` : g++ ./client/client.cpp -o server.o
# Acciones Ejecutables

Este proyecto contiene un conjunto de acciones ejecutables que pueden ser manejadas por el cliente. A continuación se describen estas acciones y las funciones asociadas que las manejan:

## Acciones del Cliente

### Conexión y Desconexión

- **connect**: Establece una conexión con el servidor.
- **disconnect**: Cierra la conexión con el servidor.

### Lobby

- **lobby_info**: Obtiene información sobre el estado actual del lobby.
- **join**: Permite unirse a un lobby existente.
- **leave**: Permite abandonar un lobby.
- **create**: Crea un nuevo lobby.

### Juego

- **game_info**: Proporciona información sobre el estado actual del juego.
- **switch**: Cambia de equipo en el juego.
- **accept**: Acepta a un jugador.
- **roll**: Realiza un lanzamiento de dados en el juego.
- **ready**: Indica que el jugador está listo para comenzar el juego.

### Configuración del Juego

- **set_max_teams**: Establece el número máximo de equipos en el juego.
- **set_max_team_size**: Establece el tamaño máximo de los equipos en el juego.
- **set_min_dice_roll**: Establece el valor mínimo para el lanzamiento de dados en el juego.
- **set_max_dice_roll**: Establece el valor máximo para el lanzamiento de dados en el juego.
- **set_max_positions**: Establece el número máximo de posiciones en el juego.

## Acciones del servidor
- **stop**: apaga el servidor.
