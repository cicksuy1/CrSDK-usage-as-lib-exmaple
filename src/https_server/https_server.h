/**
 * @file server.h
 * @brief Defines the Server class representing an HTTPS server.
 *
 * The Server class provides basic HTTP server functionality and allows association
 */

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nholman_json/json.hpp>
#include <iostream>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <string>
#include <mutex>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <sys/wait.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

#include "../CrSDK_interface/CrSDK_interface.h"
#include "../gpioPin/gpioPin.h"

using json = nlohmann::json;

#define BLACK_THRESHOLD 10.0

/**
 * @class Server
 * @brief Represents an HTTP server with optional RTSP streaming capabilities.
 *
 * The Server class provides basic HTTP server functionality and an option to associate
 * it with an instance of the RTSPStreamer class for additional streaming features.
 */
class Server 
{
public:

    /**
     * @brief Constructs a Server object with basic HTTP server parameters.
     * @param host The host address on which the server will listen.
     * @param port The port on which the server will listen.
     * @param cert_file The path to the SSL certificate file.
     * @param key_file The path to the SSL key file.
     * @param crsdkInterface instance of CrSDKInterface class.
     */
    Server(const std::string &host, int port, const std::string &cert_file, const std::string &key_file, CrSDKInterface *crsdkInterface = nullptr);


    /**
     * @brief Constructs a Server object with basic HTTP server parameters.
     * @param host The host address on which the server will listen.
     * @param port The port on which the server will listen.
     * @param cert_file The path to the SSL certificate file.
     * @param key_file The path to the SSL key file.
     * @param gpioP An instance of GPIO pin to associate with the server.
     * @param crsdkInterface instance of CrSDKInterface class.
     */
    Server(const std::string &host, int port, const std::string &cert_file, const std::string &key_file, GpioPin *gpioP = nullptr, CrSDKInterface *crsdkInterface = nullptr);

    void setGpioPin(GpioPin *gpioPin);

    /**
     * @brief Start the HTTP server to listen for incoming requests.
     */
    void run();

    /**
    * @brief Stops the server gracefully.
    *
    * @return True if the server was stopped successfully, false otherwise.
    * @throws std::exception If an error occurs while trying to stop the server.
    */
    bool stopServer();

    /**
     * @brief function to start the monitoring thread.
     */
    void startMonitoringThread(); 

    /**
     * @brief function to restart the server.
     */
    void restartServer();

private:

    httplib::SSLServer server;                                  ///< HTTPS server instance.
    std::string host_;                                          ///< Host address on which the server will listen.
    int port_;                                                  ///< Port on which the server will listen.
    CrSDKInterface *crsdkInterface_;                            ///< Add an instance of CrSDKInterface
    std::thread monitoringThread;                               ///< Thread object for monitoring
    std::atomic<bool> stopRequested{false};                     ///< A flag for stopping the server thread
    GpioPin *gpioPin;                                           ///< Declaration of GpioPin instance

    /**
     * @brief Set up HTTP routes for the server.
     */
    void setupRoutes();

    /**
     * @brief HTTP handler for the "indicator" route.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleIndicator(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to switch the camera to P mode.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleSwitchToPMode(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to switch the camera to M mode.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleSwitchToMMode(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to change brightness.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleChangeBrightness(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to change AF area position.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleChangeAFAreaPosition(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to get camera mode.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleGetCameraMode(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for download the camera setting file to PC.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleDownloadCameraSetting(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to upload the camera setting file to Camera.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleUploadCameraSetting(const httplib::Request &req, httplib::Response &res);

};    
