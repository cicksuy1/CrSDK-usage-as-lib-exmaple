#ifndef CRSDKINTERFACE_H
#define CRSDKINTERFACE_H

#include <iostream>
#include <cstdlib>
#include <experimental/filesystem>
#include <cstdint>
#include <iomanip>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <unistd.h>

#include "SonySDK/app/CRSDK/CameraRemote_SDK.h"
#include "SonySDK/app/CameraDevice.h"
#include "SonySDK/app/Text.h"

#define LIVEVIEW_ENB
#define MSEARCH_ENB
#define NUM_CAMERAS 1

namespace fs = std::experimental::filesystem;
namespace SDK = SCRSDK;

using namespace std;

typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
typedef std::vector<CameraDevicePtr> CameraDeviceList;

/**
 * @brief This class provides an interface to the CrSDK library for camera connection and information retrieval.
 */
class CrSDKInterface
{
public:

    /**
     * @brief Constructs a CrSDKInterface object.
     */
    CrSDKInterface();

    /**
     * @brief camera_list a CrSDKInterface object.
     */
    ~CrSDKInterface(); 

    /**
     * @brief Initializes the Camera Remote SDK.
     *
     * @return True if initialization was successful, false otherwise.
     */
    bool initializeSDK();

    /**
     * @brief Enumerates connected camera devices.
     *
     * @return True if enumeration was successful, false otherwise.
     */
    bool enumerateCameraDevices();

    /**
     * @brief Creating a connection to the cameras after enumerate Camera Devices.
     *
     * @return True if connection to the cameras was successful, false otherwise.
     */
    bool connectToCameras();

    /**
     * @brief Switching the camera mode to P (auto).
     * @param cameraNumber The number of the camera that the user wants to change its mode to P mode.
     * @return True if switching the camera mode was successful, false otherwise.
     */
    bool switchToPMode(int cameraNumber);

    /**
     * @brief Switching the camera mode to M (manual).
     * @param cameraNumber The number of the camera that the user wants to change its mode to P mode.
     * @return True if switching the camera mode was successful, false otherwise.
     */
    bool switchToMMode(int cameraNumber);

    /**
     * @brief Change the brightness of the camera.
     * @param cameraNumber The number of the camera that the user wants to change the brightness of the camera.
     * @param userBrightnessInput The brightness index selected by the user
     * @return True if change the brightness was successful, false otherwise.
     */
    bool changeBrightness(int cameraNumber, int userBrightnessInput);

    /**
     * @brief Change the AF area position of the camera.
     * @param cameraNumber The number of the camera that the user wants to change the AF area Position of the camera.
     * @param x position.
     * @param y position.
     * @return True if change the AF area Position was successful, false otherwise.
     */
    bool changeAFAreaPosition(int cameraNumber, int x, int y);

    /**
     * @brief Get the camera mode (auto or manual).
     * @param cameraNumber The number of the camera that the user wants to get the camera mode.
     * @return True if get the camera mode was successful, false otherwise.
     */
    bool getCameraMode(int cameraNumber);

    /**
     * @brief Retrieves the camera mode string for a given camera number.
     * 
     * @param cameraNumber The camera ID (0-based indexing)
     * @return The camera mode string (if successful)
     */
    cli::text getCameraModeStr(int cameraNumber) const; // Added const to indicate no modification

    /**
     * @brief Download the camera setting file to PC.
     * @param cameraNumber The number of the camera that the user wants to download the camera setting file to PC.
     * @return True if download the camera setting file to PC was successful, false otherwise.
     */
    bool downloadCameraSetting(int cameraNumber);

    /**
     * @brief Upload the camera setting file to Camera.
     * @param cameraNumber The number of the camera that the user wants to upload the camera setting file to Camera.
     * @return True if Upload the camera setting file to Camera was successful, false otherwise.
     */
    bool uploadCameraSetting(int cameraNumber);

// private:
    std::vector<cli::text> cameraModes; // No size argument here
    std::vector<CameraDevicePtr> cameraList;
    SDK::ICrEnumCameraObjectInfo *camera_list = nullptr;

};

#endif // CRSDKINTERFACE_H
