
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
#include <atomic>

#include "SonySDK/app/CRSDK/CameraRemote_SDK.h"
#include "SonySDK/app/CameraDevice.h"
#include "SonySDK/app/Text.h"

#include "CrSDK_interface/CrSDK_interface.h"
#include "https_server/https_server.h"

#define LIVEVIEW_ENB
#define MSEARCH_ENB
#define HOST "127.0.0.1"
#define PORT 8085

using namespace std;

std::atomic<bool> stopRequested(false);

void signalHandler(int sig) {
  if (sig == SIGINT) {
    // Print a message when the program stops
    spdlog::info("Program stopped..."); 
    
    stopRequested.store(true);
  }
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

/**
 * @brief Validates an IPv4 address using regular expressions.
 *
 * @param ipAddress The IP address to validate.
 * @return True if the IP address is valid, false otherwise.
 */
bool isValidIPAddress(const std::string &ipAddress)
{
    // Regular expression to validate an IPv4 address
    std::regex ipAddressRegex("^\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b$");
    return std::regex_match(ipAddress, ipAddressRegex);
}

/**
 * @brief Reads the IP address from a file.
 *
 * @param filePath The path to the file containing the IP address.
 * @return The IP address read from the file.
 * @throws std::runtime_error if there is an error opening the file.
 */
std::string readIPAddressFromFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    std::string ipAddress;

    if (file.is_open())
    {
        // Read the file line by line and append to the ipAddress string
        std::string line;
        while (std::getline(file, line))
        {
            ipAddress += line;
        }
        // Remove trailing newline characters, if any
        size_t lastNonSpace = ipAddress.find_last_not_of(" \t\r\n");
        if (lastNonSpace != std::string::npos)
        {
            ipAddress = ipAddress.substr(0, lastNonSpace + 1);
        }
    }
    else
    {
      return ""; // Return empty string if no matching IP found
    }

    return ipAddress;
}

int main()
{
    std::string host;

    // // Read the IP address from the file
    // #ifdef DEVELOPMENT_ENVIRONMENT
    //         std::string ipAddress = readIPAddressFromFile("/home/nvidia/Projects/low-level-detection-embedded/config/config.txt"); // The path for the development version.
    // #else
    //         std::string ipAddress = readIPAddressFromFile("/lld_sw_v1.0.0/lld/config.txt"); // Fixed the path for the production version.
    // #endif

    std::string ipAddress = readIPAddressFromFile("/lld_sw_v1.0.0/lld/config.txt"); // Fixed the path for the production version.

    if(!ipAddress.empty())
    {
      // Validate the IP address
      if (isValidIPAddress(ipAddress))
      {
        spdlog::info("The IP address from the file is valid: {}", ipAddress);
        host = ipAddress;
      }
      else
      {
        spdlog::error("Invalid IP address in the file. Exiting the program. ipAddress : {}", ipAddress);
        return EXIT_FAILURE; // Return error code
      }
    }
    else
    {
      spdlog::warn("Configuration file does not exist.");
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
    }

    CrSDKInterface *crsdk = new CrSDKInterface();
    
    // Initializes the Camera Remote SDK.
    bool initSuccess =  crsdk->initializeSDK();

    if(!initSuccess)
    {
        // Handle SDK initialization failure
        spdlog::error("Failed to initialize CrSDK. Exiting...");
        delete crsdk;
        spdlog::info("Deleting the instance of the CrSDKInterface class was successful");

        return EXIT_FAILURE;
    }

    // Enumerates connected camera devices.
    bool enumerateSuccess =  crsdk->enumerateCameraDevices();

    if(!enumerateSuccess)
    {
        // Handle camera device enumeration failure
        spdlog::error("Failed to enumerate camera devices. Exiting...");
        crsdk->releaseCameraRemoteSDK();

        delete crsdk;
        spdlog::info("Deleting the instance of the CrSDKInterface class was successful");

        return EXIT_FAILURE;
    }

    // Creating a connection to the cameras after enumerate Camera Devices.
    bool connectSuccess = crsdk->connectToCameras();

    sleep(2);

    if(!connectSuccess)
    {
        // Handle connection failure
        spdlog::error("Failed to connect to cameras. Exiting...");
        crsdk->disconnectToCameras();
        crsdk->releaseCameraList();
        crsdk->releaseCameraRemoteSDK();

        delete crsdk;
        spdlog::info("Deleting the instance of the CrSDKInterface class was successful");

        return EXIT_FAILURE;

    }

    // Get information about all cameras mode(auto or manual).
    bool getModeSuccess = crsdk->getCamerasMode();

    if(!getModeSuccess)
    {
        // Handle get cameras mode failure
        spdlog::error("Failed to get cameras mode. Exiting.");
        crsdk->disconnectToCameras();
        crsdk->releaseCameraList();
        crsdk->releaseCameraRemoteSDK();
        delete crsdk;
        spdlog::info("Deleting the instance of the CrSDKInterface class was successful");

        return EXIT_FAILURE;
    }

    sleep(1);

    // Configure server parameters
    const std::string cert_file = "/jetson_ssl/jeston-server-embedded.crt";
    const std::string key_file = "/jetson_ssl/jeston-server-embedded.key";
                
    // Initialize the Server object with host, port, SSL certificate, key file, optional streamer, and shouldVectorRun flag.
    Server server(host, PORT, cert_file, key_file, crsdk);

    // Run the server in a separate thread
    std::thread serverThread(&Server::run, &server);

    // Register signal handler for Ctrl+C
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    int i = 0;

    // Check for stop request periodically or in a loop
    while (!stopRequested.load()) 
    {
      if(i >= 50){
        for(CrInt32u i = 0; i < crsdk->cameraList.size(); ++i)
        {
          spdlog::info("Camera number {} is connected", i);
        }
        spdlog::info("The server is running");
        i = 0;
      }
      
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      i++;
    }

    // Request the server thread to stop gracefully
    server.stopServer();  // Add a stop() method to your Server class

    // Wait for the server thread to finish
    serverThread.join();  // Wait for completion before continuing

    // Release resources acquired in the constructor or during object lifetime
    crsdk->disconnectToCameras();

    // Release resources acquired in the constructor or during object lifetime.
    crsdk->releaseCameraList();

    // Releases resources associated with the Camera Remote SDK.
    crsdk->releaseCameraRemoteSDK();

    delete crsdk;
    spdlog::info("Deleting the instance of the CrSDKInterface class was successful");

    // Print a message when the program stops
    spdlog::info("The program stopped successfully, exits");

    return EXIT_SUCCESS;
}


