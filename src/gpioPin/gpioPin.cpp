#include "gpioPin.h"

std::string getModeString(GPIO::NumberingModes mode) 
{
    switch (mode) {
        case GPIO::BOARD:
            return "BOARD";
        case GPIO::BCM:
            return "BCM";
        case GPIO::CVM:
            return "CVM";
        case GPIO::TEGRA_SOC:
            return "TEGRA_SOC";
        default:
            return "Unknown";
    }
}

GpioPin::GpioPin(int pin) 
{
    this->pin = pin;

    try 
    {
        // Disable warnings
        GPIO::setwarnings(false);

        // Check current mode
        GPIO::NumberingModes mode = GPIO::getmode();

        // Set the GPIO numbering mode
        GPIO::setmode(GPIO::BOARD); // Or GPIO::BCM based on your needs

        // Set up the pin as input
        GPIO::setup(pin, GPIO::IN);

        // Get the initial state of the GPIO pin
        int initialState = GPIO::input(pin);

        // Launch the asynchronous function in a separate thread
        std::thread asyncThread;
        asyncThread = std::thread([this, pin] 
        {
            this->delayedAction(pin);
        });
        asyncThread.detach(); // Detach the thread to avoid resource management concerns
    } 
    catch (const std::runtime_error& e) 
    {
        spdlog::error("Error: Unable to access GPIO pin {}. {}", pin, e.what());
    } 
    catch (const std::exception& e) 
    {
       spdlog::error("An unexpected error occurred: {}", e.what());
    }
}

GpioPin::~GpioPin()
{
    try
    {
        // Clean up and release the GPIO pin
        GPIO::cleanup();
        spdlog::info("Clean up and release the GPIO pin was successful.");
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred when trying to Clean up and release the GPIO pin: {}.", e.what());
    }
}

void GpioPin::delayedAction(int pin) 
{
    // Wait for 20 seconds using a sleep or a timer
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Perform the desired action (e.g., setting the pin as output)
    GPIO::setup(pin, GPIO::OUT);

    GPIO::output(pin, GPIO::HIGH);
    spdlog::info("PIN {} is on!", pin); 
}

bool GpioPin::setmode(std::string mode)
{
    try
    {
        if(mode == "BOARD" || mode == "board")
        {
            GPIO::setmode(GPIO::BOARD);  // Use board pin numbering
        }
        else if (mode == "BCM" || mode == "bcm")
        {
            GPIO::setmode(GPIO::BCM);   // Use Broadcom SOC pin numbering
        }
        else if (mode == "CVM" || mode == "cvm")
        {
            GPIO::setmode(GPIO::CVM);
        }
        else if (mode == "TEGRA_SOC" || mode == "tegra_sod")
        {
            GPIO::setmode(GPIO::TEGRA_SOC);        
        }
        else
        {
            spdlog::error("Changing the modep of the PIN failed because the entered value is incorrect.");
            return false;
        }

        spdlog::info("Changing the set up of the PIN was successful.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to changing the gpio pin mode: {}", e.what());
        return false;    
    }
}

bool GpioPin::setup(std::string setUp)
{
    try
    {
        if(setUp == "OUT" || setUp == "out")
        {
            // Set up the pin as output
            GPIO::setup(pin, GPIO::OUT);   
        }
        else if (setUp == "IN" || setUp == "in")
        {
            // Set up the pin as input
            GPIO::setup(pin, GPIO::IN);    
        }
        else
        {
            spdlog::error("Changing the set up of the PIN failed because the entered value is incorrect.");
            return false;
        }

        spdlog::info("Changing the set up of the PIN was successful.");
        return true;
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to changing set up the pin: {}", e.what());
        return false;
    }
}                                                                                                                                           
                                                                                                                                                
bool GpioPin::pinOn()
{
    try
    {
        // Set up the pin as output
        GPIO::setup(pin, GPIO::OUT); 

        GPIO::output(pin, GPIO::HIGH);
        spdlog::info("PIN {} is on!", pin); 
        return true; 
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to start pin number: {}", e.what());
        return false;
    }
}

bool GpioPin::pinOff()
{
    try
    {
        // Set up the pin as output
        GPIO::setup(pin, GPIO::OUT); 

        GPIO::output(pin, GPIO::LOW);

        spdlog::info("PIN {} is off!", pin); 
        return true; 
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to stop pin number {}: {}.", pin, e.what());
        return false;
    }
}

bool GpioPin::restat()
{
    try
    {
        // Set up the pin as output
        GPIO::setup(pin, GPIO::OUT); 

        GPIO::output(pin, GPIO::LOW);

        sleep(5);

        GPIO::output(pin, GPIO::HIGH);

        return true; 
    }
    catch(const std::exception& e)
    {
        spdlog::error("An error occurred while trying to restat pin number {}: {}.", pin, e.what());
        return false;
    }
}
