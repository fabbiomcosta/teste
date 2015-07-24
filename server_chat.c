/* server_chat.c

  Fabio Costa <fabiomcosta@dcc.ufba.br>
  Jundaí Abdon <jundai@dcc.ufba.br>

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "messages.h"

// Variables

int PORT;
int mysocket, client_sock, c, read_size;
struct sockaddr_in server , client;
char client_message[2000];

// Main Program
int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        printf ("\nUso: server_chat <PORT>\n");
        return   (1);
    }
    else
    {
        PORT = atoi(argv[1]);
        printf(MSG_SRV_PORT, PORT);
    }
    return (0);



    // Create socket
    mysocket = socket(AF_INET,SOCK_STREAM,0);
    if (mysocket == -1)
    {
        printf("ERROR: Could not create socket");
    }

    // Filled the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //Bind to address and port
    if (bind(mysocket,(struct sockaddr *)&server , sizeof(server))<0)
    {
        perror("ERROR: bind()");
    }

    //
