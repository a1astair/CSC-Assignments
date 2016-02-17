#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

//This will be the struct for all the customers
struct customer_struct
{
	int arrival_no;
	int customer_no;
	int a_time;
	int s_time;
	int priority;
};

//Global Variables
int num_of_customers;
int end_of_queue = 1;
int available = 1;
int live_threads = 0;
int current_cust_no = -1;
pthread_cond_t idle;
pthread_mutex_t mutex1; 
pthread_mutex_t mutex2;
struct customer_struct *customer_queue;
struct timeval t0;
time_t starttime;

//Puts the information in the right spot in the heap
/* Rules
The one with the highest priority will be served first.
If there is a tie at the highest priority, the one whose arrival time is the earliest will be served first.
If there is still a tie, the one who has the shortest service time will be served first.
If there is still a tie, the one who appears first in the input file will be served first.
*/
void bubble_up(int i) {
	//Base Case
	if(i == 1) {
		return;	
	}
	
	if( (customer_queue[i].priority > customer_queue[i/2].priority) ||
		(customer_queue[i].priority == customer_queue[i/2].priority && customer_queue[i].a_time < customer_queue[i/2].a_time) ||
		(customer_queue[i].priority == customer_queue[i/2].priority && customer_queue[i].a_time == customer_queue[i/2].a_time && customer_queue[i].s_time < customer_queue[i/2].s_time) ||
		(customer_queue[i].priority == customer_queue[i/2].priority && customer_queue[i].a_time == customer_queue[i/2].a_time && customer_queue[i].s_time == customer_queue[i/2].s_time && customer_queue[i].arrival_no < customer_queue[i/2].arrival_no)
	) {
		//Swap
		struct customer_struct temp;
		temp = customer_queue[i];
		customer_queue[i] = customer_queue[i/2];
		customer_queue[i/2] = temp;
		bubble_up(i/2);
	}
}

//Insert the information into the heap
void insert(struct customer_struct customer) {
	customer_queue[end_of_queue] = customer;
	bubble_up(end_of_queue);
	end_of_queue++;
}

//Bubbles down after removing the root from the heap
void bubble_down(int i) {
	int j = 2*i+1;
	if(end_of_queue <= 2*i) {
		//Has no children
		return;	
	} else if (end_of_queue == 2*i+1) {
		//Has no right child		
		j = 2*i;		
	} else if( (customer_queue[2*i].priority > customer_queue[2*i+1].priority) ||
		(customer_queue[2*i].priority == customer_queue[2*i+1].priority && customer_queue[2*i].a_time < customer_queue[2*i+1].a_time) ||
		(customer_queue[2*i].priority == customer_queue[2*i+1].priority && customer_queue[2*i].a_time == customer_queue[2*i+1].a_time && customer_queue[2*i].s_time < customer_queue[2*i+1].s_time) ||
		(customer_queue[2*i].priority == customer_queue[2*i+1].priority && customer_queue[2*i].a_time == customer_queue[2*i+1].a_time && customer_queue[2*i].s_time == customer_queue[2*i+1].s_time && customer_queue[2*i].arrival_no < customer_queue[2*i+1].arrival_no)
	)
	{
		//Left child has higher priority than the right child
		j = 2*i;
	}

	//Compare child to parent
	if( (customer_queue[j].priority > customer_queue[i].priority) ||
		(customer_queue[j].priority == customer_queue[i].priority && customer_queue[j].a_time < customer_queue[i].a_time) ||
		(customer_queue[j].priority == customer_queue[i].priority && customer_queue[j].a_time == customer_queue[i].a_time && customer_queue[j].s_time < customer_queue[i].s_time) ||
		(customer_queue[j].priority == customer_queue[i].priority && customer_queue[j].a_time == customer_queue[i].a_time && customer_queue[j].s_time == customer_queue[i].s_time && customer_queue[j].arrival_no < customer_queue[i].arrival_no)
	)
	{
		//Max child is greater than what we are swapping in	
		//Swap
		struct customer_struct temp;
		temp = customer_queue[i];
		customer_queue[i] = customer_queue[j];
		customer_queue[j] = temp;
		bubble_down(j);
	}
}

//Removes the root from the heap
void remove_from_heap() {
	if (end_of_queue == 1) {
		return;
	}
	customer_queue[1] = customer_queue[end_of_queue-1];
	end_of_queue--;
	bubble_down(1);   
}

/* This function will make sure only one active thread will be able to obtain the service. The current thread will not be able to run the rest code until it obtains the rights for service */
void request_service(struct customer_struct *customer){
	pthread_mutex_lock(&mutex1);
	if (available && (end_of_queue == 1)) {
		//set clerk to busy
		available = 0;
		pthread_mutex_unlock(&mutex1);
		return;
	}
	pthread_mutex_lock(&mutex2);

	//add customer to the queue of waiting customers and sort it by priority rules; 
	insert(*customer);
	
	pthread_mutex_unlock(&mutex2);
	while(!available || end_of_queue == 1 || (*customer).customer_no != customer_queue[1].customer_no) {	
		if (!available) {
			//Print wait and served customer
			printf("Customer %2d waits for the finish of customer %2d. \n", (*customer).customer_no, current_cust_no);	
		}

		/* pthread_cond_wait routine will automatically and
atomically unlock mutex1 while it waits. */
		pthread_cond_wait(&idle, &mutex1);
	} 

	//Remove the customer from the queue
	remove_from_heap();
	available = 0;
	return;
}

//Release the service for another thread
void release_service(int customer_number){	
	struct timeval t2;
	double elapsed;

	//Get time
	gettimeofday(&t2, NULL);
	elapsed = ((t2.tv_sec-t0.tv_sec)*1000000 + t2.tv_usec-t0.tv_usec)/1000000.0;

	//Print clerk finish message
	printf("The clerk finishes the service to customer %2d at time %.2f. \n", customer_number, elapsed);
	
	//set clerk to idle;
	available = 1;
	pthread_cond_broadcast(&idle);
	pthread_mutex_unlock(&mutex1);
}

//Sleep until arrival time then populate thread structures
void* thread_control(void *arg) {
	struct timeval t1;
	struct customer_struct *customer = (struct customer_struct *)arg;
	unsigned int sleep = (*customer).a_time * 100000u; 
	double elapsed;

	//Have to accomidate for the time the program has taken to get to this point
	/* Elapsed time */
    gettimeofday(&t1, NULL);
	sleep = ((*customer).a_time * 100000 + (t0.tv_sec-t1.tv_sec)*1000000 - t1.tv_usec + t0.tv_usec);
	if (usleep(sleep) == -1) {
		perror("Error on sleep");
		return;
	}

	//Print arrival
		printf("Customer %2d arrives: arrival time (%.2f), service time (%.1f), priority (%2d). \n", (*customer).customer_no, (*customer).a_time/10.0f, (*customer).s_time/10.0f,(*customer).priority);

	/* Thread structures are populated by the customer_struct */
	request_service(customer);
	pthread_mutex_unlock(&mutex1);

	//Get current customer
	current_cust_no = (*customer).customer_no;

 	/* Elapsed time */
    gettimeofday(&t1, NULL);
	elapsed = ((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000000.0;
	printf("The clerk starts serving customer %2d at time %.2f. \n", current_cust_no, elapsed);

	//Sleep for the service time;
	sleep = (*customer).s_time * 100000u;
	if (usleep(sleep) == -1) {
		perror("Error on sleep");
	}

	/* This function will make the clerk available again and wake up the waiting threads to again compete for the service*/
	release_service(current_cust_no);
	live_threads--;
	return;
}

//For every thread read in from the file, create a thread to execute thread_control
int main(int argc, char *argv[]) {
	int i = 0;
	int min_index = -1;
	char string[100];
	pthread_t *tid;
	
	//Get Time	
	gettimeofday(&t0, 0);

	if (argc != 2) { /* Need file name */
		printf("Usage: %s 'filename'\n", argv[0]);
		return 0;
	} else {
		FILE *fp = fopen(argv[1], "r");	
		if (fp == 0) {
			perror("Could not open file");
			return 0;
		} else {
			//read in from the file and put the information into the struct

			//Set first number equal to the number of customers
			fgets(string, 100, fp);
			num_of_customers = atoi(string);

			//Create an array of threads for each customer
			tid = (pthread_t*)malloc(sizeof(pthread_t) * num_of_customers);	

			//Create the global array for the customers
			customer_queue = (struct customer_struct *)malloc(sizeof(struct customer_struct) * (num_of_customers+1));

			struct customer_struct* new_customers = (struct customer_struct *)malloc(sizeof(struct customer_struct) * (num_of_customers));

			//Loop through the file 
			while(fgets(string, 100, fp)) {
				sscanf(string, "%d:%d,%d,%d", &new_customers[i].customer_no, &new_customers[i].a_time, &new_customers[i].s_time, &new_customers[i].priority);
				
				new_customers[i].arrival_no = i+1;
				if( min_index == -1 ||
					(customer_queue[i].priority > customer_queue[min_index].priority) ||
					(customer_queue[i].priority == customer_queue[min_index].priority && customer_queue[i].a_time < customer_queue[min_index].a_time) ||
					(customer_queue[i].priority == customer_queue[min_index].priority && customer_queue[i].a_time == customer_queue[min_index].a_time && customer_queue[i].s_time < customer_queue[min_index].s_time) ||
					(customer_queue[i].priority == customer_queue[min_index].priority && customer_queue[i].a_time == customer_queue[min_index].a_time && customer_queue[i].s_time == customer_queue[min_index].s_time && customer_queue[i].arrival_no < customer_queue[min_index].arrival_no)
				)
				{
					min_index = i;				
				}
				i++;
			}
			//Close the file
			fclose(fp);

			//Create first min_index(highest priority) thread
			live_threads++;
			pthread_create(&(tid[min_index]), NULL, &thread_control, (void*) &new_customers[min_index]); 
			for(i = 0;i < num_of_customers;i++) {	
				if (i == min_index) {
					continue;
				}
			
				//Create a thread for each customer in the file to execute thread_control()
				live_threads++;
				pthread_create(&(tid[i]), NULL, &thread_control, (void*)&(new_customers[i]));
			}
		}
	}
	
	//Min_index join
	pthread_join(tid[min_index], NULL);
	
	//Join all the threads
	for(i = 0;i < num_of_customers;i++) {
		if (i == min_index) {
			continue;
		}
		pthread_join(tid[i], NULL);
	}
	return 0;
}







