#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>

#include "utils.hpp"
#include "session.hpp"
#include "controller.hpp"

class Session;

class Server {
	private:
		static const int listen_qlen = 16;
		int ls, max_players;
		bool started;
		Node<Session> *sess_list;
		Controller controller;

		void Init(long port);
		void CheckState();
		void StartGame();
		void AcceptClient();
		int PlayerCount();
		void CloseSession(Node<Session> *sess_node);

	public:
		Server(long port, int max_players);
		void Run();

};

#endif
