#include "CrSDK_interface.h"

CrSDKInterface::CrSDKInterface(){

    // Optionally initialize the vector elements here
    cameraModes.resize(4); // Allocate space for 4 elements
}

CrSDKInterface::~CrSDKInterface() {
    // Release resources acquired in the constructor or during object lifetime
    for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    {
        // Disconnect from the camera and release resources before exiting
        cameraList[i]->disconnect();
    }
    if (camera_list != nullptr) {
        camera_list->Release(); // Release the camera list object
    }
    SDK::Release(); // Release the Camera Remote SDK resources
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

    cli::tout << "Remote SDK version: ";
    cli::tout << major << "." << minor << "." << std::setfill(TEXT('0')) << std::setw(2) << patch << "\n";

    cli::tout << "Initialize Remote SDK...\n";
    cli::tout << "Working directory: " << fs::current_path() << '\n';

    auto init_success = SDK::Init();
    if (!init_success)
    {
        cli::tout << "Failed to initialize Remote SDK. Terminating.\n";
        SDK::Release();
        return false;
    }
    cli::tout << "Remote SDK successfully initialized.\n\n";

    return true;
}

bool CrSDKInterface::enumerateCameraDevices(){

    cli::tout << "Enumerate connected camera devices...\n";
    auto enum_status = SDK::EnumCameraObjects(&camera_list);
    if (CR_FAILED(enum_status) || camera_list == nullptr)
    {
        cli::tout << "No cameras detected.\n";
        SDK::Release();
        return false;
    }
    auto ncams = camera_list->GetCount();
    cli::tout << "Camera enumeration successful. " << ncams << " detected.\n\n";

    // Assuming two cameras are connected, create objects for both
    if (ncams < NUM_CAMERAS)
    {
        cli::tout << "Expected " << NUM_CAMERAS << " cameras, found " << ncams << ". Exiting.\n";
        camera_list->Release();
        SDK::Release();
        return false;
    }

    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;

    CameraDeviceList cameraList; // all

    return true;
}

bool CrSDKInterface::connectToCameras(){
    try
    {
        cli::tout << "Connecting to both cameras...\n";
        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            auto *camera_info = camera_list->GetCameraObjectInfo(i);
            cli::tout << "  - Creating object for camera " << i + 1 << "...\n";
            CameraDevicePtr camera = CameraDevicePtr(new cli::CameraDevice(i + 1, camera_info));
            cameraList.push_back(camera);
        }

        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            // Connect to the camera in Remote Control Mode
            cameraList[i]->connect(SDK::CrSdkControlMode_Remote, SDK::CrReconnecting_ON);
        }

        cli::tout << "Cameras connected successfully.\n";

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to connect to the cameras: "<< e.what() << '\n';
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

        std::cerr << "An error occurred while trying to change the camera mode to M mode: " << e.what() << '\n';
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

        std::cerr << "An error occurred while trying to change the camera mode to P mode: " << e.what() << '\n';
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
                cout << "the brightness value entered is incorrect\nEXIT...";
                return -1;
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
            std::cerr << "Changing the camera brightness is not possible because the camera is not in M(manual) mode" << '\n';
            return false;
        }   
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to change the brightness of the camera: "<< e.what() << '\n';
        return false;
    }
}

bool CrSDKInterface::changeAFAreaPosition(int cameraNumber, int x, int y){
    try
    {
        if (cameraModes[cameraNumber] == "p")
        {
            if (x < 0 || x > 639) {
                cli::tout << "Error: The selected X value is out of range\n";
                return false;
            }

            if (y < 0 || y > 479 ) {
                cli::tout << "Error: The selected Y value is out of range\n";
                return false;
            }

            int x_y = x << 16 | y;

            // Set exposure program Manual mode
            cameraList[cameraNumber]->set_manual_af_area_position(x_y);
            
            return true;
        } 
        else
        {
            std::cerr << "Changing the camera AF area position is not possible because the camera is not in P(auto) mode" << '\n';
            return false;
        }   
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to change the AF area position of the camera: "<< e.what() << '\n';
        return false;
    }
}

bool CrSDKInterface::getCameraMode(int cameraNumber){

    // Get exposure program mode
    for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    {
        cameraList[i]->get_exposure_program_mode(cameraModes[i]);
    }
    return true;
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
    try
    {
        cameraList[cameraNumber]->do_download_camera_setting_file();
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred when trying to download the camera settings file to the computer: " << e.what() << '\n';
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
            std::cerr << "An error occurred while trying to load the camera settings file from the computer: " << e.what() << '\n';
            return false;
        }
}