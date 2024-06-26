# API Documentation

## Overview
This document describes the endpoints available in the server for managing camera operations, brightness settings, and other functionalities. Each endpoint supports JSON responses and utilizes HTTPS status codes to indicate success or failure.

### Base URL

The base URL for the API is determined dynamically based on the following logic:

1. **Configuration File (`config.txt`):**
   - The application first looks for a configuration file named `config.txt`.
   - If the file exists and contains a valid IP address, the server will run on that IP address at port 8085.

2. **ZeroTier IP Address:**
   - If no valid IP address is found in the configuration file (or the file doesn't exist), the application checks for a ZeroTier IP address on the machine.
   - If a ZeroTier IP address is found, the server will run on that address at port 8085.

3. **Localhost (Default):**
   - If neither a valid IP address in the configuration file nor a ZeroTier IP address is found, the server will default to running on the local host (`https://localhost:8085`).

**Example Base URLs:**

* **Using IP from `config.txt`:** `https://192.168.1.100:8085`
* **Using ZeroTier IP:** `https://10.147.17.12:8085`
* **Default (Localhost):** `https://localhost:8085`

**Important Note:**

Ensure that the `config.txt` file is placed in the correct location relative to your application's executable. If you're having trouble, double-check the file path and permissions.

## SSL Certificates and Security

The CrSDK HTTPS Server uses HTTPS to ensure secure communication between the client and the server. To enable HTTPS, you need to provide the server with valid SSL certificates.

### Certificate Requirements

The server requires the following certificate files to be present in the project's root directory:

* **Server Certificate (`jeston-server-embedded.crt`):** This certificate is used to identify the server to clients.
* **Server Private Key (`jeston-server-embedded.key`):** This key is used by the server to decrypt encrypted messages from clients.
* **Client Certificate (`client.crt`):** This certificate is used to authenticate the client to the server.

### Generating Certificates

You can use various tools to generate self-signed certificates for testing purposes. Here are a few popular options:

* **OpenSSL:** A widely used command-line tool for creating SSL certificates.
* **mkcert:** A simple tool for generating locally-trusted development certificates.
* **Let's Encrypt:** A free, automated, and open certificate authority (CA) that provides certificates for production use.

**Note:** For production environments, it's strongly recommended to use certificates issued by a trusted CA rather than self-signed certificates.

### Configuration

The server automatically loads the certificate files from the project's root directory. If you have placed the certificates in a different location, you will need to modify the server's source code to point to the correct file paths.

### Additional Security Considerations

* **Strong Passwords:**  When generating certificates, use strong passwords to protect your private keys.
* **Certificate Expiration:** Keep track of your certificate expiration dates and renew them before they expire.
* **Firewall Configuration:**  Ensure that your firewall allows incoming traffic on port 8085 (or the port you have configured the server to use).

**Disclaimer:** This project is intended for development and testing purposes. For production environments, we recommend consulting with a security expert to ensure that your HTTPS implementation meets industry best practices.

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

### 5. Get Camera Brightness

**Endpoint**: `/get_camera_brightness`

**Method**: `GET`

**Description**: Get the brightness value of the specified camera.

**Parameters**:
- **camera_id** (required): The ID of the camera.

**Response**:
- **200 OK**: Successfully retrieved brightness.
  ```json
  {
    "message": "Successfully retrieved camera brightness"
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
- **405 Method Not Allowed**: Camera is not in M mode.
  ```json
  {
    "error": "Geting the camera brightness is not possible because the camera is not M mode."
  }
  ```
- **429 Too Many Requests**: Rate limit exceeded.
  ```json
  {
    "error": "Rate limit exceeded"
  }
  ```
- **500 Internal Server Error**: Failed to get brightness.
  ```json
  {
    "error": "Failed to retrieve camera brightness"
  }
  ```

### 6. Change AF Area Position

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

### 7. Get Camera Mode

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

### 8. Download Camera Setting

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

### 9. Upload Camera Setting

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

### 1. Get F-number

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

### 11. Set F-number

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

### 12. Start Cameras

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

### 13. Stop Cameras

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

### 14. Restart Cameras

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

### 15. Exit Program

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

