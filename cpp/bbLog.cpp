/*
 * bbLog.cpp
 *
 *  Created on: Oct 18, 2013
 *      Author: Sterling Peet <sterling.peet@gatech.edu>
 *      Author: Andrew Melim <Andrew.Melim@gatech.edu>
 */

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <string>
#include <time.h>
#include <stdio.h>

// Serial reading for GPS/IMU
#include "serial/ASIOSerialPort.h"

// FlyCapture for Point Grey camera
#include "FlyCapture2.h"

using namespace FlyCapture2;

// Forward declare diff function
timespec diff(timespec start, timespec end);

/* ************************************************************************* */
void PrintError(Error error){
  error.PrintErrorTrace();
}

/* ************************************************************************* */
// print camera information
void PrintCameraInfo( CameraInfo* pCamInfo ){
  printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

/* ************************************************************************* */
bool CheckSoftwareTriggerPresence(Camera* pCam){
  const unsigned int k_triggerInq = 0x530;
  Error error;
  unsigned int regVal = 0;

  error = pCam->ReadRegister(k_triggerInq, &regVal);
  if(error != PGRERROR_OK){
    PrintError(error);
    return false;
  }
  if((regVal & 0x10000) != 0x10000){
    return false;
  }
  return true;
}

/* ************************************************************************* */
bool FireSoftwareTrigger(Camera* pCam){
  const unsigned int k_softwareTrigger = 0x62C;
  const unsigned int k_fireVal = 0x80000000;
  Error error;

  error = pCam->WriteRegister(k_softwareTrigger, k_fireVal);
  if(error != PGRERROR_OK){
    PrintError(error);
    return false;
  }
  return true;
}

/* ************************************************************************* */
bool PollForTriggerReady(Camera* pCam){
  const unsigned int k_softwareTrigger = 0x62C;
  Error error;
  unsigned int regVal = 0;
  do{
    error = pCam->ReadRegister(k_softwareTrigger, &regVal);
    if(error != PGRERROR_OK){
      PrintError(error);
      return false;
    }
  }while((regVal >> 31) != 0);

  return true;
}

/* ************************************************************************* */
int main(int argc, char *argv[]){

  if(argc < 2){
    std::cout << "Usage: bblog /file/to/log" << std::endl;
    return -1;
  }
    
  string nextLine, gpsLine;
  time_t timer;
  clock_t clockt;
  // time_serial : High rate sensors
  // time_c* : Camera trigger timer
  timespec time_serial, time_c1, time_c2, difft;
  std::cout << "Beginning logging: " << std::endl << std::endl;

  ofstream logFile;
  logFile.open(argv[1]);
  std::cout << "Opening: " << argv[1] << std::endl;

  ASIOSerialPort imu("/dev/ttyO2", 57600);
  ASIOSerialPort gps("/dev/ttyO1", 38400);

  //PGFlyCap Objects
  Error error;
  Camera cam;
  BusManager busMgr;
  PGRGuid guid;
  unsigned int numCameras;

  // Find Camera
  error = busMgr.GetNumOfCameras(&numCameras);
  if(error != PGRERROR_OK){
    PrintError(error);
    return -1;
  }
  std::cout << "Found: " << numCameras << " cameras\n";

  error = busMgr.GetCameraFromIndex(0, &guid);
  if(error != PGRERROR_OK){
    cout << "Get camera\n";
    PrintError(error);
    return -1;
  }

  // Connect to Camera
  error = cam.Connect(&guid);
  if(error != PGRERROR_OK){
    cout << "Connect\n";
    PrintError(error);
    return -1;
  }

  CameraInfo camInfo;
  error = cam.GetCameraInfo(&camInfo);
  if(error != PGRERROR_OK){
    cout << "Camera info\n";
    PrintError(error);
    return -1;
  }

  PrintCameraInfo(&camInfo);
  // Establish software trigger
  /*TriggerMode triggerMode;
  error = cam.GetTriggerMode(&triggerMode);
  if(error != PGRERROR_OK){
    cout << "Get Trigger\n";
    PrintError(error);
    return -1;
  }

  triggerMode.onOff = true;
  triggerMode.mode = 0;
  triggerMode.parameter = 0;
  triggerMode.source = 7;// Set to software trigger

  error = cam.SetTriggerMode(&triggerMode);
  bool retVal = PollForTriggerReady(&cam);
  if(!retVal){
    printf("\nError polling for trigger ready!\n");
    return -1;
  }*/


  // Start Camera capture at automatic framerate
  error = cam.StartCapture();
  if(error != PGRERROR_OK){
    cout << "Capture\n";
    PrintError(error);
    return -1;
  }

  Image rawImage;
  // Get init camera clock
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_c1);
  long int fr = 1; // seconds
  while(1)
  {
    try{
      nextLine = imu.readln();
    }
    catch(...){
      continue;
    }
    if (nextLine != ""){
      std::cout << nextLine << std::endl;
      //time(&timer); // Get current time; same as timer = time(NULL)
      //clockt = clock();
      //difft = diff(time1, time2);
      //time1 = time2;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_serial);
      logFile << "imu " << time_serial.tv_sec << " " << time_serial.tv_nsec << " " <<  nextLine << std::endl;
    }

    try{
      gpsLine = gps.readln();
    }
    catch(...){
      std::cout << "gps read fail" << std::endl;
    }
    if(gpsLine != ""){
      std::cout << gpsLine << std::endl;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_serial);
      logFile << "gps " << time_serial.tv_sec << " " << time_serial.tv_nsec << " " <<  gpsLine << std::endl;
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_c2);
    difft = diff(time_c1, time_c2);
    if((long int)difft.tv_sec >= fr){
      //retVal = FireSoftwareTrigger(&cam);
      time_c1 = time_c2;
      error = cam.RetrieveBuffer(&rawImage);
      if(error != PGRERROR_OK){
        PrintError(error);
        continue;
      }

      char filename[512];
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_c1);
      sprintf(filename, "Image-%lld-%.9ld.pgm", (long long)time_c2.tv_sec, time_c2.tv_nsec);
      error = rawImage.Save(filename);
      if(error != PGRERROR_OK){
        PrintError(error);
        continue;
      }
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

