#include "player.hpp"

#include "stdio.h"

Player::Player() 
	: name(0), money(10000), materials(4), products(2), factories(0)
{  
	for(int i = 0; i < 2; i++) {
		Factory fact(0);
		fact.Open();
		Node<Factory>::Append(fact, factories);
	}
}

bool Player::BuildFactory()
{
	if(money < 2500)
		return false;

	money -= 2500;
	Factory fact(5);
	Node<Factory>::Append(fact, factories);
	return true;
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
	switch(type)
	{
		case product:
			if(bank.GetProductMax() < value)
				return max_price_err;
			if(count > products)
				return no_product_err;
			bank.AddProductBet(ProductBet(*this, value, count));
			break;
		case material:
			if(bank.GetMaterialMin() > value)
				return min_price_err;
			if(value * count > money)
				return no_money_err;
			bank.AddMaterialBet(MaterialBet(*this, value, count));
			money -= value * count;
			break;
	}

	return success_bet;
}

const char *Player::player_msg = 
	"#%d - %s: money  facts  mats  prod\n"
	"%%%15d  %3d%7d%6d\n";

const char *Player::GetInfo() const
{
	char *buf = new char[128];
	sprintf(buf, player_msg,
			num, name, 
			money, Factory::FreeFactsCount(factories), materials, products);
	return buf;
}

void Player::UpdateFactories()
{
	for(Node<Factory> *node = factories; node; node = node->next) {
		Factory& fact = node->data;
		fact.Unblock();
		if(!fact.IsBuilt())
			fact.DecreaseBuildTime();
		else if(!fact.IsOpen() && money >= 2500) {
			fact.Open();
			money -= 2500;
		}
	}
}

