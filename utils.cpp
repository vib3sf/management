#include "utils.hpp"
#include "string.h"
#include "stdlib.h"


bool str_to_int(char *str, int &num) {
	char *endptr;
	num = strtol(str, &endptr, 10); 
	return !(!*str || *endptr);
}

char **split_line(char *line, int& argc)
{
	int argv_size = 4;
	char *arg = strtok(line, " "), **argv;
	if(!arg)
		return 0;

	argc = 0;
	argv = new char * [argv_size];
	do
	{
		if(argc >= argv_size)
		{
			argv_size *= 2;
			char **tmp = argv;
			argv = new char * [argv_size];
			for(int i = 0; i < argc; i++)
				argv[i] = tmp[i];
		}
		argv[argc] = arg;
		argc++;
	} while((arg = strtok(0, " ")));

	return argv;
}
