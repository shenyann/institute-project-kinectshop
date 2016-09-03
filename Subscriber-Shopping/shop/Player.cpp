#include "Player.h"
#include "iostream"
Player::Player()
{

}


void Player::init(string name,int money){

_name=name;
_money=money;


}


void Player::printInventory(){

	cout <<" *** "<<  _name << "'s inventory *** \n\n ";
	cout << "Money: "<< _money <<" Euro \n\n";
	list<Item>::iterator lit;

int i = 0;

for (lit =_items.begin(); lit != _items.end(); lit++){                             //iterate items 

	cout << i <<". "<<(*lit).getName() <<" x "<<(*lit).getCount()<< "     Price: " <<(*lit).getValue() << " Euro" << endl;
	i++;

	}
}


bool Player::canAffordItem(string name,int amount,int money)                  
{
	list<Item>::iterator lit;

	for(lit =_items.begin();lit != _items.end();lit++){
		if((*lit).getName() == name ){
			if((*lit).getValue() *amount  <=money){
				return true;	
			} else {
				cout <<"***There is not enough money in the shop!" << endl;
				return false;
			}
		}
	}
	return false;
}



bool Player::removeItem(string name,int amount,Item &newItem){

	list<Item>::iterator lit;

	for(lit =_items.begin();lit != _items.end();lit++){
		if ((*lit).getName() == name){
			newItem =(*lit);
			newItem.setCount(amount);
			(*lit).remove(amount);
			if((*lit).getCount() < 0){
			newItem.setCount(amount);                                 
			(*lit).add(amount);
			cout << "***You don't have enough items in inventory!***" << endl;
			//_items.erase(lit);
			return false;
			}
			return true;
		}
	}
	return false;
}
void Player::addItem(Item newItem){                             // init
	list<Item>::iterator lit;
	for(lit =_items.begin();lit != _items.end();lit++){
		if ((*lit).getName() == newItem.getName()){
			(*lit).add(1);
			return;
		}
	}

  _items.push_back(newItem);
  //_items.back().setCount(1);

}
void Player::addItem_(int amount,Item newItem){                 // purchase
	list<Item>::iterator lit;
	for(lit =_items.begin();lit != _items.end();lit++){
		if ((*lit).getName() == newItem.getName()){
			(*lit).add(amount);
			return;
		}
	}

  _items.push_back(newItem);
  //_items.back().setCount(1);

}
	


