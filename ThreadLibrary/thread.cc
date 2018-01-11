#include "interrupt.h"
#include "thread.h"
#include <ucontext.h>
#include <queue>
#include <set>
#include <iostream>
using namespace std;
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <assert.h>

// classes
class thread {
    public:
        ucontext_t* context;
        set<unsigned int> held_locks;
        bool blocked_lock;
        unsigned int lock_wait;
        bool blocked_cv;
        unsigned int cv_lock_wait;
        unsigned int cv_wait;
};

// global variables
static queue<thread*> ready_queue;
static queue<thread*> blocked_queue;
static queue<thread*> inactive_queue;
static thread* running;
static set<unsigned int> locks_held;
static ucontext_t* middle_man;
static ucontext_t* main_ptr;
static bool init_called = false;
static bool first_thread = true;

void thread_create_helper(thread_startfunc_t func, void *arg) {
    interrupt_enable();
    func(arg);
    interrupt_disable();
    swapcontext(running->context, main_ptr);
}

int thread_create_inside(thread_startfunc_t func, void *arg) {
    if(!init_called) {
        interrupt_enable();
        return -1;
    }
    ucontext_t* create_helper_ptr;
    char* stack;
    try {
        create_helper_ptr = new ucontext_t;
        getcontext(create_helper_ptr);
        stack = new char[STACK_SIZE];
    } catch(bad_alloc e) {
        interrupt_enable();
        return -1;
    }
    create_helper_ptr->uc_stack.ss_sp = stack;
    create_helper_ptr->uc_stack.ss_size = STACK_SIZE;
    create_helper_ptr->uc_stack.ss_flags = 0;
    create_helper_ptr->uc_link = NULL;
    makecontext(create_helper_ptr, (void(*)())&thread_create_helper, 2, func, arg);

    thread* t = new thread;
    t->context = create_helper_ptr;
    t->blocked_lock = false;
    t->lock_wait = 0;
    t->blocked_cv = false;
    t->cv_lock_wait = 0;
    t->cv_wait = 0;
    ready_queue.push(t);
    return 0;
}

extern int thread_libinit(thread_startfunc_t func, void *arg) {
    interrupt_disable();
    if(init_called) {
        // throw error
        interrupt_enable();
        return -1;
    } else {
        // create ucontext and stack for the first thread
        // create ucontext for main and swapcontext with first thread
        init_called = true;
        try {
            main_ptr = new ucontext_t;
        } catch(bad_alloc e) {
            interrupt_enable();
            return -1;
        }
        getcontext(main_ptr);
        try {


            char* main_stack = new char[STACK_SIZE];
            main_ptr->uc_stack.ss_sp = main_stack;
            main_ptr->uc_stack.ss_size = STACK_SIZE;
            main_ptr->uc_stack.ss_flags = 0;
            main_ptr->uc_link = NULL;
        } catch(bad_alloc e) {
            interrupt_enable();
            return -1;
        }
        thread_create_inside(func, arg);

        while(ready_queue.size() != 0) {
            if(!first_thread) {
            thread* prev = running;
            delete (char*) (prev->context->uc_stack.ss_sp);
            delete prev->context;
	    delete prev;
            }
            first_thread = false;
            running = ready_queue.front();
            ready_queue.pop();
            swapcontext(main_ptr, running->context);
        }
    	delete (char*) (main_ptr->uc_stack.ss_sp);
	delete main_ptr;
        cout << "Thread library exiting.\n";
        interrupt_enable();
        exit(0);
    }
}

extern int thread_create(thread_startfunc_t func, void *arg) {
    interrupt_disable();
    if(!init_called) {
        interrupt_enable();
        return -1;
    }
    int return_val = thread_create_inside(func, arg);
    interrupt_enable();
    return return_val;
}

extern int thread_yield() {
    interrupt_disable();
    if(!init_called) {
        interrupt_enable();
        return -1;
    }
    if(ready_queue.size() != 0) {
        // pop off first thing from ready_queue
        // push curr_thread to ready_queue
        thread* next = ready_queue.front();
        ready_queue.pop();
        thread* prev = running;
        running = next;
        ready_queue.push(prev);
        swapcontext(prev->context, next->context);
    }
    interrupt_enable();
    return 0;
}

int thread_lock_helper(unsigned int lock) {
    while(true) {
        if(!init_called) {
            return -1;
        }
        if(locks_held.count(lock) == 0) {
            locks_held.insert(lock);
            running->held_locks.insert(lock);
            break;
        } else {
            // move thread to blocked
            // change param
            // pop from ready queue
            if(running->held_locks.count(lock) == 1) {
                return -1;
            }
            if(ready_queue.size() == 0) {
                cout << "Thread library exiting.\n";
                interrupt_enable();
                exit(0);
            }
            running->blocked_lock = true;
            running->lock_wait = lock;
            thread* next = ready_queue.front();
            ready_queue.pop();
            thread* prev = running;
            blocked_queue.push(prev);
            running = next;
            swapcontext(prev->context, next->context);
        }
    }
    return 0;
}

extern int thread_lock(unsigned int lock) {
    interrupt_disable();
    int return_val = thread_lock_helper(lock);
    interrupt_enable();
    return return_val;
}

int thread_unlock_helper(unsigned int lock) {
    if(!init_called) {
        // throw error
        return -1;
    }
    if(running->held_locks.count(lock) == 0) {
        // throw error
        return -1;
    } else {
        // remove from set
        // loop blocked queue
        // add to ready queue if not blocked anymore
        locks_held.erase(lock);
        running->held_locks.erase(lock);
        queue<thread*> buffer_queue;
        while(blocked_queue.size() > 0) {
            thread* t = blocked_queue.front();
            if(t->blocked_lock && t->lock_wait == lock) {
                t->blocked_lock = false;
                t->lock_wait = 0;
                ready_queue.push(t);
            } else {
                buffer_queue.push(t);
            }
            blocked_queue.pop();
        }
        while(buffer_queue.size() > 0) {
            blocked_queue.push(buffer_queue.front());
            buffer_queue.pop();
        }
    }
    return 0;
}

extern int thread_unlock(unsigned int lock) {
    interrupt_disable();
    int return_val = thread_unlock_helper(lock);
    interrupt_enable();
    return return_val;
}

extern int thread_wait(unsigned int lock, unsigned int cond) {
    interrupt_disable();
    if(!init_called) {
        interrupt_enable();
        return -1;
    }
    if(locks_held.count(lock) == 0) {
        // throw error
        interrupt_enable();
        return -1;
    } else {
        // unlock
        // move to blocked queue for CV
        // pop ready queue
        // lock
        thread_unlock_helper(lock);
        if(ready_queue.size() == 0) {
            cout << "Thread library exiting.\n";
            interrupt_enable();
            exit(0);
        }
        thread* next = ready_queue.front();
        ready_queue.pop();
        thread* prev = running;
        prev->blocked_cv = true;
        prev->cv_lock_wait = lock;
        prev->cv_wait = cond;
        blocked_queue.push(prev);
        running = next;
        swapcontext(prev->context, next->context);
        thread_lock_helper(lock);
    }
    interrupt_enable();
    return 0;
}

extern int thread_signal(unsigned int lock, unsigned int cond) {
    interrupt_disable();
    if(!init_called) {
        interrupt_enable();
        return -1;
    }
    // loop over blocked queue
    // find first thread blocked by CV and add to ready queue
    queue<thread*> buffer_queue;
    bool found = false;
    while(blocked_queue.size() > 0) {
        thread* t = blocked_queue.front();
        blocked_queue.pop();
        if(!found && t->blocked_cv && t->cv_lock_wait == lock && t->cv_wait == cond) {
            t->blocked_cv = false;
            t->cv_lock_wait = 0;
            t->cv_wait = 0;
            ready_queue.push(t);
            found = true;
        } else {
            buffer_queue.push(t);
        }
    }
    while(buffer_queue.size() > 0) {
        blocked_queue.push(buffer_queue.front());
        buffer_queue.pop();
    }
    interrupt_enable();
    return 0;
}

extern int thread_broadcast(unsigned int lock, unsigned int cond) {
    interrupt_disable();
    if(!init_called) {
        interrupt_enable();
        return -1;
    }
    // loop over blocked queue
    // find all threads blocked by CV and add to ready queue
    queue<thread*> buffer_queue;
    while(blocked_queue.size() > 0) {
        thread* t = blocked_queue.front();
        blocked_queue.pop();
        if(t->blocked_cv && t->cv_lock_wait == lock && t->cv_wait == cond) {
            t->blocked_cv = false;
            t->cv_lock_wait = 0;
            t->cv_wait = 0;
            ready_queue.push(t);
        } else {
            buffer_queue.push(t);
        }
    }
    while(buffer_queue.size() > 0) {
        blocked_queue.push(buffer_queue.front());
        buffer_queue.pop();
    }
    interrupt_enable();
    return 0;
}
