/*
NAME: Eric Chen
EMAIL: erchen3pro@gmail.com
ID: 
 */
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include<string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<stdlib.h>
#include <stdio.h>


struct termios initial;
pid_t status;//pid essentially
int shell;

//Function that will be provoked for the signal for SIGPIPE use
static void handleSig(int sig){
  if(sig == SIGPIPE){
  fprintf(stderr,"There's an issue error num %d and message %s",errno, strerror(errno));
  exit(1);
  }

}
//This function is used for resetting terminal mode at the end
// the first part of specs
void restore(void){
   tcsetattr(0,TCSANOW,&initial);
}

//Set up FD 0  into non canonical input mode without echo
void setInputMode(void){
  struct termios temp;
  //
  tcgetattr(0,&initial); // "normal" mode settings in the beggining in initial 
  
  tcgetattr(0,&temp); 
  temp.c_iflag = ISTRIP;	// only lower 7 bits 
  temp.c_oflag = 0;	  // no processing
  temp.c_lflag = 0;	    // no processin

  tcsetattr(0,TCSANOW,&temp);//set up the noncanonical no echo mode
  atexit(restore);   //after program termination we reset to "normal" settings using initial 
}  


int main(int argc, char** argv){

  static struct option long_options[] = { //get opt structure of options
     {"shell", no_argument,0,'s'},
     {0,0,0,0}
  };
  int opt=0;
 
  while((opt = getopt_long(argc,argv,"s", long_options, NULL)) != -1){
    switch(opt){
    case 's':
      shell=1;
      break;
    default:
      printf("Only one option exists. --shell option sir.");
      exit(1);
    }
  }
  
  setInputMode();//non-canonical input mode no echo
  
  signal(SIGPIPE,handleSig);
	 
  int termToShell[2]; //parent                                                                               
  int shellToTerm[2];// child   
  
  if(shell){//selected shell option for me
    //handle shell functionalities

    if(pipe(termToShell) == -1){ //pipe sys call error check
      fprintf(stderr,"Couldnt make the pipe. error number %d and error message %s", errno, strerror(errno));
      exit(1);
    }
    if(pipe(shellToTerm)== -1){ //makes the array elements file descriptors
      fprintf(stderr,"Couldnt make the pipe. error number %d and error message %s", errno, strerror(errno));
      exit(1);
    }

    status = fork(); //fork after pipe to share fd between parent and child
    
    if(status <0){//Issue forking, sys call error check
      fprintf(stderr,"fork failed error number %d and error is %s", errno, strerror(errno));
      exit(1);
    }
    else if(status==0){//we're in the child process, redirect to the shell
      close(termToShell[1]);//terminal to shell  write closed
      close(shellToTerm[0]);//shell to terminal read closed
      
      close(0);
      dup(termToShell[0]); //read terminal to shell, 0 refers to it 
      close(termToShell[0]);
      
      close(1);
      dup(shellToTerm[1]); //write shell to terminal, 1 refers
          
      close(2);
      dup(shellToTerm[1]); //write stderr shell to terminal, 2 refers
      close(shellToTerm[1]);

      char* file = "/bin/bash";//exec portion to replace current process image with new
      char* argv[2];
      argv[0] = file;
      argv[1] = NULL;
      if(execvp(file,argv) == -1){
	fprintf(stderr,"Execvp didnt work as expected %d and error is %s",errno, strerror(errno));
	exit(1);
      }
    }
    else{//parent process, we might be reading stuff from shell but also keyboard
    
      close(termToShell[0]);//  unused
      close(shellToTerm[1]); // and the shell to Terminal

      struct pollfd polls[2];

      //initialize it
      polls[0].fd= 0;//describes stdin
      polls[1].fd =shellToTerm[0]; //describes output form shell
      polls[0].events = POLLIN | POLLHUP |POLLERR;
      polls[1].events = POLLIN | POLLHUP |POLLERR;
      polls[0].revents =0;
      polls[1].revents =0;

      while(1){//main loop

	poll(polls,2,0);
 
	if(polls[0].revents & POLLIN){ //if pending input, reading from keyboard to shell/out 
	  char boof[256];
	  int readAm, j;

          readAm = read(0,boof,256);

	  for(j=0; j<readAm; j++){
	    if(boof[j] =='\4'){ //^D case, we should close the pipe going to shell
	      if(write(1,"^D",2) == -1){ //if I recognize i'll echo
		fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	      }
	      close(termToShell[1]);
	    }
	    else if(boof[j] =='\3'){ // ^c Case
	      if(write(1,"^C",2)==-1){//if i recognize I'll echo
      		fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	      }
	      kill(status,SIGINT);
            }
	    else if(boof[j] =='\r' ||boof[j]=='\n'){//getting special char, write to shell specially
	      if(write(1,"\r\n", 2) ==-1){//and output to stdout the standard format
	      	fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	      }
	      if(write(termToShell[1],"\n",1)== -1){//write to shell as <lf>
       		fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	      }
	    }
	    else{
	      if(write(1,&boof[j],1)==-1){//basic case
		fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	      }
	      if(write(termToShell[1],&boof[j],1) == -1){
		fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	      }
	    }
	  }
	}//end poll[0]
	/*
       conceptually
	  Keyboard -> shell/ out
	  if terminal thats it
	  if shell -> write from shell to output

	  But concurrently
	 */
	  if(polls[1].revents & POLLIN){ //the input from shells exist, write it out
	    int readAm,i;
	    char boof[256];

            readAm = read(shellToTerm[0],boof,256);//reading from shell to terminal

	    for(i=0;i<readAm; i++){
	      if(boof[i] =='\n'){ //a new line to shell  warrants this case to out
		if(write(1,"\r\n",2) == -1){
		   fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
		}
	      }
	      else{
		if( write(1,&boof[i],1)== -1){
		  fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
		}
	      }
	    }
	  }//end poll[1]
	  if(polls[1].revents & (POLLHUP | POLLERR)){//nothing left to write, closed /processed final output
	    int res;
	    int waitPID = waitpid(status,&res,0);//wait for process to change
	    if(waitPID ==-1){//system call waitPID error check
	      fprintf(stderr,"WaitPID failed. Error %d and message %s", errno, strerror(errno));
	      exit(1);
	    }
	    fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d",WTERMSIG(res), WEXITSTATUS(res));	   
	  }
      }//endwhile

    }//end else 

   }//end if 
  else{//no shell option 
    int readAmount=0,i;
    char buf[256];
   

    while(1){
      
     	readAmount = read(0,buf,256); //can read from terminal to shell or just normal stdin to stdout
         for(i=0; i< readAmount;i++){//there exists something 
	if(buf[i] == '\4'){ //^D case
	  exit(0);
	}
	if( buf[i] =='\r' || buf[i] == '\n'){ //<cr> or <lf>
	  if(write(1,"\r\n",2) == -1){//if we have the line feed/carriage return we'll write both
	    fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	  }
	}
	else{ /// default case where we write to display
	  if(write(1,&buf[i],1) == -1){
	    fprintf(stderr,"Couldn't write error=%d message=%s",errno,strerror(errno));
	  }
	} 
      }//end for
    }
  }
  exit(0);
}
