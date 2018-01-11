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

void thread2(void* args) {
	n = thread_unlock(lock);
	cout << n << " thread 3 unlocks" << endl;
	n = thread_wait(lock, cv);
	cout << n << " thread 3 waits" << endl;	
	n = thread_lock(lock);
	cout << n << " thread 3 unlocks" << endl;
	n = thread_unlock(lock);
	cout << n << " thread 3 unlocks" << endl;
	n = thread_wait(lock, cv);
	cout << n << " thread 3 waits" << endl;	
	n = thread_lock(lock);
	cout << n << " thread 3 unlocks" << endl;
	n = thread_wait(lock, cv);
	cout << n << " thread 3 waits" << endl;	
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