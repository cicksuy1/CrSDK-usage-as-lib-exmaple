
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include <cstdlib>
#include <experimental/filesystem>
#include <cstdint>
#include <iomanip>
#include <unistd.h>
#include <unistd.h>
#include <thread>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "SonySDK/app/CRSDK/CameraRemote_SDK.h"
#include "SonySDK/app/CameraDevice.h"
#include "SonySDK/app/Text.h"

#include "CrSDK_interface/CrSDK_interface.h"
#include "https_server/https_server.h"

#define LIVEVIEW_ENB
#define MSEARCH_ENB
#define NUM_CAMERAS 1
#define HOST "127.0.0.1"
#define PORT 8085

using namespace std;


/**
 * @brief Executes a command and returns the output as a string.
 *
 * @param cmd The command to execute.
 * @param useSudo If true, prepend 'sudo' to the command.
 * @return The output of the command as a string.
 * @throws std::runtime_error if the command execution fails.
 */
std::string executeCommand(const char *cmd, bool useSudo = false)
{
    std::string result = "";
    FILE *pipe;

    // Use sudo if requested
    if (useSudo)
    {
        cmd = ("sudo " + std::string(cmd)).c_str();
    }

    char buffer[128];
    pipe = popen(cmd, "r");

    if (!pipe)
    {
        throw std::runtime_error("popen() failed for command: " + std::string(cmd));
    }

    while (!feof(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            result += buffer;
        }
    }

    pclose(pipe);
    return result;
}

/**
 * @brief Checks if ZeroTier is active and starts it if not.
 *
 * @return True if ZeroTier is active or was successfully started, false otherwise.
 * @throws std::runtime_error if the user does not have sudo access.
 */
bool checkAndStartZeroTier()
{
  try {
    // Check for sudo access
    if (std::system("id -u") != 0) {
      throw std::runtime_error("User does not have sudo access");
    }

    // Check if ZeroTier is active
    std::string ztStatus = executeCommand("sudo zerotier-cli status");

    if (ztStatus.find("zerotier-cli: error") != std::string::npos) {
      // ZeroTier is not active, attempt to start it
      spdlog::info("ZeroTier is not active. Starting ZeroTier...");
      std::string startResult = executeCommand("sudo zerotier-cli start");
      spdlog::info("Result of the start command: {}", startResult);

      if (startResult.find("200 join OK") != std::string::npos) {
        // Start successful
        return true;
      } else {
        // Start failed
        spdlog::error("Failed to start ZeroTier");
        return false;
      }
    } else {
      // ZeroTier is already active
      spdlog::info("ZeroTier is already active.");
      return true;
    }
  } catch (const std::runtime_error& e) {
    spdlog::error("Error: {}", e.what());
    return false;
  }
}

/**
 * @brief Retrieves the ZeroTier IP address based on network information.
 *
 * @return The ZeroTier IP address.
 * @throws std::runtime_error if no matching IP addresses are found.
 */
std::string getZeroTierIP()
{
    // Execute the command to get the network information
    std::string ipCommand = "ip addr show";
    std::string ipInfo = executeCommand(ipCommand.c_str());

    // Use regular expressions to extract any private IP address (assuming it's in the private range)
    // Corrected regex
    std::regex regexIPv4("(10\\.(\\d{1,3}\\.){2}\\d{1,3}/\\d{1,2})|(172\\.(1[6-9]|2\\d|3[0-1])\\.(\\d{1,3}\\.){1,2}\\d{1,3}/\\d{1,2})|(192\\.168\\.\\d{1,3}\\.\\d{1,3})");
    std::smatch match;
    std::vector<std::string> filteredAddresses;

    while (std::regex_search(ipInfo, match, regexIPv4))
    {
        std::string ipAddress = match[0].str();

        // Remove the subnet part like "/24" or "/16"
        size_t slashPos = ipAddress.find('/');
        if (slashPos != std::string::npos)
        {
            ipAddress = ipAddress.substr(0, slashPos);
        }

        spdlog::info("IP: {}", ipAddress);

        std::string prefix = ipAddress.substr(0, 9);
        if (prefix.compare("10.147.17") == 0) 
        {
            filteredAddresses.push_back(ipAddress);
        }

        // Move to the next match
        ipInfo = match.suffix();
    }

    if (!filteredAddresses.empty())
    {
        for (const auto &address : filteredAddresses)
        {
            spdlog::info("Filtered IP address: {}", address);
        }

        return filteredAddresses[0]; // Returning the first filtered address as an example
    }
    else
    {
        return ""; // Return empty string if no matching IP found
    }
}

int main()
{
    std::string host;

    // bool zeroTierSuccess =  checkAndStartZeroTier();
    // if(zeroTierSuccess)
    // {
    //     std::string ZeroTierIP = getZeroTierIP();

    //     if(!ZeroTierIP.empty())
    //     {
    //         spdlog::info("host: {}", ZeroTierIP);
    //         host = ZeroTierIP;
    //     }
    //     else{
    //         spdlog::warn("ZeroTier assigned IP address not found, The server will run on the localhost");
    //         host = HOST;
    //     }
    // }
    // else
    // {
    //     spdlog::warn("zeroTier does not run");
    //     host = HOST;
    // }

    std::string ZeroTierIP = getZeroTierIP();

    if(!ZeroTierIP.empty())
    {
        spdlog::info("host: {}", ZeroTierIP);
        host = ZeroTierIP;
    }
    else{
        spdlog::warn("ZeroTier assigned IP address not found, The server will run on the localhost");
        host = HOST;
    }

    CrSDKInterface *crsdk = new CrSDKInterface();
       
    bool initSuccess =  crsdk->initializeSDK();

    if(initSuccess){
        bool enumerateSuccess =  crsdk->enumerateCameraDevices();

        if(enumerateSuccess){
            bool connectSuccess = crsdk->connectToCameras();

            sleep(2);

            bool getModeSuccess = crsdk->getCamerasMode();

            if(connectSuccess){

                sleep(1);

                // Configure server parameters
                const std::string cert_file = "/jetson_ssl/jeston-server-embedded.crt";
                const std::string key_file = "/jetson_ssl/jeston-server-embedded.key";
                
                // Initialize the Server object with host, port, SSL certificate, key file, optional streamer, and shouldVectorRun flag.
                Server server(host, PORT, cert_file, key_file, crsdk);

                // server.run();

                // Run the server in a separate thread
                std::thread serverThread(&Server::run, &server);

                // Wait for serverThread to finish
                serverThread.join();

                spdlog::info("ServerThread has finished.");

            }
            else
            {
                spdlog::info("EXIT...");
            }
        }
        else
        {
            spdlog::info("EXIT...");
        }
    }
    else
    {
        spdlog::info("EXIT...");
        spdlog::info("8");  
    }

    // Release resources acquired in the constructor or during object lifetime
    if (crsdk->camera_list != nullptr) {
        crsdk->camera_list->Release(); // Release the camera list object
    }
    
    // Print a message when the program stops
    spdlog::info("Program stopped.");

    std::exit(EXIT_SUCCESS);

    return 0;
}

    // cli::text cameraMode;

    // // Change global locale to native locale
    // std::locale::global(std::locale(""));

    // // Make the stream's locale the same as the current global locale
    // cli::tin.imbue(std::locale());
    // cli::tout.imbue(std::locale());

    // CrInt32u version = SDK::GetSDKVersion();
    // int major = (version & 0xFF000000) >> 24;
    // int minor = (version & 0x00FF0000) >> 16;
    // int patch = (version & 0x0000FF00) >> 8;
    // // int reserved = (version & 0x000000FF);

    // cli::tout << "Remote SDK version: ";
    // cli::tout << major << "." << minor << "." << std::setfill(TEXT('0')) << std::setw(2) << patch << "\n";

    // cli::tout << "Initialize Remote SDK...\n";
    // cli::tout << "Working directory: " << fs::current_path() << '\n';

    // auto init_success = SDK::Init();
    // if (!init_success)
    // {
    //     cli::tout << "Failed to initialize Remote SDK. Terminating.\n";
    //     SDK::Release();
    //     std::exit(EXIT_FAILURE);
    // }
    // cli::tout << "Remote SDK successfully initialized.\n\n";

    // cli::tout << "Enumerate connected camera devices...\n";
    // SDK::ICrEnumCameraObjectInfo *camera_list = nullptr;
    // auto enum_status = SDK::EnumCameraObjects(&camera_list);
    // if (CR_FAILED(enum_status) || camera_list == nullptr)
    // {
    //     cli::tout << "No cameras detected.\n";
    //     SDK::Release();
    //     std::exit(EXIT_FAILURE);
    // }
    // auto ncams = camera_list->GetCount();
    // cli::tout << "Camera enumeration successful. " << ncams << " detected.\n\n";

    // // Assuming two cameras are connected, create objects for both
    // if (ncams < NUM_CAMERAS)
    // {
    //     cli::tout << "Expected " << NUM_CAMERAS << " cameras, found " << ncams << ". Exiting.\n";
    //     camera_list->Release();
    //     SDK::Release();
    //     std::exit(EXIT_FAILURE);
    // }

    // typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    // typedef std::vector<CameraDevicePtr> CameraDeviceList;

    // CameraDeviceList cameraList; // all

    // cli::tout << "Connecting to both cameras...\n";
    // for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    // {
    //     auto *camera_info = camera_list->GetCameraObjectInfo(i);
    //     cli::tout << "  - Creating object for camera " << i + 1 << "...\n";
    //     CameraDevicePtr camera = CameraDevicePtr(new cli::CameraDevice(i + 1, camera_info));
    //     cameraList.push_back(camera);
    // }

    // for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    // {
    //     // Connect to the camera in Remote Control Mode
    //     cameraList[i]->connect(SDK::CrSdkControlMode_Remote, SDK::CrReconnecting_ON);
    // }

    // sleep(1);

    // cli::tout << "Cameras connected successfully.\n";

    // // Get exposure program mode
    // for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    // {
    //     cameraList[i]->get_exposure_program_mode(cameraMode);
    // }

    // sleep(1);

    // char userModeInput;
    // cout << "Please select a camera mode ('p' for Auto mode, 'm' for Manual mode): ";
    // cin >> userModeInput;

    // // Check for user mode input
    // if (userModeInput == 'p' || userModeInput == 'P')
    // {
    //     // Set exposure program Auto mode
    //     for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    //     {
    //         cameraList[i]->set_exposure_program_P_mode(cameraMode);

    //         // Set the ISO to automatic
    //         cameraList[i]->set_manual_iso(10);
    //     }

    //     cli::text input;
    //     input != TEXT("y");
        
    //     // Option to change the AF Area PositionInput
    //     cli::tout << std::endl << "Set the value of X (between 0 and 639)" << std::endl;
    //     getline(cli::tin, input);
    //     cli::text_stringstream ss1(input);
    //     CrInt32u x = 0;
    //     cin >> x;

    //     if (x < 0 || x > 639) {
    //         cli::tout << "Input cancelled.\n";
    //         return -1;
    //     }

    //     cli::tout << "input X = " << x << '\n';

    //     cli::tout << std::endl << "Set the value of Y (between 0 and 479)" << std::endl;
    //     std::getline(cli::tin, input);
    //     cli::text_stringstream ss2(input);
    //     CrInt32u y = 0;
    //     cin >> y;

    //     if (y < 0 || y > 479 ) {
    //         cli::tout << "Input cancelled.\n";
    //         return -1;
    //     }

    //     cli::tout << "input Y = "<< y << '\n';

    //     int x_y = x << 16 | y;

    //     // Set exposure program Manual mode
    //     for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    //     {
    //         cameraList[i]->set_manual_af_area_position(x_y);
    //     }
    // }
    // else if (userModeInput == 'm' || userModeInput == 'M')
    // {
    //     // Set exposure program Manual mode
    //     for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    //     {
    //         cameraList[i]->set_exposure_program_M_mode(cameraMode);

    //         // Set the ISO to automatic
    //         cameraList[i]->set_manual_iso(10);
    //     }

    //     int userBrightnessInput;
    //     cout << "Please select the desired brightness level (between 0 and 48): ";
    //     cin >> userBrightnessInput;

    //     if (cameraMode == "m")
    //     {
    //         // Checking whether the user's choice of brightness value is correct
    //         if (userBrightnessInput < 0 || userBrightnessInput > 48)
    //         {
    //             cout << "the brightness value entered is incorrect\nEXIT...";
    //             return -1;
    //         }
    //         else if (userBrightnessInput >= 0 && userBrightnessInput <= 33)
    //         {
    //             for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    //             {
    //                 // shutter_speed 0 - 33
    //                 cameraList[i]->set_manual_shutter_speed(userBrightnessInput);
    //                 // Set the ISO to automatic
    //                 cameraList[i]->set_manual_iso(10);
    //             }
    //         }
    //         else
    //         {
    //             for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    //             {
    //                 // ISO 23 - 38
    //                 cameraList[i]->set_manual_shutter_speed(33);
    //                 cameraList[i]->set_manual_iso(userBrightnessInput);
    //             }
    //         }
    //     }
    // }

    // char userInput;

    // // Loop with user input check
    // while (true)
    // {

    //     cout << "Press 'q' to quit, or any other key to continue: ";
    //     cin >> userInput;

    //     // Check for user input and exit if 'q' is pressed
    //     if (userInput == 'q' || userInput == 'Q')
    //     {
    //         break;
    //     }
    // }

    // // ... rest of your code using the cameraList ...

    // for (CrInt32u i = 0; i < NUM_CAMERAS; ++i)
    // {
    //     // Disconnect from the camera and release resources before exiting
    //     cameraList[i]->disconnect();
    // }

    // // Release resources before exiting
    // camera_list->Release();
    // SDK::Release();

    // return 0;

