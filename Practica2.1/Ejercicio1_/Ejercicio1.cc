#include <netdb.h>
#include <string.h>
#include <iostream>

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result;
    //Si los argumentos son menos de 2 lanzo error
    if (argc < 2) { 
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //Memoria para el struct que tiene los filtros de la conexion    
    memset(&hints, 0, sizeof(struct addrinfo));

    //Coge en result todas las direcciones que cumplen los filtros
    int  rc = getaddrinfo(argv[1], argv[2], &hints, &result);

    //si devuelve distinto de cero es que ha habido algun error
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
            exit(EXIT_FAILURE);
    }


    //recorremos las direcciones de result y con getnameinfo guardamos en host y serv los datos de la direccion
    for (auto i = result; i != NULL; i = i->ai_next) {

        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];

        getnameinfo(i->ai_addr,i->ai_addrlen,host,NI_MAXHOST,serv
            ,NI_MAXSERV,NI_NUMERICHOST|NI_NUMERICSERV);
        std::cout << host << " " << i->ai_family << " " << i->ai_socktype  << std::endl;

    }

    //liberamos la memoria
    freeaddrinfo(result);

    return 0;
}
