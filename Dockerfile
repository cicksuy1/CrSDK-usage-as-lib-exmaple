# Stage 1: Build Environment
FROM ubuntu:20.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libssl-dev \
    libfmt-dev 

# Set working directory
WORKDIR /app

# Copy source code
COPY . /app

# Download and install SonySDK (if not already installed)
RUN if [ ! -d "/app/include/SonySDK" ]; then \
        wget -O SonySDK.tar.gz "https://www.sony.net/Products/CameraRemoteSDK/download/index.html" && \
        tar -xzf SonySDK.tar.gz -C /app/include && \
        rm SonySDK.tar.gz \
    fi

# Build httplib and spdlog (if not found)
RUN if [ ! -d "/usr/local/include/httplib" ]; then \
        git clone https://github.com/yhirose/cpp-httplib.git /app/deps/httplib && \
        cd /app/deps/httplib && \
        mkdir build && cd build && \
        cmake .. && make -j12 && sudo make install \
    fi

RUN if [ ! -d "/usr/local/include/spdlog" ]; then \
        git clone https://github.com/gabime/spdlog.git /app/deps/spdlog && \
        cd /app/deps/spdlog && \
        mkdir build && cd build && \
        cmake .. && make -j12 && sudo make install \
    fi

# Build your project
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Stage 2: Runtime Environment (Optional, for smaller image size)
FROM ubuntu:20.04
COPY --from=0 /app/build/CrSDK_HTTPS_Server /usr/local/bin/
CMD ["CrSDK_HTTPS_Server"]
