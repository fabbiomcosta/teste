all: server client
server:
	gcc -o server_chat server_chat.c
client:
	gcc -o client_chat client_chat.c
