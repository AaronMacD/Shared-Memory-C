#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signal){
    //printf("Reader unpaused\n");
    return;
}

int main (int argc, char *argv[]) {
    int shmId;
    int processNum = strtol(argv[1], NULL, 10) - 1;

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigaction(SIGCONT, &act, NULL);

    struct myData {
      char message[50];
      bool messageReady;
      bool processDone[2];
    };

    //Unique key for shmget using ftok
    key_t my_key = ftok("./key", 12);

    //Getting Shared Data ID using key
    if ((shmId = shmget(my_key, sizeof(struct myData), 0 )) < 0) {
        perror ("i can't get no..\n");
        exit (1);
    }

    //Data for sharing.
    struct myData *my_shared;

    //Attaching to shared data. Becomes pointer to the memory location of that data
    my_shared = (struct myData *) shmat(shmId, NULL, 0);

    printf("Process %d has been created. Waiting on reader!\n", processNum);

    while(1){

	//Stoplight! Wait loop for not done, message ready, and other process not active
        while(my_shared->messageReady == false || my_shared->processDone[processNum] == true){
	    //To suspend CPU so doesn't bog everything down
	    pause();
	}

	//Critical Code
	//printf("Process %d is now in critical code", processNum);
        printf("Process %d reads: %s\n", processNum, my_shared->message);
        my_shared->processDone[processNum] = true;

	//Exit condition
        if(strcmp(my_shared->message, "quit") == 0){
            break;
        }
	
	//Trigger new read if both processes done.        
	if(my_shared->processDone[0] == true && my_shared->processDone[1] == true){
	    my_shared->messageReady = false;
	    kill(-1, SIGCONT);
	}
    }

    if (shmdt (my_shared) < 0) {
	perror ("Process can't detach: \n");
	exit(1);
    }
    return 0;
}
