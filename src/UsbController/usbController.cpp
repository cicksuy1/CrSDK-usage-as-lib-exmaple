#include "usbController.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

UsbController::UsbController(const std::string& hubLocation, int port)
    : hubLocation_(hubLocation), port_(port) {}

void UsbController::disable() {
    std::cout << "Disabling USB port " << port_ << " on hub " << hubLocation_ << std::endl;
    runUhubctlCommand("off");
}

void UsbController::enable() {
    std::cout << "Enabling USB port " << port_ << " on hub " << hubLocation_ << std::endl;
    runUhubctlCommand("on");
}

std::string UsbController::getHubLocation() const {
    return hubLocation_;
}

int UsbController::getPort() const {
    return port_;
}

void UsbController::runUhubctlCommand(const std::string& action) {
    std::string command = "sudo uhubctl -l " + hubLocation_ + " -p " + std::to_string(port_) + " -a " + action;
    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
}
