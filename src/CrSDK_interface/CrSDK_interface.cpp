#include "CrSDK_interface.h"

CrSDKInterface::CrSDKInterface(){

    // Optionally initialize the vector elements here
    cameraModes.resize(4); // Allocate space for 4 elements
}

// CrSDKInterface::~CrSDKInterface() {
//     // Release resources acquired in the constructor or during object lifetime
//     for (CrInt32u i = 0; i < cameraList.size(); ++i)
//     {
//         // Disconnect from the camera and release resources before exiting
//         cameraList[i]->disconnect();
//     }
//     if (camera_list != nullptr) {
//         camera_list->Release(); // Release the camera list object
//     }
//     SDK::Release(); // Release the Camera Remote SDK resources
// }

bool CrSDKInterface::disconnectToCameras(){
    try
    {
        for (CrInt32u i = 0; i < cameraList.size(); ++i)
        {
            // Disconnect from the camera and release resources before exiting
            cameraList[i]->disconnect();
            spdlog::info("Disconnect from the camera {}.", i);
        }
        spdlog::info("The disconnect from the all cameras was successfully.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to disconnect from the cameras: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::releaseCameraList(){
    try
    {
        if (camera_list != nullptr) 
        {
            camera_list->Release(); // Release the camera list object
        }
        else{
            spdlog::warn("The camera list object cannot be freed because it has not been allocated yet");
        }
        spdlog::info("The release the camera list object was successfully.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to release the camera list object: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::releaseCameraRemoteSDK(){
    try
    {
        SDK::Release(); // Release the Camera Remote SDK resources
        spdlog::info("The release of the camera remote SDK resources was successful.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to release of the camera remote SDK resources: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::initializeSDK(){

    // Change global locale to native locale
    std::locale::global(std::locale(""));

    // Make the stream's locale the same as the current global locale
    cli::tin.imbue(std::locale());
    cli::tout.imbue(std::locale());

    CrInt32u version = SDK::GetSDKVersion();
    int major = (version & 0xFF000000) >> 24;
    int minor = (version & 0x00FF0000) >> 16;
    int patch = (version & 0x0000FF00) >> 8;
    // int reserved = (version & 0x000000FF);

    spdlog::info("Remote SDK version: ");
    cli::tout << major << "." << minor << "." << std::setfill(TEXT('0')) << std::setw(2) << patch << "\n";

    spdlog::info("Initialize Remote SDK...\n");
    // spdlog::info("Working directory: {}", fs::current_path());

    auto init_success = SDK::Init();
    if (!init_success)
    {
        spdlog::error("Failed to initialize Remote SDK. Terminating.");
        SDK::Release();
        return false;
    }
    spdlog::info("Remote SDK successfully initialized.");

    return true;
}

bool CrSDKInterface::enumerateCameraDevices(){

    spdlog::info("Enumerate connected camera devices...");
    auto enum_status = SDK::EnumCameraObjects(&camera_list);
    if (CR_FAILED(enum_status) || camera_list == nullptr)
    {
        spdlog::error("No cameras detected.");
        SDK::Release();
        return false;
    }
    auto ncams = camera_list->GetCount();
    spdlog::info("Camera enumeration successful. {} detected.", ncams);

    // Assuming two cameras are connected, create objects for both
    if (ncams < NUM_CAMERAS)
    {
        cli::tout << "Expected " << NUM_CAMERAS << " cameras, found " << ncams << ". Exiting.\n";
        // camera_list->Release();
        // SDK::Release();
        // return false;
    }

    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;

    CameraDeviceList cameraList; // all

    return true;
}

bool CrSDKInterface::connectToCameras(){
    try
    {
        spdlog::info("Connecting to both cameras...");
        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            auto *camera_info = camera_list->GetCameraObjectInfo(i);
            spdlog::info("  - Creating object for camera {}", i + 1);
            CameraDevicePtr camera = CameraDevicePtr(new cli::CameraDevice(i + 1, camera_info));
            cameraList.push_back(camera);
        }

        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            // Connect to the camera in Remote Control Mode
            cameraList[i]->connect(SDK::CrSdkControlMode_Remote, SDK::CrReconnecting_ON);
        }
        sleep(1);

        bool allConnected = true;

        for (auto& cameraDevicePtr : cameraList) 
        {
            if (cameraDevicePtr) 
            {
                const std::atomic<bool>& connected = cameraDevicePtr->getM_connected();
                if (!connected) {
                    allConnected = false;
                    break;
                }
            }
        }

        if (allConnected) 
        {
            spdlog::info("Cameras connected successfully.");
            return true;
        } 
        else 
        {
            spdlog::error("The connection to one or more cameras failed");
            return false;
        }

    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to connect to the cameras: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::switchToMMode(int cameraNumber)
{
    try
    {
        cameraList[cameraNumber]->set_exposure_program_M_mode(cameraModes[cameraNumber]);

        // Set the ISO to automatic
        cameraList[cameraNumber]->set_manual_iso(10);
        return true;
    }
    catch(const std::exception& e)
    {

        spdlog::error("An error occurred while trying to change the camera mode to M mode: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::switchToPMode(int cameraNumber){
 try
    {
        cameraList[cameraNumber]->set_exposure_program_P_mode(cameraModes[cameraNumber]);

        // Set the ISO to automatic
        cameraList[cameraNumber]->set_manual_iso(10);
        return true;
    }
    catch(const std::exception& e)
    {

        spdlog::error("An error occurred while trying to change the camera mode to P mode: {}", e.what());
        return false;
    }}

bool CrSDKInterface::changeBrightness(int cameraNumber, int userBrightnessInput){
    try
    {
        if (cameraModes[cameraNumber] == "m")
        {
            // Checking whether the user's choice of brightness value is correct
            if (userBrightnessInput < 0 || userBrightnessInput > 48)
            {
                spdlog::error("the brightness value entered is incorrect.");
                return false;
            }
            else if (userBrightnessInput >= 0 && userBrightnessInput <= 33)
            {
                // shutter_speed 0 - 33
                cameraList[cameraNumber]->set_manual_shutter_speed(userBrightnessInput);

                // Set the ISO to automatic
                cameraList[cameraNumber]->set_manual_iso(10);
            }
            else
            {
                // ISO 23 - 38
                cameraList[cameraNumber]->set_manual_shutter_speed(33);
                cameraList[cameraNumber]->set_manual_iso(userBrightnessInput);
            }

            return true;
        } 
        else
        {
            spdlog::error("Changing the camera brightness is not possible because the camera is not M(manual) mode");
            return false;
        }   
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to change the brightness of the camera: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::changeAFAreaPosition(int cameraNumber, int x, int y){
    try
    {
        if (cameraModes[cameraNumber] == "p")
        {
            if (x < 0 || x > 639) {
                spdlog::error("Error: The selected X value is out of range");
                return false;
            }

            if (y < 0 || y > 479 ) {
                spdlog::error("Error: The selected Y value is out of range");
                return false;
            }

            int x_y = x << 16 | y;

            // Set exposure program Manual mode
            cameraList[cameraNumber]->set_manual_af_area_position(x_y);
            
            return true;
        } 
        else
        {
            spdlog::error("Changing the camera AF area position is not possible because the camera is not P(auto) mode");
            return false;
        }   
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to change the AF area position of the camera: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::getCamerasMode(){
    try
    {
        // Get information about all cameras mode
        for(CrInt32u i = 0; i < NUM_CAMERAS; ++i){
            cameraList[i]->get_exposure_program_mode(cameraModes[i]);
            if(!cameraModes[i].empty())
            {
                spdlog::info("camera {} mode: {}", i, cameraModes[i]);
            }
            else{
                spdlog::error("Camera mode number {} is currently unavailable", i);
            }

        }
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to get exposure program mode of the camera: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::getCameraMode(int cameraNumber){
    try
    {

        // Get information about a specific camera mode
        cameraList[cameraNumber]->get_exposure_program_mode(cameraModes[cameraNumber]);
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to get exposure program mode of the camera: {}", e.what());
        return false;
    }
}

cli::text CrSDKInterface::getCameraModeStr(int cameraNumber) const
{
  if (cameraNumber >= 0 && cameraNumber < cameraList.size()) {
    return cameraModes[cameraNumber];
  } else {
    // Handle invalid camera number (e.g., return default text or throw an exception)
    return cli::text("Invalid camera number");
  }
}

bool CrSDKInterface::downloadCameraSetting(int cameraNumber){
    try
    {
        cameraList[cameraNumber]->do_download_camera_setting_file();
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred when trying to download the camera settings file to the computer: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::uploadCameraSetting(int cameraNumber){
    try
        {
            cameraList[cameraNumber]->do_upload_camera_setting_file();
            return true;
        }
        catch(const std::exception& e)
        {
            spdlog::error("An error occurred while trying to load the camera settings file from the computer: {}", e.what());
            return false;
        }
}