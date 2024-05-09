#include "CrSDK_interface.h"

CrSDKInterface::CrSDKInterface(){

    // Optionally initialize the vector elements here
    cameraModes.resize(MAX_CAMERAS); // Allocate space for 4 elements
}

// CrSDKInterface::~CrSDKInterface() 
// {
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

bool CrSDKInterface::initializeSDK()
{

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

    // spdlog::info("Remote SDK version: ");
    // cli::tout << major << "." << minor << "." << std::setfill(TEXT('0')) << std::setw(2) << patch << "\n";

    spdlog::info("Initialize Remote SDK...");
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

bool CrSDKInterface::enumerateCameraDevices()
{

    spdlog::info("Enumerate connected camera devices...");

    // Create asynchronous tasks for enum_status and ncams
    auto enumTask = std::async([this]() 
    {
        return SDK::EnumCameraObjects(&camera_list);
    });

    // Wait for both tasks to finish
    auto enumStatus = enumTask.get();

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
    if (ncams < MAX_CAMERAS)
    {
        spdlog::warn("Expected {} cameras, found {}.", MAX_CAMERAS, ncams);
    }

    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;

    CameraDeviceList cameraList; // all

    return true;
}

bool CrSDKInterface::connectToCameras() 
{
    try 
    {
        spdlog::info("Connecting to both cameras...");

        std::vector<std::future<bool>> connectTasks;

        for (CrInt32u i = 0; i < camera_list->GetCount(); ++i) 
        {
            auto *camera_info = camera_list->GetCameraObjectInfo(i);
            spdlog::info("Creating object for camera {}...", i + 1);
            CameraDevicePtr camera = std::make_shared<cli::CameraDevice>(i + 1, camera_info);
            cameraList.push_back(camera);

            if (!camera->is_connected()) 
            {
                auto connectTask = std::async(std::launch::async, [camera]() 
                {
                    return camera->connect(SDK::CrSdkControlMode_Remote, SDK::CrReconnecting_ON);
                });

                // // Wait for both tasks to finish
                bool connectStatus = connectTask.get();
                connectStatus ? spdlog::info("The connection to camera number {} was successful", i) : spdlog::error("The connection to camera number {} failed", i);
                connectTasks.push_back(std::move(connectTask));  // Use move semantics
            } 
            else 
            {
                spdlog::warn("Camera {} is already connected. Please disconnect first.", i + 1);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        bool allConnected = true;

        // Wait for all async tasks to complete
        // for (auto &task : connectTasks) 
        // {
        //     bool result = task.get();  // Blocks until the async task completes
        //     if (!result) 
        //     {
        //         spdlog::error("Failed to connect to a camera.");
        //         allConnected = false;
        //         break;  // Exit early if a connection fails
        //     }
        // }

        for (auto& camera : cameraList) {
            if (camera) {
                std::promise<bool> connectionPromise;
                std::future<bool> connectionFuture = connectionPromise.get_future();

                std::async([camera, &connectionPromise]() {
                    connectionPromise.set_value(camera->is_connected());  // Set the value for the promise
                });

                // Wait for the asynchronous task to complete
                bool connected = connectionFuture.get();  // Ensure we wait for completion
                if (!connected) {
                    spdlog::error("A camera connection check failed.");
                    return false;  // Return immediately if any camera fails to connect
                }
            }
        }

        if (allConnected) 
        {
            spdlog::info("All cameras connected successfully.");
            return true;
        } else {
            spdlog::error("One or more connections failed.");
            return false;
        }

    } catch (const std::exception &e) {
        spdlog::error("Error while trying to connect to the cameras: {}", e.what());
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

bool CrSDKInterface::switchToPMode(int cameraNumber)
{
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

bool CrSDKInterface::changeBrightness(int cameraNumber, int userBrightnessInput)
{
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

bool CrSDKInterface::changeAFAreaPosition(int cameraNumber, int x, int y)
{
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

bool CrSDKInterface::getCamerasMode()
{
    try
    {
        // Get information about all cameras mode
        for(CrInt32u i = 0; i < cameraList.size(); ++i){
            cameraList[i]->get_exposure_program_mode(cameraModes[i]);
            if(!cameraModes[i].empty())
            {
                spdlog::info("camera {} mode: {}", i, cameraModes[i]);
            }
            else
            {
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

bool CrSDKInterface::getCameraMode(int cameraNumber)
{
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

bool CrSDKInterface::downloadCameraSetting(int cameraNumber)
{
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

bool CrSDKInterface::uploadCameraSetting(int cameraNumber)
{
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

bool CrSDKInterface::disconnectToCameras() 
{
    try {
        if (cameraList.empty()) {
            spdlog::warn("Camera list is empty. No cameras to disconnect.");
            return true;
        }

        for (auto &camera : cameraList) {
            if (camera && camera->is_connected()) {
                auto disconnect_status = camera->disconnect();
                if (!disconnect_status) 
                {
                    spdlog::error("Failed to disconnect camera.");
                    return false;
                }
            }
        }

        for (auto &camera : cameraList) 
        {
            auto future = std::async(std::launch::async, [&]() {
                camera->release();  // Release the camera device
            });

            auto status = future.wait_for(std::chrono::seconds(3));  // Timeout for resource release

            if (status == std::future_status::timeout) {
                spdlog::error("Timeout while releasing camera device resources.");
                return false;
            }
        }

        spdlog::info("Successfully disconnected all cameras.");
        return true;

    } catch (const std::exception &e) {
        spdlog::error("Error while disconnecting cameras: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::releaseCameraList()
{
    try
    {
        cameraModes.clear();
        cameraList.clear(); // Clear the list after releasing resources
        spdlog::info("the cleaning of the cameraList object was successfully.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to release the camera list object: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::releaseCameraRemoteSDK() 
{
    try {
        std::future<void> future = std::async(std::launch::async, [&]() {
            SDK::Release(); // Release the Camera Remote SDK resources
        });

        // Wait for the SDK release to complete, with a timeout
        auto status = future.wait_for(std::chrono::seconds(3));  // Adjust the timeout as needed

        if (status == std::future_status::timeout) {
            spdlog::error("Timeout while releasing Camera Remote SDK resources.");
            return false;  // Operation timed out
        }

        spdlog::info("The release of the Camera Remote SDK resources was successful.");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("An error occurred while trying to release the Camera Remote SDK resources: {}", e.what());
        return false;
    }
}

