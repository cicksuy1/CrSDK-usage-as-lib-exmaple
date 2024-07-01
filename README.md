
# CrSDK HTTPS Server

## Table of Contents

- [Project Overview](#project-overview)
- [Features](#features)
- [Dependencies](#dependencies)
- [Installation](#installation)
    - [Prerequisites](#prerequisites)
    - [Building from Source](#building-from-source)
- [Configuration](#configuration)
- [Running the Server](#running-the-server)
- [API Endpoints](#api-endpoints)
- [SSL Certificates and Security](#ssl-certificates-and-security)
- [Examples](#examples)
- [Docker Containerization](#docker-containerization)
- [Contributing](#contributing)


## Project Overview

The CrSDK HTTPS Server provides a flexible and easy-to-use HTTPS interface for controlling SONY cameras remotely. By leveraging the SONY Remote SDK (CrSDK), this server allows users to trigger various camera operations through simple GET requests. This makes it an ideal solution for web applications, automation scripts, and remote camera control systems.

## Features

- **Intuitive HTTP API:**  Easily control camera functions with standard HTTP GET requests.
- **SONY Remote SDK Integration:**  Robust camera control capabilities thanks to CrSDK.
- **Dynamic IP Resolution:** Automatically determines the server's IP address based on configuration or ZeroTier.
- **Comprehensive Camera Control:** Supports a wide range of camera operations, including mode switching, exposure settings, focus adjustment, and more.
- **Customizable Configuration:** Tailor the server's behavior using a `config.txt` file.

## Dependencies

The CrSDK HTTPS Server relies on the following libraries:

* **httplib:** A header-only HTTP/HTTPS server and client library for C++.
    - **Source:** [https://github.com/yhirose/cpp-httplib.git](https://github.com/yhirose/cpp-httplib.git)
    - **Installation:** The project's CMakeLists file will automatically download and compile httplib if it's not found on your system.

* **spdlog:** A fast C++ logging library.
    - **Source:** [https://github.com/gabime/spdlog.git](https://github.com/gabime/spdlog.git)
    - **Installation:** The project's CMakeLists file will automatically download and compile spdlog if it's not found on your system.

* **fmt:** A modern formatting library for C++.
    - **Installation (Ubuntu/Debian):** `sudo apt-get install -y libfmt-dev`
    - **Other Systems:** See the fmt documentation for installation instructions on other platforms.

* **JetsonGPIO:** A C++ library for controlling the GPIO pins on NVIDIA Jetson boards.
    - **Source:** [https://github.com/pjueon/JetsonGPIO.git](https://github.com/pjueon/JetsonGPIO.git)
    - **Installation:** The project's CMakeLists file will automatically download and compile JetsonGPIO if it's not found on your system.  Note that this library is specific to Jetson boards.


## Installation

### Prerequisites

- **C++ Compiler:** A C++ compiler supporting C++11 or later (e.g., GCC, Clang)
- **CMake:** The cross-platform build system generator (version 3.10 or higher)
- **SONY Remote SDK (CrSDK):** Download and install the CrSDK for your platform from the SONY developer resources.

### Building from Source

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/itzikChaver/CrSDK-HTTPS-Server.git
   cd CrSDK-HTTPS-Server
   ```

2. **Create and Navigate to the Build Directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure and Build with CMake:**
   ```bash
   cmake ..  
   make -j12 
   ```

## Configuration

The server's behavior can be customized using a `config.txt` file in the project's root directory. This file can contain:

- **IP Address:** Specify a fixed IP address for the server.
- **Other Settings:** You can add more configuration options in the future.

If the `config.txt` file is not present or doesn't contain a valid IP address, the server will automatically try to use the machine's ZeroTier IP address (if available) or default to localhost.

## Running the Server

1. **Make sure the `config.txt` file (if used) is in the project root directory.**

2. **Execute the Server:**
   ```bash
   ./CrSDK_HTTPS_Server
   ```

The server will start and output the actual IP address and port it's listening on.

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
| `/get_camera_brightness<camera_id>`                 | HTTPS handler for Receives a request to get camera brightness.
| `/download_camera_setting<camera_id>`               | HTTPS handler for Receives a request to download the camera setting file to PC.
| `/upload_camera_setting<camera_id>`                 | HTTPS handler for Receives a request to upload the camera setting file to Camera.
| `/get_f_number<camera_id>`                          | HTTPS handler for Receives a request to get F-number index.
| `/set_f_number<camera_id><f_number_value>`          | HTTPS handler for Receives a request to set F-number index.
| `/start_cameras`                                    | HTTPS handler for Receives a request to start cameras.
| `/stop_cameras`                                     | HTTPS handler for Receives a request to stop cameras.
| `/restat_cameras`                                   | HTTPS handler for Receives a request to restat cameras.
| `/exit`                                             | HTTPS handler for Receives a request to exit the program.



## SSL Certificates and Security

The CrSDK HTTPS Server prioritizes secure communication using HTTPS. To enable this, you need to provide the server with valid SSL certificates.

### Certificate Requirements

The server requires the following certificate files in the project's root directory:

* **Server Certificate (`jeston-server-embedded.crt`):** Identifies the server to clients.
* **Server Private Key (`jeston-server-embedded.key`):** Used by the server to decrypt encrypted messages.
* **Client Certificate (`client.crt`):** Authenticates the client to the server.

### Generating Certificates

You can generate self-signed certificates for testing using tools like:

* **OpenSSL:** A versatile command-line tool.
* **mkcert:** Simplifies generating locally-trusted certificates.
* **Let's Encrypt:** A free, automated CA for production-grade certificates.

**Important Note:** For production use, obtain certificates from a trusted CA, not self-signed ones.

### Configuration

By default, the server looks for certificates in the project's root. You can change this in the source code if needed.

**Fallback to HTTP**

If **any** of the required certificate files are missing, the server will automatically fall back to running in HTTP mode (unencrypted) on the same port. This is primarily for development and testing convenience, as HTTPS is strongly recommended for security in production environments.

### Additional Security Considerations

* **Strong Passwords:** Use strong passwords when generating private keys.
* **Certificate Expiration:** Monitor and renew certificates before they expire.
* **Firewall:** Allow incoming traffic on the server's port (default 8085 or your custom configuration).

**Disclaimer:** This project is intended for development and testing. For production, consult a security expert to ensure your HTTPS setup is robust.

## Examples (using curl)

```bash
curl "http://<server_ip_address>:8085/switch_to_p_mode?camera_id=1"
```

## Docker Containerization

Docker offers a convenient way to package and distribute your CrSDK HTTPS Server application, ensuring a consistent and isolated environment for running the server. This section provides instructions on how to build a Docker image for the server and run it in a container.

### Prerequisites

- **Docker:** Make sure you have Docker installed on your system. You can download and install it from the official Docker website: [https://www.docker.com/get-started](https://www.docker.com/get-started)

### Building the Docker Image

1. **Navigate to Project Root:** Open a terminal and navigate to the root directory of the CrSDK HTTPS Server project.

2. **Create a Dockerfile:** Create a file named `Dockerfile` (without any file extension) in the project's root directory.

3. **Dockerfile Content:** Paste the following content into the `Dockerfile`:

```Dockerfile
# Stage 1: Build Environment
FROM ubuntu:20.04 as builder

# Install dependencies
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata \
    build-essential \
    cmake \
    git \
    wget \
    libfmt-dev \
    libevent-dev \
    curl \
    libxml2-dev

# Set working directory
WORKDIR /app

# Copy source code
COPY . /app

# Install OpenSSL from source
RUN curl -sSL https://www.openssl.org/source/openssl-3.0.7.tar.gz -o openssl.tar.gz && \
    tar -xzvf openssl.tar.gz && \
    cd openssl-3.0.7 && \
    ./config --prefix=/usr/local/openssl --openssldir=/usr/local/openssl && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf openssl-3.0.7 openssl.tar.gz

# Ensure the new OpenSSL version is used
ENV PATH="/usr/local/openssl/bin:$PATH"
ENV LD_LIBRARY_PATH="/usr/local/openssl/lib:$LD_LIBRARY_PATH"
ENV PKG_CONFIG_PATH="/usr/local/openssl/lib/pkgconfig:$PKG_CONFIG_PATH"
ENV OPENSSL_ROOT_DIR="/usr/local/openssl"

# Download and install SonySDK
RUN if [ ! -d "/app/include/SonySDK" ]; then \
                wget -O SonySDK.tar.gz "https://developerworld.wpp.developer.sony.com/file/download/sony-camera-remote-api-beta-sdk-2/Download/index.html" && \
                tar -xzf SonySDK.tar.gz -C /app/include && \
                rm SonySDK.tar.gz; \
            fi
    
# Build httplib (if not found)
RUN if [ ! -d "/usr/local/include/httplib" ]; then \
                git clone https://github.com/yhirose/cpp-httplib.git /app/deps/httplib && \
                cd /app/deps/httplib && \
                mkdir build && cd build && \
                cmake .. && make -j12 && make install; \
            fi

# Build libevent (if not found)
RUN if [ ! -d "/usr/local/include/libevent" ]; then \
                git clone https://github.com/libevent/libevent.git /app/deps/libevent && \
                cd /app/deps/libevent && \
                mkdir build && cd build && \
                cmake .. && make -j12 && make install; \
            fi

# Build spdlog (if not found)
RUN if [ ! -d "/usr/local/include/spdlog" ]; then \
                git clone https://github.com/gabime/spdlog.git /app/deps/spdlog && \
                cd /app/deps/spdlog && \
                mkdir build && cd build && \
                cmake .. && make -j12 && make install; \
            fi

# Build JetsonGPIO (if not found)
RUN if [ ! -d "/usr/local/include/JetsonGPIO" ]; then \
                git clone https://github.com/pjueon/JetsonGPIO.git /app/deps/JetsonGPIO && \
                cd /app/deps/JetsonGPIO && \
                mkdir build && cd build && \
                cmake .. && make -j12 && make install; \
            fi

# Copy additional required files
COPY include/SonySDK_files/libmonitor_protocol_pf.so /usr/local/lib/
COPY include/SonySDK_files/libmonitor_protocol.so /usr/local/lib/
COPY include/SonySDK_files/libCr_Core.so /usr/local/lib/
COPY include/SonySDK_files/CrAdapter /usr/local/lib/CrAdapter

# Build your project
RUN if [ ! -d "build" ]; then mkdir build; fi && cd build && \
    cmake .. && \
    make

# Stage 2: Runtime Environment (Optional, for smaller image size)
FROM ubuntu:20.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y libxml2

# Copy the built application from the builder stage
COPY --from=builder /app/build/CrSDK_HTTPS_Server /usr/local/bin/

# Copy necessary libraries from the builder stage
COPY --from=builder /usr/local/lib/libJetsonGPIO.so.1 /usr/local/lib/
COPY --from=builder /usr/local/lib/libmonitor_protocol_pf.so /usr/local/lib/
COPY --from=builder /usr/local/lib/libmonitor_protocol.so /usr/local/lib/
COPY --from=builder /usr/local/lib/libCr_Core.so /usr/local/lib/
COPY --from=builder /usr/local/lib/CrAdapter /usr/local/lib/CrAdapter

# Ensure the new OpenSSL version is used
COPY --from=builder /usr/local/openssl /usr/local/openssl
ENV PATH="/usr/local/openssl/bin:$PATH"
ENV LD_LIBRARY_PATH="/usr/local/openssl/lib:$LD_LIBRARY_PATH"
ENV PKG_CONFIG_PATH="/usr/local/openssl/lib/pkgconfig:$PKG_CONFIG_PATH"
ENV OPENSSL_ROOT_DIR="/usr/local/openssl"

# Set library path
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

CMD ["CrSDK_HTTPS_Server"]
```

4. **Build the Image:** Run the following command in the terminal:

```bash
docker build -t crsdk-server .
```

This command will build a Docker image named `crsdk-server`. The `.` at the end indicates that the Dockerfile is in the current directory.

### Running the Server in a Docker Container

After building the Docker image, you can run the server in a container using the following command:

```bash
docker run -p 8085:8085 crsdk-server
```

- **`-p 8085:8085`:**  This option maps port 8085 inside the container to port 8085 on your host machine. This allows you to access the server running inside the container from your web browser or other applications.

The server is now accessible via HTTPS at https://<host>:8085, provided the necessary SSL/TLS certificate files are in place. For testing or development environments without certificates, you can also access the server via HTTP at http://<host>:8085.

### Transferring and Running the Docker Image on Another Machine

After you have successfully built the Docker image on your source machine, you can transfer it to another machine and run it there. This process involves saving the image, transferring the saved file, and loading the image on the target machine.

1. **Save the Docker Image:**
   
   ```bash
   docker save crsdk-server -o crsdk-server.tar
   ```

   This command saves the `crsdk-server` image as a `.tar` archive file named `crsdk-server.tar`.

2. **Transfer the Image:**
   
   Transfer the `crsdk-server.tar` file to the target machine using any suitable method, such as a USB drive, network share, or cloud storage service.

3. **Load the Image on the Target Machine:**
   
   On the target machine, open a terminal and navigate to the directory where you transferred the `crsdk-server.tar` file. Then, run the following command:

   ```bash
   docker load -i crsdk-server.tar
   ```

   This command will load the Docker image from the archive file.

4. **Run the Container:**

   Finally, run the Docker container using the same command you used on the source machine:

   ```bash
   docker run -p 8085:8085 crsdk-server
   ```

   The server should now be running on the target machine and accessible via HTTP/HTTPS on the mapped ports.

**Additional Considerations:**

* **Environment Variables:** If your application relies on environment variables, make sure to set them on the target machine before running the container.
* **Certificates:** If you're using HTTPS, ensure that the necessary SSL/TLS certificate files are available in the correct location on the target machine.

By following these steps, you can easily transfer and run your CrSDK HTTPS Server Docker container on any machine with Docker installed, ensuring consistent behavior across different environments.

**Additional Notes**

* If you've modified the port number in your `config.txt`, adjust the port mapping in the `docker run` command accordingly.
* Consider using Docker Compose for a more convenient way to define and run multi-container Docker applications.
* If you're using a different base image (other than Ubuntu 20.04), you may need to modify the `apt-get install` command to install the correct packages.
* For production deployments, you may want to explore additional Docker best practices, such as using a non-root user within the container and optimizing the Docker image for size and security.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

