/*
 * bbLog.cpp
 *
 *  Created on: Oct 18, 2013
 *      Author: Sterling Peet <sterling.peet@gatech.edu>
 */

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <string>
#include <time.h>

#include "serial/ASIOSerialPort.h"

// Forward declare diff function
timespec diff(timespec start, timespec end);

int main(int argc, char *argv[]){

  if(argc < 2){
    std::cout << "Usage: bblog /file/to/log" << std::endl;
    return -1;
  }
    
  string nextLine;
  time_t timer;
  clock_t clockt;
  timespec time1, time2, difft;
  std::cout << "Beginning logging: " << std::endl << std::endl;

  ofstream logFile;
  logFile.open(argv[1]);
  std::cout << "Opening: " << argv[1] << std::endl;

  ASIOSerialPort imu("/dev/ttyO2", 57600);

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
  while(1)
  {
    try
    {
      nextLine = imu.readln();
    }
    catch(...)
    {
      continue;
    }
    if (nextLine != "")
    {
      std::cout << nextLine << std::endl;
      //time(&timer); // Get current time; same as timer = time(NULL)
      //clockt = clock();
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
      difft = diff(time1, time2);
      time1 = time2;
      logFile << difft.tv_sec << " " << difft.tv_nsec << " " <<  nextLine << std::endl;
    }
  }
  return 0;
}

timespec diff(timespec start, timespec end){
    timespec temp;
      if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
      } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
      }
    return temp;
}

