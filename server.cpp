#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "server.hpp"
#include "utils.hpp"

Server::Server(long port, int max_players) 
	: ls(socket(AF_INET, SOCK_STREAM, 0)), max_players(max_players), 
	started(false), sess_list(0), controller(sess_list, max_players)
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
		CheckState();

        FD_ZERO(&readfds);       
        FD_SET(ls, &readfds);
        maxfd = ls;
        for(Node<Session> *sess_node = sess_list; sess_node; 
				sess_node = sess_node->next) {
			FD_SET(sess_node->data.GetFd(), &readfds);
			if(sess_node->data.GetFd() > maxfd)
				maxfd = sess_node->data.GetFd();
        }

        sr = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if(sr == -1) {
            perror("select");
			exit(4);
        }

        if(FD_ISSET(ls, &readfds))
			AcceptClient();
										
		Node<Session> *sess_node = sess_list, *tmp;
        while(sess_node) {
            if(FD_ISSET(sess_node->data.GetFd(), &readfds)) {
                ssr = sess_node->data.DoRead();
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

	Session *sess = new Session(sd, &addr, controller);
	Node<Session>::Append(*sess, sess_list);
}

int Server::PlayerCount()
{
	Node<Session> *node = sess_list;
	int i = 0;
	while(node) {
		if(node->data.IsReady())
			i++;
		node = node->next;
	}
	return i;
}

void Server::CloseSession(Node<Session> *sess_node)
{
	Node<Session>::Remove(sess_node, sess_list);
}

void Server::CheckState()
{
	if(max_players == PlayerCount() && !started)
		StartGame();
}

void Server::StartGame()
{
	int i = 1;
	for(Node<Session> *sess_node = sess_list; sess_node; 
			sess_node = sess_node->next, i++) {
		sess_node->data.Start();
		sess_node->data.GetPlayer().SetNum(i);
	}

	sess_list->data.TakeTurn();
	started = true;
}

