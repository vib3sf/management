#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "session.hpp"
#include "utils.hpp"
#include "bank.hpp"

class Controller
{
	private:
		int argc;
		char **argv;
		Session *session;
		Node<Session> *&sess_list;
		Bank bank;

		void MarketHandler();
		void PlayerHandler();
		void ProdHandler();
		void PlaceBet(bet_types type);
		void BuyHandler();
		void SellHandler();
		void BuildHandler();
		void TurnHandler();

	public:
		Controller(Node<Session> *&sess_list, int players)
			: sess_list(sess_list), bank(players) {  }

		void Handle(char *line, Session& session);
};

#endif