
**CrSDK HTTPS Server**

**Project Overview**

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
| `/exit`                                             | HTTPS handler for Receives a request to exit the program.


**Note:** Refer to the CrSDK documentation for specific details and limitations regarding camera control functions.

**Additional Notes**

- The project's CMakeLists file manages downloading and compiling dependencies as needed.
- This README provides a basic usage guide. Refer to the project's source code and any included documentation for further details.

