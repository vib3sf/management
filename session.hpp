#ifndef SESSION_HPP
#define SESSION_HPP

#include "player.hpp"

class Controller;

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

		Controller& controller;
		Player player;

		void CheckLf();
		void FsmStep(char *line);

	public:
		Session(int fd, struct sockaddr_in *from, Controller& controller);

		void SendMessage(const char *str) const;
		bool DoRead();
		void Start();
		inline void TakeTurn() { state = turn; };
		inline void LoseTurn() { state = waiting_turn; }
		inline int GetFd() const { return fd; };
		inline bool IsReady() const { return state == waiting_start; };
		inline bool HasTurn() const { return state == turn; }
		inline Player& GetPlayer() { return player; };

		~Session();
};

#endif
