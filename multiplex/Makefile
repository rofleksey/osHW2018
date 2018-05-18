compile: server client

server: server.c util.h
	gcc -Wall -fsanitize=address -o server server.c

client: client.c util.h
	gcc -Wall -fsanitize=address -o client client.c

clean:
	-rm server client
