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



int opt_yield=0,spinLock=0, mutex=0, numThread=1, iter=1, totalElements=0, lock=0;
SortedList_t *list;
SortedListElement_t* elements;
struct timespec start,end;
char *keyOptions ="123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
pthread_mutex_t mymutex =PTHREAD_MUTEX_INITIALIZER;





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
  int id= *(int *)tid;
  int length;
  //insertion
  for(int i=id; i<totalElements; i+=numThread){
    if(mutex){
       pthread_mutex_lock(&mymutex);
       SortedList_insert(list,&elements[i]);
       pthread_mutex_unlock(&mymutex);
    }
    else if(spinLock){
      while(__sync_lock_test_and_set(&lock, 1));
      SortedList_insert(list,&elements[i]);
      __sync_lock_release(&lock);
    }
    else{
      SortedList_insert(list,&elements[i]);
    }


   
  }

  //length
   if(mutex){
     pthread_mutex_lock(&mymutex);
     length = SortedList_length(list);
     pthread_mutex_unlock(&mymutex);
    }
    else if(spinLock){
       while(__sync_lock_test_and_set(&lock, 1));
      length = SortedList_length(list);
      __sync_lock_release(&lock);
    }
    else{      
      length = SortedList_length(list);
    }

  if( length <0){
    fprintf(stderr,"Error SortedList_length has a corruption issue.");
    exit(2);
  }

  //deletion
  SortedListElement_t *temp;
  for(int i=id; i <totalElements; i+=numThread){
    
    if(mutex){

      
      pthread_mutex_lock(&mymutex);

       temp = SortedList_lookup(list,elements[i].key);
       
       if(temp == NULL){
	 fprintf(stderr,"Error SortedList_lookup has a corruption issue.");
	 exit(2);
       }

       SortedList_delete(temp);
	 

       pthread_mutex_unlock(&mymutex);
    }
    else if(spinLock){



      while(__sync_lock_test_and_set(&lock, 1));
      temp = SortedList_lookup(list,elements[i].key);
      
      if(temp == NULL){
	fprintf(stderr,"Error SortedList_lookup has a corruption issue.");
	exit(2);
      }
      SortedList_delete(temp);
            
      __sync_lock_release(&lock);
    }
    else{//no sync
      
      
      
      temp = SortedList_lookup(list,elements[i].key);
      
      if(temp == NULL){
	fprintf(stderr,"Error SortedList_lookup has a corruption issue.");
	exit(2);
      }
     
      SortedList_delete(temp);
      
      
    }
    
  }
  return NULL;
}

/*

Function that will generate a random key that helps initalize the "nodes" in the
list object.

 */
char* createKey(){
  int keyLength=4;
  srand(time(NULL));
  char *key = (char*) malloc((keyLength+1) *(sizeof(char)));
  for(int i=0; i<keyLength;i++){
    key[i]= keyOptions[rand() % strlen(keyOptions)];
  }
  key[keyLength] = '\0'; //cstring                                              
  return key;
}




int main(int argc, char** argv){
  int opt =0;
  static struct option long_options[] = {
				      {"threads", required_argument,0,'t'},
				      {"iterations",required_argument,0,'i'},				     
				      {"yield",required_argument,0,'y'},
				      {"sync",required_argument,0,'s'},
				      {0,0,0,0}
				      
  };


  signal(SIGSEGV,segFaultCatch);//catch em segfaulting.

  while( (opt=getopt_long(argc,argv,"t:i:y:s:",long_options, NULL) )!= -1){
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

  //initialize the list  
  list = malloc(sizeof(SortedList_t));
  elements = malloc(sizeof(SortedListElement_t) *totalElements);

  list->next=list;
  list->prev= list;
  list->key =NULL;

  for(int i=0; i< totalElements; i++){//each element has an initialize key
    elements[i].key = createKey();
  }

  pthread_t threads[numThread];
  int id[numThread];
  
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
  long avgTimeOp =  duration/totalOps;
  
  if(SortedList_length(list) !=0){
    fprintf(stderr,"List length is not equal to zero, something went wrong");
    exit(2);
  }


  fprintf(stdout,"%s,%d,%d,%d,%d,%ld,%ld\n",testName,numThread,iter,1,totalOps,duration,avgTimeOp);

  exit(0);
}
