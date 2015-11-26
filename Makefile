all: server client
server:
	gcc server_chat.c -o server_chat -w -lpthread 
client:
	gcc client_chat.c -o client_chat -w -lpthread
clean:
	rm -f server_chat
	rm -f client_chat
