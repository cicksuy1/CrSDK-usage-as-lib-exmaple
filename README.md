
# CrSDK HTTPS Server

## Project Overview

The CrSDK HTTPS Server is a C++ project designed to provide an HTTP interface for controlling a camera using the SUNY Remote SDK (CrSDK). Clients can send GET requests to the server to trigger various camera operations, including:

- Switching between automatic and manual camera modes.
- Adjusting shutter speed.
- Modifying ISO value.
- Changing autofocus area position.
- Downloading camera settings to a PC.
- Uploading camera settings from a PC.
- Load zoom and focus position.
- Set the F-number index.

**Features**

- **HTTP Control:** Interact with the camera using HTTP GET requests for convenient integration into web applications or automation scripts.
- **SUNY Remote SDK Integration:** Leverages the CrSDK for robust camera control capabilities.
- **Automatic Library Management:** The project's CMakeLists file automatically downloads and compiles required libraries if they are not found.

**Prerequisites**

- A C++ compiler supporting C++11 or later (e.g., GCC, Clang)
- CMake ([https://cmake.org/](https://cmake.org/))

**Installation**

1. **Clone the Project:**
   ```bash
   https://github.com/itzikChaver/CrSDK-HTTPS-Server.git
   ```

2. **Navigate to the Project Directory:**
   ```bash
   cd CrSDK-HTTPS-Server
   ```

3. **Create a Build Directory:**
   ```bash
   mkdir build
   cd build
   ```

4. **Configure the Build (Optional):**
   You can customize the build using CMake flags (refer to CMake documentation for details). Here's an example to build in Release mode:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

5. **Build the Project:**
   ```bash
   make -j12  # Adjust -j argument based on your CPU cores for parallel compilation
   ```

**Running the Server**

1. **Navigate to the Build Directory:**
   ```bash
   cd build
   ```

2. **Run the Server:**
   ```bash
   ./CrSDK_HTTPS_Server  # The executable name might vary depending on your system
   ```
   The server will listen on port 8085

**Using the Server**

Once the server is running, you can send GET requests to the following endpoints:

| Endpoint                                            | Description                                                                       |
|---------------------------------------------        |-----------------------------------------------------------------------------------|
| `/`                                                 | HTTPS handler for getting an indication of the server.
| `/switch_to_p_mode<camera_id>`                      | HTTPS handler for Receives a request to switch the camera to P mode.
| `/switch_to_m_mode<camera_id>`                      | HTTPS handler for Receives a request to switch the camera to M mode.
| `/change_brightness<camera_id><brightness_value>`   | HTTPS handler for Receives a request to change brightness.
| `/change_af_area_position<camera_id><x><y>`         | HTTPS handler for Receives a request to change AF area position.
| `/get_camera_mode<camera_id>`                       | HTTPS handler for Receives a request to get camera mode.
| `/download_camera_setting<camera_id>`               | HTTPS handler for Receives a request to download the camera setting file to PC.
| `/upload_camera_setting<camera_id>`                 | HTTPS handler for Receives a request to upload the camera setting file to Camera.
| `/get_f_number<camera_id>`                          | HTTPS handler for Receives a request to get F-number index.
| `/set_f_number<camera_id><f_number_value>`          | HTTPS handler for Receives a request to set F-number index.
| `/start_cameras`                                    | HTTPS handler for Receives a request to start cameras.
| `/stop_cameras`                                     | HTTPS handler for Receives a request to stop cameras.
| `/restat_cameras`                                   | HTTPS handler for Receives a request to restat cameras.
| `/exit`                                             | HTTPS handler for Receives a request to exit the program.



# API Documentation

## Overview
This document describes the endpoints available in the server for managing camera operations, brightness settings, and other functionalities. Each endpoint supports JSON responses and utilizes HTTP status codes to indicate success or failure.

### Base URL
The base URL for the API is: `http://yourserver.com`

## Endpoints

### 1. Get Server Indicator

**Endpoint**: `/indicator`

**Method**: `GET`

**Description**: Check if the server is running.

**Response**:
- **200 OK**: The server is running.
  ```json
  {
    "message": "The server is running"
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to get indicator.
  ```json
  {
    "error": "Failed to get indicator"
  }
  ```

### 2. Switch Camera to P Mode

**Endpoint**: `/switchTREADME.mdoPMode`

**Method**: `GET`

**Description**: Switch the specified camera to P mode.

**Parameters**:
- **camera_id** (required): The ID of the camera to switch.

**Response**:
- **200 OK**: Successfully switched to P mode.
  ```json
  {
    "message": "Successfully switched to P mode",
    "mode": "P"
  }
  ```
- **400 Bad Request**: Missing or invalid `camera_id`.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to switch to P mode.
  ```json
  {
    "error": "Failed to switch to P mode"
  }
  ```

### 3. Switch Camera to M Mode

**Endpoint**: `/switchToMMode`

**Method**: `GET`

**Description**: Switch the specified camera to M mode.

**Parameters**:
- **camera_id** (required): The ID of the camera to switch.

**Response**:
- **200 OK**: Successfully switched to M mode.
  ```json
  {
    "message": "Successfully switched to M mode",
    "mode": "M"
  }
  ```
- **400 Bad Request**: Missing or invalid `camera_id`.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to switch to M mode.
  ```json
  {
    "error": "Failed to switch to M mode"
  }
  ```

### 4. Change Camera Brightness

**Endpoint**: `/changeBrightness`

**Method**: `GET`

**Description**: Change the brightness of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.
- **brightness_value** (required): The new brightness value (0-48).

**Response**:
- **200 OK**: Successfully changed brightness.
  ```json
  {
    "message": "Successfully changed brightness value"
  }
  ```
- **400 Bad Request**: Missing or invalid parameters.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "the brightness value entered is incorrect."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **405 Method Not Allowed**: Camera is not in M mode.
  ```json
  {
    "error": "Changing the camera brightness is not possible because the camera is not M(manual) mode."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to change brightness.
  ```json
  {
    "error": "Failed to change brightness value"
  }
  ```

### 5. Change AF Area Position

**Endpoint**: `/changeAFAreaPosition`

**Method**: `GET`

**Description**: Change the AF area position of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.
- **x** (required): The new X position (0-639).
- **y** (required): The new Y position (0-479).

**Response**:
- **200 OK**: Successfully changed AF area position.
  ```json
  {
    "message": "Successfully changed AF Area Position"
  }
  ```
- **400 Bad Request**: Missing or invalid parameters.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
  ```json
  {
    "error": "Missing or invalid parameters."
  }
  ```
  ```json
  {
    "error": "The selected X value is out of range."
  }
  ```
  ```json
  {
    "error": "The selected Y value is out of range."
  }
  ```
- **405 Method Not Allowed**: Camera is not in P mode.
  ```json
  {
    "error": "Changing the AF Area Position is not possible because the camera is not P(auto) mode."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to change AF area position.
  ```json
  {
    "error": "Failed to change AF Area Position"
  }
  ```

### 6. Get Camera Mode

**Endpoint**: `/getCameraMode`

**Method**: `GET`

**Description**: Get the mode of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.

**Response**:
- **200 OK**: Successfully retrieved camera mode.
  ```json
  {
    "message": "Successfully retrieved camera mode",
    "mode": "<camera_mode>"
  }
  ```
- **400 Bad Request**: Missing or invalid `camera_id`.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to retrieve camera mode.
  ```json
  {
    "error": "Failed to retrieve camera mode"
  }
  ```

### 7. Download Camera Setting

**Endpoint**: `/downloadCameraSetting`

**Method**: `GET`

**Description**: Download the settings of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.

**Response**:
- **200 OK**: Successfully downloaded camera setting.
  ```json
  {
    "message": "Successfully download camera setting"
  }
  ```
- **400 Bad Request**: Missing or invalid `camera_id`.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to download camera setting.
  ```json
  {
    "error": "Failed to download camera setting"
  }
  ```

### 8. Upload Camera Setting

**Endpoint**: `/uploadCameraSetting`

**Method**: `GET`

**Description**: Upload the settings of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.

**Response**:
- **200 OK**: Successfully uploaded camera setting.
  ```json
  {
    "message": "Successfully upload camera setting"
  }
  ```
- **400 Bad Request**: Missing or invalid `camera_id`.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to upload camera setting.
  ```json
  {
    "error": "Failed to upload camera setting"
  }
  ```

### 9. Get F-number

**Endpoint**: `/getFnumber`

**Method**: `GET`

**Description**: Get the F-number of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.

**Response**:
- **200 OK**: Successfully retrieved F-number.


  ```json
  {
    "message": "Getting the index of the f-number was successful",
    "f-number": "<f-number>"
  }
  ```
- **400 Bad Request**: Missing or invalid `camera_id`.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to retrieve F-number.
  ```json
  {
    "error": "Failed to getting the index of the f-number"
  }
  ```

### 10. Set F-number

**Endpoint**: `/setFnumber`

**Method**: `GET`

**Description**: Set the F-number of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.
- **f_number_value** (required): The new F-number value (0-21).

**Response**:
- **200 OK**: Successfully set F-number.
  ```json
  {
    "message": "Changing the index of the f-number was successful"
  }
  ```
- **400 Bad Request**: Missing or invalid parameters.
  ```json
  {
    "error": "Missing camera_id parameter."
  }
  ```
  ```json
  {
    "error": "Camera_id out of range."
  }
  ```
  ```json
  {
    "error": "Missing required parameters."
  }
  ```
  ```json
  {
    "error": "the F-number value entered is incorrect."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to set F-number.
  ```json
  {
    "error": "Failed to set the index of the f-number"
  }
  ```

### 11. Start Cameras

**Endpoint**: `/startCameras`

**Method**: `GET`

**Description**: Start all cameras.

**Response**:
- **200 OK**: Successfully started cameras.
  ```json
  {
    "message": "The cameras started successfully"
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to start cameras.
  ```json
  {
    "error": "Failed to start cameras"
  }
  ```
  ```json
  {
    "error": "Failed to start cameras, gpio is not active"
  }
  ```

### 12. Stop Cameras

**Endpoint**: `/stopCameras`

**Method**: `GET`

**Description**: Stop all cameras.

**Response**:
- **200 OK**: Successfully stopped cameras.
  ```json
  {"message": "Stopping the cameras was successful."}
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {"error": "Rate limit exceeded"}
  ```
- **500 Internal Server Error**: Failed to stop cameras.
  ```json
  {"error": "Stopping the cameras failed."}
  ```
  ```json
  {"error": "Stopping the cameras failed, gpio is not active"}
  ```

### 13. Restart Cameras

**Endpoint**: `/restartCameras`

**Method**: `GET`

**Description**: Restart all cameras.

**Response**:
- **200 OK**: Successfully restarted cameras.
  ```json
  {"message": "Restarting the cameras was successful."}
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {"error": "Rate limit exceeded"}
  ```
- **500 Internal Server Error**: Failed to restart cameras.
  ```json
  {"error": "Restarting the cameras failed."}
  ```
  ```json
  {"error": "Restarting the cameras failed, gpio is not active."}
  ```

### 14. Exit Program

**Endpoint**: `/exit`

**Method**: `GET`

**Description**: Exit the program.

**Response**:
- **200 OK**: Successfully exited the program.
  ```json
  {"message": "Successfully exit the program"}
  ```
- **500 Internal Server Error**: Failed to exit the program.
  ```json
  {"error": "Failed to exit the program"}
  ```

---

### Notes
- **CORS**: All endpoints support Cross-Origin Resource Sharing (CORS) with the `Access-Control-Allow-Origin` header set to `*` for development purposes. It is recommended to restrict this in production.
- **Rate Limiting**: The server implements rate limiting, returning HTTP 429 status code when the rate limit is exceeded.
- **Error Handling**: The server returns detailed error messages and HTTP status codes to indicate the type of error encountered.

For any questions or issues, please contact the server administrator.
```

This documentation provides an overview of each endpoint, the HTTP method used, a description of the endpoint's functionality, expected parameters, and potential responses with examples. Adjust the base URL and any other details specific to your deployment as needed.

**Note:** Refer to the CrSDK documentation for specific details and limitations regarding camera control functions.

**Additional Notes**

- The project's CMakeLists file manages downloading and compiling dependencies as needed.
- This README provides a basic usage guide. Refer to the project's source code and any included documentation for further details.

