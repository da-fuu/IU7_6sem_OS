all : client server

client : client.c
	gcc -o client client.c

server : server.c
	@echo gcc -o server server.c
	@gcc -Wno-int-to-pointer-cast -o server server.c

clean :
	rm -f client server
