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

unsigned int l = 5;
unsigned int l2 = 3;
unsigned int l3 = 9;
unsigned int cv = 3;
unsigned int cv2 = 3;
unsigned int cv3 = 4;
int count = 0;
int n;

void t2(void* args) {
	cout << "thread 2 starts" << endl;
	n = thread_lock(l);
	cout << n << " thread 2 locks l" << endl;
	n = thread_lock(l2);
	cout << n << " thread 2 locks l2" << endl;
	n = thread_yield();
	cout << n << " thread 2 came back from yield" << endl;
	n = thread_unlock(l);
	cout << n << " 2 unlock l" << endl;
	n = thread_wait(l2, cv);
	cout << n << " waited 2" << endl;
	n = thread_yield();
	cout << n << " 2 came back from yield 2" << endl;
	n = thread_lock(l);
	cout << n << " 2 lock l again" << endl;
	n = thread_wait(l, cv2);
	cout << n << "waited on l" << endl;
	n = thread_unlock(l2);
	cout << n << " 2 unlock l2 again" << endl;
	n = thread_unlock(l);
	cout << n << " 2 unlock l again" << endl;
}

void t3(void* args) {
	cout << n << " thread 3 starts" <<  endl;
	n = thread_lock(l);
	cout << n << " thread 3 locks" << endl;
	n = thread_yield();
	cout << n << " 3 yielded" << endl;
	n = thread_lock(l3);
	cout << n << " thread 3 l3 lock" << endl;
	n = thread_unlock(l);
	cout << n << " thread 3 unlocks" << endl;
	n = thread_yield();
	cout << n << " 3 came back from yield" << endl;
	n = thread_lock(l2);
	cout << n << " thread locks l2" << endl;
	n = thread_unlock(l2);
	cout << n << " thread 3 unlocks l2" << endl;
	n = thread_unlock(l3);
	cout << n << " thread 3 unlock l3" << endl;
	thread_broadcast(l, cv2);
}

void t4(void* args){
	cout << "thread 4 starts" << endl;
	n = thread_lock(l);
	thread_broadcast(l2, cv);
	thread_wait(l, cv2);
	cout << n << " thread 4 locks" << endl;
	n = thread_yield();
	cout << n << " man" << endl;
	n = thread_unlock(l);
	cout << n << " thread 4 unlocks" << endl;
}

void t5(void* args){
	cout << "thread 5 starts" << endl;
	n = thread_lock(l2);
	thread_signal(l, cv2);
	cout << n << " thread 5 locks" << endl;
	n = thread_lock(l3);
	cout << n << " thread 5 locks l" << endl;
	n = thread_yield();
	cout << n << " 5 came back from yield1" << endl;
	n = thread_yield();
	cout << n << " 5 came back from yield2" << endl;
	n = thread_unlock(l3);
	cout << n << " 5 unlock l 2" << endl;
	n = thread_yield();
	thread_signal(l2, cv);
	cout << n << " 5 came back from yield3" << endl;
	n = thread_unlock(l2);
	cout << n << " 5 unlock l2 2" << endl;
	thread_broadcast(l, cv3);
}

void t6(void* args){
	cout << "thread 6 starts" << endl;
	n = thread_lock(l2);
	cout << n << " thread 6 locks1" << endl;
	n = thread_lock(l3);
	cout << n << " thread 6 locks2" << endl;
	n = thread_unlock(l2);
	cout << n << " thread 6 locks3" << endl;
	n = thread_lock(l);
	cout << n << " thread 6 locks4" << endl;
	n = thread_yield();
	thread_wait(l, cv3);
	cout << n << " thread 6 locks5" << endl;
	n = thread_unlock(l3);
	cout << n << " thread 6 locks6" << endl;
	n = thread_yield();
	cout << n << " thread 6 locks7" << endl;
	n = thread_unlock(l);
	cout << n << " ";
}


void t1(void* args) {

	cout << "thread 1 starts" << endl;

	n = thread_create(&t2, args);
	cout << n << " created t2" << endl;
	n = thread_create(&t3, args);
	cout << n << " created t3" << endl;
	n = thread_create(&t4, args);
	cout << n << " created t4" << endl;
	n = thread_create(&t5, args);
	cout << n << " created t5" << endl;
	n = thread_create(&t6, args);
	cout << n << " created t6, exiting t1" << endl;
}


int main(int argc, char* argv[]) {
//	start_preemptions(true, true, 13); //303 giving errors
	cout << "main starts" << endl;
	thread_libinit(&t1, static_cast<void*>(&argv));


}
