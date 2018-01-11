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

unsigned int lock = 1;
unsigned int cv = 2;
int count = 0;
int n;

void thread3(void* args) {
	cout << "thread 3 starts" << endl;
	n = thread_lock(lock);
	cout << n << " thread 3 obtains lock" << endl;
	n = thread_unlock(lock);
	cout << n << " thread 3 unlocks" << endl;
}

void thread2(void* args) {
	cout << "thread 2 starts" << endl;
	for(int i=0; i<50; i++) {
		n = thread_lock(lock);
		cout << n << " thread 2 obtains lock; about to yield" << endl;
		n = thread_yield();
		cout << n << " thread 2 comes back" << endl;
	}
	n = thread_create(&thread3, args);
	cout << n << " thread 3 created";
	n = thread_yield();
	cout << n << " thread 2 comes back" << endl;
	n = thread_unlock(lock);
	cout << n << " thread 2 finished" << endl;
}

void main_thread(void* args) {
	cout << "thread 1 starts" << endl;
	n = thread_create(&thread2, args);
}

int main(int argc, char* argv[]) {
	//start_preemptions(true, true, 1024); //5 98 1024 giving errors 309 wait correct
	cout << "main starts" << endl;
	thread_libinit(&main_thread, static_cast<void*>(&argv));
	cout << "successful!" << endl;
}
