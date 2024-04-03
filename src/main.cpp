#include <iostream>
#include <cstdlib>
#include <experimental/filesystem>
#include <cstdint>
#include <iomanip>
#include <unistd.h>
#include <unistd.h>

#include "SonySDK/app/CRSDK/CameraRemote_SDK.h"
#include "SonySDK/app/CameraDevice.h"
#include "SonySDK/app/Text.h"

#include "CrSDK_interface/CrSDK_interface.h"

#define LIVEVIEW_ENB
#define MSEARCH_ENB
#define NUM_CAMERAS 1

namespace fs = std::experimental::filesystem;
namespace SDK = SCRSDK;

using namespace std;

int main()
{
    CrSDKInterface crsdk;
    bool initSuccess =  crsdk.initializeSDK();

    if(initSuccess){
        bool enumerateSuccess =  crsdk.enumerateCameraDevices();

        if(enumerateSuccess){
            bool connectSuccess = crsdk.connectToCameras();

            if(connectSuccess){
                
                bool downloadSuccess = crsdk.downloadCameraSetting(1);

                bool uploadSuccess = crsdk.uploadCameraSetting(1);
            }
        }
    }

    return 0;

    cli::text cameraMode;

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
        std::exit(EXIT_FAILURE);
    }
    cli::tout << "Remote SDK successfully initialized.\n\n";

    cli::tout << "Enumerate connected camera devices...\n";
    SDK::ICrEnumCameraObjectInfo *camera_list = nullptr;
    auto enum_status = SDK::EnumCameraObjects(&camera_list);
    if (CR_FAILED(enum_status) || camera_list == nullptr)
    {
        cli::tout << "No cameras detected.\n";
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    auto ncams = camera_list->GetCount();
    cli::tout << "Camera enumeration successful. " << ncams << " detected.\n\n";

    // Assuming two cameras are connected, create objects for both
    if (ncams < NUM_CAMERAS)
    {
        cli::tout << "Expected " << NUM_CAMERAS << " cameras, found " << ncams << ". Exiting.\n";
        camera_list->Release();
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }

    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;

    CameraDeviceList cameraList; // all

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

    sleep(1);

    cli::tout << "Cameras connected successfully.\n";

    // Get exposure program mode
    for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    {
        cameraList[i]->get_exposure_program_mode(cameraMode);
    }

    sleep(1);

    char userModeInput;
    cout << "Please select a camera mode ('p' for Auto mode, 'm' for Manual mode): ";
    cin >> userModeInput;

    // Check for user mode input
    if (userModeInput == 'p' || userModeInput == 'P')
    {
        // Set exposure program Auto mode
        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            cameraList[i]->set_exposure_program_P_mode(cameraMode);

            // Set the ISO to automatic
            cameraList[i]->set_manual_iso(10);
        }

        cli::text input;
        input != TEXT("y");
        
        // Option to change the AF Area PositionInput
        cli::tout << std::endl << "Set the value of X (between 0 and 639)" << std::endl;
        getline(cli::tin, input);
        cli::text_stringstream ss1(input);
        CrInt32u x = 0;
        cin >> x;

        if (x < 0 || x > 639) {
            cli::tout << "Input cancelled.\n";
            return -1;
        }

        cli::tout << "input X = " << x << '\n';

        cli::tout << std::endl << "Set the value of Y (between 0 and 479)" << std::endl;
        std::getline(cli::tin, input);
        cli::text_stringstream ss2(input);
        CrInt32u y = 0;
        cin >> y;

        if (y < 0 || y > 479 ) {
            cli::tout << "Input cancelled.\n";
            return -1;
        }

        cli::tout << "input Y = "<< y << '\n';

        int x_y = x << 16 | y;

        // Set exposure program Manual mode
        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            cameraList[i]->set_manual_af_area_position(x_y);
        }
    }
    else if (userModeInput == 'm' || userModeInput == 'M')
    {
        // Set exposure program Manual mode
        for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
        {
            cameraList[i]->set_exposure_program_M_mode(cameraMode);

            // Set the ISO to automatic
            cameraList[i]->set_manual_iso(10);
        }

        int userBrightnessInput;
        cout << "Please select the desired brightness level (between 0 and 48): ";
        cin >> userBrightnessInput;

        if (cameraMode == "m")
        {
            // Checking whether the user's choice of brightness value is correct
            if (userBrightnessInput < 0 || userBrightnessInput > 48)
            {
                cout << "the brightness value entered is incorrect\nEXIT...";
                return -1;
            }
            else if (userBrightnessInput >= 0 && userBrightnessInput <= 33)
            {
                for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
                {
                    // shutter_speed 0 - 33
                    cameraList[i]->set_manual_shutter_speed(userBrightnessInput);
                    // Set the ISO to automatic
                    cameraList[i]->set_manual_iso(10);
                }
            }
            else
            {
                for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
                {
                    // ISO 23 - 38
                    cameraList[i]->set_manual_shutter_speed(33);
                    cameraList[i]->set_manual_iso(userBrightnessInput);
                }
            }
        }
    }

    char userInput;

    // Loop with user input check
    while (true)
    {

        cout << "Press 'q' to quit, or any other key to continue: ";
        cin >> userInput;

        // Check for user input and exit if 'q' is pressed
        if (userInput == 'q' || userInput == 'Q')
        {
            break;
        }
    }

    // ... rest of your code using the cameraList ...

    for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    {
        // Disconnect from the camera and release resources before exiting
        cameraList[i]->disconnect();
    }

    // Release resources before exiting
    camera_list->Release();
    SDK::Release();

    return 0;
}
