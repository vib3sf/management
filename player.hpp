#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "bank.hpp"
#include "utils.hpp"
#include "stdio.h"

class Factory {
	private:
		int n, build_time;
		bool is_used, is_open;
		
	public:
		static const char *factory_info;
		Factory(int n, int build_time) : 
			n(n), build_time(build_time), is_used(false), is_open(false) {  }

		inline int GetN() const { return n; }
		inline bool IsUsed() const { return is_used; }
		inline void Block() { is_used = true; }
		inline void Unblock() { is_used = false; }
		inline void DecreaseBuildTime() { build_time--; }
		inline bool IsBuilt() const { return build_time == 0; }
		inline bool IsOpen() const { return is_open; }
		inline void Open() { is_open = true; }

		static inline int FreeFactsCount(Node<Factory> *facts) 
		{
			return facts ? FreeFactsCount(facts->next) + 
				facts->data.is_open : 0; 
		}
};

enum bet_results 
{ 
	success_bet, already_placed_err, 
	min_price_err, max_price_err, no_money_bet_err, no_product_err
};

enum prod_results 
{
	success_prod, no_resources_err, no_factories_err 
};

enum openfact_results
{
	success_open, no_money_open_err, 
	wrong_fact_err, no_built_err, already_open_err
};

class Player {
	private:
		static const char *player_msg; 

		char *name;
		int num, money, materials, products;
		Node<Factory> *factories;
		bool sell_placed, buy_placed;
		
	public:
		Player();

		bool BuildFactory();
		bool FindFactory(int n, Factory *&fact);
		openfact_results OpenFactory(int n);
		prod_results CreateProduct(int count);
		inline void AddMoney(int money) { this->money += money; }
		void AddMaterial(int materials) { this->materials += materials; }
		void ReduceProduct(int products) { this->products -= products; }
		bet_results PlaceBet(Bank& bank, int value, int count, bet_types type);
		inline int GetNum() const { return num; }
		inline bool IsPlaced(bet_types type) 
		{ 
			return type == product ? sell_placed : buy_placed; 
		}
		inline bool IsBankrupt() { return money < 0; }
		inline void SetNum(int num) { this->num = num; }
		inline const char *GetName() const { return name; }
		inline void SetName(char *name) 
		{ 
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name); 
		}
		void Update();

		inline void GetInfo(char *buf) const
		{
			int offset = sprintf(buf, player_msg,
								num, name, 
								money, materials, products);

			for(const Node<Factory> *node = factories; node; node = node->next) {
				const Factory& fact = node->data;
				offset += sprintf(buf + offset, Factory::factory_info,
								fact.GetN(), 
								fact.IsBuilt(), fact.IsOpen(), fact.IsUsed());
			}
		}

		inline int GetInfoSize() const 
		{ 
			return 128 + Node<Factory>::Len(factories) * 64;
		}
};

#endif
