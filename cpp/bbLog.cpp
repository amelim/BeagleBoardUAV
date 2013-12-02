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

#include "serial/ASIOSerialPort.h"

int main(int argc, char *argv[]){

  if(argc < 2){
    std::cout << "Usage: bblog /file/to/log" << std::endl;
    return -1;
  }
    
  string nextLine;
  std::cout << "Beginning logging: " << std::endl << std::endl;

  ofstream logFile;
  logFile.open(argv[1]);
  std::cout << "Opening: " << argv[1] << std::endl;

  ASIOSerialPort imu("/dev/ttyO2", 57600);

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
      logFile << nextLine << std::endl;
    }
  }
  return 0;
}

