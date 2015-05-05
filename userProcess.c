#include "definitions.h"

#include <math.h>

int main(int argc, char **argv)
{
	//setup rand
	time_t t;
	srand((unsigned) time(&t));
	int quantum;
	
	//setup logical clock
	LogicalClock *clock;
	int shmid = shmget(ftok(path, 1), sizeof(LogicalClock), IPC_CREAT | 0666);
	if(shmid == -1){
		//error
		exit(-1);
	}
	clock = shmat(shmid, NULL, 0);
	
	//setup time struct for nanosleep
	struct timespec ts;
	ts.tv_sec = 0;
	
	//Get key information for shared items
	int processNumber;
	if(argc > 2){
		processNumber = atoi(argv[1]);
	} else {
		//error 
		exit(-1);
	}
	key_t key = ftok(path, processNumber);
	int semid = semget(key, 1, IPC_CREAT | 0666);
	if(shmid == -1){
		//error
		exit(-1);
	}

	//set shared memory 	
	shmid = shmget(key, 1, IPC_CREAT | 0666);
	if(semid == -1){
		//error
		exit(-1);
	}
	
	//semaphore creation
	struct sembuf wait,signal;
	wait.sem_num = processNumber;
	wait.sem_op = -1;
	wait.sem_flg = SEM_UNDO;

	signal.sem_num = processNumber;
	signal.sem_op = 1;
	signal.sem_flg = SEM_UNDO;
	
	ProcessControlBlock *processControl;
	processControl = (ProcessControlBlock*) shmat(shmid, NULL, 0);
	
	do{
		semop(semid,&wait,1);
		//Wait here until signalled to work
		//reset quantum
		quantum = INITIAL_QUANTUM;
		//pick rand to decide if using full quantum
		if(rand()%2){														
			quantum = round((double)(quantum * ((float)rand()/RAND_MAX)));
		}
		
		//sleep for processing time
		ts.tv_nsec = quantum * 1000000;
		nanosleep(&ts, NULL);
		
		//update total CPU time, time in system, last burst
		processControl->inCPUtime = processControl->inCPUtime + quantum;
		processControl->lastBurstTime = quantum;
		
		if(processControl->inCPUtime >= 50 && rand()%2){
				processControl->schedule = false;
				semop(semid,&signal,1);
				break;
		}
		semop(semid,&signal,1);
	}while(true);
}
