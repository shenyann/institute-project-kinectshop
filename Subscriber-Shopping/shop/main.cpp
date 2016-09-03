#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include "zmq.hpp"
#include "Item.h"
#include "Shop.h"
#include "Player.h"

using namespace std; 
using std::string;
int  product_id;

//Prototype
void initShops(list<Shop> &shops);
void initPlayer(Player &player);
void enterShop(Player &palyer,Shop &shop);

int main()
{	
	// Create ZMQ context to receive the product ID from the classification group
	zmq::context_t context(3);
	zmq::socket_t subscriber(context,ZMQ_SUB);
	subscriber.connect("tcp://localhost:5556");
	subscriber.setsockopt(ZMQ_SUBSCRIBE,"",0);

	int flags=0;
	zmq::message_t msg;
	subscriber.recv(&msg,flags);

	cout << "received product_id: \n"<< *(static_cast<int*>(msg.data()))<<endl;
	product_id=*(static_cast<int*>(msg.data()));

	// TODO: implement a shop application here:
	list<Shop>shops;
	Player player;
	list<Shop>::iterator lit;
	string shopName;

	// initialize
	initPlayer(player);
	initShops(shops);

	// Init and start program loop:
	bool isDone =false;  //bool variable to set exit flag for this loop
	while(!isDone)
	{

		// This is a default application a poor and probably underpaid student came up with during the last hours before project start
		// Don't expect it to be great, instead use your brains to imagine your own simple shop application
		// So basically your TODO:

		cout << "***Hello , welcome to kinenctshop!***\n";
		cout << "\n";
		cout << "***Shops:***\n";
		int i = 1; 
		for(lit = shops.begin();lit != shops.end();lit++)
		{
			cout << i << ". " <<(*lit).getName()<< endl;
			i++;
		}


		cout<<"\nWhich shop would you like to enter? Q to quit: ";
		getline(cin,shopName);

		if(shopName == "Q" || shopName == "q")
		{
			isDone=true;

		}else{
			cout << endl;
			bool validShop =false;
			for(lit = shops.begin();lit != shops.end();lit++)
			{
				if((*lit).getName() == shopName)
				{

					enterShop(player,(*lit));	           //enter the shop
					validShop=true;

				}

			}

			if(validShop == false)
			{
				cout <<"\n" << endl;
				cout << "Invalid Shop!\n";
				system("PAUSE");
			}
		}

	}
	system("PAUSE");
	return 0;

}

void initShops(list<Shop> &shops){
	shops.push_back(Shop("Amazon",1000));
	shops.back().addItem(Item("Cup",5,5));
	shops.back().addItem(Item("Sodawasser",2,5)); //name value count
	shops.back().addItem(Item("Schocolade",3,5));

	shops.push_back(Shop("Ebay",1500));
	shops.back().addItem(Item("Cup",5,5));
	shops.back().addItem(Item("Sodawasser",2,5));
	shops.back().addItem(Item("Schocolade",3,5));
}

void initPlayer(Player &player){
	cout <<"***Enter thy name sir: ";
	string name ;
	getline(cin,name);
	player.init(name,20);

	player.addItem(Item("Book",15,1));

	cout << endl;
}

void enterShop(Player &player,Shop &shop){

	bool isDone = false;
	char input;
	string itemName;
	Item newItem("NO_ITEM",0,1);

	while(isDone == false){
		shop.printShop();                                                        //shop's information
		player.printInventory();                                                 //player's inventory
		cout <<"***Would you like to buy or sell?Q to quit. (B/S): \n";
		cin >> input;
		cin.ignore(64,'\n');
		cin.clear();

		if(input == 'Q'|| input=='q') return;

		if(input == 'B' || input == 'b'){                                          //buy items
			//buy
			switch(product_id-1){
				case 0:
					itemName = "Cup";

					cout << "***The product what you want: \n" << itemName << endl;
					break;
				case 1:
					itemName = "Sodawasser";

					cout << "***The product what you want: \n" << itemName << endl;
					break;
				case 2:
					itemName = "Schocolade";

					cout << "***The product what you want: \n" << itemName << endl;
					break;
			}


			int amount;
			cout << "***How many items would you like to add to the list?***" <<endl;
			cin >> amount;
			while(amount > 5){
				cout << "***The only valid choices amount are 1-5.Please re-enter.***\n";
				cin >> amount;
			}

			if(shop.purchaseItem(itemName,amount,newItem) == true){                    //if shop has enough items 
				if(shop.canAffordItem(itemName,amount,player.getMoney())==true){           //if player can afford the item
					player.addMoney(amount,-newItem.getValue());                           //substract money from player
					player.addItem_(amount,newItem);                                       //add item in player's inventory
					shop.addMoney(amount,newItem.getValue());}							   //add money to shop

			} else{ 
				system("PAUSE \n");
			}
		}


		if(input == 'S' || input == 'S'){                                                  //sell items
			//sell 
			cout << "***Enter the item you wish to sell :";
			getline(cin,itemName);
			cout << "***How many items would you like to remove from the list?***" <<endl;
			int amount;
			cin >> amount;
			if(player.removeItem(itemName,amount,newItem)== true){                  //if player has enough items in inventory
				if(player.canAffordItem(itemName,amount,shop.getMoney()) == true){      //if shop can afford the item
					shop.addMoney(amount,-newItem.getValue());                          //substract money from shop
					shop.addItem_(amount,newItem);                                      //add item in shop
					player.addMoney(amount,newItem.getValue());}						//add money to player's inventory

			} else {  
				cout <<"***invalid item!***" << endl;
				system("PAUSE");
			}	

		} else {                                                                          //invalid press button
			cout << "***please enter the correct button!***" << endl;
		}
	}
}
