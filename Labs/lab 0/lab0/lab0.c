/*
  Eric Chen
  ID: 
  e-mail: erchen3pro@gmail.com
 */

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h> //catch case
#include <errno.h>
#include <string.h>
#include <fcntl.h>


int errno;



//Function that prints the error message when we seg fault
//apart of Catch
void segFaultCatch(int n){
  if(n == SIGSEGV){
    fprintf(stderr,"Segmentation Fault caught error is %d and message is %s \n",errno , strerror(errno));
    exit(4);
  }
}

int main(int argc, char**argv){
  
  //option structure
  static struct option long_options[] = {
    {"input", required_argument, 0,  'I' },
    {"output", required_argument, 0,  'O'},
    {"segfault", no_argument  , 0 ,  'S' },
    {"catch", no_argument, 0,  'C' },
    {0,0,0, 0 } // will tell us when to stop
  };
  char *iFile =NULL;
  char *oFile = NULL;
  int segfault, catch;
  int opt = 0;
  //Will get a value from option DS -> and select flag corresponding to the option selected.
  //Processing arguments and storing result in variable
  // keep looking for the options until no more (-1)
  while((opt = getopt_long(argc, argv, "I:O:SC", long_options, NULL)) != -1) {
    switch(opt){
    case 'I':
      iFile = optarg;//file name will be stored
      break;
    case 'O':
      oFile = optarg;
      break;
    case 'S':
      segfault =1;
      break;
    case 'C':
      catch=1;
      break;
    default:
      printf("Usage: lab0 [--input filename] [--output filename] [segfault] [catch] \n");
      exit(1); //get to here, unrecognized argument and remind user of options
    }
  }
  //Implementation of options, carrying out the action
  //-------------------
  int iFD, oFD;
  if(iFile){ //input, file descriptor 0 will refer to iFile
    iFD = open(iFile,O_RDONLY);
    if(iFD >=0){
      close(0);
      dup(iFD);
      close(iFD); 
    }
    else{//unable to open
      fprintf(stderr,"Unable to open file %s, Error is %d and message is %s \n",iFile,errno , strerror(errno));
      exit(2);
    }
  }
  if(oFile){//output redirection
    oFD =creat(oFile,S_IRWXU);
    if(oFD >= 0){
      close(1);
      dup(oFD);
      close(oFD);
    }
    else{//unable to create output file
      fprintf(stderr,"Unable to create file %s, Error is %d and message is %s \n",oFile,errno , strerror(errno));
      exit(3);
    }
  }
  if(catch){
     //implementation of catch registering seg fault handler                                 
    signal(SIGSEGV,segFaultCatch);
   }
  if(segfault){
    //implementation of segfault storing a value to a null ptr
    char *ptr = NULL;
    *ptr = '$';
  }
  
   //Copying stdin to stdout
   
   ssize_t val=0,rite=0;
   char buf[1];
   for(val = read(0,buf,1); val >0; val = read(0,buf,1)){
     rite = write(1,buf,1);
     if(rite != 1){//error handling, not returning the # of bytes
       fprintf(stderr,"Can't write. Error is %d and message is %s \n",errno , strerror(errno));
       exit(3); //cant write, so we shouldnt be able to open output file.
     }
   }
   exit(0);
}

