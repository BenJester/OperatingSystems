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
	int id = curr_cashier;
	curr_cashier += 1;
	thread_unlock(sw_lock);

	char buffer[255];
	strcpy(buffer, files[id].c_str());
/*
	stringstream strs;
	strs << id;
	string temp_str = strs.str();
	char* char_type = (char*) temp_str.c_str();
*/

	ifstream f(buffer);
    vector<int> v;
    int num;
    
    while(f >> num) {
	   v.push_back(num);
    }

	int arr[v.size()];
	for(int i=0; i<v.size(); i++) {
		arr[i] = v.at(i);
	}

    order last_order;
    bool first_order = true;

    for(int index=0; index<v.size(); index++) {
        order new_order;
        new_order.cashier = id;
        new_order.sandwich = arr[index];
        new_order.ready = false;

        thread_lock(sw_lock);

	while((first_order && boardFull()) || (!first_order && (!status[id] || boardFull()))) {
	    //cout << "sleeping" << endl;
            thread_wait(sw_lock, cashier_cv);
            
        }
//cout<< "cashier " << buffer << "woken up" << endl;



        first_order = false;
        board.push_back(new_order);
	status[id] = false;
        cout << "POSTED: cashier " << id << " sandwich " << arr[index] << endl;
        last_order = board[board.size()-1];
	//cout << "signal chef" << endl;
        thread_signal(sw_lock, chef_cv);
        thread_unlock(sw_lock);
    }
    thread_lock(sw_lock);
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
	
	thread_lock(sw_lock);
        //cout << "num c " << num_cashiers << endl;

        
        while(!boardFull()) {
	//cout << "sleeping " << num_cashiers << endl;
//cout << "sleep" << endl;
            thread_wait(sw_lock, chef_cv);
//cout << "wake" << endl;
	
        }
	if (num_cashiers == 0) break;
//cout << "break" << endl;
	//cout << "chef woken up" << endl;
     //printBoard();
        order closest = closestSandwich();
        closest.ready = true;
	status[closest.cashier] = true;
  
	last_sandwich = closest.sandwich;
        cout << "READY: cashier " << closest.cashier << " sandwich " << closest.sandwich << endl;
        thread_broadcast(sw_lock, cashier_cv);
        thread_unlock(sw_lock);
	
    }
}

void initCashierAndChef(void* args) {
	//start_preemptions(true, false, 987);
    for(int n=0; n<num_cashiers; n++) {
    	char buffer[255];
    	strcpy(buffer, "sw.in");
    	stringstream strs;
    	strs << n;
    	string temp_str = strs.str();
    	char* char_type = (char*) temp_str.c_str();
    	strcat(buffer, char_type);
        initCashier(n);
    }
    thread_create(&chef, args);
}

int main(int argc, char* argv[]) {
    max_orders = atoi(argv[1]);
    num_cashiers = argc-2;
    for(int n=2; n<argc; n++) {
	
	string f(argv[n]);
//cout << f << endl;
	files.push_back(f);
//cout << "hi" << endl;
	}

    thread_libinit(&initCashierAndChef, static_cast<void*>(&argv));
	
}
