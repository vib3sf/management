#include <stdio.h>
#include <stdlib.h>

#include "server.hpp"

int main(int argc, const char * const *argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: server <port> <max players>\n");
		return 1;
	}
	
	char *endptr;
	long port = strtol(argv[1], &endptr, 10); 
    if(!*argv[1] || *endptr) {
        fprintf(stderr, "Invalid port number\n");
        return 2;
    }
	int max_players = strtol(argv[2], &endptr, 10);
    if(!*argv[2] || *endptr) {
        fprintf(stderr, "Invalid max players value\n");
        return 2;
    }

	Server server(port, max_players);
	server.Run();
	return 0;
}
