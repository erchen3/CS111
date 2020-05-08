/*
NAME: Eric Chen
E-MAIL: erchen3pro@gmail.com
ID:
*/
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <poll.h>
#include <sys/stat.h>
 #include <unistd.h>
#include <zlib.h>


int socketFD,compressFlag=0;
struct termios initial;

z_stream clientToServer;
z_stream serverToClient;

void release(void){
  inflateEnd(&clientToServer);
  deflateEnd(&serverToClient);

}


//Function to use to deflate/compress
//
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

//function used to decompress/inflate stuff
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
  
  return(2048-serverToClient.avail_out);//amount of bytes

}

//This function is used for resetting terminal mode at the end                                      
// and now deflate if so
// the first part of specs                                                                          
void restore(void){
  tcsetattr(0,TCSANOW,&initial);
  close(socketFD);  
}

void setUpCompress(void){
  atexit(release);
  serverToClient.zalloc =Z_NULL;
  serverToClient.zfree =Z_NULL;
  serverToClient.opaque = Z_NULL;
  if(inflateInit(&serverToClient)!=Z_OK){
      fprintf(stderr,"Issue With inflate initialization");
      exit(1);
  }
  clientToServer.zalloc = Z_NULL;
  clientToServer.zfree =Z_NULL;
  clientToServer.opaque = Z_NULL;
  if(deflateInit(&clientToServer,Z_DEFAULT_COMPRESSION) != Z_OK){
    fprintf(stderr,"Issue With deflate initialization");
    exit(1);
  }
  
}

//Set into non canonical input mode without echo                                           
void setInputMode(void){
  struct termios temp;
  //                                                                                                
  tcgetattr(0,&initial); // "normal" mode settings in the beggining in initial                    
  tcgetattr(0,&temp);
  temp.c_iflag = ISTRIP;        // only lower 7 bits
  temp.c_oflag = 0;       // no processing
  temp.c_lflag = 0;         // no processin                                                         
  tcsetattr(0,TCSANOW,&temp);//set up the noncanonical no echo mode                                 
  atexit(restore);   //after program termination we reset to "normal" settings using initial        
}

int main(int argc, char** argv){

  static struct option long_options[] = {
					 {"port",required_argument,0,'p'},
					 {"log",required_argument,0,'l'},
					 {"compress",no_argument,0,'c'},
					 {0,0,0,0}
  };
  
  int opt=0;
  int port,log=0;
  
  struct sockaddr_in addy;
  struct hostent *server;
  int fdFile;
  
  while((opt= getopt_long(argc,argv,"p:", long_options, NULL)) != -1){
    switch(opt){
    case 'p':
      port =atoi(optarg);//port number make sure to be greater than 1024
      break;
    case 'l':
      log=1;
      fdFile=creat(optarg,S_IRWXU);//create file and give me FD
      break;
    case 'c':     
      compressFlag=1;
      setUpCompress();
      break;
    default:
      fprintf(stderr,"Invalid argument");
      exit(1);
    }
  }
  //make it                                                                                           
  socketFD = socket(AF_INET,SOCK_STREAM,0);
  if(socketFD == -1){
    fprintf(stderr,"Error with socket call error number=%d message=%s",errno, strerror(errno));
    exit(1);
  }

  server = gethostbyname("localhost");
  
  //fill in address stuff
  memset((char*)&addy,0,sizeof(addy));
  addy.sin_family =AF_INET;//always set
  addy.sin_port=htons(port); //comand line pass this in bro
  memcpy((char*)&addy.sin_addr.s_addr,(char*)server->h_addr,server->h_length);
  
  
  //connect it
  if(connect(socketFD,(struct sockaddr*)&addy,sizeof(addy)) == -1){
    fprintf(stderr,"Error connecting error number=%d message=%s",errno,strerror(errno));
    exit(1);
  }
  
  setInputMode();//noncanonical no ech
  
  //pollin to send & Receive
    struct pollfd polls[2];
    polls[0].fd=0;
    polls[1].fd=socketFD;
    polls[0].events =polls[1].events = POLLIN | POLLHUP | POLLERR;
    polls[0].revents = polls[1].revents = 0;
    
    char boof[2048];
    int readAm;
    int j;
    
    /*
      TO DO:
      
      For Client when reading from keyboard I need to compress(deflate) to socket
      When reading from socket I need to inflate or decompress to out
      
    */
    while(1){
      
      poll(polls,2,0);
      
      if(polls[0].revents & POLLIN){ //read from keyboard written to socket into server, raw
	
	readAm = read(0,boof,2048);
	
	for(j=0; j < readAm; j++){///wrote out
          if(boof[j] =='\r' || boof[j] == '\n'){ //newline or carriage return
	    write(1,"\r\n", 2); //when we receive one of the cases we echo it from P1A
	    
	  }
	  else{ //basic case
	    write(1,&boof[j],1);
	    
	  }
	}
	
	if(compressFlag){//we're sending it compressed version to  socket, update the bytes
	  readAm= compressMe(boof,readAm);
	}

	//write it all to socket to server
	write(socketFD,boof,readAm);
	
	//log option                                                                                                                        
	  if(log){
	    dprintf(fdFile,"SENT %d bytes: ",readAm);
	    write(fdFile,boof,readAm);
	    write(fdFile,"\n",1);
          }	  	  
	  	
	
	  
	  
      }//end poll0
      
      //reset the bois
      memset(boof,0,2048*sizeof(char));
      readAm=0;
      
      
      if(polls[1].revents & POLLIN){ //read from socket to output (Process output )
	
 	readAm = read(socketFD,boof,2048);
	if(readAm ==0){//if theres no more bytes to read EOF was received and get out
	  break;
        }
	else{
	  if(log){ /*now we log it, if theres compression the received should be compressed and if not, its regular stuff           */
	    dprintf(fdFile,"RECEIVED %d bytes: ",readAm);
	    write(fdFile,boof,readAm);
	    write(fdFile,"\n",1);
            }

	  if(compressFlag){ //inflating case ,  reading from server socket to term
	    readAm = decompressMe(boof, readAm);
	  }//we aint inflating
	  
	  for(j=0; j <readAm; j++){
	    if((boof[j]=='\r') || (boof[j] =='\n')){
	      write(1,"\r\n",2);
	    }
	    else{
	      write(1,&boof[j],1);
	    }
	    
	  }
	} 
	
	if(polls[1].revents & (POLLHUP |POLLERR)){
	  break; //nothing left
	}
      }//end poll
    }  
    exit(0);
}

