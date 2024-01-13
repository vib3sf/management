#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "server.hpp"

SessionNode::SessionNode(const Session &sess)
	: sess(sess)
{  }

Server::Server(long port, int max_players) 
	: ls(socket(AF_INET, SOCK_STREAM, 0)), max_players(max_players), 
	started(false), sess_list(0)
{
	Init(port);
}

void Server::Init(long port)
{
	if(ls == -1) {
        perror("socket");
		exit(1);
    }

    int opt = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));       

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if(bind(ls, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind");
		exit(2);
    }

    listen(ls, listen_qlen);
}

void Server::Run()
{
    fd_set readfds;
    int sr, maxfd;
	bool ssr;
    while(true) {  
        FD_ZERO(&readfds);       
        FD_SET(ls, &readfds);
        maxfd = ls;
        for(SessionNode *sess_node = sess_list; sess_node; 
				sess_node = sess_node->next) {
			FD_SET(sess_node->sess.GetFd(), &readfds);
			if(sess_node->sess.GetFd() > maxfd)
				maxfd = sess_node->sess.GetFd();
        }

        sr = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if(sr == -1) {
            perror("select");
			exit(4);
        }

        if(FD_ISSET(ls, &readfds))
			AcceptClient();
										
		SessionNode *sess_node = sess_list, *tmp;
        while(sess_node) {
            if(FD_ISSET(sess_node->sess.GetFd(), &readfds)) {
                ssr = sess_node->sess.DoRead(sess_list);
                if(!ssr)
				{
					tmp = sess_node->next;
                    CloseSession(sess_node);
					sess_node = tmp;
					continue;
				}
            }
			sess_node = sess_node->next;
        }

		CheckState();
    }
}

void Server::AcceptClient() 
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int sd = accept(ls, (struct sockaddr*) &addr, &len);

    if(sd == -1) {
        perror("accept");
        return;
    }
	Session *sess = new Session(sd, &addr);

	SessionNode *sess_node = new SessionNode(*sess);
	if(!sess_list) {
		sess_list = sess_node;
	}
	else {
		SessionNode *last = sess_list;
		for(; last->next; last = last->next) 
		{  }

		last->next = sess_node;
	}

}

int Server::PlayerCount()
{
	SessionNode *node = sess_list;
	int i = 0;
	while(node)
	{
		i++;
		node = node->next;
	}
	return i;
}

void Server::CloseSession(SessionNode *sess_node)
{
	if(sess_list == sess_node)
		sess_list = sess_node->next;
	else {
		SessionNode *tmp = sess_list;
		while(sess_node != tmp->next)
			tmp = tmp->next; 
		tmp->next = sess_node->next;
	}
	
	delete sess_node;
}

void Server::CheckState()
{
	if(max_players == PlayerCount() && !started)
		StartGame();
}

void Server::StartGame()
{
	for(SessionNode *sess_node = sess_list; sess_node; 
			sess_node = sess_node->next)
		sess_node->sess.Start();

	sess_list->sess.TakeTurn();
	started = true;
}

Session::Session(int fd, struct sockaddr_in *from) 
	: state(waiting_start), fd(fd), from_ip(htonl(from->sin_addr.s_addr)), 
	from_port(htons(from->sin_port)), buf_used(0)
{
	SendMessage("Type your name: ");
}

void Session::SendMessage(const char *str) const
{
	write(fd, str, strlen(str));
}

bool Session::DoRead(SessionNode *sess_list)
{
	int rc, bufp = buf_used;
	rc = read(fd, buf + bufp, buf_size - bufp);
	if(rc <= 0) {
		return false;
	}

	buf_used = rc;
	CheckLf(sess_list);
	
	return true;
}

void Session::Start()
{
	state = waiting_turn;
	SendMessage("Game started.\n");
}

void Session::CheckLf(SessionNode *sess_list)
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

	FsmStep(line, sess_list);
}

void Session::FsmStep(char *line, SessionNode *sess_list)
{
	switch(state) {
		case waiting_start:
			SendMessage("Game is not started\n");
			break;
		case waiting_turn:
			SendMessage("It's not your turn now\n");
			break;
		case turn:
			SendMessage("Your turn\n");
			GiveTurn(sess_list);
			break;
	}
}

void Session::GiveTurn(SessionNode *sess_list)
{
	state = waiting_turn;
	SessionNode *tmp = sess_list;
	while(!tmp || &tmp->sess != this) { 
		tmp = tmp->next; 
	}
	tmp = (tmp->next) ? tmp->next : sess_list;
	tmp->sess.TakeTurn();
}

void Session::TakeTurn()
{
	state = turn;
}

Session::~Session()
{
	close(fd);
}
