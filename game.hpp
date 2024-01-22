#ifndef GAME_HPP
#define GAME_HPP

#include "string.h"
#include "utils.hpp"

class Player;
class Bank;

enum bet_types { product, material };

class Bet {
	protected:
		Player& player;
		int value, count;

	public:
		Bet(Player& player, int value, int count)
			: player(player), value(value), count(count) {  }
		
		inline Player& GetPlayer() { return player; }
		inline int GetValue() const { return value; }
		inline int GetCount() const { return count; }
		inline void SetCount(int count) { this->count = count; }

		virtual ~Bet() = 0;
};

class ProductBet : public Bet {
	public:
		ProductBet(Player& player, int value, int count)
			: Bet(player, value, count) {  }

		bool operator>(const ProductBet& bet) const 
		{ 
			return value < bet.GetValue(); 
		}
};

class MaterialBet : public Bet {
	private:
		int cash_back;
	public:
		MaterialBet(Player& player, int value, int count)
			: Bet(player, value, count), cash_back(0) {  }

		inline int GetCashBack() const { return cash_back; }
		inline void CalculateCashBack(int available) 
		{ 
			 cash_back = (count - available) * value; 
		}

		bool operator>(const MaterialBet& bet) const
		{
			return value > bet.GetValue();
		}

		virtual ~MaterialBet();
};

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

class Bank {
	private:
		const static int level_change[5][5];
		const static char *market_msg;

		template<class T>
		void CollectWinningBets(
				Node<Bet&> *&winlist, Node<T> *item_list, int available);

		void FreeBetLists();
		void CollectWinningList(
				Node<Bet&> *&winlist, int available_pr, int available_mat);
		template <class T>
		void AddBet(T bet, Node<T> *&list);

		int month, level, traders;
		Node<ProductBet> *betpr_list;
		Node<MaterialBet> *betmat_list;

	public:
		Bank(int players) 
			: month(1), level(3), traders(players), 
			betpr_list(0), betmat_list(0) {  }

		Node<Bet&> *GetWinningBets();

		const char *GetMarketInfo(int players) const;
		void FinishMonth(int players);
		void UpdateMarket();

		void AddProductBet(ProductBet bet);
		void AddMaterialBet(MaterialBet bet);

		inline int GetProductMax() const { return 7000 - level * 500; }
		inline int GetProductCount() const { return 0.5 * (level + 1) * traders; }
		inline int GetMaterialMin() const { return 925 - level * 125; }
		inline int GetMaterialCount() const { return 0.5 * (7 - level) * traders; }
};

#endif
