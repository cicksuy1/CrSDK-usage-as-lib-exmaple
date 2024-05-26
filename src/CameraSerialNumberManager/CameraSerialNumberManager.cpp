#include "CameraSerialNumberManager.h"

CameraSerialNumberManager::CameraSerialNumberManager() 
{
    libusb_init(&ctx); // Initialize libusb context
    populateCameraSerialNumbers();
}

CameraSerialNumberManager::~CameraSerialNumberManager() 
{
    if (ctx) {
        libusb_exit(ctx); // Clean up libusb context
    }
}

std::string CameraSerialNumberManager::getLeftCameraSerialNumber() const 
{
    return leftCameraSerialNumber;
}

void CameraSerialNumberManager::setLeftCameraSerialNumber(const std::string &serialNumber) 
{
    leftCameraSerialNumber = serialNumber;
}

std::string CameraSerialNumberManager::getRightCameraSerialNumber() const 
{
    return rightCameraSerialNumber;
}

void CameraSerialNumberManager::setRightCameraSerialNumber(const std::string &serialNumber) 
{
    rightCameraSerialNumber = serialNumber;
}

void CameraSerialNumberManager::populateCameraSerialNumbers() 
{
    // Get the serial numbers of the cameras.
    leftCameraSerialNumber = getCameraSerialNumber(LEFT_CAMERA_BUS, LEFT_CAMERA_DEVICE);
    rightCameraSerialNumber = getCameraSerialNumber(RIGHT_CAMERA_BUS, RIGHT_CAMERA_DEVICE);

    spdlog::info("Left Camera Serial Number: {}", leftCameraSerialNumber);
    spdlog::info("Right Camera Serial Number: {}", rightCameraSerialNumber);
}

std::string CameraSerialNumberManager::getCameraSerialNumber(uint8_t busNumber, uint8_t deviceAddress) 
{
    spdlog::info("Getting camera serial number for bus: {}, device: {}", busNumber, deviceAddress);
    libusb_device_handle *dev_handle;
    libusb_device **devs;

    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) 
    {
        spdlog::error("Failed to get USB device list");
        return "Serial number not found";
    }

    for (ssize_t i = 0; i < cnt; i++) 
    {
        libusb_device *dev = devs[i];
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc);

        uint8_t bus = libusb_get_bus_number(dev);
        uint8_t address = libusb_get_device_address(dev);

        if (bus == busNumber && address == deviceAddress) 
        {
            if (desc.idVendor == 0x054c) 
            {
                if (libusb_open(dev, &dev_handle) == 0) 
                {
                    unsigned char serialNumber[256];
                    int bytesRead = libusb_get_string_descriptor_ascii(dev_handle, desc.iSerialNumber, serialNumber, 256);
                    if (bytesRead > 0) 
                    {
                        std::string serialStr(reinterpret_cast<char *>(serialNumber), bytesRead);
                        libusb_close(dev_handle);
                        libusb_free_device_list(devs, 1);
                        return serialStr;
                    }
                    libusb_close(dev_handle);
                }
            }
        }
    }

    libusb_free_device_list(devs, 1);
    return "Serial number not found";
}
