/* client_chat.c

  Fabio Costa <fabiomcosta@dcc.ufba.br>
  Jundaí Abdon <jundai@dcc.ufba.br>

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include  <signal.h>
#include "messages.h"

#define MAXDATASIZE 2000 // max number of bytes we can get at once 


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// Main Program
int main (int argc, char *argv[])
{

    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 4)
    {
        printf ("\nUso: client_chat <CLIENT_NAME> <SERVER_ADDRESS> <SERVER_PORT>\n");
        return (1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    printf(MSG_CL_CONNECTED);

    freeaddrinfo(servinfo); // all done with this structure

    if (send(sockfd, argv[1], 30, 0) == -1) perror("send");

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("%s\n",buf);
 
    void  INThandler(int sig) {
         char  c;

         signal(sig, SIG_IGN);
         printf("\nDo you really want to quit? [y/n] ");
         c = getchar();
         if (c == 'y' || c == 'Y'){
              if (send(sockfd, argv[1], 30, 0) == -1) perror("send");
              exit(0);
         }else
              signal(SIGINT, INThandler);
         getchar(); // Get new line character
     }
    
    signal(SIGINT, INThandler);
    while (1) pause();

    close(sockfd);

    return 0;
}

    
