#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "check.h"
#include <syscall.h>
#include <unistd.h>

#define RAND_DIVISOR 300000000
#define TRUE 1

typedef int buffer_item;
#define BUFFER_SIZE 5
#define true 1
#define false 0

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

/* buffer counter */
int counter;

/* the locks */
int _full, _empty, lock1, lock2, lock3;

pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

void *producer(void *param); /* the producer thread */
void *consumer(void *param); /* the consumer thread */


void inicializeDataBanana(){
    pthread_attr_init(&attr);
    counter = 0;
    _full = 0;
    _empty = BUFFER_SIZE;
    lock1 = 0;
    lock2 = 0;
    lock3 = 0;
}

/* Add an item to the buffer */
int insert_item(buffer_item item) {
   /* When the buffer is not full add the item
      and increment the counter*/
    if(counter < BUFFER_SIZE) {
        buffer[counter] = item;
        counter++;
        return 0;
    }
    else { /* Error the buffer is full */
        return -1;
    }
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item) {
    /* When the buffer is not empty remove the item
        and decrement the counter */
    if(counter > 0) {
        *item = buffer[(counter-1)];
        counter--;
        return 0;
    }
    else { /* Error buffer empty */
        return -1;
    }
}
/* Producer Thread */
void *producer(void *param) {
    int produced=0;  //Amount of items produced
    buffer_item item;
    while(TRUE) {
        int key1 = true;
        int key2 = true;
        int key3 = true;
        int rNum = rand() / RAND_DIVISOR;
        /* sleep for a random period of time */
       // sleep(rNum);
        /* generate a random number */
        item = rand()%100;
        while(key1)
            key1 = __sync_lock_test_and_set(&lock1, 1);
        //---------☢----------//
        _empty--;
	
	if(insert_item(item) == -1 ){
	    lock1 = false;
	}
      	else{
		produced++;
		_full++;
		printf("Producer %06ld | item %03d -- counter: %3d -- produced: %d\n",
        	syscall(SYS_gettid), item,counter,produced);
	}
	
	//---------☢----------// 
        lock1 = false;
    }
}
/* Consumer Thread */
void *consumer(void *param) {
    int consumed=0;//Amount of items consumed
   buffer_item item;
   while(TRUE) {
        /* sleep for a random period of time */
        int rNum = rand() / RAND_DIVISOR;
      //  sleep(rNum);
        int key1 = true;

        while(key1)
	    key1 = __sync_lock_test_and_set(&lock1, 1);
	//---------☢----------// 
        _full--;
        lock2 = false;
      
        if(remove_item(&item)==-1) {
		lock1 = false;
	}
	else{
		consumed++;
		_empty++;
		printf("Consumer %06ld | item %03d -- counter: %3d -- consumed: %d\n",
        	syscall(SYS_gettid), item,counter,consumed);
	}
        lock1 = false;   
        //---------☢----------// 

        
   }
}



int main(int argc, char *argv[]) {
    /* Loop counter */
    int i;
    srand(time(NULL));
    /* Verify the correct number of arguments were passed in */
    if(argc != 4) {
        fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
    }

    int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
    int numProd = atoi(argv[2]); /* Number of producer threads */
    int numCons = atoi(argv[3]); /* Number of consumer threads */

    /* Initialize the app */
    inicializeDataBanana();

    pthread_t thread [numProd+numCons];
    /* Create the producer threads */
    for(i = 0; i < numProd; i++) {
        /* Create the thread */
        thread[i]=pthread_create(&tid,&attr,producer,NULL);
    }

    /* Create the consumer threads */
    for(i = 0; i < numCons; i++) {
        /* Create the thread */
        thread[numProd+i]=pthread_create(&tid,&attr,consumer,NULL);
    }

    /* Sleep for the specified amount of time in milliseconds */
    sleep(mainSleepTime);

    /* Exit the program */
    printf("Exit the program\n");
    exit(0);
}
