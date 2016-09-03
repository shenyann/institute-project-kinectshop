#pragma once
#include "string"
#include <list>
#include "Item.h"

#include "list"
using namespace std;
using std::string;

class Player
{
public:
	Player();

	void init(string name,int money);                                //initialize the player's information

	void printInventory();                                          //cout inventory information

	bool canAffordItem(string name,int amount,int money);           

	
	bool removeItem(string name,int amount,Item &newItem);            
	void addItem(Item newItem);
	void addItem_(int amount,Item newItem);              

	void addMoney(int amount,int money){_money += amount * money; }
	         
	//Getters
	int getMoney(){return _money;}
	void subtractMoney(int amount){_money -= amount;}
	string getName(){return _name;}
	int getAmount(){return _amount;}

private:
	string _name;
	int _money;
	int _amount;
	list<Item> _items;
	
};

