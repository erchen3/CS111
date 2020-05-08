/*                                                                                                                                             
NAME: Eric Chen                                                                                                                               
EMAIL: erchen3pro@gmail.com                                                                                                                   
 ID:                                                                                                                                 
*/



#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

long long counter =0;
int iter=1;
int opt_yield=0, lock =0;
char synchronization;
pthread_mutex_t mymutex =PTHREAD_MUTEX_INITIALIZER;


/*

Default add function provided to demonstrate race condition for later
 */
void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if(opt_yield)
    sched_yield();
  *pointer = sum;
}

/*

Function to start/stop the clock in timing specifications.
 */
void clocking(struct timespec* tp){
  if(clock_gettime(CLOCK_MONOTONIC,tp)==-1){
    fprintf(stderr,"Failed to make this call.");
    exit(2);
  }
}
/*

Thread sub-addition that handles mutex, compare and swap, and spin lock case

*/
void* threadAddition(){

  for(int i=0; i<iter; i++){
    if(synchronization =='m'){//do the add with mutex
      pthread_mutex_lock(&mymutex);
      add(&counter,1);
      pthread_mutex_unlock(&mymutex);
    }
    else if( synchronization == 's'){ // do the add with spin lock
      while(__sync_lock_test_and_set(&lock,1));
      add(&counter,1);
      __sync_lock_release(&lock);
    }
    else if(synchronization == 'c'){// do the compare and swap add
      long long oldVal, newVal;
      do{
	oldVal = counter;
	newVal = oldVal +1;
	if(opt_yield){
	  sched_yield();
	}
      }while(__sync_val_compare_and_swap(&counter,oldVal,newVal) != oldVal);
    }
    else{
      add(&counter, 1);
    }
  }
  for(int i=0;i<iter; i++){// the subtraction
    if(synchronization =='m'){
      pthread_mutex_lock(&mymutex);
      add(&counter,-1);
      pthread_mutex_unlock(&mymutex);
    }
    else if( synchronization == 's'){
      while(__sync_lock_test_and_set(&lock,1));
      add(&counter,-1);
      __sync_lock_release(&lock);
    }
    else if(synchronization == 'c'){
      long long	oldVal,	newVal;
      do{
        oldVal = counter;
        newVal = oldVal	-1;
        if(opt_yield){
          sched_yield();
	}
      }while(__sync_val_compare_and_swap(&counter,oldVal,newVal) != oldVal);
    }
    else{
    add(&counter,-1);
    }
    
  }
  return NULL;
}
int main(int argc,char** argv){
  int opt=0,numThread=1;
  struct timespec start, end;
  char* testName = NULL;
  static struct option long_options[] ={
					{"threads",required_argument,0,'t'},
					{"iterations",required_argument,0,'i'},
					{"yield",no_argument,0,'y'},
					{"sync",required_argument,0,'s'},
					{0,0,0,0 }
  };
 
  while((opt = getopt_long(argc,argv,"t:i:ys:",long_options,NULL)) != -1){
    switch(opt){
    case 't':
      numThread = atoi(optarg);
      break;
    case 'i':
      iter = atoi(optarg);
      break;
    case 'y':
      opt_yield=1;
      break;
    case 's':
      if((*optarg !='s') & (*optarg!='m') & (*optarg !='c')){
	fprintf(stderr,"Not the right option for synchronization");
	exit(1);
      }
      else{
	synchronization = *optarg;
      }
      break;
    default:
      fprintf(stderr,"Invalid command-line parameter ");
      exit(1);
    }
  }
  //--yield functionality to select the tag
  if(opt_yield){ //yielding
    if( !synchronization){
      testName = "add-yield-none"; //yield no synchronize
    }
    else if(synchronization == 'c'){//compare and swap
      testName="add-yield-c";
    }
    else if(synchronization == 's'){//spin lock
      testName="add-yield-s";
    }
    else if(synchronization == 'm'){//mutex
      testName="add-yield-m"; 
    }
     
  }
  else { //not yielding
    testName="add-none"; //no yield no synchronization case
    if(synchronization == 'c'){
      testName= "add-c";
    }
    else if(synchronization == 's'){
      testName="add-s";
    }
     else if(synchronization == 'm'){
       testName="add-m";
     }
  }
  
  pthread_t threads[numThread];
  //begin clock
  clocking(&start); 
  //make threads
  for(int i=0; i<numThread; i++){
    if(pthread_create(&threads[i],NULL,threadAddition,NULL) ==1){
      fprintf(stderr,"Error creating pthreads");
      exit(2);
    }
  }
  //wait for all threads exit
  for(int i=0; i<numThread; i++){
    if(pthread_join(threads[i],NULL)==1){
      fprintf(stderr,"Error joining pthreads");
      exit(2);
    }
  }

  clocking(&end);
  
  long duration = (end.tv_sec- start.tv_sec) * 1000000000 ; //nano seconds
  duration = duration + (end.tv_nsec -start.tv_nsec);


  int numOps = numThread * iter * 2; //number of operations
  long avgTimeOp =duration/numOps; //total average time per operation
  fprintf(stdout, "%s,%d,%d,%d,%ld,%ld,%lld\n", testName,numThread,iter,numOps, duration, avgTimeOp, counter);

  exit(0);
}
