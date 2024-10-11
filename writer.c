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
    //printf("Writer unpaused\n");
    return;
}

int main () {
    int shmId;
    char buff[50];

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

    //Creating Shared Data
    if ((shmId = shmget(my_key, sizeof(struct myData), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0) {
        perror ("i can't get no..\n");
        exit (1);
    }

    printf("Shared memory created with shmid: %d\n", shmId);

    //Data for sharing.
    struct myData *my_shared;

    //Attaching to shared data. Becomes pointer to the memory location of that data
    my_shared = (struct myData *) shmat(shmId, NULL, 0);
    my_shared->messageReady = false;
    my_shared->processDone[0] = true;
    my_shared->processDone[1] = true;

    while(1){

	//Wait for both reads to be complete before prompting for a new message
	while(my_shared->messageReady == true){
	    pause();
	}

        //Request message and read it into shared mem struct
        printf("Please enter the message: ");
        //fgets(my_shared->message, 50, stdin);
        fgets(buff, 50, stdin);
	int len = strlen(buff);
	len--;
	if (buff[len] == '\n'){
	    buff[len] = '\0';
        }
	strcpy(my_shared->message, buff);
        my_shared->processDone[0] = false;
        my_shared->processDone[1] = false;
        my_shared->messageReady = true;
	kill(-1, SIGCONT);
	
	if(strcmp(my_shared->message, "quit") == 0){
	    break;
	}
    }

    if (shmdt (my_shared) < 0){
	perror("Can't detach: \n");
	exit (1);
    }

        //Deallocating shared data
    if (shmctl (shmId, IPC_RMID, 0) < 0) {
        perror ("can't deallocate\n");
        exit (1);
    }
    return 0;
}
