all: server client
server:
	gcc server_chat.c -o server_chat -lpthread
client:
	gcc client_chat.c -o client_chat -lpthread
