#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include <string>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

/**
 * @file usbController.h
 * @brief Declaration of the UsbController class for managing USB ports using the uhubctl tool.
 *
 * This file contains the declaration of the UsbController class, which provides methods
 * to enable and disable USB ports on a specific hub using the `uhubctl` command-line tool.
 * 
 * Usage Example:
 * @code
 * UsbController controller("2-3", 2);
 * controller.disable();
 * std::this_thread::sleep_for(std::chrono::seconds(5));
 * controller.enable();
 * @endcode
 */

/**
 * @class UsbController
 * @brief A class for controlling USB ports on a specific hub using uhubctl.
 *
 * The UsbController class allows you to enable and disable USB ports on a given hub location.
 * It internally calls the `uhubctl` command-line utility to perform the actions.
 */
class UsbController 
{
public:
    /**
     * @brief Constructs a UsbController object.
     * @param hubLocation The location of the USB hub (e.g., "2-3").
     * @param port The port number on the hub to control.
     *
     * This constructor initializes the UsbController with the given hub location and port number.
     */
    UsbController(const std::string& hubLocation, int port);

    /**
     * @brief Disables the USB port.
     *
     * This method disables the USB port on the specified hub. It uses the `uhubctl` tool
     * to turn off power to the USB port.
     */
    void disable();

    /**
     * @brief Enables the USB port.
     *
     * This method enables the USB port on the specified hub. It uses the `uhubctl` tool
     * to turn on power to the USB port.
     */
    void enable();

    /**
     * @brief Gets the hub location.
     * @return A string representing the hub location.
     *
     * This method returns the location of the USB hub that this controller is managing.
     */
    std::string getHubLocation() const;

    /**
     * @brief Gets the port number.
     * @return An integer representing the port number.
     *
     * This method returns the port number on the hub that this controller is managing.
     */
    int getPort() const;

private:
    std::string hubLocation_; /**< The location of the USB hub (e.g., "2-3"). */
    int port_; /**< The port number on the hub to control. */

    /**
     * @brief Runs the uhubctl command to control the USB port.
     * @param action The action to perform ("on" or "off").
     * @return true if the command was successful, false otherwise.
     *
     * This method constructs and executes the `uhubctl` command to enable or disable the USB port.
     * It suppresses the output and returns a boolean indicating the success or failure of the command.
     */
    bool runUhubctlCommand(const std::string& action);
};

#endif // USBCONTROLLER_H


