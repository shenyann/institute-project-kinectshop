#pragma once
#include "string"
#include "Item.h"
#include "list"
using std::string;
using namespace std;


using std::string;
class Shop
{
public:
	Shop(string name,int money);

	void printShop();
	bool canAffordItem(string name,int amount,int money);
	bool purchaseItem(string name,int amount,Item &newItem);
	void addItem(Item newItem);
	void addItem_(int amount,Item newItem);

	void addMoney(int amount,int money){_money += amount * money; }
	//Getters
	string getName (){return _name; }
	int getMoney(){return _money; }
	int getAmount(){return _amount;}



private:
	string _name;
	list<Item> _items;
	int _money;
	int _amount;

	
};

