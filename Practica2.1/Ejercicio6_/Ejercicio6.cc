#include <netdb.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string>

//Sockets
#include <sys/types.h>
#include <sys/socket.h>

//Threads
#include <thread>

//Constantes
const size_t MESSAGE_MAX_SIZE = 100;
#define MAX_THREADS 5 //Representa el numero de conexiones totales que se pueden dar al mismo tiempo

//Clase para gestionar los threads
class MessageThread
{
private:
    int sd; //socket del servidor

public:
    //Constructora 
    MessageThread(int sd_) : sd(sd_) {}

    //Metodo a realizar en los threads
    void dameFechaUHora()
    {
        time_t rawtime;
        size_t tam;
        char buffer[MESSAGE_MAX_SIZE];
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];

        struct sockaddr client;
        socklen_t clientLength = sizeof(struct sockaddr);

        while (true)
        {
            //Para probar la concurrencia
            sleep(3); 

            int bytesReceived = recvfrom(sd, (void *)buffer, (MESSAGE_MAX_SIZE - 1) * sizeof(char), 0, &client, &clientLength);
            buffer[MESSAGE_MAX_SIZE] = '\0';
            
            int i = 0;
            
            if (bytesReceived == -1)
            {
                std::cerr << "Error en la recepcion de bytes, cerrando conexión\n";
                return;
            }

            //Obtenemos nombre y puerto del cliente y lo mostramos información por consola
            getnameinfo(&client, clientLength, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
            std::cout << bytesReceived << " bytes de " << host << ":" << serv << " thread: " << std::this_thread::get_id() << "\n";

            //Procesamos comandos
            switch (buffer[0])
            {

            //Mostrar hora
            case 't':
                time(&rawtime);
                tam = strftime(buffer, MESSAGE_MAX_SIZE - 1, "%T %p", localtime(&rawtime));
                sendto(sd, buffer, tam, 0, &client, clientLength);
                break;

            //Mostrar fecha
            case 'd':
                time(&rawtime);
                tam = strftime(buffer, MESSAGE_MAX_SIZE - 1, "%F", localtime(&rawtime));
                sendto(sd, buffer, tam, 0, &client, clientLength);
                break;

            //Comando desconocido
            default:
                std::cout << "Comando no soportado: " << buffer[0] << "\n";
                sendto(sd, "Comando no valido\n", 19, 0, &client, clientLength);
            }
        }
    }
};

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

    std::thread threads[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++)
    {
        MessageThread *mssgThread = new MessageThread(sd);
        std::thread([&mssgThread]() 
        {
            mssgThread->dameFechaUHora(); 
            delete mssgThread; 
        });
    }

    //El thread principal(que es el main) cierra el server cuando recibe una q
    char letra;
    do
    {
        std::cin >> letra;
    } while (letra != 'q');

    //Esto cierra el socket
    close(sd);

    return 0;
}