/**
 * @file server.h
 * @brief Defines the Server class representing an HTTPS server.
 *
 * The Server class provides basic HTTP server functionality and allows association
 */
#pragma once

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

#include "CrSDK_interface/CrSDK_interface.h"

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
     */
    Server(const std::string &host, int port, const std::string &cert_file, const std::string &key_file);

    /**
     * @brief Start the HTTP server to listen for incoming requests.
     */
    void run();

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
    CrSDKInterface crsdkInterface;                              ///< Add an instance of CrSDKInterface


    /**
     * @brief Set up HTTP routes for the server.
     */
    void setupRoutes();

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

}    
