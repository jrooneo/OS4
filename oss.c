#include "definitions.h"

int processCount;
int child[MAX_CREATED_PROCESSES+1];
int shmidArr[MAX_CREATED_PROCESSES+1];
char *shmArr[MAX_CREATED_PROCESSES+1];
int semidArr[MAX_CREATED_PROCESSES+1];
char *semArr[MAX_CREATED_PROCESSES+1];
void timeIncrement(LogicalClock *);
 
static void signal_handler(int signum){
	int i;
	for(i=0;i<=processCount;i++){
		shmdt(shmArr[i]);
		shmctl(shmidArr[i], IPC_RMID, NULL); //delete
	}
	//print message
	exit(1);
}

int main(int argc, char **argv)
{
	processCount = 10;
	int i, status, result, shmid;
	char projHolder[2];
	char *shm;
	
	//array of slot status. 0 is empty & not created, 1 is full & needing quantum, -1 is completed.
	int emptySlots[processCount+1]; 
	for(i = 0; i<=processCount; i++) emptySlots[i] = 0;
	
	//setup time struct for nanosleep
	struct timespec ts;
	ts.tv_sec = 0;
	int clockShmID;
	
	int PPID = getpid();
	if(argc == 2){
		processCount = atoi(argv[1]);
		if(processCount > MAX_CREATED_PROCESSES){
			printf("This program restricts process to %i. Process count set to default (10)\n", MAX_CREATED_PROCESSES);
			processCount = 10;
		}
		if(processCount < 2){
			printf("This program must have at least 2 process. Process count set to default (10)\n");
			processCount = 10;
		}
	}
	if(argc > 2){
		printf("This program is designed to take a single argument for process count.\nCount set to default of 10\n");
	}
	//Get key information for shared items
	key_t key;
	
	signal(SIGINT, signal_handler); //Don't allow program to close without clean up after this point
	ProcessControlBlock *processControl[processCount+1]; //create the data structures
	
	//semaphore creation
	struct sembuf wait[processCount+1],signal[processCount+1];
	for(i= 0; i<=processCount; i++){
		wait[i].sem_num = i;
		wait[i].sem_op = -1;
		wait[i].sem_flg = SEM_UNDO;

		signal[i].sem_num = i;
		signal[i].sem_op = 1;
		signal[i].sem_flg = SEM_UNDO;
		

	}		
	int semid = semget(key,processCount+1,IPC_CREAT | 0666);
	int semval = 1;
	semctl(semid,0,SETVAL,semval);
	
	//initialize clock
	LogicalClock *clock;
	shmidArr[0] = shmget(ftok(path, 20), sizeof(LogicalClock), IPC_CREAT | 0666);
	if(shmidArr[0] == -1){
		//error
		exit(-1);
	}
	clock = shmat(shmidArr[0], NULL, 0);
	clock->seconds = 0;
	clock->nano = 0;
	int position = 1;
	do{
		//process creation
		if(createNewProcess(clock->seconds) == 0){
			for(i= 1; i <= processCount; i++){
				if(emptySlots[i] == 0){
					sprintf(projHolder,"%d", i); //copy this project key into a string, will be used as an arg for userProcess
					key = ftok(path, i); //use i to base the project id on so that each shm gets a unique key (can't use 0 or 1 proj num)
					shmid = shmget(key, sizeof(ProcessControlBlock), IPC_CREAT | 0666);
					if(shmid == -1){
						//error
						exit(-1);
					}
					shmidArr[i] = shmid;
					shm = shmat(shmid, NULL, 0);
					shmArr[i] == shm;
					processControl[i] = (ProcessControlBlock *) shm; //set the shm pointer, which is now also in shmArr, to hold the process control block
					//initialize processControl values
					processControl[i]->parentID = PPID;
					processControl[i]->schedule = true;
					processControl[i]->priority = 0;
					processControl[i]->inCPUtime = 0;
					processControl[i]->inSystemTime = 0;
					processControl[i]->lastBurstTime = 0;
					child[i] = fork();
					if(child[i] == -1){
						//error
						exit(-1);
					}
					if(child[i] == 0){
						execl("./userProcess","userProcess",projHolder, (const char *) 0);
					}
					emptySlots[i] = 1;
					processControl[i]->processID = child[i];
					break; //only allow one process to create at a time instead of pumping them all out
				}
			}
		}
		// begin scheduling
		if(position > processCount) position = 1; 
		while(true){
			if(emptySlots[position] == 0 || emptySlots[position] == -1){
				position++;
				continue;
			}
			if(emptySlots[position] == 1){
				if(processControl[position]->schedule){
					emptySlots[position] = -1;
					position++;
					continue;
				}
				semop(semid,&signal[processCount],1);
				semop(semid,&wait[processCount],1);
			}
			position++;
			if(position == processCount) position = 1;
		}
		timeIncrement(clock);
	}while(true);

	for(i=0;i<=processCount;i++){
		shmdt(shmArr[i]);
		shmctl(shmidArr[i], IPC_RMID, NULL); //delete
	}
	printf("\n");
	
	return 0;
}
 
void timeIncrement(LogicalClock *clock){
	clock->nano = clock->nano + rand()%1000; //overhead
	while(clock->nano > 1000000000){
		clock->seconds = clock->seconds + (int)(clock->nano / 1000000000);
		clock->nano = clock->nano % 1000000000;
	}
}
 
int createNewProcess(int seconds){
	static int lastSet = 0;
	static int timeToNextProcess = 10;
	if(lastSet + timeToNextProcess < seconds){
		lastSet = seconds;
		timeToNextProcess = rand()%20;
		return 0;
	}
	return 1;
}
