#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include "utils.hpp"
#include "game.hpp"

class Session {
	private:
		enum fsm_states {
			typing_name, waiting_start, waiting_turn, turn
		} state;

		static const int buf_size = 1024;

		int fd;
		unsigned long from_ip;
		unsigned short from_port;
		char buf[buf_size];
		int buf_used;

		Player player;

		inline void SendMessage(const char *str) const;
		void CheckLf(Node<Session> *sess_list, Bank& game);
		void FsmStep(char *line, Node<Session> *sess_list, Bank& game);
		void HandleCommand(char *line, Node<Session> *sess_list, 
				Bank& game, bool turn=false);
		void HandleCommand(char *line);
		char **SplitLine(char *line);
		void MakeBuf(char *buf, int& buf_size, int& cur_size);
		void GiveTurn(Node<Session> *sess_list, Bank& game);

	public:
		Session(int fd, struct sockaddr_in *from);
		bool DoRead(Node<Session> *sess_list, Bank& game);
		inline void Start();
		inline int GetFd() const { return fd; };
		inline void TakeTurn();
		inline bool IsReady() const { return state == waiting_start; };
		inline bool HasTurn() const { return state == turn; }
		inline Player& GetPlayer() { return player; };

		void MarketHandler(const Bank& game, Node<Session> *sess_list);
		void PlayerHandler(char **argv, int argc, Node<Session> *sess_list);
		void ProdHandler(char **argv, int argc);
		void PlaceBet(char **argv, int argc, Bank& bank, bet_types type);
		void BuyHandler(char **argv, int argc, Bank& bank);
		void SellHandler(char **argv, int argc, Bank& bank);
		void BuildHandler();
		void TurnHandler(Node<Session> *sess_list, Bank& game);
		~Session();
};

class Server {
	private:
		static const int listen_qlen = 16;
		int ls, max_players;
		bool started;
		Node<Session> *sess_list;
		Bank game;

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
