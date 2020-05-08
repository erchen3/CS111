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
mraa_gpio_context button;
char buffer[10];
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

  strftime(buffer,10,"%H:%M:%S",times);

}
/*

Function that prints the temperature with a timestamp
 and if will log it if specified
*/
void printTemp(){
  getTime();
  float temp =getTemp();

  fprintf(stdout,"%s",buffer);
  printf(" ");
  fprintf(stdout,"%.1f\n",temp);
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
  getTime();

  fprintf(stdout,"%s", buffer);
  printf(" ");
  printf("SHUTDOWN\n");
  if(logFlag){
    dprintf(logFd,"%s", buffer);
    dprintf(logFd," ");
    dprintf(logFd,"SHUTDOWN\n");
  }

  mraa_aio_close(tempSensor);
  mraa_gpio_close(button);
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
      fprintf(stdout,"Didn't specify a log file");
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

int main(int argc, char** argv){

  int opt= 0;

  static struct option long_options[] = {
    {"period",required_argument,0,'p'},
    {"scale",required_argument,0,'s'},
    {"log",required_argument,0,'l'},
    {0,0,0,0}
  };
  while( (opt = getopt_long(argc,argv,"p:s:l:", long_options, NULL)) !=-1){
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
    default:
      fprintf(stderr,"Usage: lab4b command line options invalid");
      exit(1);
    }
  }
  //initialize it
  tempSensor=  mraa_aio_init(1);
  button = mraa_gpio_init(60);
  mraa_gpio_dir(button, MRAA_GPIO_IN);

  int commandIndex=0;
  time_t start , end;
  struct pollfd polls[1];
  polls[0].fd= 0; //STDIN FD
  polls[0].events = POLLIN |POLLHUP |POLLERR;


  while(1)
    {
      if(!stopFlag)
	printTemp();

      time(&start);
      time(&end);


      while(difftime(end,start) <period){ // each sample period

	if(mraa_gpio_read(button)==1){ //pushed button
	  printShutdown();
	}
	if(poll(polls,1,1000) <0){ 
	  fprintf(stderr,"Error polling\n");
	  exit(1);
	}
	if(polls[0].revents & POLLIN){ // Pending input
	  char command[140];
	  char input[140];
	  memset(command,0,140*sizeof(char));
	  memset(input,0,140*sizeof(char));

	  int numBytes = read(0,input,140*sizeof(char));
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
  mraa_gpio_close(button);
  close(logFd);
  exit(0);
}
