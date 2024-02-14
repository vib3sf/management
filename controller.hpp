#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "session.hpp"
#include "utils.hpp"
#include "bank.hpp"

struct Handler
{
	const char *name;
	void (Controller::*method)();
	bool turn;
};

class Controller
{
	private:
		int argc;
		char **argv;
		Session *session;
		Node<Session> *&sess_list;
		Bank bank;

		void MarketHandler();
		void InfoHandler();
		void PlayerHandler();
		void ProdHandler();
		void PlaceBet(bet_types type);
		void BuyHandler();
		void SellHandler();
		void BuildHandler();
		void OpenHandler();
		void TurnHandler();

		static const Handler handlers[];

		bool ArgcCheck(const char *err_msg, int right);
		bool NumCheck(const char *err_msg, int i, int& num);

	public:
		Controller(Node<Session> *&sess_list, int players)
			: sess_list(sess_list), bank(players) {  }

		void Handle(char *line, Session& session);
};

#endif
