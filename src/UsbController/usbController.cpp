#include "usbController.h"


UsbController::UsbController(const std::string& hubLocation, int port)
    : hubLocation_(hubLocation), port_(port) {}

void UsbController::disable() 
{
    spdlog::info("Disabling USB port {} on hub {}.", port_, hubLocation_ );
    runUhubctlCommand("off");
}

void UsbController::enable() 
{
    spdlog::info("Enabling USB port {} on hub {}.", port_, hubLocation_ );
    runUhubctlCommand("on");
}

std::string UsbController::getHubLocation() const 
{
    return hubLocation_;
}

int UsbController::getPort() const 
{
    return port_;
}

void UsbController::runUhubctlCommand(const std::string& action) 
{
    std::string command = fmt::format("sudo uhubctl -l {} -p {} -a {} > /dev/null 2>&1", hubLocation_, port_, action);
    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
}
