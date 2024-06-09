/**
 * @file server.cpp
 * @brief Implementation of the Server class for an HTTP server with optional RTSP streaming capabilities.
 */

#include "https_server.h"
#include <spdlog/spdlog.h>
#include <fmt/format.h>

std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

int get_pid_using_port(int port)
{
    std::string command = "lsof -t -i:" + std::to_string(port);
    std::string output = exec(command.c_str());
    if (!output.empty())
    {
        return std::stoi(output); // Assuming the output is the PID
    }
    return -1; // Indicate that no process was found
}

bool terminate_process(int pid)
{
    if (pid > 0)
    {
        int status = kill(pid, SIGTERM); // Send the SIGTERM signal
        if (status == 0)
        {
            spdlog::info("Successfully terminated process with PID {}", pid);
            return true;
        }
        else
        {
            spdlog::error("Failed to terminate process with PID {}", pid);
            return false;
        }
    }
    spdlog::info("No process to terminate.");
    return false;
}

bool is_port_available(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        spdlog::error("Failed to create socket");
        return false;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bool available = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0;
    close(sockfd);
    return available;
}

Server::Server(const std::string &host, int port, const std::string &cert_file, const std::string &key_file, std::atomic<bool> &stopRequested, CrSDKInterface *crsdkInterface)
    : server(cert_file.c_str(), key_file.c_str()), host_(host), port_(port), stopRequested(stopRequested), crsdkInterface_(crsdkInterface)
{
    setupRoutes();

    // Initialize token bucket with maxTokens and refillRate parameters
    initializeTokenBucket(/* maxTokens */ 3, /* refillRate */ 1, /* refillNumber */ 3); // Adjust these values as needed
}

Server::Server(const std::string &host, int port, const std::string &cert_file, const std::string &key_file, std::atomic<bool> &stopRequested, GpioPin *gpioP, CrSDKInterface *crsdkInterface)
    : server(cert_file.c_str(), key_file.c_str()), host_(host), port_(port), stopRequested(stopRequested), gpioPin(gpioP), crsdkInterface_(crsdkInterface)
{
    setupRoutes();

    // Initialize token bucket with maxTokens and refillRate parameters
    initializeTokenBucket(/* maxTokens */ 3, /* refillRate */ 1, /* refillNumber */ 3); // Adjust these values as needed
}

void Server::setupRoutes()
{
    server.Get("/", [this](const httplib::Request &req, httplib::Response &res)
               { handleIndicator(req, res); });

    server.Get("/switch_to_p_mode", [this](const httplib::Request &req, httplib::Response &res)
               { handleSwitchToPMode(req, res); });

    server.Get("/switch_to_m_mode", [this](const httplib::Request &req, httplib::Response &res)
               { handleSwitchToMMode(req, res); });

    server.Get("/change_brightness", [this](const httplib::Request &req, httplib::Response &res)
               { handleChangeBrightness(req, res); });

    server.Get("/change_af_area_position", [this](const httplib::Request &req, httplib::Response &res)
               { handleChangeAFAreaPosition(req, res); });

    server.Get("/get_camera_mode", [this](const httplib::Request &req, httplib::Response &res)
               { handleGetCameraMode(req, res); });

    server.Get("/get_camera_brightness", [this](const httplib::Request &req, httplib::Response &res)
               { handleGetCameraBrightness(req, res); });

    server.Get("/download_camera_setting", [this](const httplib::Request &req, httplib::Response &res)
               { handleDownloadCameraSetting(req, res); });

    server.Get("/upload_camera_setting", [this](const httplib::Request &req, httplib::Response &res)
               { handleUploadCameraSetting(req, res); });

    server.Get("/get_f_number", [this](const httplib::Request &req, httplib::Response &res)
               { handleGetFnumber(req, res); });

    server.Get("/set_f_number", [this](const httplib::Request &req, httplib::Response &res)
               { handleSetFnumber(req, res); });

    server.Get("/start_cameras", [this](const httplib::Request &req, httplib::Response &res)
               { handleStartCameras(req, res); });

    server.Get("/stop_cameras", [this](const httplib::Request &req, httplib::Response &res)
               { handleStopCameras(req, res); });

    server.Get("/restat_cameras", [this](const httplib::Request &req, httplib::Response &res)
               { handleRestatCameras(req, res); });

    server.Get("/exit", [this](const httplib::Request &req, httplib::Response &res)
               { handleExit(req, res); });
}

void Server::setGpioPin(GpioPin *gpioPin)
{
    this->gpioPin = gpioPin;
}

void Server::run()
{
    try
    {
        // // Check if the port is available
        int pid = get_pid_using_port(port_);
        if (pid != -1)
        {
            spdlog::info("Port {} is already in use by PID {}. Attempting to terminate.", port_, pid);
            if (!terminate_process(pid))
            {
                spdlog::error("Could not terminate existing process. Server startup aborted.");
                return;
            }
        }

        // Print a message to the console indicating the server address and port
        spdlog::info("The server runs at address: {}:{}", host_, port_);
        // Start the monitoring thread
        // startMonitoringThread();

        while (!stopRequested.load())
        {
            // Start listening for incoming requests on the specified host and port
            server.listen(host_, port_);
        }
    }
    catch (const std::exception &e)
    {
        // Handle the server error and generate an error message
        spdlog::error("Server Error: {}", e.what());
    }

    // Wait for the monitoring thread to finish (optional)
    if (monitoringThread.joinable())
    {
        monitoringThread.join();
    }
}

void Server::initializeTokenBucket(int maxTokens, int refillRate, int refillNumber)
{
    try
    {
        maxTokens_ = maxTokens;
        currentTokens_ = maxTokens;

        // Start a thread to refill tokens at the specified rate
        std::thread([this, refillRate, refillNumber]()
        {
            while (true) 
            {
                try 
                {
                    refillTokens(refillNumber);
                } 
                catch (const std::exception& e) 
                {
                    spdlog::error("Error in refillTokens: {}", e.what());
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 * refillRate)); 
            } 
        }).detach();
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Error occurred during token bucket initialization: {}", e.what());
    }
}

bool Server::consumeToken()
{
    try
    {
        std::lock_guard<std::mutex> lock(tokenMutex_);
        if (currentTokens_ > 0)
        {
            currentTokens_--;
            return true;
        }
        return false;
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Error occurred during token consumption: {}", e.what());
        return false; // Return an error value or handle it according to your application's logic
    }
}

void Server::refillTokens(int refillNumber) 
{
    try 
    {
        std::lock_guard<std::mutex> lock(tokenMutex_);
        currentTokens_ = std::min(maxTokens_, currentTokens_ + refillNumber); 
    } 
    catch (const std::exception& e) 
    {
        spdlog::error("Error occurred during token refill: {}", e.what());
    }
}

bool Server::stopServer()
{
    try
    {
        spdlog::info("stop server...");
        server.stop();
        stopRequested.store(true); // Set the flag to stop
        spdlog::info("The server stopped successfully.");
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::error("An error occurred while trying to stop the server: {}", e.what());
        return false;
    }
}

void Server::startMonitoringThread()
{

    // std::this_thread::sleep_for(std::chrono::seconds(0));
    monitoringThread = std::thread([this]()
                                   {

        while (true) {
            try {
                // Create a new HTTP client with SSL and specify the CA certificate
                httplib::SSLClient cli(host_, port_); // host, port

                // Use your CA bundle
                cli.set_ca_cert_path("/jetson_ssl/client.crt");

                // Disable cert verification
                cli.enable_server_certificate_verification(false);

                // Perform a simple GET request
                httplib::Result result = cli.Get("/");

                if (!result) { // Check if the request failed
                    // Handle the error case (request failed)
                    spdlog::error("Error sending GET request");
                    
                    // Check for connection-related errors
                    if (result.error() == httplib::Error::Connection ||
                        result.error() == httplib::Error::ConnectionTimeout ||
                        result.error() == httplib::Error::ProxyConnection) {
                        spdlog::error("Connection error! Restarting server...");
                        restartServer(); // Call your existing restartServer function
                    } else {
                        // Handle other errors
                        // Consider retrying or taking appropriate action
                    }
                } else {
                    // Access the response object using a temporary variable
                    httplib::Response response = result.value();

                    // Use the response object here (check status code, body etc.)
                    bool isServerRunning = (response.status == 200);

                    if (!isServerRunning) {
                        spdlog::error("Server crashed. Restarting...");
                        restartServer();
                    } else {
                        spdlog::info("The server running");
                    }
                }

            }
            catch (const std::exception &e) { // **Capture 'e' by reference**
                // Handle the exception and determine if it indicates a server crash
                spdlog::error("Unexpected error in monitoring thread: {}", e.what());
            }

            // Adjust sleep duration based on your needs
            std::this_thread::sleep_for(std::chrono::seconds(60));
            } });
}

void Server::restartServer()
{
    // Wait some time before restarting (to prevent flooding errors)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Restart the server by creating a new instance
    server.listen(host_, port_);

    spdlog::info("Server initialization succeeded");
}

void Server::handleIndicator(const httplib::Request &req, httplib::Response &res)
{
    // Create an empty JSON object
    nlohmann::json response_json = {};

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // Adjust for production

        if (consumeToken())
        {
            response_json["message"] = "The server is running";
            res.status = 200; // OK
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        spdlog::error("Indicator Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to get indicator";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleSwitchToPMode(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    nlohmann::json response_json = {};
    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        // Check if the query parameter 'camera_id' is present
        auto camera_id_param = req.get_param_value("camera_id");

        if (consumeToken())
        {
            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // switch to P mode logic...
            bool success = false;
            if (crsdkInterface_)
            {
                spdlog::info("switch to P mode...");
                success = crsdkInterface_->switchToPMode(camera_id);
            }
            else
            {
                spdlog::error("ERROR: crsdkInterface_ is nullptr");
            }

            if (success)
            {
                // Success message
                spdlog::info("Changing the camera {} mode to P mode was successful", camera_id);
                response_json["message"] = "Successfully switched to P mode";
                response_json["mode"] = std::string(1, 'a' - 32);
                res.status = 200; // OK
            }
            else
            {
                // Error message
                spdlog::error("Failed to change camera mode to P mode");
                response_json["error"] = "Failed to switch to P mode";
                res.status = 500; // Internal Server Error
            }
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Switch to P mode Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to switch to P mode";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleSwitchToMMode(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // switch to M mode logic...
            bool success = false;
            if (crsdkInterface_)
            {
                spdlog::info("switch to M mode...");
                success = crsdkInterface_->switchToMMode(camera_id);
            }
            else
            {
                spdlog::error("ERROR: crsdkInterface_ is nullptr");
            }

            if (success)
            {
                // Success message
                spdlog::info("Changing the camera {} mode to M mode was successful", camera_id);
                response_json["message"] = "Successfully switched to M mode";
                response_json["mode"] = std::string(1, 'm' - 32);
                res.status = 200; // OK
            }
            else
            {

                spdlog::error("Failed to change camera mode to M mode");

                spdlog::info("Returns the camera to P mode...");
                success = crsdkInterface_->switchToPMode(camera_id);
                if (success)
                {
                    // Success message
                    spdlog::info("Changing the camera {} mode back to P mode was successful", camera_id);
                    response_json["message"] = "Successfully switched back to P mode";
                    response_json["mode"] = std::string(1, 'a' - 32);
                    res.status = 200; // OK
                }
                else
                {
                    // Error message
                    response_json["error"] = "Failed to switch to M mode";
                    res.status = 500; // Internal Server Error
                }
            }
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Switch to M mode Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to switch to M mode";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");    
    }
}

void Server::handleChangeBrightness(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object.
    json response_json;

    try
    {
        // Enable CORS.
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production.

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present.
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Checking whether the camera is in manual mode
            if (crsdkInterface_->cameraModes[camera_id] != "m")
            {
                // Handling camera mode is not M.
                spdlog::error("Changing the camera {} brightness is not possible because the camera is not M(manual) mode", camera_id);
                response_json["error"] = "Changing the camera brightness is not possible because the camera is not M(manual) mode.";
                res.status = 405; // Method not allowed.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Check if the query parameters brightness value are present.
            auto brightness_value_param = req.get_param_value("brightness_value");

            if (brightness_value_param.empty())
            {
                // Handle missing parameters
                response_json["error"] = "Missing required parameters.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }
            else
            {
                // Convert the parameters to numbers.
                int brightnessValue = std::stoi(brightness_value_param);

                // Treatment in case the brightness value entered is incorrect.
                if (brightnessValue < 0 || brightnessValue > 48)
                {
                    // Error message
                    response_json["error"] = "the brightness value entered is incorrect.";
                    res.status = 405; // Method not allowed.
                }
                else
                {
                    // change the brightness value logic...
                    bool success = crsdkInterface_->changeBrightness(camera_id, brightnessValue);

                    if (success)
                    {
                        // Success message
                        response_json["message"] = "Successfully changed brightness value";
                        res.status = 200; // OK
                    }
                    else
                    {
                        // Error message
                        response_json["error"] = "Failed to change brightness value";
                        res.status = 500; // Internal Server Error
                    }
                }
            }
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Change brightness Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to change brightness value";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleChangeAFAreaPosition(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON.
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            if (crsdkInterface_->cameraModes[camera_id] != "p")
            {
                // Handling camera mode is not P.
                spdlog::error("Changing the AF Area Position is not possible because the camera is not P(auto) mode");
                response_json["error"] = "Changing the AF Area Position is not possible because the camera is not P(auto) mode.";
                res.status = 405; // Method not allowed.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Check if the query parameters 'x', 'y' are present
            auto x_param = req.get_param_value("x");
            auto y_param = req.get_param_value("y");

            if (x_param.empty() || y_param.empty())
            {
                // Missing or invalid parameters
                res.status = 400; // Bad Request.
                response_json["error"] = "Missing or invalid parameters.";

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }
            else
            {
                // Convert the parameters to numbers
                int x = std::stoi(x_param);
                int y = std::stoi(y_param);

                if (x < 0 || x > 639)
                {
                    // Error message
                    spdlog::error("Error: The selected X value is out of range");
                    response_json["error"] = "The selected X value is out of range.";
                    res.status = 405; // Method not allowed.

                    // Set the response content type to JSON
                    res.set_content(response_json.dump(), "application/json");
                    return;
                }

                if (y < 0 || y > 479)
                {
                    // Error message
                    spdlog::error("Error: The selected Y value is out of range");
                    response_json["error"] = "The selected Y value is out of range.";
                    res.status = 405; // Method not allowed.

                    // Set the response content type to JSON
                    res.set_content(response_json.dump(), "application/json");
                    return;
                }

                // change the AF Area Position logic...
                bool success = crsdkInterface_->changeAFAreaPosition(camera_id, x, y);

                if (success)
                {
                    // Success message
                    response_json["message"] = "Successfully changed AF Area Position";
                    res.status = 200; // OK
                }
                else
                {
                    // Error message
                    response_json["error"] = "Failed to change AF Area Position";
                    res.status = 500; // Internal Server Error
                }
            }           
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Change AF Area Position Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to change AF Area Position";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleGetCameraMode(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // get camera mode logic...
            bool success = crsdkInterface_->getCameraMode(camera_id);

            if (success)
            {
                // Success message
                response_json["message"] = "Successfully retrieved camera mode";
                response_json["mode"] =  crsdkInterface_->getCameraModeStr(camera_id);
                res.status = 200; // OK
            }
            else
            {
                // Error message
                response_json["error"] = "Failed to retrieve camera mode";
                res.status = 500; // Internal Server Error
            }           
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Get camera mode Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to retrieve camera mode";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleGetCameraBrightness(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Checking whether the camera is in manual mode
            if (crsdkInterface_->cameraModes[camera_id] != "m")
            {
                // Handling camera mode is not M.
                spdlog::error("Geting the camera {} brightness value is not possible because the camera is not M mode", camera_id);
                response_json["error"] = "Geting the camera brightness value is not possible because the camera is not M mode.";
                res.status = 405; // Method not allowed.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // get camera brightness logic...
            int brightness = crsdkInterface_->getCameraBrightness(camera_id);

            if (brightness != -1)
            {
                // Success message
                response_json["message"] = "Successfully retrieved camera brightness";
                response_json["brightness value"] =  brightness;
                res.status = 200; // OK
            }
            else
            {
                // Error message
                response_json["error"] = "Failed to retrieve camera brightness";
                res.status = 500; // Internal Server Error
            }           
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Get camera brightness Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to retrieve camera brightness";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleDownloadCameraSetting(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Download camera setting logic...
            bool success = crsdkInterface_->downloadCameraSetting(camera_id);

            if (success)
            {
                // Success message
                response_json["message"] = "Successfully download camera setting";
                res.status = 200; // OK
            }
            else
            {
                // Error message
                response_json["error"] = "Failed to download camera setting";
                res.status = 500; // Internal Server Error
            }            
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Download camera setting Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to download camera setting";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleUploadCameraSetting(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // upload camera setting logic...
            bool success = crsdkInterface_->uploadCameraSetting(camera_id);

            if (success)
            {
                // Success message
                response_json["message"] = "Successfully upload camera setting";
                res.status = 200; // OK
            }
            else
            {
                // Error message
                response_json["error"] = "Failed to upload camera setting";
                res.status = 500; // Internal Server Error
            }           
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Upload camera setting Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to upload camera setting";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleGetFnumber(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Get F-number setting logic...
            std::string Fnumber = crsdkInterface_->getFnumber(camera_id);

            if (!Fnumber.empty())
            {
                // Success message
                response_json["message"] = "Geting the index of the f-number was successful";
                response_json["f-number"] = Fnumber;
                res.status = 200; // OK
            }
            else
            {
                // Error message
                response_json["error"] = "Failed to geting the index of the f-number";
                res.status = 500; // Internal Server Error
            }
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }
        
        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Get F-number Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to geting the index of the f-number";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleSetFnumber(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            // Check if the query parameter 'camera_id' is present
            auto camera_id_param = req.get_param_value("camera_id");

            if (camera_id_param.empty())
            {
                // Handle missing camera_id.
                response_json["error"] = "Missing camera_id parameter.";
                res.status = 400; // Bad Request.

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            int camera_id = std::stoi(camera_id_param);

            // Use the macro to get the reversed index
            camera_id = REVERSE_INDEX(camera_id);

            if (camera_id < 0 || camera_id >= crsdkInterface_->cameraList.size())
            {
                // Handling camera_id out of range
                response_json["error"] = "Camera_id out of range.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Check if the query parameters F-number value are present.
            auto f_number_value_param = req.get_param_value("f_number_value");

            if (f_number_value_param.empty())
            {
                // Handle missing parameters
                response_json["error"] = "Missing required parameters.";
                res.status = 400; // Bad Request

                // Set the response content type to JSON
                res.set_content(response_json.dump(), "application/json");
                return;
            }

            // Convert the parameters to numbers.
            int fNumberValue = std::stoi(f_number_value_param);

            // Treatment in case the F-number value entered is incorrect.
            if (fNumberValue < 0 || fNumberValue > 21)
            {
                // Error message
                response_json["error"] = "the F-number value entered is incorrect.";
                res.status = 405; // Method not allowed.
            }
            else
            {
                // change the F-number value logic...
                bool success = crsdkInterface_->setFnumber(camera_id, fNumberValue);

                if (success)
                {
                    // Success message
                    response_json["message"] = "Changing the index of the f-number was successful";
                    res.status = 200; // OK
                }
                else
                {
                    // Error message
                    response_json["error"] = "Failed to change the index of the f-number";
                    res.status = 500; // Internal Server Error
                }
            }           
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Set F-number Route Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to seting the index of the f-number";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleStartCameras(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            if (gpioPin != nullptr)
            {
                bool success = gpioPin->pinOff();
                if (success)
                {
                    response_json["message"] = "The cameras started successfully";
                    res.status = 200; // OK
                }
                else
                {
                    response_json["error"] = "Failed to start cameras";
                    res.status = 500; // Internal Server Error
                }
            }
            else
            {
                response_json["error"] = "Failed to start cameras, gpio is not active";
                res.status = 500; // Internal Server Error
            }            
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Error occurred in the start cameras route: {}", e.what());

        // Error message
        response_json["error"] = "Failed to start the cameras";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleStopCameras(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            if (gpioPin != nullptr)
            {
                bool success = gpioPin->pinOn();
                if (success)
                {
                    response_json["message"] = "Stopping the cameras was successful.";
                    res.status = 200; // OK
                }
                else
                {
                    response_json["error"] = "Stopping the cameras failed.";
                    res.status = 500; // Internal Server Error
                }
            }
            else
            {
                response_json["error"] = "Stopping the cameras failed, gpio is not active";
                res.status = 500; // Internal Server Error
            }            
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Error occurred in the stop cameras route: {}", e.what());

        // Error message
        response_json["error"] = "Failed to stop the cameras";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleRestatCameras(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        if (consumeToken())
        {
            if (gpioPin != nullptr)
            {
                bool success = gpioPin->restat();
                if (success)
                {
                    response_json["message"] = "Restarting the cameras was successful.";
                    res.status = 200; // OK
                }
                else
                {
                    response_json["error"] = "Restarting the cameras failed.";
                    res.status = 500; // Internal Server Error
                }
            }
            else
            {
                response_json["error"] = "Restarting the cameras failed, gpio is not active.";
                res.status = 500; // Internal Server Error
            }           
        }
        else
        {
            response_json["error"] = "Rate limit exceeded";
            res.status = 429; // HTTP 429 Too Many Requests
        }

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Error occurred in the restat cameras route: {}", e.what());

        // Error message
        response_json["error"] = "Failed to restat the cameras";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}

void Server::handleExit(const httplib::Request &req, httplib::Response &res)
{
    // Create a JSON object
    json response_json;

    try
    {
        // Enable CORS
        res.set_header("Access-Control-Allow-Origin", "*"); // You might want to restrict this in production

        spdlog::info("Program stopped...");
        stopRequested.store(true);

        // Success message
        response_json["message"] = "Successfully exit the program";
        res.status = 200; // OK

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        // Handle the exception and generate an error message
        spdlog::error("Exit the program Error: {}", e.what());

        // Error message
        response_json["error"] = "Failed to exit the program";
        res.status = 500; // Internal Server Error

        // Set the response content type to JSON
        res.set_content(response_json.dump(), "application/json");
    }
}
