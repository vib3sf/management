#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "bank.hpp"

enum bet_results { 
	success_bet, min_price_err, max_price_err, no_money_err, no_product_err};

enum prod_results {
	success_prod, no_resources_err, no_factories_err };

class Player {
	private:
		static const char *player_msg; 

		char *name;
		int num, money, factories, fact_used, materials, products;
		
	public:
		Player() 
			: name(0), money(10000), factories(2), fact_used(0), 
			materials(4), products(2) {  }

		bool BuildFactory();
		prod_results CreateProduct(int count);
		inline void AddMoney(int money) { this->money += money; }
		void AddMaterial(int materials) { this->materials += materials; }
		void ReduceProduct(int products) { this->products -= products; }
		bet_results PlaceBet(Bank& bank, int value, int count, bet_types type);
		const char *GetInfo() const;
		inline int GetNum() const { return num; }
		inline void SetNum(int num) { this->num = num; }
		inline void SetName(char *name) 
		{ 
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name); 
		}
		void Update();
};

#endif
