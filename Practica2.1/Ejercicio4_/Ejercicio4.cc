#include <netdb.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <unistd.h>

//Includes para los sockets
#include <sys/types.h>
#include <sys/socket.h>

const size_t MESSAGE_MAX_SIZE = 100;

int main(int argc, char **argv)
{

    struct addrinfo hints;
    struct addrinfo *result;

    //Si argumentos es distinto de 3 lanzo error
    if(argc != 3){
        std::cerr << "Parámetros incorrectos \n";
        exit(EXIT_FAILURE);
    }

    //Memoria para el struct que tiene los filtros de la conexion
    memset((void *)&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP

    //Coge en result todas las direcciones que cumplen los filtros
    int rc = getaddrinfo(argv[1], argv[2], &hints, &result);

    //Si getaddrinfo devuelve distinto de 0 es que ha dado error
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    //Creacion del socket
    int sd = socket(result->ai_family, result->ai_socktype, 0);

    //Si no se puede crear el socket lanzo error
    if (sd == -1)
    {
        std::cerr << "Error en la creación del socket \n";
        exit(EXIT_FAILURE);
    }

    if (bind(sd, result->ai_addr, result->ai_addrlen) == -1)
    {
        std::cerr << "Error en la llamada al metodo bind \n";
        exit(EXIT_FAILURE);
    }

    //Libero la memoria de result
    freeaddrinfo(result);

    //Poner el socket en el estado LISTEN
    if(listen(sd, 10)== -1){
        std::cerr << "Error al poner el socket en LISTEN \n";
        exit(EXIT_FAILURE);
    }


    //Aceptar conexión
    struct sockaddr addrClient;
    socklen_t sockLength = sizeof(sockaddr);

    //accept crea un socket en el otro extremo de la conexion, siendo la conexion
    //la primera de la cola
    int primeraConexion = accept(sd, &addrClient, &sockLength);

    if(primeraConexion == -1){
        std::cerr << "Error al aceptar la conexión \n";
        exit(EXIT_FAILURE);
    }
    
    //Variables para gestionar al cliente
    time_t timeRaw;
    size_t tam;
    bool serverAct = true;
    char buffer[MESSAGE_MAX_SIZE];
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    //Obtenemos nombre y puerto del cliente y lo mostramos información por consola
    getnameinfo(&addrClient, sockLength, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    std::cout << "Conexion desde " << host << " " << serv << "\n";

    while (serverAct)
    {
        //Recibo el mensaje del cliente
        int bytesRcv = recv(primeraConexion, (void *)buffer, (MESSAGE_MAX_SIZE - 1)*sizeof(char), 0);
        buffer[MESSAGE_MAX_SIZE] = '\0';

        //si recibe un mensaje vacio entonces termino el bucle
        if (bytesRcv <= 0)
        {
            serverAct = false;
            continue;
        }

        //el servidor contesta al cliente con el mismo mensaje que el cliente ha enviado
        send(primeraConexion, buffer, bytesRcv, 0);
    }

    //Esto cierra el socket
    close(sd);
    close(primeraConexion);
    std::cout << "Conexion terminada \n";
    return 0;
}