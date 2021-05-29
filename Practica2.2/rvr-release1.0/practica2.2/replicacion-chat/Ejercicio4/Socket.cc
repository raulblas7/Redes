#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * address, const char * port):sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    struct addrinfo hints;
    struct addrinfo* result;

    //Memoria para el struct que tiene los filtros de la conexion
    memset((void*)&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP

    //Coge en result todas las direcciones que cumplen los filtros
    int rc = getaddrinfo(address, port, &hints, &result);

    //Si getaddrinfo devuelve distinto de 0 es que ha dado error
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    //Creacion del socket
    sd = socket(result->ai_family, result->ai_socktype, 0);

    //Si no se puede crear el socket lanzo error
    if (sd == -1)
    {
        std::cerr << "Error en la creación del socket \n";
        exit(EXIT_FAILURE);
    }

    //Asignacion variable sa y sa_len
    sa = *result->ai_addr;
    sa_len = result->ai_addrlen;

    //Libero la memoria de result
    freeaddrinfo(result);
}

int Socket::recv(Serializable &obj, Socket * &sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if ( bytes <= 0 )
    {
        return -1;
    }

    if ( sock != 0 )
    {
        sock = new Socket(&sa, sa_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    //Serializar el objeto
    //Enviar el objeto binario a sock usando el socket sd
    obj.to_bin();

    ssize_t b = sendto(sock.sd, obj.data(), obj.size(), 0, &sock.sa, &sock.sa_len);

    if (b <= 0) {
        return -1;
    }

    return 0;
}

bool operator== (const Socket &s1, const Socket &s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere

    if (s1.sa.sa sa_family != s2.sa.sa_family) {
        return false;
    }

    struct sockaddr_in* s1_ = (struct sockaddr_in*)&(s1.sa);
    struct sockaddr_in* s2_ = (struct sockaddr_in*)&(s2.sa);

    if (s1_->sin_addr.s_addr != s2_->sin_addr.s_addr) {
        return false;
    }

    if (s1_->sin_port != s2_->sin_port) {
        return false;
    }

    return true;
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};

