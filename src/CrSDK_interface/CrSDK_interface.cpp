#include "CrSDK_interface.h"

CrSDKInterface::CrSDKInterface(){

    // Optionally initialize the vector elements here
    cameraModes.resize(4); // Allocate space for 4 elements
}


bool CrSDKInterface::initializeSDK(){
    return false;
}

bool CrSDKInterface::enumerateCameraDevices(){
    return false;
}

bool CrSDKInterface::connectToCameras(){
    return false;
}

bool CrSDKInterface::switchToMMode(int cameraNumber){
    return false;
}

bool CrSDKInterface::switchToPMode(int cameraNumber){
    return false;
}

bool CrSDKInterface::changeBrightness(int cameraNumber){
    return false;
}

bool CrSDKInterface::changeAFAreaPosition(int cameraNumber, int x, int y){
    return false;
}

bool CrSDKInterface::getCameraMode(int cameraNumber){
    return false;
}

cli::text CrSDKInterface::getCameraModeStr(int cameraNumber) const
{
  if (cameraNumber >= 0 && cameraNumber < cameraModes.size()) {
    return cameraModes[cameraNumber];
  } else {
    // Handle invalid camera number (e.g., return default text or throw an exception)
    return cli::text("Invalid camera number");
  }
}

bool CrSDKInterface::downloadCameraSetting(int cameraNumber){
    return false;
}

bool CrSDKInterface::uploadCameraSetting(int cameraNumber){
    return false;
}