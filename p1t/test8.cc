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


void t2(void* args1) {
	char* args = (char*) args1;
	cout << "thread 2 starts" << endl;
	cout << args[0] << args[1] << args[2] << endl;
	//int* ip = (int*) args;
	//cout << *ip << endl;
	//char* p = (char*) args;
	//cout << p[0] << endl;
	//cout << p[1] << endl;
}

void t3(void* args) {
	cout << "thread 3 starts" <<  endl;
}

void t4(void* args){
	cout << "thread 4 starts" << endl;
}

void t5(void* args){
	cout << "thread 5 starts" << endl;
}

void t1(void* args1) {
	//int* num_1 = (int*) args;
	//int num2 = *num_1 + 3;
	char* args = (char*) args1;
	cout << args[0] << args[1] << args[2] << endl;
	//cout << num2 << endl;
	cout << "thread 1 starts" << endl;
	thread_create(&t2, args);
	cout << "created t2" << endl;
	thread_create(&t3, args);
	cout << "created t3" << endl;
	thread_create(&t4, args);
	cout << "created t4" << endl;
	thread_create(&t5, args);
	cout << "created t5, exiting t1" << endl;
}


int main(int argc, char* argv[]) {
	//start_preemptions(true, true, 13); //303 giving errors
	cout << "main starts" << endl;
	int num = 823;
	char arr[] = {'l', 'o', 'l', '\0'};
	int* num_p = &num;
	thread_libinit(&t1, static_cast<void*>(arr));


}
