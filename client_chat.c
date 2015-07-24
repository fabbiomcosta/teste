/* client_chat.c

  Fabio Costa <fabiomcosta@dcc.ufba.br>
  Jundaí Abdon <jundai@dcc.ufba.br>

*/


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/sockets.h>
#include <netinet/in.h>
#include <netdb.h>
#include "messages.h"

// Variables
char CLIENT_NAME[30];
int SERVER_ADDRESS;
int SERVER_PORT;

// Main Program
int main (int argc, char *argv[])
{
    if (argc != 4)
    {
        printf ("\nUso: client_chat <CLIENT_NAME> <SERVER_ADDRESS> <SERVER_PORT>\n");
        return (1);
    }
    else
    {
        strcpy(CLIENT_NAME, argv[1]);
        SERVER_ADDRESS = atoi(argv[2]);
        SERVER_PORT = atoi(argv[3]);
        printf(MSG_CL_CONNECTED);
    }
    return (0);

int mysocket;


}
