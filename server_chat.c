/*
** Pragram : server_chat.c
** Students: Fabio Costa <fabiomcosta@dcc.ufba.br> and Jundaí Abdon <jundai@dcc.ufba.br>
** Date: 26th November, 2015
** Professor: Paul Pregnier
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

//################### Constant Variable ##################

#define BACKLOG 10
#define CLIENTS 10

#define BUFFSIZE 2000
#define NAMELEN 32
#define CMDLEN 16

//################### Data structure ##################

struct PACKET {
    char command[CMDLEN]; // instruction
    char name[NAMELEN]; // client's name
    char buff[BUFFSIZE]; // payload
};

struct THREADINFO {
    pthread_t thread_ID; // thread's pointer
    int sockfd; // socket file descriptor
    char name[NAMELEN]; // client's name
};

struct LLNODE {
    struct THREADINFO threadinfo;
    struct LLNODE *next;
};

struct LLIST {
    struct LLNODE *head, *tail;
    int size;
};

//################### Prototypes ##################

void *io_handler(void *param);
void *connections_handler();
void *client_handler();
void *message_handler(void *fd);

//################### Function #################

int compare(struct THREADINFO *a, struct THREADINFO *b) {
    return a->sockfd - b->sockfd;
}

int compare_client(struct THREADINFO *a, struct THREADINFO *b) {
    if(!strcmp(a->name, b->name)) { 
        return 0;
    }
    return 1;
}

void list_init(struct LLIST *ll) {
    ll->head = ll->tail = NULL;
    ll->size = 0;
}

int list_insert(struct LLIST *ll, struct THREADINFO *thr_info) {
    if(ll->size == CLIENTS) return -1;
    if(ll->head == NULL) {
        ll->head = (struct LLNODE *)malloc(sizeof(struct LLNODE));
        ll->head->threadinfo = *thr_info;
        ll->head->next = NULL;
        ll->tail = ll->head;
    }
    else {
        ll->tail->next = (struct LLNODE *)malloc(sizeof(struct LLNODE));
        ll->tail->next->threadinfo = *thr_info;
        ll->tail->next->next = NULL;
        ll->tail = ll->tail->next;
    }
    ll->size++;
    return 0;
}

int list_delete(struct LLIST *ll, struct THREADINFO *thr_info) {
    struct LLNODE *curr, *temp;
    if(ll->head == NULL) return -1;
    if(compare(thr_info, &ll->head->threadinfo) == 0) {
        temp = ll->head;
        ll->head = ll->head->next;
        if(ll->head == NULL) ll->tail = ll->head;
        free(temp);
        ll->size--;
        return 0;
    }
    for(curr = ll->head; curr->next != NULL; curr = curr->next) {
        if(compare(thr_info, &curr->next->threadinfo) == 0) {
            temp = curr->next;
            if(temp == ll->tail) ll->tail = curr;
            curr->next = curr->next->next;
            free(temp);
            ll->size--;
            return 0;
        }
    }
    return -1;
}

void list_dump(struct LLIST *ll) {
    struct LLNODE *curr;
    struct THREADINFO *thr_info;
    printf("Connection count: %d\n", ll->size);
    for(curr = ll->head; curr != NULL; curr = curr->next) {
        thr_info = &curr->threadinfo;
        printf("[%d] %s\n", thr_info->sockfd, thr_info->name);
    }
}

// get current time
void printTime() {
    char hora[20];
    time_t t;
    time(&t);
    localtime(&t);
    strftime (hora,80,"%R",localtime(&t));
    printf("%s\t", hora);
}



//################### Grobal Variable ##################
int err_ret, sin_size;
int sockfd, newfd, port, fds[2];
struct THREADINFO thread_info[CLIENTS];
struct LLIST client_list;
pthread_mutex_t clientlist_mutex;
struct sockaddr_storage client_addr;


//################### Main program ##################


int main(int argc, char **argv) {
    int rv, yes=1; // yes = allow use local addresses
    struct addrinfo hints, *servinfo, *p;
    pthread_t interrupt, T1, T2;


    if (argc != 2) {
        printf ("\nUso: server_chat <PORT>\n");
        return   (1);
    }

    // convert string to int
    port = atoi(argv[1]);


    /* initialize linked list */
    list_init(&client_list);

    /* initiate mutex */
    pthread_mutex_init(&clientlist_mutex, NULL);


    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        // create socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // specifies that the rules used in validating addresses supplied to bind() should allow reuse of local addresses
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // bind to address and port
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


    /* start listening for connection */
    if(listen(sockfd, BACKLOG) == -1) {
        err_ret = errno;
        fprintf(stderr, "listen() failed...\n");
        return err_ret;
    }

    if (pipe(fds)){
      fprintf (stderr, "Pipe failed.\n");
      return err_ret;
    }


    /* initiate interrupt handler for IO controlling (T1) */
    if(pthread_create(&interrupt, NULL, io_handler, NULL) != 0) {
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed...\n");
        return err_ret;
    }

    /* initiate T1 to handler connections */
    if(pthread_create(&T1, NULL, connections_handler, NULL) != 0){
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed\n");
        return err_ret;
    }

    /* initiate T2 to handler opened connections */
    if(pthread_create(&T2, NULL, client_handler, NULL) != 0){
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed\n");
        return err_ret;
    }

    while(1){
    }       
   return 0;
}

void *io_handler(void *param) {
    char command[CMDLEN];
    while(scanf("%s", command)==1) {
        if(!strcmp(command, "exit")) {
            /* clean up */
            printf("Terminating server...\n");
            pthread_mutex_destroy(&clientlist_mutex);
            close(sockfd);
            exit(0);
        }
        else if(!strcmp(command, "list")) {
            pthread_mutex_lock(&clientlist_mutex);
            list_dump(&client_list);
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else {
            fprintf(stderr, "Unknown command: %s...\n", command);
        }
    }
    return NULL;
}

void *connections_handler(){

    /* keep accepting connections */
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if((newfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&sin_size)) == -1) {
            err_ret = errno;
            fprintf(stderr, "accept() failed...\n");
        }
        else {
            if(client_list.size == CLIENTS) {
                fprintf(stderr, "Connection full, request rejected...\n");
                continue;
            }
            write (fds[1], &newfd,1);
        }
    }
}

void *client_handler() {

    struct THREADINFO threadinfo;
    fd_set set;
    int newfd;
    
    /* Initialize the file descriptor set. */
    FD_ZERO(&set);
    FD_SET(fds[0], &set);

    while(1){
        int ret = select(FD_SETSIZE, &set, NULL, NULL, NULL);
        if (ret < 0){
           printf("error select");// error occurred
	}
	else
	{
           read (fds[0], &newfd,1);          
           threadinfo.sockfd = newfd;
           pthread_mutex_lock(&clientlist_mutex);
           list_insert(&client_list, &threadinfo);
           pthread_mutex_unlock(&clientlist_mutex);  
           pthread_create(&threadinfo.thread_ID, NULL, message_handler, (void *)&threadinfo);
    	}
    }
}

void *message_handler(void *fd) {
    struct THREADINFO threadinfo = *(struct THREADINFO *)fd;
    struct PACKET packet;
    struct LLNODE *curr;
    int bytes, sent;
    while(1) {
        bytes = recv(threadinfo.sockfd, (void *)&packet, sizeof(struct PACKET), 0);
        if(!bytes || !strcmp(packet.command, "EXIT")) {
            printTime();
            printf("%s \thas disconnected...\n", threadinfo.name);
            pthread_mutex_lock(&clientlist_mutex);
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
        if(!strcmp(packet.command, "LOGIN")) {
 
            int skfd;
            struct PACKET spacket;
            
            pthread_mutex_lock(&clientlist_mutex);
            strcpy(threadinfo.name, packet.name);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(compare_client(&curr->threadinfo, &threadinfo) == 0) {
                    memset(&spacket, 0, sizeof(struct PACKET));
                    strcpy(spacket.command, "EXIT");
                    strcpy(spacket.name, packet.name);
                    skfd = threadinfo.sockfd;
                    sent = send(skfd, (void *)&spacket, sizeof(struct PACKET), 0);           
                    break;
                }
            }
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(compare(&curr->threadinfo, &threadinfo) == 0) {
                    strcpy(curr->threadinfo.name, packet.name);
                    strcpy(threadinfo.name, packet.name);
                    break;
                }
            }              
            pthread_mutex_unlock(&clientlist_mutex);
            printTime();
            printf("%s \t has conected...\n", packet.name);
        }
        if(!strcmp(packet.command, "NAME")) {
            printTime();
            printf("%s \t %s \t executed: No\n", packet.name,  packet.command);
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(compare(&curr->threadinfo, &threadinfo) == 0) {
                    strcpy(curr->threadinfo.name, packet.name);
                    strcpy(threadinfo.name, packet.name);
                    break;
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.command, "SENDTO")) {
            int i;
            char target[NAMELEN];
            for(i = 0; packet.buff[i] != ' '; i++); packet.buff[i++] = 0;
            strcpy(target, packet.buff);
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(strcmp(target, curr->threadinfo.name) == 0) {
                    struct PACKET spacket;
                    memset(&spacket, 0, sizeof(struct PACKET));
                    if(!compare(&curr->threadinfo, &threadinfo)) continue;
                    strcpy(spacket.command, "msg");
                    strcpy(spacket.name, packet.name);
                    strcpy(spacket.buff, &packet.buff[i]);
                    sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(struct PACKET), 0);
                    if (sent == -1){
                        printTime();
                        printf("%s \t %s \t executed: No\n", packet.name,  packet.command);
                    } else {
                        printTime();
                        printf("%s \t %s \t executed: Sim\n", packet.name,  packet.command);
                    }
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.command, "SEND")) {
            printTime();
            printf("%s \t %s \t executed: Sim\n", packet.name,  packet.command);
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                struct PACKET spacket;
                memset(&spacket, 0, sizeof(struct PACKET));
                if(!compare(&curr->threadinfo, &threadinfo)) continue;
                strcpy(spacket.command, "msg");
                strcpy(spacket.name, packet.name);
                strcpy(spacket.buff, packet.buff);
                sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(struct PACKET), 0);
                if (sent == -1){
                    printTime();
                    printf("%s \t %s \t executed: No\n", packet.name,  packet.command);
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.command, "WHO")) {
            int skfd;
            struct PACKET spacket;

            memset(&spacket, 0, sizeof(struct PACKET));
            strcpy(spacket.command, "WHO");
            strcpy(spacket.name, packet.name);

            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(strcmp(packet.name, curr->threadinfo.name) == 0) {
                    skfd = threadinfo.sockfd;
                    strcpy(spacket.buff, curr->threadinfo.name);
                }
            }
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(strcmp(packet.name, curr->threadinfo.name) != 0) {
                    strcat(spacket.buff, "    ");
                    strcat(spacket.buff, curr->threadinfo.name);
                }
            }
            sent = send(skfd, (void *)&spacket, sizeof(struct PACKET), 0);
            if (sent == -1){
                printTime();
                printf("%s \t %s \t executed: No\n", packet.name,  packet.command);
            }
            printTime();
            printf("%s \t %s \t executed: Sim\n", packet.name,  packet.command);
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else {
//            fprintf(stderr, "Please keep in touch with %s. He doesn't know how to use the client_chat!\n", threadinfo.name);
        }
    }

    /* clean up */
    close(threadinfo.sockfd);

    return NULL;
}
