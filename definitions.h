#ifndef DEFS
#define DEFS

typedef int bool;
enum { false, true };


//Standard set of includes needed in each program
#include <errno.h>
#include <time.h>
#include <semaphore.h>      
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MAX_CREATED_PROCESSES 18
const int INITIAL_QUANTUM = 20; //in millisecond
const char *path = "./keyFile";

typedef struct{
	int seconds;
	int nano;
}LogicalClock;

typedef struct{
	int processID;
	int parentID;
	int UserID;
	
	int priority; //priority if needed
	bool schedule; //If it needs more time

	int inCPUtime; //in nanoseconds
	int inSystemTime; //in nanoseconds
	int lastBurstTime; //in nanoseconds
}ProcessControlBlock;

#endif
