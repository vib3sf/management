#include "player.hpp"

#include "stdio.h"

bool Player::BuildFactory()
{
	if(money < 5000)
		return false;

	money -= 5000;
	factories++;
	return true;
}

prod_results Player::CreateProduct(int count)
{
	if(fact_used + count > factories)
		return no_factories_err;
	if(money < 2000 * count || materials < count)
		return no_resources_err;
	
	money -= 2000 * count;
	fact_used += count;
	materials -= count;
	products += count;
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
			money, factories, materials, products);
	return buf;
}

void Player::Update()
{
	fact_used = 0;
}

