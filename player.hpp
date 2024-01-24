#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "bank.hpp"
#include "utils.hpp"
#include "stdio.h"

class Factory {
	private:
		int build_time;
		bool is_used, is_open;
		
	public:
		Factory(int build_time) : 
			build_time(build_time), is_used(false), is_open(false) {  }

		inline bool IsUsed() { return is_used; }
		inline void Block() { is_used = true; }
		inline void Unblock() { is_used = false; }
		inline void DecreaseBuildTime() { build_time--; }
		inline bool IsBuilt() { return build_time == 0; }
		inline bool IsOpen() { return is_open; }
		inline void Open() { is_open = true; }

		static inline int FreeFactsCount(Node<Factory> *facts) 
		{
			return facts ? FreeFactsCount(facts->next) + 
				facts->data.is_open : 0; 
		}
};

enum bet_results { 
	success_bet, min_price_err, max_price_err, no_money_err, no_product_err};

enum prod_results {
	success_prod, no_resources_err, no_factories_err };

class Player {
	private:
		static const char *player_msg; 

		char *name;
		int num, money, materials, products;
		Node<Factory> *factories;
		
	public:
		Player();

		bool BuildFactory();
		prod_results CreateProduct(int count);
		inline void AddMoney(int money) { this->money += money; }
		void AddMaterial(int materials) { this->materials += materials; }
		void ReduceProduct(int products) { this->products -= products; }
		bet_results PlaceBet(Bank& bank, int value, int count, bet_types type);
		inline int GetNum() const { return num; }
		inline void SetNum(int num) { this->num = num; }
		inline const char *GetName() const { return name; }
		inline void SetName(char *name) 
		{ 
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name); 
		}
		void UpdateFactories();
		
		inline void GetInfo(char *buf) const
		{
			sprintf(buf, player_msg,
				num, name, 
				money, Factory::FreeFactsCount(factories), 
				materials, products);
		}
};

#endif
