#ifndef GPIO_PIN_H
#define GPIO_PIN_H

#include <iostream> // For error and informational messages (can be removed if not desired)
#include <exception> // For handling exceptions
#include <unistd.h>  // For sleep function in restat()
#include <thread>

// Include the JetsonGPIO library for GPIO pin control
#include "JetsonGPIO.h"

/**
 * @class GpioPin
 * @brief This class provides an abstraction for controlling a GPIO pin on a Jetson device.
 *
 * The GpioPin class simplifies interacting with a GPIO pin by offering methods
 * for setting the pin mode (input or output), setting the pin state (high or low),
 * and cleaning up the pin resources when the object goes out of scope.
 */
class GpioPin {
public:
  /**
   * @brief Constructor for the GpioPin class.
   *
   * This constructor takes an integer representing the physical GPIO pin number
   * and sets the pin numbering mode to BOARD mode by default.
   *
   * @param pin The physical GPIO pin number.
   */
  GpioPin(int pin);

  /**
   * @brief Destructor for the GpioPin class.
   *
   * The destructor ensures the GPIO pin is cleaned up and resources are released
   * when the GpioPin object goes out of scope. It attempts to call `GPIO::cleanup()`
   * and prints a message indicating success or failure.
   */
  ~GpioPin();

  /**
   * @brief Performs an action on a GPIO pin after a 20-second delay.
   *
   * This function takes an integer representing the GPIO pin number and performs
   * an asynchronous operation to set the pin as output after a 20-second delay.
   * The main program's execution continues without being blocked by the delay.
   *
   * @param pin The integer representing the GPIO pin number.
  */
  void delayedAction(int pin);

  /**
   * @brief Sets the GPIO pin numbering mode.
   *
   * This method allows you to choose the numbering mode for the GPIO pins.
   * Supported modes are BOARD, BCM, CVM, and TEGRA_SOC. The method returns
   * true on success and false on failure (e.g., invalid mode provided).
   *
   * @param mode A string representing the desired pin numbering mode (BOARD, BCM, CVM, TEGRA_SOC).
   * @return bool True on success, false on failure.
   */
  bool setmode(std::string mode);

  /**
   * @brief Sets the GPIO pin mode (input or output).
   *
   * This method configures the specified pin as either an input or output pin.
   * It takes a string argument ("OUT" or "out" for output, "IN" or "in" for input)
   * and returns true on success, false on failure (e.g., invalid mode provided).
   *
   * @param setUp A string representing the desired pin mode (OUT/out for output, IN/in for input).
   * @return bool True on success, false on failure.
   */
  bool setup(std::string setUp);

  /**
   * @brief Turns the GPIO pin on (sets the pin state to HIGH).
   *
   * This method sets the specified pin to a high state (typically corresponding to 3.3V).
   * It first ensures the pin is configured as output and then calls `GPIO::output(pin, GPIO::HIGH)`.
   * The method returns true on success, false on failure (e.g., error during pin configuration).
   *
   * @return bool True on success, false on failure.
   */
  bool pinOn();

  /**
   * @brief Turns the GPIO pin off (sets the pin state to LOW).
   *
   * This method sets the specified pin to a low state (typically corresponding to 0V).
   * It first ensures the pin is configured as output and then calls `GPIO::output(pin, GPIO::LOW)`.
   * The method returns true on success, false on failure (e.g., error during pin configuration).
   *
   * @return bool True on success, false on failure.
   */
  bool pinOff();

  /**
   * @brief Resets the GPIO pin by turning it on and then off after a 10-second delay.
   *
   * This method sets the pin to high for 10 seconds and then sets it back to low.
   * It's intended for a simple reset functionality and can be modified based on specific needs.
   * The method returns true on success, false on failure (e.g., error during pin configuration).
   *
   * @return bool True on success, false on failure.
   */
  bool restat();

private:
  int pin; // The physical GPIO pin number
 
};

#endif //
