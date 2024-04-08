
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
#include <signal.h>

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

bool running = true;
bool shouldRunServer = true;

void handle_sigint(int sig) {
    running = false;
    shouldRunServer = false;
}

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

    // Register signal handler for Ctrl+C
    signal(SIGINT, handle_sigint);

    CrSDKInterface *crsdk = new CrSDKInterface();
       
    bool initSuccess =  crsdk->initializeSDK();

    if(initSuccess){
        bool enumerateSuccess =  crsdk->enumerateCameraDevices();

        if(enumerateSuccess){
            bool connectSuccess = crsdk->connectToCameras();

            sleep(2);

            if(connectSuccess)
            {
                bool getModeSuccess = crsdk->getCamerasMode();

                if(getModeSuccess)
                {

                    sleep(1);

                    // Configure server parameters
                    const std::string cert_file = "/jetson_ssl/jeston-server-embedded.crt";
                    const std::string key_file = "/jetson_ssl/jeston-server-embedded.key";
                
                    // Initialize the Server object with host, port, SSL certificate, key file, optional streamer, and shouldVectorRun flag.
                    Server server(host, PORT, cert_file, key_file, crsdk);

                    // Run the server in a separate thread
                    std::thread serverThread(&Server::run, &server);

                    while (running) 
                    {
                        if (shouldRunServer) 
                        {

                            // Server thread has finished or Ctrl+C was received
                            spdlog::info("ServerThread has finished or Ctrl+C received.");

                            // Wait for the server thread to exit
                            serverThread.join();

                            // Exit the loop if Ctrl+C was received
                            if (!running) 
                            {
                                break;
                            }
                        } 
                        else 
                        {
                            // Server is not running, wait for user input
                            for(CrInt32u i = 0; i < crsdk->cameraList.size(); ++i)
                            {
                                spdlog::info("Camera number {} is connected", i);
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                    }

                    // Server thread has finished or Ctrl+C was received
                    spdlog::info("ServerThread has finished or Ctrl+C received.");

                    // Wait for the server thread to exit
                    serverThread.join();  

                    goto cleanup; // Jump to the cleanup section

                }
                else
                {
                    spdlog::error("Failed to get cameras mode. Exiting.");
                    // Perform any necessary cleanup (e.g., releasing resources) before exiting
                    goto cleanup; // Jump to the cleanup section
                }
            }
            else
            {
                // Handle connection failure
                spdlog::error("Failed to connect to cameras. Exiting.");
                // Perform any necessary cleanup (e.g., releasing resources) before exiting
                goto cleanup; // Jump to the cleanup section
            }
        } 
        else 
        {
            // Handle camera device enumeration failure
            spdlog::error("Failed to enumerate camera devices. Exiting.");
            goto cleanup;
        }
    } 
    else 
    {
        // Handle SDK initialization failure
        spdlog::error("Failed to initialize CrSDK. Exiting.");
        goto cleanup;
    }

cleanup:

    // Release resources acquired in the constructor or during object lifetime
    for (CrInt32u i = 0; i < crsdk->cameraList.size(); ++i)
    {
        // Disconnect from the camera and release resources before exiting
        crsdk->cameraList[i]->disconnect();
        spdlog::info("Disconnect from the camera {}.", i);
    }

    // Release resources acquired in the constructor or during object lifetime.
    if (crsdk->camera_list != nullptr) {
        crsdk->camera_list->Release(); // Release the camera list object.
        spdlog::info("Release the camera list object.");
    }
    
    // Print a message when the program stops
    spdlog::info("Program stopped.");

    // std::exit(EXIT_SUCCESS);


    return EXIT_SUCCESS;
}
