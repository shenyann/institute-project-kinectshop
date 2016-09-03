#include "Shop.h"
#include <iostream>


Shop::Shop(string name,int money)
{

	_name=name;
	_money=money;
}

void Shop::printShop(){                                                  //print shop's information

cout << "*** Welcome to "<< _name <<" ***\n\n";
cout << "Money: "<< _money <<" Euro \n\n";
list<Item>::iterator lit;

int i = 0;

for (lit =_items.begin();lit != _items.end();lit++){                    //iterate items and cout items information

	cout << i <<". "<<(*lit).getName() <<" x "<<(*lit).getCount()<< "     Price: " << (*lit).getValue()<< " Euro"<< endl;
	i++;

	}

    cout << endl;
}


bool Shop::canAffordItem(string name,int amount,int money){              //if player can afford the items
	list<Item>::iterator lit;
	for(lit =_items.begin();lit != _items.end();lit++){
		if((*lit).getName() == name ){
		if((*lit).getValue() * amount <= money){
				return true;	 
			}
			else{
				
			    (*lit).add(amount);
				cout<<"***You don't have enough money!***\n\n";
				return false;	
			}
		}
			
	}
	return false;
}


 bool Shop::purchaseItem(string name,int amount,Item &newItem){          //if there are enough items in shop
	list<Item>::iterator lit;
	for(lit =_items.begin();lit != _items.end();lit++){
		if((*lit).getName() == name ){
			newItem =(*lit);
			newItem.setCount(amount);
			(*lit).remove(amount);
		if ((*lit).getCount() < 0 ){
				//_items.erase(lit);
				newItem.setCount(amount);
			   (*lit).add(amount);
				cout <<"***There are not enough Items in shop!***\n "<<endl;
				return false;	
			}
		return true;
		}
		
	}
	return false;
}

void Shop::addItem(Item newItem){                                       //init 

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
void Shop::addItem_(int amount,Item newItem){                          //add items in shop

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




