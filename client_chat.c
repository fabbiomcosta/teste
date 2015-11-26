/* 
** Pragram : client_chat.c 
** Students: Fabio Costa <fabiomcosta@dcc.ufba.br> and Jundaí Abdon <jundai@dcc.ufba.br>
** Date: 2626 November, 2015
** Professor: Paul Pregnier
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
 
//################### Constant Variable ################## 

#define BUFFSIZE 2000
#define NAMELEN 32
#define CMDLEN 16
#define LINEBUFF 2048

//################### Data structure ##################

struct PACKET {
    char command[CMDLEN]; // instruction
    char name[NAMELEN]; // client's name
    char buff[BUFFSIZE]; // message
};
 
struct USER {
        int sockfd; // user's socket descriptor
        char name[NAMELEN]; // user's name
};
 
struct THREADINFO {
    pthread_t thread_ID; // thread's pointer
    int sockfd; // socket file descriptor
};
 
//################### Grobal Variable ##################
char server[] = "";
int port;
int isconnected, sockfd;
char command[LINEBUFF];
struct USER me;

//################### Prototypes ##################

int connect_with_server();
void setname(struct USER *me);
void logout(struct USER *me);
void login(struct USER *me);
void *receiver(void *param);
void sendtoall(struct USER *me, char *msg);
void sendtoname(struct USER *me, char * target, char *msg);
void showclients(struct USER *me);

//################### Main program ##################

int main(int argc, char **argv) {
    int sockfd, namelen;

    if (argc != 4){
        printf ("\nUso: client_chat <CLIENT_NAME> <SERVER_ADDRESS> <SERVER_PORT>\n");
        return (1);
    }


    strcpy(server, argv[2]);
    port = atoi(argv[3]);
    memset(&me, 0, sizeof(struct USER));
    strcpy(me.name, argv[1]);
    login(&me);

    while(isconnected) {
        gets(command);
        if(!strncmp(command, "EXIT", 4)) {
            logout(&me);
            break;
        }
        if(!strncmp(command, "HELP", 4)) {
            FILE *fin = fopen("help.txt", "r");
            if(fin != NULL) {
                while(fgets(command, LINEBUFF-1, fin)) puts(command);
                fclose(fin);
            }
            else {
                fprintf(stderr, "Help file not found...\n");
            }
        }
        else if(!strncmp(command, "NAME", 4)) {
            char *ptr = strtok(command, " ");
            ptr = strtok(0, " ");
            memset(me.name, 0, sizeof(char) * NAMELEN);
            if(ptr != NULL) {
                namelen =  strlen(ptr);
                if(namelen > NAMELEN) ptr[NAMELEN] = 0;
                strcpy(me.name, ptr);
                setname(&me);
            }
        }
        else if(!strncmp(command, "SENDTO", 6)) {
            char *ptr = strtok(command, " ");
            char temp[NAMELEN];
            ptr = strtok(0, " ");
            memset(temp, 0, sizeof(char) * NAMELEN);
            if(ptr != NULL) {
                namelen =  strlen(ptr);
                if(namelen > NAMELEN) ptr[NAMELEN] = 0;
                strcpy(temp, ptr);
                while(*ptr) ptr++; ptr++;
                while(*ptr <= ' ') ptr++;
                sendtoname(&me, temp, ptr);
            }
        }
        else if(!strncmp(command, "WHO", 3)) {
            showclients(&me);
        }
        else if(!strncmp(command, "SEND", 4)) {
            sendtoall(&me, &command[5]);
        }
        else fprintf(stderr, "Unknown command...\n");
    }
    return 0;
}

//################### Function #################

void login(struct USER *me) {
    int recvd;
    sockfd = connect_with_server();
    if(sockfd >= 0) {
        isconnected = 1;
        me->sockfd = sockfd;
        struct THREADINFO threadinfo;
        int sent;
        struct PACKET packet;
    
        memset(&packet, 0, sizeof(struct PACKET));
        strcpy(packet.command, "LOGIN");
        strcpy(packet.name, me->name);
    
               
        /* send request to close this connetion */
        sent = send(sockfd, (void *)&packet, sizeof(struct PACKET), 0);

        pthread_create(&threadinfo.thread_ID, NULL, receiver, (void *)&threadinfo);
 
    }
    else {
        fprintf(stderr, "Connection rejected...\n");
    }
}
 
int connect_with_server() {
    int newfd, err_ret;
    struct sockaddr_in serv_addr;
    struct hostent *to;
 
    /* generate address */
    if((to = gethostbyname(server))==NULL) {
        err_ret = errno;
        fprintf(stderr, "gethostbyname() error...\n");
        return err_ret;
    }
 
    /* open a socket */
    if((newfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        fprintf(stderr, "socket() error...\n");
        return err_ret;
    }
 
    /* set initial values */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)to->h_addr);
    memset(&(serv_addr.sin_zero), 0, 8);
 
    /* try to connect with server */
    if(connect(newfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        fprintf(stderr, "connect() error...\n");
        return err_ret;
    }
    else {
        printf("Sucessfully Connected...\n");
        return newfd;
    }
}
 
void logout(struct USER *me) {
    int sent;
    struct PACKET packet;
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    memset(&packet, 0, sizeof(struct PACKET));
    strcpy(packet.command, "EXIT");
    strcpy(packet.name, me->name);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(struct PACKET), 0);
    isconnected = 0;
    close(sockfd);
}

void setname(struct USER *me) {
    int sent;
    struct PACKET packet;
    
    memset(&packet, 0, sizeof(struct PACKET));
    strcpy(packet.command, "NAME");
    strcpy(packet.name, me->name);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(struct PACKET), 0);
}
 
void *receiver(void *param) {
    int recvd;
    struct PACKET packet;
    
    while(isconnected) {
        recvd = recv(sockfd, (void *)&packet, sizeof(struct PACKET), 0);
        if(!strcmp(packet.command, "EXIT")){
            printf("Client's name has been used... Try again!\n");
            logout(&me);
	    exit(0);
            break; 
        }else if(!strcmp(packet.command, "WHO")){
            printf("List of clients: %s \n", packet.buff);
        }else if (recvd > 0) {
            printf("%s: %s\n", packet.name, packet.buff);
        }else if (!recvd){
            fprintf(stderr, "Connection lost from server...\n");
            isconnected = 0;
            close(sockfd);
            break;
        }else{
            fprintf(stderr, "Receipt of message has failed...\n");
            break;
        }
        memset(&packet, 0, sizeof(struct PACKET));
    }
    return NULL;
}
 
void sendtoall(struct USER *me, char *msg) {
    int sent;
    struct PACKET packet;
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    msg[BUFFSIZE] = 0;
    
    memset(&packet, 0, sizeof(struct PACKET));
    strcpy(packet.command, "SEND");
    strcpy(packet.name, me->name);
    strcpy(packet.buff, msg);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(struct PACKET), 0);
}
 
void sendtoname(struct USER *me, char *target, char *msg) {
    int sent, targetlen;
    struct PACKET packet;

    if(target == NULL) {
        return;
    }
    
    if(msg == NULL) {
        return;
    }
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    msg[BUFFSIZE] = 0;
    targetlen = strlen(target);
    
    memset(&packet, 0, sizeof(struct PACKET));
    strcpy(packet.command, "SENDTO");
    strcpy(packet.name, me->name);
    strcpy(packet.buff, target);
    strcpy(&packet.buff[targetlen], " ");
    strcpy(&packet.buff[targetlen+1], msg);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(struct PACKET), 0);
}

void showclients(struct USER *me) {
    int sent, targetlen;
    struct PACKET packet;
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    memset(&packet, 0, sizeof(struct PACKET));
    strcpy(packet.command, "WHO");
    strcpy(packet.name, me->name);
    sent = send(sockfd, (void *)&packet, sizeof(struct PACKET), 0);
}
