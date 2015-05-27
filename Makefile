client: client.c
	gcc -g -o bin/client client.c -I/opt/twitter/include -L/opt/twitter/lib -luv

server: server.c
	gcc -g -o bin/server server.c -I/opt/twitter/include -L/opt/twitter/lib -luv
