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

    //Libero la memoria de result
    freeaddrinfo(result);
    
    //Establecer conexión, connect nos devuelve un socket
    int id = connect(sd, result->ai_addr, result->ai_addrlen);
    if(id == -1){
        std::cerr << "Error al conectar \n";
        exit(EXIT_FAILURE);
    }


    bool serverAct = true;
    char buffer[MESSAGE_MAX_SIZE];

    while (serverAct)
    {
        //el cliente lee lo que ha puesto en consola y lo almacena en el buffer
        std::cin >> buffer;
        //le manda el mensaje al servidor
        send(id, buffer, MESSAGE_MAX_SIZE - 1, 0);

        //si es una letra Q cierra la conexion
        if(buffer[0] == 'Q' && buffer[1] == '\0')
        {
            serverAct = false;
            continue;
        }
        
        //recibe la respuesta del servidor y la almacena en el buffer
        recv(id, (void *)buffer, (MESSAGE_MAX_SIZE - 1)*sizeof(char), 0);
        //escribe en la consola la respuesta del servidor
        std::cout << buffer << "\n";
    }

    //Esto cierra el socket
    close(sd);
    return 0;
}