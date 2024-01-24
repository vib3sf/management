#ifndef GAME_HPP
#define GAME_HPP

#include "string.h"
#include "utils.hpp"
#include "stdio.h"

class Player;

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

		void FinishMonth(int players);
		void UpdateMarket();

		void AddProductBet(ProductBet bet);
		void AddMaterialBet(MaterialBet bet);

		inline int GetProductMax() const 
		{ return 7000 - level * 500; }

		inline int GetProductCount() const 
		{ return 0.5 * (level + 1) * traders; }

		inline int GetMaterialMin() const 
		{ return 925 - level * 125; }

		inline int GetMaterialCount() const 
		{ return 0.5 * (7 - level) * traders; }

		inline void GetMarketInfo(int players, char *buf) const
		{
			sprintf(buf, market_msg, 
				month,
				players,
				GetMaterialCount(), GetMaterialMin(),
				GetProductCount(), GetProductMax());
		}

};

#endif
