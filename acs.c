/*
CSC360 Assignment2
shuwenli
V00024025
*/
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

//define the customer structure
typedef struct _cust{
	int id;
	int priority;
  int arrivetime;
	int servicetime;
	double starttime;
	double timeafterwait;
	double endtime;
}cust;

//define the clerk structure
typedef struct _clerk{
	int id;
	int status;
}clerk;

//define the queue
typedef struct node_t{
	cust *customer;
  struct node_t *next;
}node_t;

//define new node
node_t *new_node(cust *customer){
	node_t *temp = (node_t *)malloc(sizeof(node_t));
	temp->customer = customer;
  temp->next = NULL;
	return temp;
}

//add new node to queue(to tail)
node_t *add_queue(node_t *queue, node_t *new){
	node_t *curr;
    if(queue == NULL){
        return new;
    }
    for(curr = queue; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return queue;
}
//remove node from queue(to head)
node_t *remove_queue(node_t *queue){
	if(queue == NULL){
		return queue;
	}
	node_t *curr = queue;
	node_t *head = queue->next;
	free(curr);
	return head;
}

//peek node of queue
int peek(node_t *queue){
	if(queue == NULL){
		return 0;
	}
	return queue->customer->id;
}

//count the customer in different class
int finalecon = 0;
int finalbusi = 0;
//count how many node in two queue
int econcount = 0;
int busicount = 0;
//make sure the specific clerk serve which customer
int clerkone = 0;
int clerktwo = 0;
int clerkthree = 0;
int clerkfour = 0;
//build a global array to store all customer information
cust information[100];
//build a global array to store 4 clerk information
clerk clinfo[4];
int clerk_id = 0;
//build two array to store the customer of different class
node_t *busiqueue = NULL;
node_t *econqueue = NULL;
struct timeval start;
//define some variable for final count waiting time
double waittime;
double busiwait;
double econwait;
//define the mutex and conditional variable of two queue
pthread_mutex_t busilock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t busiconvar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t econlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t econconvar = PTHREAD_COND_INITIALIZER;
//define the mutex and conditional variable of four clerk
pthread_mutex_t clerk1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convar1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convar2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convar3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convar4 = PTHREAD_COND_INITIALIZER;
//define the mutex and conditional variable of two queue are all empty
pthread_mutex_t clerksignal = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convarsignal = PTHREAD_COND_INITIALIZER;
//define the mutex of wait
pthread_mutex_t wait = PTHREAD_MUTEX_INITIALIZER;
//define the mutex and conditional variable of track
//0 means work in customer function, 1 means work in clerk function.
pthread_mutex_t clerktrack = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convartrack = PTHREAD_COND_INITIALIZER;
//this function is to get the system time of machine
double systemtime(){
	struct timeval current;
	gettimeofday(&current, NULL);
	return ((current.tv_sec + (double)current.tv_usec/1000000)-(start.tv_sec + (double)start.tv_usec/1000000));
}
//store data to information array by using customer type
void store(char data[256][256]){
	int i;
	for(i = 1;i <= atoi(data[0]);i++){
		data[i][1] = ',';
		char* divide = strtok(data[i],",");
		int j;
		int result[4];
		for(j = 0;j < 4;j++){
			result[j] = atoi(divide);
			divide = strtok(NULL, ",");
		}
		//store information from inside array to global array to ensure other customer can use easily.
		information[i-1].id = result[0];
		information[i-1].priority = result[1];
		information[i-1].arrivetime = result[2];
		information[i-1].servicetime = result[3];
	}
}

//store id to clerk, initialize
void stid(){
	int i;
	for(i = 0;i < 4;i++){
		clinfo[i].id = i+1;
	}
}

//this function is to make sure the specific clerk serve which customer,
//if the clerk is equal to 1, return the number stored in clerkone.
int servecustomer(int clerk){
	if(clerk == 1){
		return clerkone;
	}else if(clerk == 2){
		return clerktwo;
	}else if(clerk == 3){
		return clerkthree;
	}else{
		return clerkfour;
	}
}
//customer function help to create customer thread
void *customer_function(void *ptr){
	//transfer the format of ptr to cust struct
	cust * cPtr = (cust *)ptr;
	usleep(cPtr->arrivetime*100000);
	printf("A customer arrives: customer ID %2d. \n", cPtr->id);
	if(cPtr->priority == 0){
		finalecon++;
		//lock to make sure just 1 customer add and remove queue at one time
		if(pthread_mutex_lock(&econlock)){
			perror("There is an error of fail on lock mutex.\n");
		}
		//add node in econqueue
		econqueue = add_queue(econqueue, new_node(cPtr));
		//add size of econqueue
		econcount++;
		if(pthread_cond_signal(&convarsignal)){
			perror("There is an error of fail on signal conditional variable.\n");
		}
		printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",cPtr->priority, econcount);
		cPtr->starttime = systemtime();
		if(pthread_cond_wait(&econconvar, &econlock)){
			perror("There is an error of fail on wait conditional variable.\n");
		}
		int record = clerk_id;
		//this while loop is work on current customer is not the customer that clerk serve, wait continuely
		while(cPtr->id != servecustomer(record)){
			pthread_cond_wait(&econconvar, &econlock);
		}
		econqueue = remove_queue(econqueue);
		econcount--;

		if(pthread_mutex_unlock(&econlock)){
			perror("There is an error of fail on unlock mutex.\n");
		}
		//get the system time of arrive and line
		cPtr->timeafterwait = systemtime();
		//lock to ensure one person change overall waittime
		pthread_mutex_lock(&wait);
		//calculate the overall wait time and economic queue wait time
		waittime = waittime + (cPtr->timeafterwait-cPtr->starttime);
		econwait = econwait + (cPtr->timeafterwait-cPtr->starttime);
		pthread_mutex_unlock(&wait);
		printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", cPtr->timeafterwait, cPtr->id,record);
		//this usleep is to sleep until the service finish.
		usleep(cPtr->servicetime*100000);
		cPtr->endtime = systemtime();
		printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", cPtr->endtime,cPtr->id,record);
		//this if else statement is to send signal to specific clerk(to distinguish different record)
		if(record == 1){
			pthread_mutex_lock(&clerk1);
			pthread_cond_signal(&convar1);
			pthread_mutex_unlock(&clerk1);
		}else if(record == 2){
			pthread_mutex_lock(&clerk2);
			pthread_cond_signal(&convar2);
			pthread_mutex_unlock(&clerk2);
		}else if(record == 3){
			pthread_mutex_lock(&clerk3);
			pthread_cond_signal(&convar3);
			pthread_mutex_unlock(&clerk3);
		}else{
			pthread_mutex_lock(&clerk4);
			pthread_cond_signal(&convar4);
			pthread_mutex_unlock(&clerk4);
		}
	}else{
		finalbusi++;
		if(pthread_mutex_lock(&busilock)){
			perror("There is an error of fail on lock mutex.\n");
		}
		//add node in queue
		busiqueue = add_queue(busiqueue, new_node(cPtr));
		//add size of queue
		busicount++;
		if(pthread_cond_signal(&convarsignal)){
			perror("There is an error of fail on signal conditional variable.\n");
		}
		printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",cPtr->priority, busicount);
		cPtr->starttime = systemtime();
		pthread_cond_wait(&busiconvar, &busilock);
		int record = clerk_id;
		//this while loop is work on current customer is not the customer that clerk serve, wait continuely
		while(cPtr->id != servecustomer(record)){
			pthread_cond_wait(&busiconvar, &busilock);
		}
		//remove node from queue and then unlock the whole process
		busiqueue = remove_queue(busiqueue);
		busicount--;
		pthread_mutex_unlock(&busilock);
		//get the system time of arrive and line
		cPtr->timeafterwait = systemtime();
		//lock to ensure one person change overall waittime
		pthread_mutex_lock(&wait);
		waittime = waittime + (cPtr->timeafterwait-cPtr->starttime);
		busiwait = busiwait + (cPtr->timeafterwait-cPtr->starttime);
		pthread_mutex_unlock(&wait);
		printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", cPtr->timeafterwait,cPtr->id,record);
		//this usleep is to sleep until the service finish.
		usleep(cPtr->servicetime*100000);
		cPtr->endtime = systemtime();
		printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", cPtr->endtime,cPtr->id,record);
		//this if else statement is to send signal to specific clerk(to distinguish different record)
		if(record == 1){
			pthread_mutex_lock(&clerk1);
			pthread_cond_signal(&convar1);
			pthread_mutex_unlock(&clerk1);
		}else if(record == 2){
			pthread_mutex_lock(&clerk2);
			pthread_cond_signal(&convar2);
			pthread_mutex_unlock(&clerk2);
		}else if(record == 3){
			pthread_mutex_lock(&clerk3);
			pthread_cond_signal(&convar3);
			pthread_mutex_unlock(&clerk3);
		}else{
			pthread_mutex_lock(&clerk4);
			pthread_cond_signal(&convar4);
			pthread_mutex_unlock(&clerk4);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

//clerk function to get customer and send signal
void *clerk_function(void *ptr){
	clerk* cPtr = (clerk *)ptr;
	int clerk_id_copy = cPtr->id;
	//the while loop is to check continuesly
	while(1){
		//when both of two queue are empty
		if(pthread_mutex_lock(&clerksignal)){
			perror("There is an error of fail on lock mutex.\n");
		}
		//if two class is all empty, keep wait and check until one of queue is not empty
		while(busicount == 0 && econcount == 0){
			pthread_cond_wait(&convarsignal, &clerksignal);
		}
		if(pthread_mutex_unlock(&clerksignal)){
				perror("There is an error of fail on unlock mutex.\n");
		}
		//if business class is not empty
		if(busicount != 0){
			pthread_mutex_lock(&busilock);
			//change the global variable of clerk_id
			clerk_id = clerk_id_copy;
			//if the queue is not null, then we can store the first customer in queue to the serving clerk
			if(peek(busiqueue) != 0){
				if(clerk_id_copy == 1){
					clerkone = peek(busiqueue);
				}else if(clerk_id_copy == 2){
					clerktwo = peek(busiqueue);
				}else if(clerk_id_copy == 3){
					clerkthree = peek(busiqueue);
				}else{
					clerkfour = peek(busiqueue);
				}
			}
			//send signal to clerk
			if(pthread_cond_signal(&busiconvar)){
					perror("There is an error of fail on signal conditional variable.\n");
			}
			pthread_mutex_unlock(&busilock);
			//wait the request to get the new customer
			if(clerk_id_copy == 1){
					pthread_mutex_lock(&clerk1);
					pthread_cond_wait(&convar1, &clerk1);
					pthread_mutex_unlock(&clerk1);
			}else if(clerk_id_copy == 2){
					pthread_mutex_lock(&clerk2);
					pthread_cond_wait(&convar2, &clerk2);
					pthread_mutex_unlock(&clerk2);
			}else if(clerk_id_copy == 3){
					pthread_mutex_lock(&clerk3);
					pthread_cond_wait(&convar3, &clerk3);
					pthread_mutex_unlock(&clerk3);
			}else{
					pthread_mutex_lock(&clerk4);
					pthread_cond_wait(&convar4, &clerk4);
					pthread_mutex_unlock(&clerk4);
			}

		}else{
			if(pthread_mutex_lock(&econlock)){
					perror("There is an error of fail on lock mutex.\n");
			}
			clerk_id = clerk_id_copy;
			//if the queue is not null, then we can store the first customer in queue to the serving clerk
			if(peek(econqueue) != 0){
				if(clerk_id_copy == 1){
					clerkone = peek(econqueue);
				}else if(clerk_id_copy == 2){
					clerktwo = peek(econqueue);
				}else if(clerk_id_copy == 3){
					clerkthree = peek(econqueue);
				}else{
					clerkfour = peek(econqueue);
				}
			}
			//send signal to clerk
			if(pthread_cond_signal(&econconvar)){
				perror("There is an error of fail on signal conditional variable.\n");
			}
			if(pthread_mutex_unlock(&econlock)){
				perror("There is an error of fail on unlock mutex.\n");
			}
			//wait the request to get the new customer
			if(clerk_id_copy == 1){
					pthread_mutex_lock(&clerk1);
					pthread_cond_wait(&convar1, &clerk1);
					pthread_mutex_unlock(&clerk1);
			}else if(clerk_id_copy == 2){
					pthread_mutex_lock(&clerk2);
					pthread_cond_wait(&convar2, &clerk2);
					pthread_mutex_unlock(&clerk2);
			}else if(clerk_id_copy == 3){
					pthread_mutex_lock(&clerk3);
					pthread_cond_wait(&convar3, &clerk3);
					pthread_mutex_unlock(&clerk3);
			}else{
					pthread_mutex_lock(&clerk4);
					pthread_cond_wait(&convar4, &clerk4);
					pthread_mutex_unlock(&clerk4);
			}
		}
	}
	return NULL;
}
int main(int argc, char* argv[]){
	//read file
    char data[256][256];
    FILE* file;
		//open file by using fopen
    file = fopen(argv[1],"r");
    if(file == NULL){
        printf("Error on opening file!");
        return(-1);
    }
    int i = 0;
		//use fgets to gat all data from file to array
    while(fgets(data[i], 256, file) != NULL){
        i++;
    }
    fclose(file);
		//call store and stid function
		store(data);
		stid();
		pthread_t custthrd[atoi(data[0])];
		pthread_t clerkthrd[4];
	  int j;
	  int k;
  	int p;
  	int custc;
  	int clerkc;
		//create clerk thread
  	for(p = 0;p < 4;p++){
			if((clerkc = pthread_create(&clerkthrd[p], NULL, clerk_function, &clinfo[p]))){
				fprintf(stderr, "error: pthread_create, rc: %d\n", clerkc);
				exit(-1);
			}
		}
		gettimeofday(&start, NULL);
		//create customer thread
		for(j = 0;j < atoi(data[0]);j++){
			if((custc = pthread_create(&custthrd[j], NULL, customer_function, &information[j]))){
				fprintf(stderr, "error: pthread_create, rc: %d\n", custc);
				exit(-1);
			}
		}
		//join thread back to main thread
		for(k = 0;k < atoi(data[0]);k++){
			pthread_join(custthrd[k], NULL);
		}
		//print the final time(average waiting time, average business waiting time, average economy waiting time)
		printf("The average waiting time for all customers in the system is: %.2f seconds. \n",waittime/atoi(data[0]));
		printf("The average waiting time for all business-class customers is: %.2f seconds. \n",busiwait/finalbusi);
		printf("The average waiting time for all economy-class customers is: %.2f seconds. \n",econwait/finalecon);
    return 0;
}
