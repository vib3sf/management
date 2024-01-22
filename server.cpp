#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "server.hpp"
#include "utils.hpp"

Server::Server(long port, int max_players) 
	: ls(socket(AF_INET, SOCK_STREAM, 0)), max_players(max_players), 
	started(false), sess_list(0), game(max_players)
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
                ssr = sess_node->data.DoRead(sess_list, game);
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

	Session *sess = new Session(sd, &addr);
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

Session::Session(int fd, struct sockaddr_in *from) 
	: state(typing_name), fd(fd), from_ip(htonl(from->sin_addr.s_addr)), 
	from_port(htons(from->sin_port)), buf_used(0)
{
	SendMessage("Type your name: ");
}

void Session::SendMessage(const char *str) const
{
	write(fd, str, strlen(str));
}

bool Session::DoRead(Node<Session> *sess_list, Bank& game)
{
	int rc, bufp = buf_used;
	rc = read(fd, buf + bufp, buf_size - bufp);
	if(rc <= 0) {
		return false;
	}

	buf_used = rc;
	CheckLf(sess_list, game);
	
	return true;
}

void Session::Start()
{
	state = waiting_turn;
	SendMessage("Bank started.\n");
}

void Session::CheckLf(Node<Session> *sess_list, Bank& game)
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

	FsmStep(line, sess_list, game);
}

void Session::FsmStep(char *line, Node<Session> *sess_list, Bank& game)
{
	switch(state) {
		case typing_name:
			GetPlayer().SetName(line);
			state = waiting_start;
			break;
		case waiting_start:
			SendMessage("Bank is not started\n");
			break;
		case waiting_turn:
			HandleCommand(line, sess_list, game);
			break;
		case turn:
			HandleCommand(line, sess_list, game, true);
			break;
	}
}

// market, player, prod, buy, sell, build, turn, help

void Session::HandleCommand(char *line, Node<Session> *sess_list, Bank& game, bool turn)
{
	int argc;
	char **argv = split_line(line, argc);
	if(!argv)
		return;
	
	if(!strcmp(argv[0], "market")) {
		MarketHandler(game, sess_list);
	}
	else if(!strcmp(argv[0], "player")) {
		PlayerHandler(argv, argc, sess_list);
	}
	else if(!strcmp(argv[0], "prod") && HasTurn()) {
		ProdHandler(argv, argc);
	}
	else if(!strcmp(argv[0], "buy") && HasTurn()) {
		BuyHandler(argv, argc, game);
	}
	else if(!strcmp(argv[0], "sell") && HasTurn()) {
		SellHandler(argv, argc, game);
	}
	else if(!strcmp(argv[0], "build") && HasTurn())
		BuildHandler();
	else if(!strcmp(argv[0], "turn") && HasTurn())
		TurnHandler(sess_list, game);
}

void Session::MarketHandler(const Bank& game, Node<Session> *sess_list)
{
	const char *info = game.GetMarketInfo(Node<Session>::Len(sess_list));
	SendMessage(info);
	delete info;
}

void Session::PlayerHandler(char **argv, int argc, Node<Session> *sess_list)
{
	bool ok;
	int num;
	if(argc != 2)
	{
		SendMessage("Usage: player <int:num>\n");
		return;
	}
	
	ok = str_to_int(argv[1], num);
	if(!ok) {
		SendMessage("Invalid player num\n");
		return;
	}

	Node<Session> *node = sess_list;
	while(node && node->data.GetPlayer().GetNum() != num) {
		node = node->next;
	}

	if(node)
	{
		const char *info = node->data.GetPlayer().GetInfo();
		SendMessage(info);
		delete info;
	}
	else
		SendMessage("Player num doesn't exist.\n");
}

void Session::ProdHandler(char **argv, int argc)
{
	bool ok;
	int num;

	if(argc != 2)
	{
		SendMessage("Usage: prod <int:num>\n");
		return;
	}
	
	ok = str_to_int(argv[1], num);
	if(!ok) {
		SendMessage("Invalid product count\n");
		return;
	}

	prod_results res = GetPlayer().CreateProduct(num);
	switch(res) {
		case no_resources_err:
			SendMessage("Not enough resources for creating\n");
			break;
		case no_factories_err:
			SendMessage("Not enough factories for creating\n");
			break;
		case success_prod:
			SendMessage("Production was successful\n");
			break;
	}
}

void Session::BuyHandler(char **argv, int argc, Bank& bank)
{
	PlaceBet(argv, argc, bank, material);
}

void Session::SellHandler(char **argv, int argc, Bank& bank)
{
	PlaceBet(argv, argc, bank, product);
}

void Session::BuildHandler()
{
	bool ok = GetPlayer().BuildFactory();
	if(!ok)
		SendMessage("Not enough money\n");
}

void Session::PlaceBet(char **argv, int argc, Bank& bank, bet_types type)
{
	bool ok;
	int value, count;
	if(argc != 3)
	{
		SendMessage("Usage: bet <int:count> <int:value>\n");
		return;
	}
	
	ok = str_to_int(argv[1], count);
	if(!ok) {
		SendMessage("Invalid count\n");
		return;
	}

	ok = str_to_int(argv[2], value);
	if(!ok) {
		SendMessage("Invalid vaule count\n");
		return;
	}

	bet_results res = GetPlayer().PlaceBet(bank, value, count, type);
	char buf[32];
	switch(res)
	{
		case max_price_err:
			sprintf(buf, "Max selling price is %d\n", bank.GetProductMax());
			SendMessage(buf);
			break;
		case min_price_err:
			sprintf(buf, "Min buying price is %d\n", bank.GetMaterialMin());
			SendMessage(buf);
			break;
		case no_money_err:
			SendMessage("Not enough money for buying\n");
			break;
		case no_product_err:
			SendMessage("Not enough products for selling\n");
			break;
		case success_bet:
			SendMessage("Bet is placed\n");
			break;
	}
}

void Session::TurnHandler(Node<Session> *sess_list, Bank& game)
{
	GiveTurn(sess_list, game);
}

void Session::GiveTurn(Node<Session> *sess_list, Bank& game)
{
	state = waiting_turn;
	Node<Session> *tmp = sess_list;
	while(!tmp || &tmp->data != this) { 
		tmp = tmp->next; 
	}
	tmp->data.GetPlayer().Update();
	if(tmp->next)
		tmp->next->data.TakeTurn();
	else {
		sess_list->data.TakeTurn();
		game.FinishMonth(Node<Session>::Len(sess_list));
	}
}

void Session::TakeTurn()
{
	state = turn;
}

Session::~Session()
{
	close(fd);
}

