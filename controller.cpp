#include "controller.hpp"

#include "stdio.h"
#include "stdlib.h"

void Controller::Handle(char *line, Session& session)
{
	argv = split_line(line, argc);
	this->session = &session;

	if(!argv)
		return;
	
	if(!strcmp(argv[0], "market"))
		MarketHandler();
	else if(!strcmp(argv[0], "player"))
		PlayerHandler();
	else if(!strcmp(argv[0], "prod") && this->session->HasTurn())
		ProdHandler();
	else if(!strcmp(argv[0], "buy") && this->session->HasTurn())
		BuyHandler();
	else if(!strcmp(argv[0], "sell") && this->session->HasTurn())
		SellHandler();
	else if(!strcmp(argv[0], "build") && this->session->HasTurn())
		BuildHandler();
	else if(!strcmp(argv[0], "turn") && this->session->HasTurn())
		TurnHandler();

}

void Controller::MarketHandler()
{
	const char *info = bank.GetMarketInfo(Node<Session>::Len(sess_list));
	session->SendMessage(info);
	delete info;
}

void Controller::PlayerHandler()
{
	bool ok;
	int num;
	if(argc != 2)
	{
		session->SendMessage("Usage: player <int:num>\n");
		return;
	}
	
	ok = str_to_int(argv[1], num);
	if(!ok) {
		session->SendMessage("Invalid player num\n");
		return;
	}

	Node<Session> *node = sess_list;
	while(node && node->data.GetPlayer().GetNum() != num) {
		node = node->next;
	}

	if(node)
	{
		const char *info = node->data.GetPlayer().GetInfo();
		session->SendMessage(info);
		delete info;
	}
	else
		session->SendMessage("Player num doesn't exist.\n");
}

void Controller::ProdHandler()
{
	bool ok;
	int num;

	if(argc != 2)
	{
		session->SendMessage("Usage: prod <int:num>\n");
		return;
	}
	
	ok = str_to_int(argv[1], num);
	if(!ok) {
		session->SendMessage("Invalid product count\n");
		return;
	}

	prod_results res = session->GetPlayer().CreateProduct(num);
	switch(res) {
		case no_resources_err:
			session->SendMessage("Not enough resources for creating\n");
			break;
		case no_factories_err:
			session->SendMessage("Not enough factories for creating\n");
			break;
		case success_prod:
			session->SendMessage("Production was successful\n");
			break;
	}
}

void Controller::BuyHandler()
{
	PlaceBet(material);
}

void Controller::SellHandler()
{
	PlaceBet(product);
}

void Controller::PlaceBet(bet_types type)
{
	bool ok;
	int value, count;
	if(argc != 3)
	{
		session->SendMessage("Usage: bet <int:count> <int:value>\n");
		return;
	}
	
	ok = str_to_int(argv[1], count);
	if(!ok) {
		session->SendMessage("Invalid count\n");
		return;
	}

	ok = str_to_int(argv[2], value);
	if(!ok) {
		session->SendMessage("Invalid vaule count\n");
		return;
	}

	bet_results res = session->GetPlayer().PlaceBet(bank, value, count, type);
	char buf[32];
	switch(res) {
		case max_price_err:
			sprintf(buf, "Max selling price is %d\n", bank.GetProductMax());
			session->SendMessage(buf);
			break;
		case min_price_err:
			sprintf(buf, "Min buying price is %d\n", bank.GetMaterialMin());
			session->SendMessage(buf);
			break;
		case no_money_err:
			session->SendMessage("Not enough money for buying\n");
			break;
		case no_product_err:
			session->SendMessage("Not enough products for selling\n");
			break;
		case success_bet:
			session->SendMessage("Bet is placed\n");
			break;
	}
}

void Controller::BuildHandler()
{
	bool ok = session->GetPlayer().BuildFactory();
	if(!ok)
		session->SendMessage("Not enough money\n");
}


void Controller::TurnHandler()
{
	session->LoseTurn();
	Node<Session> *tmp = sess_list;
	while(tmp && &tmp->data != session) { 
		tmp = tmp->next; 
	}
	tmp->data.GetPlayer().UpdateFactories();
	if(tmp->next)
		tmp->next->data.TakeTurn();
	else {
		sess_list->data.TakeTurn();
		bank.FinishMonth(Node<Session>::Len(sess_list));
	}
}
