#include "Item.h"
#include "string"
using std::string;


Item::Item(string name,int value, int count)
{
	_name = name;
	_value = value;
	_count = count;
}
void Item::add(int amount){

	_count+=amount;

}
void Item::remove(int amount){

	if (_count >= 0){
	_count-=amount;
	}
}



