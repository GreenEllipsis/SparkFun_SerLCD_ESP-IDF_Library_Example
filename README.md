ESP-IDF SerLCD Library
===========================================================
This is an ESP-IDF component for the SparkFun SerLCD.

![SparkFun SerLCD](https://cdn.sparkfun.com//assets/parts/1/1/9/2/7/14074-SparkFun_20x4_SerLCD_-_Black_on_RGB_3.3V-05.jpg)

[*SparkFun 20x4 SerLCD - Black on RGB 3.3V (LCD-14074). (Image copyright SparkFun Electronics)*](https://www.sparkfun.com/products/14074)

The SparkFun SerLCD is an AVR-based, serial enabled LCD that provides a simple and cost effective solution for adding a 20x4 Black on RGB Liquid Crystal Display into your project. We’ve overhauled the design to include an ATmega328P that handles all of the screen control, meaning a backpack is no longer needed! This display can now accept three different types of communication protocols: serial, I2C, and SPI. This simplifies the number of wires needed and allows your project to display all kinds of text and numbers.

The SerLCD is Qwiic compatible! SparkFun recommends adding a [Qwiic Adapter](https://www.sparkfun.com/products/14495) to the back of the SerLCD to get it onto the Qwiic bus.

The on-board ATmega328P AVR microcontroller utilizes 11.0592 MHz crystal for greater communication accuracy with adjustable baud rates of 1200 through 1000000 but is default set at 9600. The firmware for this SerLCD is fully opensource and allows for any customizations you may need.

This is an ESP-IDF port of the [Arduino library](https://github.com/sparkfun/SparkFun_SerLCD_Arduino_Library) written by Gaston Williams and Nathan Seidle ([SparkFun](http://www.sparkfun.com)).

Repository Contents
-------------------
- **/main** - "Hello, World!" example. If you're using CMake, add *SparkFun_SerLCD_ESP-IDF_Library esp_timer* to the REQUIRES parameter of
the CMakeLists.txt file in your main application folder. (Yeah, that's a long one, but I'm emulating SparkFun's pattern.)
* **/components/SparkFun_SerLCD_ESP-IDF_Library** the ESP-IDF component that you will copy into your project's component folder . 
See [SparkFun's repository](https://github.com/sparkfun/SparkFun_SerLCD_Arduino_Library/tree/master/examples) for Arduino examples you can easily port.  


License Information
-------------------

This product is _**open source**_! 

Various bits of the code have different licenses applied. In the spirit of SparkFun's original license, all original derivative work in this repository is beerware; if you see me at the local, and you've found this code helpful, please buy us a round! 

Please use, reuse, and modify these files as you see fit. Please maintain attribution to Green Ellipsis, SparkFun Electronics and other creators and release anything derivative under the same license.

Distributed as-is; no warranty is given.

Known Issues
---------------
- Using an Olimenx ESP32-EVB (REV I), I was getting ESP_ERR_TIMEOUTS every 26 s on average at 100 kHz bus speed. The errors are always generated in i2c_master_cmd_begin() where it checks if the hw FSM is stuck. Increasing the delays after sending write commands even up to 100 ms did not help. Using the internal pullups in addition to the 4.7 kOhm ones on the SerLCD didn't help. Changing CONFIG_FREERTOS_HZ from 100 to 1000 Hz did not make a difference. Shortening my I2C wires from 40 to 20 cm didn't help. Ultimately, the only thing that fixed it was lowering the bus speed to 50 kHz. No errors at that speed after 10s of thousands of consecutive writes.

- Currently only supports I2C communication to the SerLCD (because that's all I needed). No support for serial stream or SPI yet. Otherwise this would be a 1.0.0 release.