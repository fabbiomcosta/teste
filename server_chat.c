/* server_chat.c

  Fabio Costa <fabiomcosta@dcc.ufba.br>
  Jundaí Abdon <jundai@dcc.ufba.br>

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "messages.h"


//################### Constant Variable ##################
#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 2000 // max number of bytes we can get at once 
#define MAXCLIENT 10 // max number of conected client  



//################### Function ################## 

// Simplest dead child cleanup in a SIGCHLD handle
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// get current time
char* printTime() {
    char hora[20];
    time_t t;
    time(&t);
    localtime(&t);
    strftime (hora,80,"%R",localtime(&t));
    printf("%s\t", hora);
}

//################### Main Program ####################

int main (int argc, char *argv[])
{

    // Variables
    int sockfd, new_fd, numbytes; // listen on sock_fd, new connection on new_fd, number of bytes recieved
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char client_message[MAXDATASIZE], list_client[MAXCLIENT][30], s[INET6_ADDRSTRLEN];
    int rv;
    struct sigaction sa;
    int c, yes=1;  // count to list of client, allow use local addresses


    // initiate counter
    c = 0;

    if (argc != 2)
    {
        printf ("\nUso: server_chat <PORT>\n");
        return   (1);
    }

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        // Create socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        // Specifies that the rules used in validating addresses supplied to bind() should allow reuse of local addresses
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        //Bind to address and port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    
    // listen
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    while(1) {  // main accept() loop
        
        // now accept an incoming connection:
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        // receive client's name
        if ((numbytes = recv(new_fd, client_message, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        client_message[numbytes] = '\0';
        printTime();
        printf("%s \t conected\n",client_message);
 
       // store client's name into list 
        if (MAXCLIENT > c) {
            strcpy(list_client[c], client_message); 
            c++; 
        }

   
        // create process child
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1) perror("send");
            

            // disconect client
 
            if ((numbytes = recv(new_fd, client_message, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            client_message[numbytes] = '\0';
            printTime();
            printf("%s \t disconected\n",client_message);

            close(new_fd);
            exit(0);
        }
    
    }
    return 0;
}
