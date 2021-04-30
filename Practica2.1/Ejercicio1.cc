#include <netdb.h>
#include <string.h>
#include <iostream>

int main(int argc, char *argv[])
       {
           struct addrinfo hints;
           struct addrinfo *result;

           if (argc < 2) {
               fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
               exit(EXIT_FAILURE);
           }
        
           memset(&hints, 0, sizeof(struct addrinfo));

         int  rc = getaddrinfo(argv[1], argv[2], &hints, &result);

           if (rc != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
               exit(EXIT_FAILURE);
           }


           for (auto i = result; i != NULL; i = i->ai_next) {

               char host[NI_MAXHOST];
               char serv[NI_MAXSERV];

               getnameinfo(i->ai_addr,i->ai_addrlen,host,NI_MAXHOST,serv
               ,NI_MAXSERV,NI_NUMERICHOST|NI_NUMERICSERV);
               std::cout << host << " " << i->ai_family << " " << i->ai_socktype  << std::endl;

           }

           freeaddrinfo(result);

          return 0;
    }
