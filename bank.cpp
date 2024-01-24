#include "bank.hpp"
#include "player.hpp"

#include "utils.hpp"
#include "stdio.h"
#include "stdlib.h"

Bet::~Bet() {  }

MaterialBet::~MaterialBet() { player.AddMoney(cash_back); }

const int Bank::level_change[5][5] = {
	{4, 4, 2, 1, 1},
	{3, 4, 3, 1, 1},
	{1, 3, 4, 3, 1},
	{1, 1, 3, 4, 3},
	{1, 1, 2, 4, 4}
};

void Bank::UpdateMarket()
{
	int i = 0, r = 1 + (int)(12.0 * rand() / (RAND_MAX+1.0));
	while(r > 0) {
		r -= level_change[level][i];
		i++;
	}
	level = i;

	FreeBetLists();
}

void Bank::FreeBetLists()
{
	Node<ProductBet>::Free(betpr_list);
	Node<MaterialBet>::Free(betmat_list);
}

template <class T>
void Bank::AddBet(T bet, Node<T> *&list)
{
	Node<T> *node = new Node<T>(bet);

	if(!list)
		list = node;
	else {
		Node<T> *tmp = list, *prev = 0;
		while(tmp && tmp->data > node->data){
			prev = tmp;
			tmp = tmp->next;
		}
		node->next = tmp;
		if(prev)
			prev->next = node;
		else
			list = node;
	}
}

void Bank::AddProductBet(ProductBet bet)
{
	AddBet(bet, betpr_list);
}

void Bank::AddMaterialBet(MaterialBet bet)
{
	AddBet(bet, betmat_list);
}

Node<Bet&> *Bank::GetWinningBets()
{
	int available_pr = GetProductCount(), available_mat = GetMaterialCount();

	Node<Bet&> *winlist = 0;
	CollectWinningList(winlist, available_pr, available_mat);

	return winlist;
}

void Bank::CollectWinningList(Node<Bet&> *&winlist, int available_pr, 
		int available_mat)
{
	CollectWinningBets(winlist, betpr_list, available_pr);
	CollectWinningBets(winlist, betmat_list, available_mat);
}

template<class T>
void Bank::CollectWinningBets(Node<Bet&> *&winlist, Node<T> *item_list, int available)
{
	for(Node<T> *node = item_list; node; node = node->next) {
		T& bet = node->data;
		if(available < bet.GetCount()) {
			MaterialBet *cast = dynamic_cast<MaterialBet*>(&bet);
			if(cast)
				cast->CalculateCashBack(available);

			bet.SetCount(available);
			Node<Bet&>::Append(bet, winlist);
			break;
		}
		Node<Bet&>::Append(bet, winlist);
		available -= bet.GetCount();
	}
}

void Bank::FinishMonth(int players) 
{ 
	month++; 
	printf("Month: %d\n", month); 

	Node<Bet&> *winlist = GetWinningBets();
	for(Node<Bet&> *node = winlist; node; node = node->next) {
		Bet& bet = node->data;
		ProductBet *p_cast = dynamic_cast<ProductBet*>(&bet);
		if(p_cast) {
			p_cast->GetPlayer().AddMoney(
					p_cast->GetValue() * p_cast->GetCount());
			p_cast->GetPlayer().ReduceProduct(p_cast->GetCount());
		}
		else {
			MaterialBet *m_cast = static_cast<MaterialBet*>(&bet);
			m_cast->GetPlayer().AddMaterial(m_cast->GetCount());
		}
	}

	traders = players;
	UpdateMarket();
}

const char *Bank::market_msg = 
	"Current month is %dth\n"
	"Players still active:\n"
	"%%%12d\n"
	"Bank sells: items  min.price\n"
	"%%%12d%9d\n"
	"Bank buys:  items  max.price\n"
	"%%%12d%10d\n";

