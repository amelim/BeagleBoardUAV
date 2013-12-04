/**
 * @file pgCam.cpp
 * @author Jing Dong
 * @date Nov 13, 2013
 *
 * This is an example file for use PointGrey USB2.0 Camera in Asynchronous
 * mode (trigger mode).
 */

#include "FlyCapture2.h"

#include <stdio.h>
#include <string>
#include <sstream>

// Software trigger the camera instead of using an external hardware trigger
#define SOFTWARE_TRIGGER_CAMERA

using namespace FlyCapture2;

/* ************************************************************************* */
// print a FlyCapture2 error
void PrintError( Error error )
{
    error.PrintErrorTrace();
}

/* ************************************************************************* */
// print camera information
void PrintCameraInfo( CameraInfo* pCamInfo )
{
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
// Check whether this camera support software trigger
bool CheckSoftwareTriggerPresence( Camera* pCam )
{
	const unsigned int k_triggerInq = 0x530;

	Error error;
	unsigned int regVal = 0;

	error = pCam->ReadRegister( k_triggerInq, &regVal );

	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return false;
	}

	if( ( regVal & 0x10000 ) != 0x10000 )
	{
		return false;
	}

	return true;
}

/* ************************************************************************* */
// Check whether the trigger is ready to fire
bool PollForTriggerReady( Camera* pCam )
{
    const unsigned int k_softwareTrigger = 0x62C;
    Error error;
    unsigned int regVal = 0;

    do 
    {
        error = pCam->ReadRegister( k_softwareTrigger, &regVal );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
			return false;
        }

    } while ( (regVal >> 31) != 0 );

	return true;
}

/* ************************************************************************* */
// Fire the trigger now!
bool FireSoftwareTrigger( Camera* pCam )
{
    const unsigned int k_softwareTrigger = 0x62C;
    const unsigned int k_fireVal = 0x80000000;
    Error error;    

    error = pCam->WriteRegister( k_softwareTrigger, k_fireVal );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return false;
    }

    return true;
}

/* ************************************************************************* */
// Example main function, include initialization, fire the trigger, and clean-up
int main()
{
 
    // ---------------------------------------------------------------------------
    // camera init  
  
    BusManager busMgr;
    unsigned int numCameras;
    PGRGuid guid;
    Error error;
    
    // check camera
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    printf( "Number of cameras detected: %u\n", numCameras );
    
    printf( "Connecting to 1st one ...\n" );
    
    // get guid for 1st cam
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    
    // Connect to the first camera
    Camera cam;
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    
	  // Power on the camera
	  const unsigned int k_cameraPower = 0x610;
	  const unsigned int k_powerVal = 0x80000000;
	  
	  error  = cam.WriteRegister( k_cameraPower, k_powerVal );
	  if (error != PGRERROR_OK)
	  {
		  PrintError( error );
		  return -1;
	  }

	  const unsigned int millisecondsToSleep = 100;
	  unsigned int regVal = 0;
	
	  // Wait for camera to complete power-up
	  do 
	  {
		  error = cam.ReadRegister(k_cameraPower, &regVal);
		  if (error != PGRERROR_OK)
		  {
			  PrintError( error );
			  return -1;
		  }
	  } while ((regVal & k_powerVal) == 0);    

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    
    PrintCameraInfo(&camInfo);
    
    // ---------------------------------------------------------------------------
    // trigger init
    
#ifndef SOFTWARE_TRIGGER_CAMERA
    // Check for external trigger support
    TriggerModeInfo triggerModeInfo;
    
    error = cam.GetTriggerModeInfo( &triggerModeInfo );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    if ( triggerModeInfo.present != true )
    {
        printf( "Camera does not support external trigger! Exiting...\n" );
        return -1;
    }
#endif
    
    // Get current trigger settings
    TriggerMode triggerMode;
    
    error = cam.GetTriggerMode( &triggerMode );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Set camera to trigger mode 0
    triggerMode.onOff = true;
    triggerMode.mode = 0;
    triggerMode.parameter = 0;

#ifdef SOFTWARE_TRIGGER_CAMERA
    // A source of 7 means software trigger
    triggerMode.source = 7;
#else
    // Triggering the camera externally using source 0.
    triggerMode.source = 0;
#endif
    
    error = cam.SetTriggerMode( &triggerMode );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    
    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady( &cam );
	  if( !retVal )
	  {
		  printf("\nError polling for trigger ready!\n");
		  return -1;
	  }
    
    // ---------------------------------------------------------------------------
    // Set camera mode/configuration
    
    // Get the camera configuration
    FC2Config config;
    error = cam.GetConfiguration( &config );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    } 
    
    // Set the camera configuration
    // Set the grab timeout to 2 seconds
    config.grabTimeout = 2000;
    config.numBuffers = 10;
    config.grabMode = BUFFER_FRAMES;
    
    
    // set configuration
    error = cam.SetConfiguration( &config );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    } 
    
    // Get the camera Resolution and Frame Rate
    VideoMode vmode;
    FrameRate fps;
    
    error = cam.GetVideoModeAndFrameRate(&vmode, &fps);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    printf("\n[OLD] VIDEO MODE: %d, FRAME RATE: %d\n", vmode, fps);
    
    // Check whether the camera support the Resolution/Frame Rate we want
    bool modeSupport;
    
    vmode = VIDEOMODE_640x480RGB;   // 1280*960, YUV422
    error = cam.GetVideoModeAndFrameRateInfo (vmode, fps, &modeSupport);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    
    // if support, we set them
    if(modeSupport)
    {
      error = cam.SetVideoModeAndFrameRate(vmode, fps);
      if (error != PGRERROR_OK)
      {
          PrintError( error );
          return -1;
      }
      printf("\n[NEW] VIDEO MODE: %d, FRAME RATE: %d\n", vmode, fps);
    }


    // Camera is ready, start capturing images
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }   
    
#ifdef SOFTWARE_TRIGGER_CAMERA
	  if (!CheckSoftwareTriggerPresence( &cam ))
	  {
		  printf( "SOFT_ASYNC_TRIGGER not implemented on this camera!  Stopping application\n");
		  return -1;
	  }
#else	
	  printf( "Trigger the camera by sending a trigger pulse to GPIO%d.\n", 
        triggerMode.source );
#endif
    
    
  // ---------------------------------------------------------------------------
  // loop
    Image rawImage;
    PixelFormat fmt = rawImage.GetPixelFormat();
     
   
    for(int i = 0; i < 10; i++)
    {
        Image rawImage;
        
#ifdef SOFTWARE_TRIGGER_CAMERA        
        PollForTriggerReady(&cam);
                
        retVal = FireSoftwareTrigger( &cam );
        if ( !retVal )
        {
			      printf("\nError firing software trigger!\n");
			      return -1;        
		    }
#endif        
        
        // capture & save
        error = cam.RetrieveBuffer( &rawImage );
        
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1; 
        }
        
        std::ostringstream ss;
        ss << "/home/root/log/test" << i << ".bmp";
        std::string filename = ss.str(); 
        error = rawImage.Save(filename.c_str());

        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1; 
        }
        
        printf("\nFired and Captured!\n");
    }
    // ---------------------------------------------------------------------------
    // close and clean-up!
  
    // Turn trigger mode off.
    triggerMode.onOff = false;    
    error = cam.SetTriggerMode( &triggerMode );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    printf( "\nFinished grabbing images\n" );

    // Stop capturing images
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }      

    // Turn off trigger mode
    triggerMode.onOff = false;
    error = cam.SetTriggerMode( &triggerMode );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }    

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    
    return 0;
}

