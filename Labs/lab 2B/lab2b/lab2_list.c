/*
NAME: Eric Chen
EMAIL: erchen3pro@gmail.com
ID: 
*/

#include "SortedList.h"
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>



int opt_yield=0,spinLock=0, mutex=0, numThread=1, iter=1, totalElements=0, numLists=1;
SortedListElement_t* elements;
struct timespec start,end;
char *keyOptions ="123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
long long* threadLockTime;

pthread_mutex_t *mymutex;
int *lock;
SortedList_t *list;



/*
Function to catch Segmentation Faults.

 */
void segFaultCatch(int n){
  if(n == SIGSEGV){
    fprintf(stderr,"Segmentation Fault caught error is %d and message is %s \n",errno , strerror(errno));
    exit(2);
  }
}




/*

Function to either set/stop the time.

 */
void clocking(struct timespec* tp){
  if(clock_gettime(CLOCK_MONOTONIC,tp)==-1){
    fprintf(stderr,"Failed to make this call.");
    exit(2);
  }
}




/*
  
Function that will run SortedList_insert,delete,lookup, and length
but corresponding to the lock mechanism specified This gets passed into pthread_create.


 */
void* threadWork(void *tid){
  struct timespec lockStart, lockEnd;
  int id= *(int *)tid;
  int length;


  //insertion
  for(int i=id; i<totalElements; i+=numThread){
    int hash =elements[i].key[0] %numLists;
    //  fprintf(stdout,"Value: %d\n",hash);
    if(mutex){
      clocking(&lockStart);
      pthread_mutex_lock(&mymutex[hash]);
      clocking(&lockEnd);     
     
      SortedList_insert(&list[hash], &elements[i]);
      pthread_mutex_unlock(&mymutex[hash]);
      
    }
    else if(spinLock){
      clocking(&lockStart);
      while(__sync_lock_test_and_set(&lock[hash], 1));
      clocking(&lockEnd);
     
      SortedList_insert(&list[hash],&elements[i]);
      __sync_lock_release(&lock[hash]);
    }
    else{//not locking
      SortedList_insert(&list[hash], &elements[i]);
    }
    
    //add time for the tread
    threadLockTime[id] += 1000000000*(lockEnd.tv_sec-lockStart.tv_sec) + (lockEnd.tv_nsec -lockStart.tv_nsec);

   
  }

  //length

  for(int i =0; i <numLists; i++){
    int hash =elements[i].key[0] %numLists;
    if(mutex){
      clocking(&lockStart);
      pthread_mutex_lock(&mymutex[hash]);
      clocking(&lockEnd);
      
      length += SortedList_length(&list[hash]);
      pthread_mutex_unlock(&mymutex[hash]);
    }
    else if(spinLock){
      
      clocking(&lockStart);
      while(__sync_lock_test_and_set(&lock[hash], 1));
      clocking(&lockEnd);
      
      length += SortedList_length(&list[hash]);
      __sync_lock_release(&lock[hash]);
    }
    else{      
      length += SortedList_length(&list[hash]);
    }
  }
  if( length <0){
    fprintf(stderr,"Error SortedList_length has a corruption issue.");
    exit(2);
  }
  //add the time for lock on length
  threadLockTime[id] += 1000000000*(lockEnd.tv_sec-lockStart.tv_sec) +	(lockEnd.tv_nsec -lockStart.tv_nsec);
  
  //deletion
  SortedListElement_t *temp;
  for(int i=id; i <totalElements; i+=numThread){
    int hash =elements[i].key[0] %numLists;
    if(mutex){

      clocking(&lockStart);
      pthread_mutex_lock(&mymutex[hash]);
      clocking(&lockEnd);
      temp = SortedList_lookup(&list[hash],elements[i].key);
      
      if(temp == NULL){
	fprintf(stderr,"Error SortedList_lookup has a corruption issue.");
	exit(2);
      }

      SortedList_delete(temp);
      
      
      pthread_mutex_unlock(&mymutex[hash]);
      
    }
    else if(spinLock){


      clocking(&lockStart);
      while(__sync_lock_test_and_set(&lock[hash], 1));
      clocking(&lockEnd);
      
      temp = SortedList_lookup(&list[hash],elements[i].key);
      
      if(temp == NULL){
	fprintf(stderr,"Error SortedList_lookup has a corruption issue.");
	exit(2);
      }
      SortedList_delete(temp);
            
       __sync_lock_release(&lock[hash]);
    }
    else{//no sync
      
      
      
      temp = SortedList_lookup(&list[hash],elements[i].key);
      
      if(temp == NULL){
	fprintf(stderr,"Error SortedList_lookup has a corruption issue.");
	exit(2);
      }
     
      SortedList_delete(temp);
      
      
    }
    //add the time                                                                                                                                            
    threadLockTime[id] += 1000000000*(lockEnd.tv_sec-lockStart.tv_sec) + (lockEnd.tv_nsec -lockStart.tv_nsec);
 
  }
  return NULL;
}

int main(int argc, char** argv){
  int opt =0;
  static struct option long_options[] = {
				      {"threads", required_argument,0,'t'},
				      {"iterations",required_argument,0,'i'},				     
				      {"yield",required_argument,0,'y'},
				      {"sync",required_argument,0,'s'},
				      {"lists",required_argument,0,'l'},
				      {0,0,0,0}
				      
  };


  signal(SIGSEGV,segFaultCatch);//catch em segfaulting.

  while( (opt=getopt_long(argc,argv,"t:i:y:s:l:",long_options, NULL) )!= -1){
    switch(opt){
    case 't':
      numThread = atoi(optarg);
      break;
    case 'i':
      iter= atoi(optarg);
      break;
    case 'y':
      for(int i=0; i<(int)strlen(optarg); i++){
	if (optarg[i] =='i'){
            opt_yield = opt_yield |INSERT_YIELD;
          }
          else if(optarg[i] =='l'){
            opt_yield = opt_yield |LOOKUP_YIELD;
          }
          else if(optarg[i] == 'd'){
            opt_yield = opt_yield |DELETE_YIELD;
          }
          else{
            fprintf(stderr,"Invalid argument");
            exit(1);
          }
	
      }
      break;
    case 's':
      if(*optarg =='m'){
	mutex=1;
      }
      else if(*optarg=='s'){
	spinLock=1;
      }
      else{
	fprintf(stderr,"Incorrect synchronization mechanism");
	exit(1);
      }
      break;
    case 'l':
      numLists= atoi(optarg);
      break;
    default:
      fprintf(stderr,"invalid command-line option");
      exit(1);
    }

    
  }

  //Test Name Selection
  char testName[20] ="list-";
  if(opt_yield){
    if(opt_yield & INSERT_YIELD){
      strcat(testName,"i");
    }
    if(opt_yield & DELETE_YIELD){
      strcat(testName,"d");
    }
    if(opt_yield & LOOKUP_YIELD){
      strcat(testName,"l");
    }
  }
  else{
    strcat(testName,"none");
  }
  
  strcat(testName,"-");
  
  if(mutex){
    strcat(testName,"m");
  }
  else if(spinLock){
    strcat(testName,"s");
  }
  else{
    strcat(testName,"none");
  }
  
  totalElements = numThread * iter;
  //  printf("%d",totalElements);

  list= (SortedList_t*) malloc(numLists *sizeof(SortedList_t));
  mymutex = malloc(numLists * sizeof(pthread_mutex_t));
  lock = malloc(numLists * sizeof(int));
  //initialize the Amount of Lists  
  elements = malloc(sizeof(SortedListElement_t) *totalElements);

  
  for(int i =0; i<numLists; i++){//Initialize heads
    list[i].next=&list[i];
    list[i].prev=&list[i];
    list[i].key =NULL;

  }
  for(int i=0; i<numLists;i++){
     if(mutex){
       pthread_mutex_init(&mymutex[i],NULL);
     }
     if(spinLock){
       lock[i]=0;
     }
  }
  
  srand(time(NULL));
  for(int i=0; i< totalElements; i++){//Initialize the keys
    int keyLength=4;
    char *key = (char*) malloc((keyLength+1) *(sizeof(char)));
    for(int i=0; i<keyLength;i++){
      if(i==0)
       key[i]= rand() % numLists;
      key[i]= keyOptions[rand() % strlen(keyOptions)];
    }                                                                                                                            
    key[keyLength] = '\0'; //cstring    
    elements[i].key = key;
  }
  
  pthread_t threads[numThread];
  int id[numThread];
  threadLockTime = malloc(numThread * sizeof(long long));
 
  //start clocking
  clocking(&start);
  //pthread create
  for(int i =0; i<numThread; i++){
    id[i] =i;
     if(pthread_create(&threads[i],NULL,threadWork , &id[i]) ==1){
       fprintf(stderr,"Error creating pthreads");
       exit(2);
      }
  }
  //pthread join
  for(int i =0; i<numThread; i++){
    if(pthread_join(threads[i],NULL)==1){
      fprintf(stderr,"Error joining pthreads");
      exit(2);
    } 
  }
  
  //end clocking
  clocking(&end);
  
  long duration = (end.tv_sec -start.tv_sec) * 1000000000; //nano seconds
  duration = duration + (end.tv_nsec -start.tv_nsec);
  int  totalOps = totalElements*3;
  //  printf("%d", totalOps);
  long avgTimeOp =  duration/totalOps;// this is secs/op
  long long lockTime=0,avgLock=0;

  //get time
  for(int i=0; i <numThread; i++){
    lockTime+=threadLockTime[i];
   }
  avgLock = lockTime/totalOps;
 
  
  fprintf(stdout,"%s,%d,%d,%d,%d,%ld,%ld,%lld\n",testName,numThread,iter,numLists,totalOps,duration,avgTimeOp,avgLock);

  exit(0);
}
