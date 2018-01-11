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
	cout << "thread 2 starts" << endl;
	n = thread_lock(lock);
	cout << n << " thread 2 obtains lock" << endl;
	n = thread_unlock(lock);
	cout << n << " thread 2 unlocks" << endl;
}

void main_thread(void* args) {
	cout << "thread 1 starts" << endl;
	n = thread_create(&thread2, args);
	cout << n << " created thread2" << endl;
}

int main(int argc, char* argv[]) {
	//start_preemptions(true, true, 1132);
	cout << "main starts" << endl;
	n = thread_create(&thread2, argv);
	cout << n << " attempted to create thread2 before init" << endl;
	n = thread_libinit(&main_thread, static_cast<void*>(&argv));
	cout << n << " attempted to init" << endl;
	n = thread_libinit(&main_thread, static_cast<void*>(&argv));
	cout << n << " attempted to init twice" << endl;
}
