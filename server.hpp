#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include "player.hpp"

struct SessionNode;

class Session {
	private:
		enum fsm_states {
			waiting_start, waiting_turn, turn
		} state;

		static const int buf_size = 1024;

		int fd;
		unsigned long from_ip;
		unsigned short from_port;
		char buf[buf_size];
		int buf_used;

		Player player;

		inline void SendMessage(const char *str) const;
		void CheckLf(SessionNode *sess_list);
		void FsmStep(char *line, SessionNode *sess_list);
		void GiveTurn(SessionNode *sess_list);

	public:
		Session(int fd, struct sockaddr_in *from);
		bool DoRead(SessionNode *sess_list);
		inline void Start();
		inline int GetFd() { return fd; };
		inline void TakeTurn();
		~Session();
};

struct SessionNode {
	Session sess;
	SessionNode *next;

	SessionNode(const Session &sess);
};

class Server {
	private:
		static const int listen_qlen = 16;
		int ls, max_players;
		bool started;
		SessionNode *sess_list;

		void Init(long port);
		void CheckState();
		void StartGame();
		void AcceptClient();
		int PlayerCount();
		void CloseSession(SessionNode *sess_node);

	public:
		Server(long port, int max_players);
		void Run();

};

#endif
