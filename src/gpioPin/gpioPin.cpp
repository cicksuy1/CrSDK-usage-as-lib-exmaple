#include "gpioPin.h"

std::string getModeString(GPIO::NumberingModes mode) {
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

    //To disable warnings
    GPIO::setwarnings(false); 

    // To check which mode has been set.
    GPIO::NumberingModes mode = GPIO::getmode();

    std::cout << "GPIO mode: " << getModeString(mode) << std::endl;

    // Set the GPIO numbering mode (choose one based on your needs)
    GPIO::setmode(GPIO::BOARD);  // Use board pin numbering 

    // Set up the pin as intput
    GPIO::setup(pin, GPIO::IN);  

    // Get the initial state of the GPIO pin
    int initialState = GPIO::input(pin);

    std::cout << "Initial GPIO state: " << initialState << std::endl; // Print the initial state

    // Launch the asynchronous function in a separate thread
    std::thread asyncThread;
    asyncThread = std::thread([this, pin] {
        this->delayedAction(pin);
    });
    asyncThread.detach();  // Detach the thread to avoid resource management concerns
}

GpioPin::~GpioPin()
{
    try
    {
        // Clean up and release the GPIO pin
        GPIO::cleanup();
        std::cout << "Clean up and release the GPIO pin was successful" << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred when trying to Clean up and release the GPIO pin: " << e.what() << std::endl;
    }
}

void GpioPin::delayedAction(int pin) {
    // Wait for 20 seconds using a sleep or a timer
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Perform the desired action (e.g., setting the pin as output)
    GPIO::setup(pin, GPIO::OUT);

    GPIO::output(pin, GPIO::HIGH);
    std::cout << "PIN " << pin << " is on!" << std::endl; 
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
            std::cerr << "Changing the modep of the PIN failed because the entered value is incorrect." << std::endl;
            return false;
        }

        std::cout << "Changing the set up of the PIN was successful." << std::endl;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to changing the gpio pin mode: " << e.what() << std::endl;
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
            std::cerr << "Changing the set up of the PIN failed because the entered value is incorrect." << std::endl;
            return false;
        }

        std::cout << "Changing the set up of the PIN was successful." << std::endl;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to changing set up the pin: {}" << e.what() << std::endl;
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
        std::cout << "PIN " << pin << " is on!" << std::endl;  
        return true; 
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to start pin number: {}" << e.what() << std::endl;
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

        std::cout << "PIN " << pin << " is off!" << std::endl;  
        return true; 
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to stop pin number :" << e.what() << std::endl;
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

        sleep(10);

        GPIO::output(pin, GPIO::HIGH);

        return true; 
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error occurred while trying to stop pin number :" << e.what() << std::endl;
        return false;
    }
}
