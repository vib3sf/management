#include "player.hpp"

#include "stdio.h"

const char *Player::player_msg = 
	"#%d - %s: money  facts  mats  prod\n"
	"%%%15d  %3d%7d%6d\n";

Player::Player() 
	: name(0), money(10000), materials(4), products(2), factories(0)
{  
	for(int i = 0; i < 2; i++) {
		Factory fact(i, 0);
		fact.Open();
		Node<Factory>::Append(fact, factories);
	}
}

bool Player::BuildFactory()
{
	if(money < 2500)
		return false;

	money -= 2500;
	Factory fact(Node<Factory>::Len(factories), 5);
	Node<Factory>::Append(fact, factories);
	return true;
}

bool Player::FindFactory(int n, Factory *&fact)
{
	Node<Factory> *tmp = factories;
	for(tmp = tmp; tmp && tmp->data.GetN() != n; tmp = tmp->next) 
	{  }
		
	if(tmp)
		fact = &tmp->data;

	return tmp;
}

openfact_results Player::OpenFactory(int n)
{
	if(money < 2500)
		return no_money_open_err;
	
	Factory *fact;
	if(!FindFactory(n, fact))
		return wrong_fact_err;

	if(!fact->IsBuilt())
		return no_built_err;

	if(fact->IsOpen())
		return already_open_err;
	
	fact->Open();
	money -= 2500;
	return success_open;
}

prod_results Player::CreateProduct(int count)
{
	if(money < 2000 * count || materials < count)
		return no_resources_err;
	
	Factory **facts = new Factory*[count];
	int free_facts = 0;
	for(Node<Factory> *node = factories; node; node = node->next) {
		Factory& fact = node->data;
		if(!fact.IsUsed()) {
			facts[free_facts] = &fact;
			free_facts++;

			if(free_facts == count)
				break;
		}
	}

	if(count != free_facts) {
		delete[] facts;
		return no_factories_err;
	}

	money -= 2000 * count;
	materials -= count;
	products += count;
	for(int i = 0; i < count; i++)
		facts[i]->Block();

	delete[] facts;
	return success_prod;
}

bet_results Player::PlaceBet(Bank& bank, int value, int count, bet_types type)
{
	if(IsPlaced(type))
		return already_placed_err;

	switch(type)
	{
		case product:
			if(bank.GetProductMax() < value)
				return max_price_err;
			if(count > products)
				return no_product_err;

			bank.AddProductBet(ProductBet(*this, value, count));
			sell_placed = true;
			break;

		case material:
			if(bank.GetMaterialMin() > value)
				return min_price_err;
			if(value * count > money)
				return no_money_bet_err;

			bank.AddMaterialBet(MaterialBet(*this, value, count));
			money -= value * count;
			buy_placed = true;
			break;
	}

	return success_bet;
}

void Player::Update()
{
	for(Node<Factory> *node = factories; node; node = node->next) {
		Factory& fact = node->data;
		fact.Unblock();

		if(!fact.IsBuilt())
			fact.DecreaseBuildTime();
	}

	money -= (
			300 * materials + 
			500 * products + 
			1000 * Node<Factory>::Len(factories)
			);

	sell_placed = false;
	buy_placed = false;
}

