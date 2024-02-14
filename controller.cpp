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
	{ "open",	&Controller::OpenHandler,	true	},
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
	char info[256];
	bank.GetMarketInfo(Node<Session>::Len(sess_list), info);
	session->SendMessage(info);
}

void Controller::InfoHandler()
{
	char *info = new char[session->GetPlayer().GetInfoSize()];
	session->GetPlayer().GetInfo(info);
	session->SendMessage(info);
	delete[] info;
}

void Controller::PlayerHandler()
{
	if(!ArgcCheck("Usage: player <int:num>\n", 2)) 
		return;

	int num = str_to_int(argv[1], num);
	Node<Session> *node = sess_list;
	const Player& player = node->data.GetPlayer(); 

	while(node && player.GetNum() != num && strcmp(argv[1], player.GetName()))
		node = node->next;

	if(node)
	{
		char *info = new char[session->GetPlayer().GetInfoSize()];
		node->data.GetPlayer().GetInfo(info);
		session->SendMessage(info);
		delete[] info;
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
		case already_placed_err:
			void OpenHandler();
			session->SendMessage("This type of bet has already been placed\n");
			break;
		case max_price_err:
			sprintf(buf, "Max selling price is %d\n", bank.GetProductMax());
			session->SendMessage(buf);
			break;
		case min_price_err:
			sprintf(buf, "Min buying price is %d\n", bank.GetMaterialMin());
			session->SendMessage(buf);
			break;
		case no_money_bet_err:
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
	session->SendMessage(session->GetPlayer().BuildFactory() ? 
		"Factory building started\n" : "Not enough money\n");
}

void Controller::OpenHandler()
{
	int num;
	if(!(ArgcCheck("Usage: open <int:num>\n", 2) && 
			NumCheck("Invalid factory\n", 1, num)))
		return;

	switch(session->GetPlayer().OpenFactory(num)) {
		case success_open:
			session->SendMessage("Factory is open\n");
			break;
		case no_money_open_err:
			session->SendMessage("Not enough money for opening\n");
			break;
		case wrong_fact_err:
			session->SendMessage("Wrong factory\n");
			break;
		case no_built_err:
			session->SendMessage("Factory is not been built\n");
			break;
		case already_open_err:
			session->SendMessage("Factory is open yet\n");
			break;
	}
}

void Controller::TurnHandler()
{
	session->LoseTurn();

	Node<Session> *tmp = sess_list;
	while(tmp && &tmp->data != session) 
		tmp = tmp->next; 

	if(tmp->next)
		tmp->next->data.TakeTurn();
	else {
		sess_list->data.TakeTurn();
		bank.FinishMonth(Node<Session>::Len(sess_list));
		for(Node<Session> *node = sess_list; node; node = node->next) {
			Player& player = node->data.GetPlayer();
			player.Update();
			if(player.IsBankrupt())
			{
				session->SendMessage("You are bankrupt\nGame over\n");
				Node<Session>::Remove(node, sess_list);
			}
		}
		if(Node<Session>::Len(sess_list) == 1) {
			sess_list->data.SendMessage("You won\n");
			exit(0);
		}
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
