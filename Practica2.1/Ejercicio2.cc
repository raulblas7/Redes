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
    if (argc != 3)
    {
        fprintf(stderr, "Parametros incorrectos \n");
        exit(EXIT_FAILURE);
    }

    //Memoria para el struct que tiene los filtros de la conexion
    memset((void *)&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP

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

    //Asigna el socket a una direccion
    if (bind(sd, result->ai_addr, result->ai_addrlen) == -1)
    {
        std::cerr << "Error en la llamada al metodo bind \n";
        exit(EXIT_FAILURE);
    }

    //Libero la memoria de result
    freeaddrinfo(result);

    //Variables para gestionar al cliente
    time_t timeRaw;
    size_t tam;
    bool serverAct = true;
    char buffer[MESSAGE_MAX_SIZE];
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    while (serverAct)
    {
        struct sockaddr client;
        socklen_t clientLength = sizeof(struct sockaddr);

        int bytesRcv = recvfrom(sd, (void *)buffer, (MESSAGE_MAX_SIZE - 1)*sizeof(char), 0, &client, &clientLength);
        buffer[MESSAGE_MAX_SIZE] = '\0';

        if (bytesRcv == -1)
        {
            std::cerr << "Error en la recepcion de bytes, cerrando conexión\n";
            return EXIT_FAILURE;
        }

        //Obtenemos nombre y puerto del cliente y lo mostramos información por consola
        getnameinfo(&client, clientLength, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        std::cout << bytesRcv << " bytes de " << host << ":" << serv << "\n";

        //Procesamos comandos
        switch (buffer[0])
        {

        //Muestra la hora
        case 't':
            time(&timeRaw);
            tam = strftime(buffer, MESSAGE_MAX_SIZE - 1, "%T %p", localtime(&timeRaw));
            sendto(sd, buffer, tam, 0, &client, clientLength);
            break;

        //Muestra la fecha
        case 'd':
            time(&timeRaw);
            tam = strftime(buffer, MESSAGE_MAX_SIZE - 1, "%F", localtime(&timeRaw));
            sendto(sd, buffer, tam, 0, &client, clientLength);
            break;

        //Cerrar servidor
        case 'q':
            std::cout << "Saliendo...\n";
            serverAct = false;
            sendto(sd, "Has cerrado el servidor\n", 25, 0, &client, clientLength);
            break;

        //Comando desconocido
        default:
            std::cout << "Comando no soportado: " << buffer[0] << "\n";
            sendto(sd, "Comando no valido\n", 19, 0, &client, clientLength);
            break;
        }
    }

    //Esto cierra el socket
    close(sd);

    return 0;
}