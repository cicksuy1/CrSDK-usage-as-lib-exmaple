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
#include <future>
#include <chrono>

#include "SonySDK/app/CRSDK/CameraRemote_SDK.h"
#include "SonySDK/app/CameraDevice.h"
#include "SonySDK/app/Text.h"

#define LIVEVIEW_ENB
#define MSEARCH_ENB
#define MAX_CAMERAS 2

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

    // /**
    //  * @brief camera_list a CrSDKInterface object.
    //  */
    // ~CrSDKInterface(); 

    /**
    * @brief Disconnects from all connected cameras and releases associated resources.
     *
     * This function iterates through the `cameraList` member variable and calls the `disconnect`
     * method on each camera object. If successful, it logs a message and returns `true`.
     * Otherwise, it logs an error message with the exception details and returns `false`.
     *
     * @return `true` if disconnection and resource release were successful, `false` otherwise.
     * @throws std::exception If an error occurs during disconnection or resource release.
    */
    bool disconnectToCameras();

    /**
     * @brief Release objects of the cameras in case they exist.
     * @return `true` if the release of the cameras' objects was successful, `false` otherwise.
     * @throws std::exception If an error occurs during release of the cameras' objects.
    */
    bool releaseCameras();

    /**
     * @brief Releases the Camera Remote SDK camera list object.
     *
     * This function checks if the `camera_list` member variable is not a null pointer. If
     * it is not null, it calls the `Release` method on the object to release resources.
     * Otherwise, it logs a warning message indicating that the object cannot be freed because
     * it hasn't been allocated yet. Regardless of the outcome, it logs a message about the
     * attempt and returns `true`. If an exception occurs during the release process, an error
     * message with exception details is logged, and `false` is returned.
     *
     * @return `true` if the camera list object was successfully released or wasn't allocated,
     * `false` if an error occurred during release.
     * @throws std::exception If an error occurs during camera list object release.
    */
    bool releaseCameraList();
    
    /**
     * @brief Releases resources associated with the Camera Remote SDK.
     *
     * This function calls the `SDK::Release` method to release resources associated with
     * the Camera Remote SDK. If successful, it logs a message and returns `true`. Otherwise,
     * it logs an error message with exception details and returns `false`.
     *
     * @return `true` if the release of Camera Remote SDK resources was successful, `false` otherwise.
     * @throws std::exception If an error occurs during Camera Remote SDK resource release.
    */
    bool releaseCameraRemoteSDK();

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
     * @brief Get information about all cameras mode(auto or manual).
     * @return True if get the cameras mode was successful, false otherwise.
     */
    bool getCamerasMode();

    /**
     * @brief Get information about a specific camera mode (auto or manual).
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

    /**
     * @brief Change the camera's focus settings.
     * @param cameraNumber The number of the camera that the user wants to change the camera's focus settings.
     * @return True if change the camera's focus settings was successful, false otherwise.
    */
    bool loadZoomAndFocusPosition(int cameraNumber);

// private:
    std::vector<cli::text> cameraModes; // No size argument here
    std::vector<CameraDevicePtr> cameraList;
    SDK::ICrEnumCameraObjectInfo *camera_list = nullptr;

};

#endif // CRSDKINTERFACE_H
