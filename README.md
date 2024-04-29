# Servidor Cliente distribuido Dados 100

Este proyecto consiste en un servidor y un cliente simple que se comunican a través de sockets. El servidor escucha en el puerto 20001 de la dirección IP local, mientras que el cliente busca el servidor en la ruta especificada en un archivo `.env`. 

## Archivos

- `server.o`: El archivo ejecutable del servidor.
- `client.o`: El archivo ejecutable del cliente.
- `.env`: Archivo de configuración que especifica la ruta donde el cliente buscará el servidor.
## Compilación
- `server` : g++ ./server/server.cpp -o server.o
- `cliente` : g++ ./client/client.cpp -o server.o
