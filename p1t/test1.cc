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

unsigned int l = 2;
unsigned int cv = 3;
int count = 0;


void t2(void* args) {
	cout << "thread 2 starts" << endl;
	thread_lock(l);
	cout << "thread 2 gets the lock" << endl;
	
	cout << "thread 2 about to wait" << endl;
	int n = thread_wait(l, cv);
        cout << n << endl;
	cout << "thread 2 wakes up" << endl;
	thread_unlock(l);
	cout <<  "thread 2 unlocks and exits" << endl;

}

void t3(void* args) {
	cout << "thread 3 starts, tries to get lock" <<  endl;
	thread_lock(l);
	cout << "thread 3 gets the lock, signaling and yielding" << endl;
	thread_signal(l, cv);
	cout << "signaled" << endl;
	thread_unlock(l);
	//deadlocked
	
}

void t4(void* args){
	cout << "thread 4 starts, tries to get lock" << endl;
	thread_lock(l);
	cout << "thread 4 gets lock" << endl;
	thread_unlock(l);
	cout << "thread 4 unlocks" << endl;
}

void t1(void* args) {
	cout << "thread 1 starts" << endl;
	thread_create(&t2, args);
	cout << "created t2" << endl;
	thread_create(&t3, args);
	cout << "created t3" << endl;
	thread_create(&t4, args);
	cout << "created t4, exiting" << endl;


}


int main(int argc, char* argv[]) {
	//start_preemptions(true, true, 1198); //5 98 1024 giving errors 309 wait correct
	cout << "main starts" << endl;
	thread_libinit(&t1, static_cast<void*>(&argv));


}
