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
