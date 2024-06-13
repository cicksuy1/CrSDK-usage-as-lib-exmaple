#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include <string>

class UsbController {
public:
    UsbController(const std::string& hubLocation, int port);
    void disable();
    void enable();
    std::string getHubLocation() const;
    int getPort() const;

private:
    std::string hubLocation_;
    int port_;
    void runUhubctlCommand(const std::string& action);
};

#endif // USBCONTROLLER_H
