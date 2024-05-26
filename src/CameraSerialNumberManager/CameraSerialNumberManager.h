#ifndef CAMERA_SERIAL_NUMBER_MANAGER_H
#define CAMERA_SERIAL_NUMBER_MANAGER_H

#include <string>
#include <libusb-1.0/libusb.h>
#include <spdlog/spdlog.h>

// Define USB bus and device numbers as macros
#define LEFT_CAMERA_BUS 2
#define LEFT_CAMERA_DEVICE 3
#define RIGHT_CAMERA_BUS 2
#define RIGHT_CAMERA_DEVICE 2

/**
 * @class CameraSerialNumberManager
 * @brief Manages the serial numbers of connected cameras.
 */
class CameraSerialNumberManager {
public:
    /**
     * @brief Constructor that initializes the manager and populates the camera serial numbers.
     */
    CameraSerialNumberManager();

    /**
     * @brief Destructor that cleans up resources.
     */
    ~CameraSerialNumberManager();

    /**
     * @brief Gets the serial number of the left camera.
     * @return The serial number of the left camera.
     */
    std::string getLeftCameraSerialNumber() const;

    /**
     * @brief Sets the serial number of the left camera.
     * @param serialNumber The serial number to set.
     */
    void setLeftCameraSerialNumber(const std::string &serialNumber);

    /**
     * @brief Gets the serial number of the right camera.
     * @return The serial number of the right camera.
     */
    std::string getRightCameraSerialNumber() const;

    /**
     * @brief Sets the serial number of the right camera.
     * @param serialNumber The serial number to set.
     */
    void setRightCameraSerialNumber(const std::string &serialNumber);

    /**
     * @brief Populates the serial numbers of the connected cameras.
     */
    void populateCameraSerialNumbers();

private:
    std::string leftCameraSerialNumber;
    std::string rightCameraSerialNumber;

    libusb_context *ctx = nullptr;

    /**
     * @brief Gets the serial number of a camera based on its USB bus and device address.
     * @param busNumber The USB bus number of the camera.
     * @param deviceAddress The USB device address of the camera.
     * @return The serial number of the camera.
     */
    std::string getCameraSerialNumber(uint8_t busNumber, uint8_t deviceAddress);
};

#endif // CAMERA_SERIAL_NUMBER_MANAGER_H
