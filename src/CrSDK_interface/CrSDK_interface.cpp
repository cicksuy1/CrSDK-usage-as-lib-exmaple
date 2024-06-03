#include "CrSDK_interface.h"

CrSDKInterface::CrSDKInterface()
{
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
                               { return SDK::EnumCameraObjects(&camera_list); });

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
            spdlog::info("Creating object for camera {}...", i);
            CameraDevicePtr camera = std::make_shared<cli::CameraDevice>(i + 1, camera_info);
            cameraList.push_back(camera);

            if (!camera->is_connected())
            {
                auto connectTask = std::async(std::launch::async, [camera]()
                { 
                    return camera->connect(SDK::CrSdkControlMode_Remote, SDK::CrReconnecting_ON); 
                });

                // Wait for both tasks to finish
                bool connectStatus = connectTask.get();
                connectStatus ? spdlog::info("The connection to camera number {} was successful", i) : spdlog::error("The connection to camera number {} failed", i);
                connectTasks.push_back(std::move(connectTask)); // Use move semantics
            }
            else
            {
                spdlog::warn("Camera {} is already connected. Please disconnect first.", i + 1);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        bool allConnected = true;

        int camera_id = 0;

        for (auto &camera : cameraList)
        {
            if (camera)
            {
                std::promise<bool> connectionPromise;
                std::future<bool> connectionFuture = connectionPromise.get_future();

                std::async([camera, &connectionPromise]()
                {
                    connectionPromise.set_value(camera->is_connected()); // Set the value for the promise
                });

                // Wait for the asynchronous task to complete
                bool connected = connectionFuture.get(); // Ensure we wait for completion
                if (!connected)
                {
                    spdlog::error("A camera connection check failed.");
                    return false; // Return immediately if any camera fails to connect
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Load Zoom and Focus Position Enable Preset.
                bool loadZoomAndFocusPositionSuccess = loadZoomAndFocusPosition(camera_id);
                if (loadZoomAndFocusPositionSuccess)
                {
                    spdlog::info("Load Zoom and Focus Position Enable Preset was successful");
                }
                else
                {
                    spdlog::error("Failed to load Zoom and Focus Position Enable Preset");
                }
                camera_id++;
            }
        }

        if (allConnected)
        {
            spdlog::info("All cameras connected successfully.");
            return true;
        }
        else
        {
            spdlog::error("One or more connections failed.");
            return false;
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error while trying to connect to the cameras: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::switchToMMode(int cameraNumber)
{
    try
    {
        // change of the camera's mode.
        cameraList[cameraNumber]->set_exposure_program_M_mode(cameraModes[cameraNumber]);

        // Set the ISO to automatic
        bool setManualIsoSuccess = cameraList[cameraNumber]->set_manual_iso_bool(10);

        // Checking whether changing the ISO to automatic succeeded or failed
        if(!setManualIsoSuccess)
        {
            spdlog::error("Setting the ISO for camera {} to automatic failed", cameraNumber);
            return false;
        }

        // Introduce a small delay to allow the camera to process the mode change
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Create a promise and future pair
        std::promise<void> prom;
        std::future<void> fut = prom.get_future();

        // Call the asynchronous function to update the variable that holds the mode of the camera and pass the promise
        cameraList[cameraNumber]->get_exposure_program_mode(prom, cameraModes[cameraNumber]);

        // Wait for the asynchronous function to complete
        fut.wait();

        // Log the current mode for debugging
        spdlog::info("Current camera {} mode: {}", cameraNumber, cameraModes[cameraNumber]);

        // Checking whether the change of the camera's mode was successful.
        if (cameraModes[cameraNumber] == "m")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to change the camera {} mode to M mode: {}", cameraNumber, e.what());
        return false;
    }
}

bool CrSDKInterface::switchToPMode(int cameraNumber)
{
    try
    {
        // change of the camera's mode.
        cameraList[cameraNumber]->set_exposure_program_P_mode(cameraModes[cameraNumber]);

        // Set the ISO value to 12800
        bool setManualIsoSuccess = cameraList[cameraNumber]->set_manual_iso_bool(33);

        // Checking whether changing the ISO to automatic succeeded or failed
        if(!setManualIsoSuccess)
        {
            spdlog::error("Setting the ISO for camera {} to automatic failed", cameraNumber);
            return false;
        }

        // Introduce a small delay to allow the camera to process the mode change
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Create a promise and future pair
        std::promise<void> prom;
        std::future<void> fut = prom.get_future();

        // Call the asynchronous function to update the variable that holds the mode of the camera and pass the promise
        cameraList[cameraNumber]->get_exposure_program_mode(prom, cameraModes[cameraNumber]);

        // Wait for the asynchronous function to complete
        fut.wait();

        // Log the current mode for debugging
        spdlog::info("Current camera {} mode: {}", cameraNumber, cameraModes[cameraNumber]);

        // Checking whether the change of the camera's mode was successful.
        if (cameraModes[cameraNumber] == "p")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to change the camera {} mode to P mode: {}", cameraNumber, e.what());
        return false;
    }
}

bool CrSDKInterface::changeBrightness(int cameraNumber, int userBrightnessInput)
{
    try
    {
        // Check if the camera is in manual mode
        if (cameraModes[cameraNumber] != "m")
        {
            spdlog::error("Changing the camera {} brightness is not possible because the camera is not in manual mode.", cameraNumber);
            return false;
        }

        // Validate the user input for brightness
        if (userBrightnessInput < 0 || userBrightnessInput > 48)
        {
            spdlog::error("The brightness value entered is incorrect.");
            return false;
        }

        bool setManualShutterSpeedSuccess = false;
        bool setManualIsoSuccess = false;

        if (userBrightnessInput <= 33)
        {
            // Set shutter speed (0-33) and ISO value to 12800
            setManualShutterSpeedSuccess = cameraList[cameraNumber]->set_manual_shutter_speed_bool(userBrightnessInput);
            setManualIsoSuccess = cameraList[cameraNumber]->set_manual_iso_bool(33); 
        }
        else
        {
            // Set fixed shutter speed (33) and ISO (23-38)
            setManualShutterSpeedSuccess = cameraList[cameraNumber]->set_manual_shutter_speed_bool(33);
            setManualIsoSuccess = cameraList[cameraNumber]->set_manual_iso_bool(userBrightnessInput);
        }

        // Check if setting ISO was successful
        if (!setManualIsoSuccess)
        {
            spdlog::error("Setting the ISO failed.");
            return false;
        }

        // Check if setting shutter speed was successful
        if (!setManualShutterSpeedSuccess)
        {
            spdlog::error("Setting the shutter speed failed.");
            return false;
        }

        spdlog::info("Setting the ISO and shutter speed was successful.");
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("An error occurred while trying to change the brightness of the camera {}: {}", cameraNumber, e.what());
        return false;
    }
}

bool CrSDKInterface::changeAFAreaPosition(int cameraNumber, int x, int y)
{
    try
    {
        // Check if the camera is in auto mode
        if (cameraModes[cameraNumber] != "p")
        {
            spdlog::error("Changing the camera {} AF area position is not possible because the camera is not in auto mode.", cameraNumber);
            return false;
        } 

        // Validate the user input for X and Y coordinates
        if (x < 0 || x > 639)
        {
            spdlog::error("Error: The selected X value is out of range.");
            return false;
        }

        if (y < 0 || y > 479)
        {
            spdlog::error("Error: The selected Y value is out of range.");
            return false;
        }

        int x_y = (x << 16) | y;

        spdlog::info("change the camera {} AF area position...", cameraNumber);

        // Create a future to run the asynchronous function to change to P_Auto mode
        std::future<bool> changeToPModeFuture = std::async(std::launch::async, [=]() 
        {
            return cameraList[cameraNumber]->set_exposure_program_P_Auto_mode(cameraModes[cameraNumber]);
        });

        // Wait for the asynchronous function to complete and get the result
        bool changeModeStatus = changeToPModeFuture.get();

        if (!changeModeStatus) 
        {
            spdlog::error("Failed to change the camera {} mode to P_Auto mode.", cameraNumber);
            return false;
        }

        spdlog::info("change the camera {} mode to P_Auto mode succeeded", cameraNumber);

        // Introduce a small delay to allow the camera to process the mode change
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        bool setAFAreeaPositionSuccess = true;

        // Attempt to set the AF area position
        std::future<bool> setAFPositionFuture = std::async(std::launch::async, [=]() 
        {
            return cameraList[cameraNumber]->set_manual_af_area_position(x_y);
        });

        // Wait for the asynchronous function to complete and get the result
        bool setAFPositionStatus = setAFPositionFuture.get();

        if (!setAFPositionStatus)
        {
            spdlog::error("Failed to set the camera {} AF area position.", cameraNumber);
            setAFAreeaPositionSuccess = false;
        }

        // Create a future to run the asynchronous function to change to Movie_P mode
        std::future<bool> changeToMovieModeFuture = std::async(std::launch::async, [=]() 
        {
            return cameraList[cameraNumber]->set_exposure_program_P_mode(cameraModes[cameraNumber]);
        });

        // Wait for the asynchronous function to complete and get the result
        changeModeStatus = changeToMovieModeFuture.get();

        if (!changeModeStatus) 
        {
            spdlog::error("Failed to change the camera {} mode to Movie_P mode.", cameraNumber);
            return false;
        }

        spdlog::info("change the camera {} mode back to Movie_P mode succeeded", cameraNumber);

        // Introduce a small delay to allow the camera to process the mode change
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if(setAFAreeaPositionSuccess)
        {
            spdlog::info("Successfully set the camera {} AF area position.", cameraNumber);
        } 

        return setAFAreeaPositionSuccess;
    }
    catch (const std::exception& e)
    {
        spdlog::error("An error occurred while trying to change the AF area position of the camera {}: {}", cameraNumber, e.what());
        return false;
    }
}

bool CrSDKInterface::getCamerasMode()
{
    try
    {
        // Get information about all cameras mode
        for (CrInt32u i = 0; i < cameraList.size(); i++)
        {
            // Create a promise and future pair
            std::promise<void> prom;
            std::future<void> fut = prom.get_future();

            // Call the asynchronous function to update the variable that holds the mode of the camera and pass the promise
            cameraList[i]->get_exposure_program_mode(prom, cameraModes[i]);

            // Wait for the asynchronous function to complete
            fut.wait();

            // Checking whether the get camera's mode was successful.
            if (cameraModes[i] == "p" || cameraModes[i] == "m")
            {
                spdlog::info("camera {} mode: {}", i, cameraModes[i]);
            }
            else
            {   
                // switch to P mode logic...
                spdlog::info("The camera mode is not correct, change the mode to P...");
               
                // Create a future to run the asynchronous function to change to Movie_P mode
                std::future<bool> changeToMovieModeFuture = std::async(std::launch::async, [=]() 
                {
                    return cameraList[i]->set_exposure_program_P_mode(cameraModes[i]);
                });

                // Wait for the asynchronous function to complete and get the result
                bool changeModeStatus = changeToMovieModeFuture.get();

                if (!changeModeStatus) 
                {
                    spdlog::error("Failed to change the camera {} mode to Movie_P mode.", i);
                    return false;
                }
                else                
                {
                    // Success message
                    spdlog::info("Changing the camera {} mode to P mode was successful", i);
                    spdlog::info("camera {} mode: {}", i, cameraModes[i]);
                    return true;
                }              
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to get exposure program mode of the cameras: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::getCameraMode(int cameraNumber)
{
    try
    {
        // Create a promise and future pair
        std::promise<void> prom;
        std::future<void> fut = prom.get_future();

        // Call the asynchronous function to get information about a specific camera mode and pass the promise
        cameraList[cameraNumber]->get_exposure_program_mode(prom, cameraModes[cameraNumber]);

        // Wait for the asynchronous function to complete
        fut.wait();

        // Check if the camera mode is either "p" (program mode) or "m" (manual mode)
        return (cameraModes[cameraNumber] == "p" || cameraModes[cameraNumber] == "m");
    }
    catch (const std::exception& e)
    {
        spdlog::error("An error occurred while trying to get exposure program mode of the camera {}: {}", cameraNumber, e.what());
        return false;
    }
}

cli::text CrSDKInterface::getCameraModeStr(int cameraNumber) const
{
    if (cameraNumber >= 0 && cameraNumber < cameraList.size())
    {
        return cameraModes[cameraNumber];
    }
    else
    {
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
    catch (const std::exception &e)
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
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to load the camera settings file from the computer: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::disconnectToCameras()
{
    try
    {
        if (cameraList.empty())
        {
            spdlog::warn("Camera list is empty. No cameras to disconnect.");
            return true;
        }

        std::vector<std::future<bool>> disconnectFutures;
        for (auto& camera : cameraList)
        {
            if (camera && camera->is_connected())
            {
                // Start the disconnect operation asynchronously
                disconnectFutures.push_back(std::async(std::launch::async, [camera]() {
                    return camera->disconnect(); 
                }));
            }
        }

        // Wait for all disconnect operations to complete
        for (auto& future : disconnectFutures)
        {
            if (!future.get()) // Wait for the result and check if it succeeded
            {
                spdlog::error("Failed to disconnect a camera.");
                return false; 
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        spdlog::info("Successfully disconnected all cameras.");
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error while disconnecting cameras: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::releaseCameras()
{
    try
    {
        // Get information about all cameras mode
        for (CrInt32u i = 0; i < cameraList.size(); i++)
        {
            if(cameraList[i])
            {
                spdlog::info("Camera {} object exists, releasing...", i);

                 auto future = std::async(std::launch::async, [&]()
                {
                    cameraList[i]->release(); // Release the camera device
                });

                auto status = future.wait_for(std::chrono::seconds(3)); // Timeout for resource release

                if (status == std::future_status::timeout)
                {
                    spdlog::error("Timeout while releasing camera {} device resources.", i);
                    return false;
                }
            }
        }
        spdlog::info("Released resources of all camera devices successfully.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("Failed to release all camera device resources: {}", e.what());
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
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to release the camera list object: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::releaseCameraRemoteSDK()
{
    try
    {
        std::future<void> future = std::async(std::launch::async, [&]()
        {
            SDK::Release(); // Release the Camera Remote SDK resources
        });

        // Wait for the SDK release to complete, with a timeout
        auto status = future.wait_for(std::chrono::seconds(3)); // Adjust the timeout as needed

        if (status == std::future_status::timeout)
        {
            spdlog::error("Timeout while releasing Camera Remote SDK resources.");
            return false; // Operation timed out
        }

        spdlog::info("The release of the Camera Remote SDK resources was successful.");
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to release the Camera Remote SDK resources: {}", e.what());
        return false;
    }
}

bool CrSDKInterface::loadZoomAndFocusPosition(int cameraNumber)
{
    try
    {
        // Execute preset focus.
        bool executePresetFocusSuccess = cameraList[cameraNumber]->execute_preset_focus_bool();

        // Introduce a small delay to allow the camera to process the set focus position setting
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
       
        if(executePresetFocusSuccess)
        {
            spdlog::info("Execute preset focus for camera {} was successful", cameraNumber);
            return true;
        }
        else
        {
            spdlog::error("Failed to execute preset focus for camera {}", cameraNumber);
            return false;
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to to execute preset focus for camera {}: {}", cameraNumber, e.what());
        return false;
    }
}

cli::text CrSDKInterface::getFnumber(int cameraNumber) 
{
    try 
    {
        // Check manual mode early for clarity
        if (cameraModes[cameraNumber] != "m") 
        {
            spdlog::error("Camera {} not in manual mode. Cannot get F-number.", cameraNumber);
            return "";
        }

        spdlog::info("Getting F-number of camera {}...", cameraNumber);

        // Get both F-number and its string representation concurrently
        auto fnumberFuture = std::async(std::launch::async, [this, cameraNumber] 
        {
            return std::make_pair(
                cameraList[cameraNumber]->get_manual_aperture(),
                cameraList[cameraNumber]->get_manual_aperture_str()
            );
        });

        auto [getFnumberStatus, fnumberStr] = fnumberFuture.get();

        if (getFnumberStatus && !fnumberStr.empty()) 
        {
            spdlog::info("Successfully get F-number for camera {}.", cameraNumber);
            return fnumberStr;
        } 
        else 
        {
            spdlog::error("Failed to get F-number for camera {}.", cameraNumber);
            return "";
        }

    } catch (const std::exception& e) {
        spdlog::error("Error getting F-number for camera {}: {}", cameraNumber, e.what());
        return "";
    }
}

bool CrSDKInterface::setFnumber(int cameraNumber, int FnumberValue) 
{
    try 
    {
        // Input validation: Combined conditions and clearer error message
        if (cameraModes[cameraNumber] != "m" || FnumberValue < 0 || FnumberValue > 21) 
        {
            spdlog::error("Cannot set F-number for camera {}: Invalid mode or value ({})", cameraNumber, FnumberValue);
            return false;
        }

        // Using auto for readability and type safety
        auto& camera = cameraList.at(cameraNumber); // Throw if cameraNumber is invalid
        spdlog::info("Getting F-number of camera {}...", cameraNumber);

        // Getting the F-number: Simplified, no need for async here
        if (!camera->get_manual_aperture()) 
        {
            spdlog::error("Failed to get the camera {} F-number.", cameraNumber);
            return false;
        }

        spdlog::info("Setting F-number of camera {}...", cameraNumber);

        // Setting the F-number: Removed unnecessary async and variable
        bool setSuccess = camera->set_manual_aperture(FnumberValue);

        // Logging and returning result: Simplified ternary
        if (setSuccess) 
        {
            spdlog::info("Successfully set the camera {} F-number.", cameraNumber);
        } 
        else 
        {
            spdlog::error("Failed to set the camera {} F-number.", cameraNumber);
        }

        return setSuccess; // Return the result directly

    } 
    catch (const std::out_of_range& e) 
    { 
        // Handle the specific error of accessing a camera that doesn't exist
        spdlog::error("Camera {} does not exist.", cameraNumber);
        return false;
    } 
    catch (const std::exception& e) 
    {
        spdlog::error("Error setting F-number for camera {}: {}", cameraNumber, e.what());
        return false;
    }
}
