#include "controller.hpp"

#include "stdio.h"
#include "stdlib.h"

const Handler Controller::handlers[] = {
	{ "market",	&Controller::MarketHandler, false 	},
	{ "info", 	&Controller::InfoHandler, 	false	},
	{ "player", &Controller::PlayerHandler, false	},
	{ "prod", 	&Controller::ProdHandler,	true	},
	{ "buy", 	&Controller::BuyHandler,	true  	},
	{ "sell", 	&Controller::SellHandler,	true	},
	{ "build", 	&Controller::BuildHandler, 	true	},
	{ "turn", 	&Controller::TurnHandler, 	true	},
	0
};

void Controller::Handle(char *line, Session& session)
{
	argv = split_line(line, argc);
	this->session = &session;

	if(!argv)
		return;

	for(const Handler *h = handlers; h->name; h++)
		if(!strcmp(line, h->name)) {
			if(!session.HasTurn() && h->turn)
				session.SendMessage("It is not your turn\n");
			else
				((*this).*(h->method))();
			return;
		}
	session.SendMessage("Wrong command\n");
}

void Controller::MarketHandler()
{
	const char *info = bank.GetMarketInfo(Node<Session>::Len(sess_list));
	session->SendMessage(info);
	delete info;
}

void Controller::InfoHandler()
{
	const char *info = session->GetPlayer().GetInfo();
	session->SendMessage(info);
	delete info;
}

void Controller::PlayerHandler()
{
	int num;
	if(!(ArgcCheck("Usage: player <int:num>\n", 2) && 
			NumCheck("Invalid player num\n", 1, num)))
		return;

	Node<Session> *node = sess_list;
	while(node && node->data.GetPlayer().GetNum() != num)
		node = node->next;

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
	int num;
	if(!(ArgcCheck("Usage: prod <int:num>\n", 2) && 
			NumCheck("Invalid product count\n", 1, num)))
		return;

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
	int value, count;

	if(!(ArgcCheck("Usage: bet <int:count> <int:value>\n", 3) && 
			NumCheck("Invalid count\n", 1, count) &&
			NumCheck("Invalid value count\n", 2, value)))
		return;

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

bool Controller::ArgcCheck(const char *err_msg, int right)
{
	if(argc != right)
	{
		session->SendMessage(err_msg);
		return false;
	}
	return true;
}

bool Controller::NumCheck(const char *err_msg, int i, int& num)
{
	if(!str_to_int(argv[i], num)) {
		session->SendMessage(err_msg);
		return false;
	}
	return true;
}
