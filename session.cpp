#include "session.hpp"
#include "controller.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include <netinet/in.h>

Session::Session(int fd, struct sockaddr_in *from, Controller& controller) 
	: state(typing_name), fd(fd), from_ip(htonl(from->sin_addr.s_addr)), 
	from_port(htons(from->sin_port)), buf_used(0), controller(controller)
{
	SendMessage("Type your name: ");
}

void Session::SendMessage(const char *str) const
{
	write(fd, str, strlen(str));
}

bool Session::DoRead()
{
	int rc, bufp = buf_used;
	rc = read(fd, buf + bufp, buf_size - bufp);
	if(rc <= 0) {
		return false;
	}

	buf_used = rc;
	CheckLf();
	
	return true;
}

void Session::Start()
{
	state = waiting_turn;
	SendMessage("Game started.\n");
}

void Session::CheckLf()
{
    int pos = -1;
    char *line;
    for(int i = 0; i < buf_used; i++) {
        if(buf[i] == '\n') {
            pos = i;
            break;
        }
    }
    if(pos == -1)
        return;
    line = new char[pos + 1];
    memcpy(line, buf, pos);
    line[pos] = 0;
    buf_used -= (pos+1);
    memmove(buf, buf+pos+1, buf_used);

    if(line[pos-1] == '\r') line[pos-1] = 0;

	FsmStep(line);
}

void Session::FsmStep(char *line)
{
	switch(state) {
		case typing_name:
			GetPlayer().SetName(line);
			state = waiting_start;
			break;
		case waiting_start:
			SendMessage("Game is not started\n");
			break;
		case waiting_turn: case turn:
			controller.Handle(line, *this);
			break;
	}
}

Session::~Session()
{
	close(fd);
}

