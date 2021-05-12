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

    //Si argumentos es distinto de 4 lanzo error
    if(argc != 4){
        std::cerr << "Parámetros incorrectos \n";
        return EXIT_FAILURE;
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

    //Libero la memoria de result
    freeaddrinfo(result);

    char buffer[MESSAGE_MAX_SIZE];

    //Envia un mensaje al otro conector esperando una respuesta
    if(sendto(sd, argv[3], strlen(argv[3])+1, 0, result->ai_addr, result->ai_addrlen) == -1){
        std::cerr << "Error en el envío de información al servidor [sendTo]\n";
        exit(EXIT_FAILURE);
    }

    //Recibe la respuesta esperada del servidor
    if(recvfrom(sd, buffer, (MESSAGE_MAX_SIZE - 1) * sizeof(char), 0, result->ai_addr, &result->ai_addrlen) == -1){
        std::cerr << "Error en la recepción bytes de [recvfrom]\n";
        exit(EXIT_FAILURE);
    }

    std::cout << buffer << "\n";
    //Esto cierra el socket
    close(sd);

    return 0;
}