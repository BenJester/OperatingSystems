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
unsigned int l2 = 4;
unsigned int l3 = 9;
unsigned int cv = 3;
int count = 0;


void t2(void* args) {
	cout << "thread 2 starts" << endl;
	thread_lock(l);
	cout << "thread 2 locks l" << endl;
	thread_lock(l2);
	cout << "thread 2 locks l2" << endl;
	thread_yield();
	cout << "thread 2 came back from yield" << endl;
	thread_unlock(l);
	cout << "2 unlock l" << endl;
	thread_yield();
	cout << "2 came back from yield 2" << endl;
	thread_lock(l);
	cout << "2 lock l again" << endl;
	thread_unlock(l2);
	cout << "2 unlock l2 again" << endl;
	thread_unlock(l);
	cout << "2 unlock l again" << endl;
}

void t3(void* args) {
	cout << "thread 3 starts" <<  endl;
	thread_lock(l);
	cout << "thread 3 locks" << endl;
	thread_yield();
	cout << "3 yielded" << endl;
	thread_lock(l3);
	cout << "thread 3 l3 lock" << endl;
	thread_unlock(l);
	cout << "thread 3 unlocks" << endl;
	thread_yield();
	cout << "3 came back from yield" << endl;
	thread_lock(l2);
	cout << "thread 3 locks l2" << endl;
	thread_unlock(l2);
	cout << "thread 3 locks l2" << endl;
	thread_unlock(l3);
	cout << "thread 3 unlock l3" << endl;
}

void t4(void* args){
	cout << "thread 4 starts" << endl;
	thread_lock(l);
	cout << "thread 4 locks" << endl;
	thread_yield();
	cout << "man" << endl;
	thread_unlock(l);
	cout << "thread 4 unlocks" << endl;
}

void t5(void* args){
	cout << "thread 5 starts" << endl;
	thread_lock(l2);
	cout << "thread 5 locks" << endl;
	thread_lock(l3);
	cout << "thread 5 locks l" << endl;
	thread_yield();
	cout << "5 came back from yield1" << endl;
	thread_yield();
	cout << "5 came back from yield2" << endl;
	thread_unlock(l3);
	cout << "5 unlock l 2" << endl;
	thread_yield();
	cout << "5 came back from yield3" << endl;
	thread_unlock(l2);
	cout << "5 unlock l2 2" << endl;
}

void t6(void* args){
	cout << "thread 6 starts" << endl;
	thread_lock(l2);
	cout << "thread 6 locks1" << endl;
	thread_lock(l3);
	cout << "thread 6 locks2" << endl;
	thread_unlock(l2);
	cout << "thread 6 locks3" << endl;
	thread_lock(l);
	cout << "thread 6 locks4" << endl;
	thread_yield();
	cout << "thread 6 locks5" << endl;
	thread_unlock(l3);
	cout << "thread 6 locks6" << endl;
	thread_yield();
	cout << "thread 6 locks7" << endl;
	thread_unlock(l);
}


void t1(void* args) {

	cout << "thread 1 starts" << endl;

	thread_create(&t2, args);
	cout << "created t2" << endl;
	thread_create(&t3, args);
	cout << "created t3" << endl;
	thread_create(&t4, args);
	cout << "created t4" << endl;
	thread_create(&t5, args);
	cout << "created t5" << endl;
	thread_create(&t6, args);
	cout << "created t6, exiting t1" << endl;
}


int main(int argc, char* argv[]) {
	//start_preemptions(true, true, 13); //303 giving errors
	cout << "main starts" << endl;
	thread_libinit(&t1, static_cast<void*>(&argv));


}
