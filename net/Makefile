compile: server client

server: server.c
	gcc -Wall -fsanitize=address -o server server.c

client: client.c
	gcc -Wall -fsanitize=address -o client client.c

clean:
	-rm server client
