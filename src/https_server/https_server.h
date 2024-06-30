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
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <filesystem>

#include "../CrSDK_interface/CrSDK_interface.h"
#include "../gpioPin/gpioPin.h"

using json = nlohmann::json;

#define BLACK_THRESHOLD 10.0

// Macro to reverse the index
#define REVERSE_INDEX(index) ((index) == 0 ? 1 : 0)

// Macro to keep the index the same
#define NORMAL_INDEX(index) (index)

namespace fs = std::experimental::filesystem;

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
     * @param isSSlServer Flag whether the server is an SSL server (HTTPS) or a regular server (HTTP).
     * @param cert_file The path to the SSL certificate file.
     * @param key_file The path to the SSL key file.
     * @param client_file The path to the SSL client file.
     * @param stopRequested A pointer to std::atomic<bool> flag.
     * @param crsdkInterface instance of CrSDKInterface class.
     */
    Server(const std::string &host, int port, bool isSSlServer, const std::string &cert_file, const std::string &key_file, std::string &client_file, std::atomic<bool> &stopRequested, CrSDKInterface *crsdkInterface = nullptr);

    /**
     * @brief Constructs a Server object with basic HTTP server parameters.
     * @param host The host address on which the server will listen.
     * @param port The port on which the server will listen.
     * @param isSSlServer Flag whether the server is an SSL server (HTTPS) or a regular server (HTTP).
     * @param cert_file The path to the SSL certificate file.
     * @param key_file The path to the SSL key file.
     * @param client_file The path to the SSL client file.
     * @param stopRequested A pointer to std::atomic<bool> flag.
     * @param gpioP An instance of GPIO pin to associate with the server.
     * @param crsdkInterface instance of CrSDKInterface class.
     */
    Server(const std::string &host, int port, bool isSSlServer, const std::string &cert_file, const std::string &key_file, std::string &client_file, std::atomic<bool> &stopRequested, GpioPin *gpioP = nullptr, CrSDKInterface *crsdkInterface = nullptr);

    /**
     * Sets the GpioPin object associated with the server.
     * @param gpioPin A pointer to the GpioPin object to be associated with the server.
     * @precondition gpioPin must be a valid pointer to a GpioPin object.
     * @postcondition The server's associated GpioPin object is set to the provided gpioPin.
    */
    void setGpioPin(GpioPin *gpioPin);

    /**
     * @brief Start the HTTP server to listen for incoming requests.
     */
    void run();

    /**
     * @brief Initializes the token bucket parameters.
     * @param maxTokens The maximum number of tokens in the bucket.
     * @param refillRate The rate at which tokens are refilled per second.
     * @param refillNumber The number of the tokens to refill.
     */
    void initializeTokenBucket(int maxTokens, int refillRate, int refillNumber);

    /**
     * @brief Consumes a token from the token bucket.
     * @return True if a token was successfully consumed, false otherwise.
     */
    bool consumeToken();

    /**
     * @brief Refills tokens in the token bucket based on the elapsed time.
     * @param refillNumber The number of the tokens to refill.
     */
    void refillTokens(int refillRate);

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

    bool isSSlServer;                                           ///< Flag whether the server is an SSL server (HTTPS) or a regular server (HTTP).

private:

    httplib::SSLServer sslServer;                               ///< HTTPS server instance.
    httplib::Server server;                                     ///< HTTP server instance.
    std::string host_;                                          ///< Host address on which the server will listen.
    int port_;                                                  ///< Port on which the server will listen.
    std::string &client_file_;                                  ///< The path to the SSL client file.
    CrSDKInterface *crsdkInterface_;                            ///< Add an instance of CrSDKInterface
    std::thread monitoringThread;                               ///< Thread object for monitoring
    std::atomic<bool> &stopRequested;                           ///< A flag for stopping the server thread
    GpioPin *gpioPin;                                           ///< Declaration of GpioPin instance
    
    // Token bucket parameters
    int maxTokens_;                                             ///< Maximum number of tokens in the bucket
    int currentTokens_;                                         ///< Current number of tokens in the bucket
    std::chrono::steady_clock::time_point lastTokenTime_;       ///< Last time tokens were added
    std::mutex tokenMutex_;                                     ///< Mutex for thread safety

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
     * @brief HTTP handler for Receives a request to get the brightness value of the camera.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleGetCameraBrightness(const httplib::Request &req, httplib::Response &res);


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

    /**
     * @brief HTTP handler for Receives a request to get F-number.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleGetFnumber(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for Receives a request to set F-number.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleSetFnumber(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for starting the cameras.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleStartCameras(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief HTTP handler for stopping the cameras.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleStopCameras(const httplib::Request &req, httplib::Response &res);

     /**
     * @brief HTTP handler for restat the cameras.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleRestatCameras(const httplib::Request &req, httplib::Response &res);

     /**
     * @brief HTTP handler for Receives a request to exit the program.
     * @param req HTTP request received.
     * @param res HTTP response to be sent.
     */
    void handleExit(const httplib::Request &req, httplib::Response &res);

};    
