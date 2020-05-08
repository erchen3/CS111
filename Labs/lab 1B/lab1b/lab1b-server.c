/*
NAME: Eric Chen
EMAIL: erchen3pro@gmail.com
ID: 
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include<stdio.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <zlib.h>

int sockfd, newsockfd, compressFlag=0;
int pID;
z_stream clientToServer;
z_stream serverToClient;
int pipeTo[2];//To shell                                                                                      
int pipeFrom[2];//From shell

void release(void){
      inflateEnd(&clientToServer);
    deflateEnd(&serverToClient);

    
}

//Set up for the zlib structures
void setUpCompress(void){
  atexit(release);
  clientToServer.zalloc = Z_NULL;
  clientToServer.zfree =Z_NULL;
  clientToServer.opaque = Z_NULL;
  if(deflateInit(&clientToServer,Z_DEFAULT_COMPRESSION) != Z_OK){
    fprintf(stderr,"Issue With deflate initialization");
    exit(1);
  }
  
  serverToClient.zalloc =Z_NULL;
  serverToClient.zfree =Z_NULL;
  serverToClient.opaque = Z_NULL;
  if(inflateInit(&serverToClient) !=Z_OK){
    fprintf(stderr,"Issue With inflate initialization");
    exit(1);
    
  }
}

//Function to used to deflate                                                                                                                      

int compressMe(char * boof, int bytes){

  char temp[2048];
  memcpy(temp, boof,bytes); //move to temp                                                                                                         

  clientToServer.avail_in=bytes;
  clientToServer.avail_out=2048;
  clientToServer.next_in=(Bytef *) temp;
  clientToServer.next_out = (Bytef *) boof;

  do{
    deflate(&clientToServer,Z_SYNC_FLUSH);
  }while(clientToServer.avail_in >0);

  return (2048-clientToServer.avail_out);

}

//function used to decompress stuff                                                                                                                
int decompressMe(char* boof, int bytes){
  char temp[2048];
  memcpy(temp,boof,bytes);

  serverToClient.avail_in=bytes;
  serverToClient.avail_out=2048;
  serverToClient.next_in=(Bytef *) temp;
  serverToClient.next_out=(Bytef *) boof;

  do{
    inflate(&serverToClient,Z_SYNC_FLUSH);
  }while(serverToClient.avail_in >0);
  return(2048-serverToClient.avail_out);
  
}




void restore(void){
  int res;
        int waitPID = waitpid(pID,&res,0);
        if(waitPID ==-1){
          fprintf(stderr,"WaitPID failed. Error %d and message %s", errno, strerror(errno));
          exit(1);
        }

        fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d",WTERMSIG(res), WEXITSTATUS(res));
        close(newsockfd);
	close(sockfd);
	close(pipeTo[1]);
	close(pipeFrom[0]);
}
static void handleSig(int sig){
  if(sig == SIGPIPE){
    fprintf(stderr,"There's an issue error num %d and message %s",errno, strerror(errno));
    exit(0);
  }

}
int main(int argc, char** argv){
  int port;
 struct sockaddr_in serv_addr, cli_addr;
 int pID,readAm,i;
 char boof[2048];
 int newsockfd;
 socklen_t clilen;
 static struct option long_option[] = {
					{"port",required_argument,0,'p'},
					{"compress",no_argument,0,'c'},
					{0,0,0,0}
					 
  };				       
 int opt=0;

    while((opt= getopt_long(argc,argv,"p:", long_option, NULL)) != -1){
      switch(opt){
    case 'p':
      port =atoi(optarg);//port number make sure to be greater than 1024                            
      break;
    case 'c':
      compressFlag=1;
      setUpCompress();
      break;
    default:
      printf("Invalid argument");
      exit(1);
    }

    }
  
  //make it
  sockfd =socket(AF_INET,SOCK_STREAM,0);
  //initialize it(address stuff)
  serv_addr.sin_family=AF_INET;
  serv_addr.sin_addr.s_addr =INADDR_ANY;
  serv_addr.sin_port =htons(port);

  //bind it, assigning name to a socket
  bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
  //listen, sockfd used to accept incoming connection request
  listen(sockfd,5); 
  //listening for sockfd from the client and will return an active FD not in listening state
  clilen=sizeof(cli_addr);
  newsockfd= accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);

  if(newsockfd <0){
    fprintf(stderr,"Error calling accept, not good FD");
    exit(1);
  }
  signal(SIGPIPE,handleSig);

  //TODO Pipes
  if(pipe(pipeTo) == -1){
    fprintf(stderr,"Couldnt make the pipe. error number %d and error message %s", errno, strerror(errno));
    exit(1);

  }
  if(pipe(pipeFrom) == -1){
    fprintf(stderr,"Couldnt make the pipe. error number %d and error message %s", errno, strerror(errno));
    exit(1);
  }
  //fork em
  pID = fork();
  if(pID <0){ //not it yo
    fprintf(stderr,"Fork failed error number =%d and message=%s",errno,strerror(errno));
    exit(1);
  }
  else if(pID ==0 ){ //exec em
    close(pipeTo[1]);
    close(pipeFrom[0]);

    close(0);
    dup(pipeTo[0]);//read "term" to shell, 0 refers to it
    close(pipeTo[0]);

    close(1);
    dup(pipeFrom[1]); //write shell to "term", 1 refers to it

    close(2); //write stderr shell to term, 2 refers
    dup(pipeFrom[1]);
    close(pipeFrom[1]);

    char* file = "/bin/bash";//exec portion to replace current process image with a new one
    char* argv[2];
    argv[0] = file;
    argv[1] =NULL;
    if(execvp(file,argv) == -1){
      fprintf(stderr,"Execvp didn't work as expected error=%d message=%s", errno, strerror(errno));
      exit(1);
    }
  }
  else{//parent
    close(pipeTo[0]);
    close(pipeFrom[1]);

    struct pollfd polls[2];
    polls[0].fd= newsockfd;//describes stuff from socket                                                      
    polls[1].fd =pipeFrom[0]; //describes output form shell                            
    polls[0].events = POLLIN | POLLHUP |POLLERR;
    polls[1].events = POLLIN | POLLHUP |POLLERR;
    polls[0].revents =0;
    polls[1].revents =0;


    while(1){
      poll(polls,2,0);

      if(polls[0].revents & POLLIN){ 
	readAm= read(newsockfd,boof,2048);//read from socket  unzipped to shell 

	if(compressFlag){ //if I have compression from socket update
	  readAm = decompressMe(boof,readAm);
	}
	

	for(i=0; i<readAm;i++){
	  if(boof[i] =='\4'){ //^D
	    close(pipeTo[1]);
	  }
	  else if(boof[i] == '\3'){//^C
	    kill(pID,SIGINT);
	  }
	  else if( (boof[i] == '\r') |(boof[i] =='\n') ){
	    write(pipeTo[1],"\n",sizeof(char));
	  }
	  else{//basic case
	    write(pipeTo[1],&boof[i],1);//write to shell
	  }
	}
	

	
      }//end poll 0

      //reset the bois
      memset(boof,0,2048*sizeof(char));
      readAm=0;
      
      if(polls[1].revents & POLLIN){

	readAm= read(pipeFrom[0],boof,2048); //read from shell zipped to server socket

	if(compressFlag){ 
	  readAm= compressMe(boof,readAm);
	}

	//write it all to the socket to server from shell
       	write(newsockfd,boof,readAm);
      }//end poll 1
      if(polls[1].revents & (POLLHUP | POLLERR)){
	break;
      }//end poll thing
      
    }//while
    
  }//end else
  atexit(restore);

  
  
  exit(0);
}
