#include "thread.h"
#include <iostream>
using namespace std;
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <cstring>
#include <sstream>

// classes
class order {
    public:
        bool ready;
        int cashier;
        int sandwich;
};

// global variables
unsigned int sw_lock = 0;  
unsigned int cashier_cv = 100;            
unsigned int chef_cv = 2;
int max_orders;
int num_cashiers;
int last_sandwich = -1;
int curr_cashier = 0;
vector<order> board;
vector<bool> status;
vector<string> files;


bool boardFull() {
	//cout << "numc " << num_cashiers << " ";
	//cout << board.size() << " " << max_orders << endl;
    if(num_cashiers < max_orders) {

        return board.size() >= num_cashiers;
    }
    return board.size() >= max_orders;
}

order closestSandwich() {
    int minDistance = 1000;
    int index;
    order closest;
    for(int n=0; n<board.size(); n++) {

        order curr = board.at(n);
	//cout << abs(curr.sandwich - last_sandwich) << " curr: " << curr.sandwich << " min: " << minDistance << endl;
        if(abs(curr.sandwich - last_sandwich) <= minDistance) {
            minDistance = abs(curr.sandwich - last_sandwich);
            index = n;
            closest = curr;

        }
    }
    
    board.erase(board.begin() + index);
    return closest;
}

void cashier(void *args) {
	thread_lock(sw_lock);
cout << "cashier gets lock top of code" << endl;
	int id = curr_cashier;
	curr_cashier += 1;
	thread_unlock(sw_lock);
/*
	char buffer[255];
	strcpy(buffer, files[id].c_str());

	stringstream strs;
	strs << id;
	string temp_str = strs.str();
	char* char_type = (char*) temp_str.c_str();


	ifstream f(buffer);
    vector<int> v;
    int num;
    
    while(f >> num) {
	   v.push_back(num);
    }
*/
	cout << "HI" << endl;
	int size = 5;
	cout << "HI" << endl;
	int arr[size] = {1, 385, 28, 983, 384};
	//int arr[v.size()];
	cout << "HI" << endl;
	
    order last_order;
    bool first_order = true;

    for(int index=0; index<size; index++) {
        order new_order;
        new_order.cashier = id;
        new_order.sandwich = arr[index];
        new_order.ready = false;

        thread_lock(sw_lock);
	cout << "cashier " << id << " gets the lock" << endl;
	while((first_order && boardFull()) || (!first_order && (!status[id] || boardFull()))) {
	    cout << "cashier sleeping " << id << endl;
            thread_wait(sw_lock, cashier_cv);
            cout << "cashier " << id << " wakes up afer gets the lock" << endl;
        }
//cout<< "cashier " << buffer << "woken up" << endl;



        first_order = false;
        board.push_back(new_order);
	status[id] = false;
        cout << "POSTED: cashier " << id << " sandwich " << arr[index] << endl;
        last_order = board[board.size()-1];
	//cout << "signal chef" << endl;
        thread_signal(sw_lock, chef_cv);
	cout << "cashier " << id << "unlocks" << endl;
        thread_unlock(sw_lock);
	
    }
    thread_lock(sw_lock);
cout << "cash gets lock again" << endl;
    while (!status[id]) {
 	thread_wait(sw_lock, cashier_cv);
    }

    num_cashiers -= 1;
    //thread_broadcast(sw_lock, cashier_cv);
    if (num_cashiers > 0) thread_signal(sw_lock, chef_cv);
    thread_unlock(sw_lock);
	//cout << "done!" << endl;
}

void initCashier(int id) {
    int* ptr = &id;
    thread_create(&cashier, (void*)ptr);
    status.push_back(false);
}

void printBoard() {
	for(int i=0; i<board.size(); i++) {
	cout << board[i].sandwich << " ";
	}
	cout << endl;
}

void chef(void *args) {
    while(num_cashiers != 0) {
	cout << "chef enters, tries to lock" << endl;
	thread_lock(sw_lock);
        cout << "chef gets lock" << endl;

        
        while(!boardFull()) {
	//cout << "sleeping " << num_cashiers << endl;
	cout << "chef sleep" << endl;
            thread_wait(sw_lock, chef_cv);
	cout << "chef wake" << endl;
	
        }
	if (num_cashiers == 0) break;
	//cout << "break" << endl;
	cout << "chef work" << endl;
        //printBoard();
        order closest = closestSandwich();
cout << "a" << endl;
        closest.ready = true;
cout << "b" << endl;
	status[closest.cashier] = true;
cout << "c" << endl;
  
	last_sandwich = closest.sandwich;
        cout << "READY: cashier " << closest.cashier << " sandwich " << closest.sandwich << endl;
        thread_broadcast(sw_lock, cashier_cv);
	cout << "chef broadcasted" << endl;
        thread_unlock(sw_lock);
	cout << "chef loops" << endl;
    }
}

void initCashierAndChef(void* args) {
	//start_preemptions(true, false, 987);
    for(int n=0; n<num_cashiers; n++) {

        initCashier(n);
    }
    thread_create(&chef, args);
}

int main(int argc, char* argv[]) {
    //start_preemptions(true, true, 2312);
    max_orders = 3;
    num_cashiers = 2;
	cout << "HI" << endl;
    thread_libinit(&initCashierAndChef, static_cast<void*>(&argv));
	
}
