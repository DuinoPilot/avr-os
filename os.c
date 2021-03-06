#include <stdio.h>
#include <string.h>
#include "os.h"
#include "os_internal.h"


extern void _os_platform_init();
extern void _os_platform_loop();
extern void _os_platform_sleep();
extern int _os_platform_schedule_task(os_task_function function, void *arg, uint16_t start_delay_secs, uint8_t clear_interrupts);
extern void _os_platform_switch_tasks();
extern void _os_platform_do_something_else();

extern void _os_platform_spinlock_acquire();
extern void _os_platform_spinlock_release();


extern int main();

task_definition tasks[MAX_TASKS];

volatile int cur_task = -1;
volatile int num_tasks = 0;

uint64_t uptime_secs = 0;
uint64_t uptime_millis = 0;

int start_task(void *addr, void *arg, uint16_t start_delay) {
    if(num_tasks == MAX_TASKS) {
        // search for a task that is marked as done, take it over
        int x;
        for(x = 0; x < num_tasks; x++) {
            if(tasks[x].done == 1) {
                tasks[x].address = addr;
                tasks[x].arg = arg;
                tasks[x].start_delay_secs = start_delay;
                tasks[x].done = 0;
                tasks[x].running = 0;
                tasks[x].saved_sp = NULL;
                tasks[x].delayMillis = 0;
                return 0;
            }
        }
        return -1;
    }
    else {
        tasks[num_tasks].address = addr;
        tasks[num_tasks].arg = arg;
        tasks[num_tasks].start_delay_secs = start_delay;
        num_tasks++;
        return 0;
    }
}

int os_schedule_task(os_task_function function, void *arg, uint16_t start_delay_secs) {
   return _os_platform_schedule_task(function, arg, start_delay_secs, 1);
}

void do_something_else() {
    _os_platform_do_something_else();
}

void os_sleep(uint16_t millis) {
    tasks[cur_task].delayMillis = millis;
    do_something_else();
}

void _os_hangout() {
    while(1) {
        _os_platform_sleep();
    }
}

void _os_task_delay_starter(void *arg) {
    int x;
    while(1) {
        os_sleep(1000);
        uptime_secs++;

        for(x = 0; x < num_tasks; x++) {
            if(tasks[x].running == 0 && tasks[x].done != 1 &&
                tasks[x].start_delay_secs > 0) {

                tasks[x].start_delay_secs -= 1;

            }
        }
    }
}

void os_init() {
    _os_platform_init();
    
    _os_platform_schedule_task(_os_hangout, NULL, 0, 0);
    _os_platform_schedule_task(_os_task_delay_starter, NULL, 0, 0);
}

void os_exit_task() {
    tasks[cur_task].running = 0;
    tasks[cur_task].done = 1;

    do_something_else();
}

uint64_t os_get_uptime() {
    return uptime_secs;
}

uint64_t os_get_uptime_millis() {
    return uptime_millis;
}

void os_loop() {
    _os_platform_loop();
}


void spinlock_init(spinlock_t *lock) {
    *lock = 0;
}

void spinlock_acquire(spinlock_t *lock) {
    _os_platform_spinlock_acquire(lock);
}

void spinlock_release(spinlock_t *lock) {
    _os_platform_spinlock_release(lock); 
}
