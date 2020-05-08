/*
NAME:Eric Chen
EMAIL: erchen3pro@gmail.com
ID: 305099495


*/

#define _POSIX_C_SOURCE 200809L
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <mraa.h>
#include "mraa/aio.h"
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/ssl.h>


/*
 1. Use AIO funcitions of MRAA library to get readings from temp sensor
 2.sample temperature sensor at configurable rate using --period option
 that can specify the sampling interval
 3. implement scale option to do celsius or fahrenheit
 4. Make a report for each sample, write to stdout
 5. Append report to a logfile if enabled with --log option


 TO-DO:
 Implement the user inputs from STDIN function
 Check if anything else is needed
*/

const float B = 4275;               // B value of the thermistor
const float R0 = 100000;            // R0 = 100k

long temperature=0;
int period=1;
char scale='F'; //default fahrenheit
int logFd= -10,logFlag=0,stopFlag=0;
mraa_aio_context tempSensor;
//mraa_gpio_context button;
char buffer[50];
int id,port, socketFD;
char *host="";
SSL_CTX* ctx;
SSL *ssl;

/*
Helper Fcn
Function to get the temperature 
Citation : http://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
*/
float getTemp(){
  int inputVoltage = mraa_aio_read(tempSensor);
  float R =   (1023.0/inputVoltage) - 1.0;
  R= R0 *R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet, celsius

  if(scale == 'F'){//fahrenheit

    return  (temperature * (9.0/5.0) +32);
  }
  else{//celsius
    return temperature;
  }

}

/*
Helper fcn
function that gets the local time going into buffer
*/
void getTime(){
  time_t localTime;
  struct tm *times;

  localTime = time(NULL); //curr callendar time
  times = localtime(&localTime);

  strftime(buffer,10,"%H:%M:%S ",times);

}
/*

Function that prints the temperature with a timestamp
 and if will log it if specified
*/
void printTemp(){
  getTime();
  float temp =getTemp();
  char boof[50];
  sprintf(boof,"%.1f\n",temp);
  strcat(buffer, boof);

 SSL_write(ssl,buffer,strlen(buffer));
  if(logFlag){
    dprintf(logFd,"%s",buffer);
    dprintf(logFd," ");
    dprintf(logFd,"%.1f\n",temp);

  }
}

/*
When button gets pressed we log and write shutdown

*/
void printShutdown(){
  memset((char*)&buffer,0,sizeof(buffer));
  getTime();

  char boofar[50];
  sprintf(boofar,"SHUTDOWN\n");
  strcat(buffer,boofar);
 SSL_write(ssl, buffer, strlen(buffer));
  if(logFlag){
    dprintf(logFd,"%s", buffer);
    dprintf(logFd," ");
    dprintf(logFd,"SHUTDOWN\n");
  }

  mraa_aio_close(tempSensor);
  //mraa_gpio_close(button);
  exit(0);
}

/*
Function to process all the stdin commands by user
*/
void processResult(char* input){
  if(!strcmp(input,"SCALE=F")){
    if(logFlag){
      dprintf(logFd,"SCALE=F\n");
    }
    scale='F';
  }
  else if(!strcmp(input,"SCALE=C")){
    if(logFlag){
      dprintf(logFd,"SCALE=C\n");
    }
    scale='C';
  }
  else if(!strncmp(input,"PERIOD=", 7)){ // log
    for(int i=0; isdigit(input[i+7]); i++){

      if(logFlag){
	dprintf(logFd, "%s\n", input);
      }

      if(!isdigit(input[i+7])){
	fprintf(stderr,"Not a valid input ");
	exit(1);
      }
      period = atoi(&input[7]);
    }
  }
  else if(!strcmp(input,"STOP")){

    if(logFlag){
      dprintf(logFd, "STOP\n");
    }
    stopFlag=1;


  }
  else if(!strcmp(input,"START")){

    if(logFlag){
      dprintf(logFd, "START\n");
    }
    stopFlag =0;

  }
  else if(!strncmp(input,"LOG",3)){
    if(logFlag){
      dprintf(logFd,"%s\n", input);
    }
    else{
      fprintf(stderr,"Didn't specify a log file");
    }
  }
  else if(!strcmp(input,"OFF")){
    if(logFlag){
      dprintf(logFd,"OFF\n");
    }
    printShutdown();
  }
  else{
    fprintf(stderr,"Invalid option passed.");
  }
  return;
}

/*
  Function whose purpose is to initialize the SSL set-up.
*/
void initializeSSL(){

  if(SSL_library_init() < 0){
    fprintf(stderr,"Error: Issue with SSL_library_init");    
    exit(2);
  }
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  ctx= SSL_CTX_new(TLSv1_client_method());
  if(ctx ==NULL){
    fprintf(stderr,"Error: Issue with the SSL_CTX_new");
    exit(2);
  }

  ssl = SSL_new(ctx);
  if(ssl ==NULL){
    fprintf(stderr,"Error: Issue with the SSL_new");
    exit(2);
  }

  if(SSL_set_fd(ssl,socketFD) != 1){
    fprintf(stderr,"Error: Issue with the SSL_set_fd");
    exit(2);
  }
  if (SSL_connect(ssl) <1){
    fprintf(stderr,"Error: Issue with the SSL_connect");
    exit(2);
  }


  return;
}
int main(int argc, char** argv){

  int opt= 0;

  static struct option long_options[] = {
    {"period",required_argument,0,'p'},
    {"scale",required_argument,0,'s'},
    {"log",required_argument,0,'l'},
    {"id",required_argument,0,'i'},
    {"host",required_argument,0,'h'},
    {0,0,0,0}
  };
  while( (opt = getopt_long(argc,argv,"p:s:l:i:h:", long_options, NULL)) !=-1){
    switch(opt){
    case'p':  //Period option to specify sampling interval in seconds.
      period = atoi(optarg);
      break;
    case's'://scale option
      if( (*optarg != 'C') && (*optarg != 'F') && optarg !=NULL){
	fprintf(stderr,"Did not pass in the right argument");
	exit(1);
      }
      else{
	scale =*optarg;
      }
      break;
    case'l':// log option
      logFd= creat(optarg,S_IRWXU);// some FD
      logFlag=1;
      break;
    case 'i':
      id=atoi(optarg); //9 digit string
      break;
    case 'h':
      host=optarg; // name or address
      break;
    default:
      fprintf(stderr,"Usage: lab4b command line options invalid");
      exit(1);
    }
  }
  //initialize it
  tempSensor=  mraa_aio_init(1);
  port =atoi(argv[optind]);

  //make socket
  socketFD = socket(AF_INET,SOCK_STREAM,0);
  if(socketFD == -1){
    fprintf(stderr,"Error with socket call error number=%d message=%s",errno, strerror(errno));
    exit(2);
  }
  //host
  struct hostent *server;
  struct sockaddr_in addy;
  server = gethostbyname(host);

  memset((char*)&addy,0,sizeof(addy));
  addy.sin_family =AF_INET;//always set
  memcpy((char*)&addy.sin_addr.s_addr,(char*)server->h_addr_list[0],server->h_length);
  addy.sin_port=htons(port);
  //conect to server
  if(connect(socketFD,(struct sockaddr*)&addy,sizeof(addy)) < 0){
    fprintf(stderr,"Error connecting error number=%d message=%s",errno,strerror(errno));
    exit(2);
  }

  //set up SSL
  initializeSSL();

  //send the id
  char boofer[100];
  sprintf(boofer,"ID=%d\n",id);
  if(SSL_write(ssl,boofer,strlen(boofer)) <=0){
    fprintf(stderr,"Error with ssl_write");
    exit(2);
  }
   dprintf(logFd,"ID=%d\n",id);
  //dprintf(socketFD,"ID=%d\n",id);  //  to SSLWRITE!!!!!!!
 

  //button = mraa_gpio_init(60);
  //mraa_gpio_dir(button, MRAA_GPIO_IN);

  int commandIndex=0;
  time_t start , end;
  struct pollfd polls[1];
  polls[0].fd= socketFD; //server readin
  polls[0].events = POLLIN |POLLHUP |POLLERR;


  while(1)
    {
      if(!stopFlag)
	printTemp();

      time(&start);
      time(&end);


      while(difftime(end,start) <period){ // each sample period

	//if(mraa_gpio_read(button)==1){ //pushed button
	//  printShutdown();
	//}
	if(poll(polls,1,1000) <0){
	  fprintf(stderr,"Error polling\n");
	  exit(1);
	}
	if(polls[0].revents & POLLIN){ // Pending input
	  char command[140];
	  char input[140];
	  memset(command,0,140*sizeof(char));
	  memset(input,0,140*sizeof(char));

	  int numBytes = SSL_read(ssl,input,140*sizeof(char)); // to ssl_READ !!!!
	  /*
	            The algorithm, go through each byte and if its not a newline
		        append the character to the command we're constructing.
			        Once we found the newline, that signals the end of the command being constructed.
				    We then reset that char array and reset the index to be used later.
	  */
	  for(int i =0; i<numBytes;i++){
	    if(input[i] == '\n'){
	      processResult(command);
	      memset(command,0,140*sizeof(char));
	      commandIndex=0;
	    }
	    else{
	      command[commandIndex]= input[i];
	      commandIndex++;
	    }
	  }
	}
	time(&end);
      }
    }

  mraa_aio_close(tempSensor);
  //mraa_gpio_close(button);
  close(logFd);
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(socketFD);
 
    exit(0);
}
