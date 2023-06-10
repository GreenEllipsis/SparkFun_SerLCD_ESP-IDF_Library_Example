/*
 * LCD library for SparkFun RGB 3.3v Serial Open LCD display
 * with an attached Qwiic adapter.
 *
 * By: Gaston R. Williams
 * Date: August 22, 2018
 * Update: March 23, 2020 - fixed missing return value in write(uint8_t)
 * Ported: June 7, 2023 from Arduino to ESP-IDF
 * 
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 *
 * This library is based heavily on the LiquidCrystal_I2C library and the sample code provided with
 * the SparkFun Serial OpenLCD display.  The original LiquidCrystal library by David A. Mellis and
 * modified by Limor Fried and the OpenLCD code by Nathan Seidle at SparkFun.
 *
 * The LiquidCrystal_I2C library was based on the work by DFRobot.
 * (That's the only attribution I found in the code I have. If anyone can provide better information,
 * Plese let me know and I'll be happy to give credit where credit is due.)
 *
 * Original information copied from OpenLCD:
 *
 * The OpenLCD display information is based based on code by
 * Nathan Seidle
 * SparkFun Electronics
 * Date: April 19th, 2015
 *
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 * OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original
 * serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
 * This is an updated serial LCD.
 *
 * Please Note: 0x72 is the 7-bit I2C address. If you are using a different language than Arduino you will probably
 * need to add the Read/Write bit to the end of the address. This means the default read address for the OpenLCD
 * is 0b.1110.0101 or 0xE5 and the write address is 0b.1110.0100 or 0xE4.
 * For more information see https://learn.sparkfun.com/tutorials/i2c
 * Note: This code expects the display to be listening at the default I2C address. If your display is not at 0x72, you can
 * do a hardware reset. Tie the RX pin to ground and power up OpenLCD. You should see the splash screen
 * then "System reset Power cycle me" and the backlight will begin to blink. Now power down OpenLCD and remove
 * the RX/GND jumper. OpenLCD is now reset.
 *
 * To get this code to work, attach a Qwiic adapter to an OpenLCD. Use the Qwiic cable to attach adapter to a SparkFun Blackboard or
 * an Arduino Uno with the Qwiic shield.
 *
 * The OpenLCD has 4.7k pull up resistors on SDA and SCL. If you have other devices on the
 * I2C bus then you may want to disable the pull up resistors by clearing the PU (pull up) jumper.

 * OpenLCD will work at 400kHz Fast I2C. Use the .setClock() call shown below to set the data rate
 * faster if needed.
 * Command cheat sheet:
 * ASCII / DEC / HEX
 * '|'    / 124 / 0x7C - Put into setting mode
 * Ctrl+c / 3 / 0x03 - Change width to 20
 * Ctrl+d / 4 / 0x04 - Change width to 16
 * Ctrl+e / 5 / 0x05 - Change lines to 4
 * Ctrl+f / 6 / 0x06 - Change lines to 2
 * Ctrl+g / 7 / 0x07 - Change lines to 1
 * Ctrl+h / 8 / 0x08 - Software reset of the system
 * Ctrl+i / 9 / 0x09 - Enable/disable splash screen
 * Ctrl+j / 10 / 0x0A - Save currently displayed text as splash
 * Ctrl+k / 11 / 0x0B - Change baud to 2400bps
 * Ctrl+l / 12 / 0x0C - Change baud to 4800bps
 * Ctrl+m / 13 / 0x0D - Change baud to 9600bps
 * Ctrl+n / 14 / 0x0E - Change baud to 14400bps
 * Ctrl+o / 15 / 0x0F - Change baud to 19200bps
 * Ctrl+p / 16 / 0x10 - Change baud to 38400bps
 * Ctrl+q / 17 / 0x11 - Change baud to 57600bps
 * Ctrl+r / 18 / 0x12 - Change baud to 115200bps
 * Ctrl+s / 19 / 0x13 - Change baud to 230400bps
 * Ctrl+t / 20 / 0x14 - Change baud to 460800bps
 * Ctrl+u / 21 / 0x15 - Change baud to 921600bps
 * Ctrl+v / 22 / 0x16 - Change baud to 1000000bps
 * Ctrl+w / 23 / 0x17 - Change baud to 1200bps
 * Ctrl+x / 24 / 0x18 - Change the contrast. Follow Ctrl+x with number 0 to 255. 120 is default.
 * Ctrl+y / 25 / 0x19 - Change the TWI address. Follow Ctrl+x with number 0 to 255. 114 (0x72) is default.
 * Ctrl+z / 26 / 0x1A - Enable/disable ignore RX pin on startup (ignore emergency reset)
 * '+'    / 43 / 0x2B - Set RGB backlight with three following bytes, 0-255
 * ','    / 44 / 0x2C - Display current firmware version
 * '-'    / 45 / 0x2D - Clear display. Move cursor to home position.
 * '.'    / 46 / 0x2E - Enable system messages (ie, display 'Contrast: 5' when changed)
 * '/'    / 47 / 0x2F - Disable system messages (ie, don't display 'Contrast: 5' when changed)
 * '0'    / 48 / 0x30 - Enable splash screen
 * '1'    / 49 / 0x31 - Disable splash screen
 *        / 128-157 / 0x80-0x9D - Set the primary backlight brightness. 128 = Off, 157 = 100%.
 *        / 158-187 / 0x9E-0xBB - Set the green backlight brightness. 158 = Off, 187 = 100%.
 *        / 188-217 / 0xBC-0xD9 - Set the blue backlight brightness. 188 = Off, 217 = 100%.
 *		For example, to change the baud rate to 115200 send 124 followed by 18.
 *
 */
#include "SerLCD.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "driver/i2c.h"

/**
 * Macro to emulate Arduino delay()
*/
#ifndef DELAY
#define DELAY(x) vTaskDelay(pdMS_TO_TICKS(x))
#endif

#define TAG_SERLCD  "SerLCD"

//<<constructor>> setup using defaults
SerLCD::SerLCD()
{
}

//<<destructor>>
SerLCD::~SerLCD()
{ /*nothing to destruct*/
}

/*
 * Set up the i2c communication with the SerLCD.
 * wirePort - TwoWire port
 * ic2_addr - I2C address
 */
void SerLCD::begin(i2c_port_t &wirePort, uint8_t i2c_addr)
{
  _i2cAddr = i2c_addr;

  begin(wirePort);
} // begin

/*
 * Set up the i2c communication with the SerLCD.
 */
void SerLCD::begin(i2c_port_t &wirePort)
{
  _i2cPort = wirePort; //Grab which port the user wants us to use
 
  //Call init function since display may have been left in unknown state
  init();
} // begin

/*
 * Set up the serial communication with the SerLCD.
 * TODO
 */
// void SerLCD::begin(Stream &serialPort)
// {
//   _serialPort = &serialPort; //Grab which port the user wants us to use
//   _i2cPort = NULL;           //Set to null to be safe
//   _spiPort = NULL;           //Set to null to be safe

//   //Call init function since display may have been left in unknown state
//   init();
// } // begin

//Only available in Arduino 1.6 or later
#ifdef SPI_HAS_TRANSACTION
/*
 * Set up the SPI communication with the SerLCD using SPI transactions
 *
 * NB we pass SPISettings by value, since many of the examples for the SPI
 * transactions create the settings object in the function call, and that only
 * works if the function passes the object by value.
 *
 * TODO
 */
// void SerLCD::begin(SPIClass &spiPort, uint8_t csPin, SPISettings spiSettings)
// {
//   _spiSettings = spiSettings;
//   _spiTransaction = true;

//   begin(spiPort, csPin);
// } // begin
#endif

/*
 * Set up the SPI communication with the SerLCD.
 *
 * TODO
 */
// void SerLCD::begin(SPIClass &spiPort, uint8_t csPin)
// {
//   _csPin = csPin;

//   pinMode(csPin, OUTPUT);    //set pin to output, in case user forgot
//   digitalWrite(csPin, HIGH); //deselect dispaly, in case user forgot

//   _spiPort = &spiPort; //Grab the port the user wants us to use
//   _i2cPort = NULL;     //Set to null to be safe
//   _serialPort = NULL;  //Set to null to be safe

//   _spiPort->begin(); //call begin, in case the user forgot

//   //Call init function since display may have been left in unknown state
//   init();
// } // begin

//private functions for serial transmission
/*
 * Begin transmission to the device
 */
// void SerLCD::beginTransmission()
// {
  //do nothing if using serialPort
  // if (_i2cPort)
  // {
    // _i2cPort->beginTransmission(_i2cAddr); // transmit to device
  // }
//   else if (_spiPort)
//   {
// #ifdef SPI_HAS_TRANSACTION
//     if (_spiTransaction)
//     {
//       _spiPort->beginTransaction(_spiSettings); //gain control of the SPI bus
//     }                                           //if _spiSettings
// #endif
//     digitalWrite(_csPin, LOW);
//     DELAY(10); //wait a bit for display to enable
//   }            // if-else
// } //beginTransmission

/*
 * Send data to the device
 *
 * data - uint8_t to send
 */
// void SerLCD::transmit(uint8_t data)
// {
//   // if (_i2cPort)
//   // {
//     _i2cPort->write(data); // transmit to device
//   // else if (_serialPort)
//   // {
//   //   _serialPort->write(data);
//   // }
//   // else if (_spiPort)
//   // {
//   //   _spiPort->transfer(data);
//   // } // if-else
// } //transmit

/*
 * Begin transmission to the device
 */
// void SerLCD::endTransmission()
// {
  //do nothing if using Serial port
// if (_i2cPort)
//   {
    // _i2cPort->endTransmission(); // transmit to device
//   }
//   else if (_spiPort)
//   {
//     digitalWrite(_csPin, HIGH); //disable display
// #ifdef SPI_HAS_TRANSACTION
//     if (_spiTransaction)
//     {
//       _spiPort->endTransaction(); //let go of the SPI bus
//     }                             //if _spiSettings
// #endif
//     DELAY(10); //wait a bit for display to disable
  // }            // if-else
// } //beginTransmission

/*
 * Initialize the display
 *
 */
void SerLCD::init()
{
  const uint8_t buffer[] = 
  {
    (SPECIAL_COMMAND),                  //Send special command character
    (uint8_t)(LCD_DISPLAYCONTROL | _displayControl), //Send the display command
    (SPECIAL_COMMAND),                      //Send special command character 
    (uint8_t)(LCD_ENTRYMODESET | _displayMode),      //Send the entry mode command
    (SETTING_COMMAND),                      //Put LCD into setting mode 
    (CLEAR_COMMAND)                     //Send clear display command
  };
  write(buffer, sizeof(buffer), 60); // 50 test
}

/*
  * Send a command to the display.
  * Used by other functions.
  *
  * uint8_t command to send
  */
void SerLCD::command(uint8_t command)
{
  const uint8_t buffer[] = 
  {
    (SETTING_COMMAND), //Put LCD into setting mode
    (command)       //Send the command code
  };
  write(buffer, sizeof(buffer));
}

/*
 * Send a special command to the display.  Used by other functions.
 *
 * uint8_t command to send
 */
void SerLCD::specialCommand(uint8_t command)
{
  const uint8_t buffer[] = {       // transmit to device
    (SPECIAL_COMMAND), //Send special command character
    (command)         //Send the command code
  };
  write(buffer, sizeof(buffer), 50); //Wait a bit longer for special display commands
}

/*
 * Send multiple special commands to the display.
 * Used by other functions.
 *
 * uint8_t command to send
 * uint8_t count number of times to send
 */
void SerLCD::specialCommand(uint8_t command, uint8_t count)
{
  uint8_t buffer[count * 2];
  for (int i = 0; i < count*2; i+=2)
  {
    buffer[i] = (SPECIAL_COMMAND); //Send special command character
    buffer[i+1] = (command);         //Send command code
  }                            // for  
  write(buffer, sizeof(buffer), 60); //Wait a bit longer for special display commands
}

/*
 * Send the clear command to the display.  This clears the
 * display and forces the cursor to return to the beginning
 * of the display.
 */
void SerLCD::clear()
{
  command(CLEAR_COMMAND);
}

/*
 * Send the home command to the display.  This returns the cursor
 * to return to the beginning of the display, without clearing
 * the display.
 */
void SerLCD::home()
{
  specialCommand(LCD_RETURNHOME);
}

/**
 * @brief Arduino map() 
*/
long SerLCD::map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
 * Set the cursor position to a particular column and row.
 *
 * column - uint8_t 0 to 19
 * row - uint8_t 0 to 3
 *
 */
void SerLCD::setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = {0x00, 0x40, 0x14, 0x54};

  //kepp variables in bounds
  row = std::min(row, (uint8_t)(MAX_ROWS - 1)); //row cannot be greater than max rows

  //send the command
  specialCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
  DELAY(10); // extra delay for you
} // setCursor

/*
 * Create a customer character
 * uint8_t   location - character number 0 to 7
 * uint8_t[] charmap  - uint8_t array for character
 */
void SerLCD::createChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  //Send request to create a customer character
  uint8_t buffer[1 + 1 + 8];
  buffer[0] = (SETTING_COMMAND); //Put LCD into setting mode
  buffer[1] = (27 + location);
  for (int i = 0; i < 8; i++)
  {
    buffer [i+2] =(charmap[i]);
  } // for
  write(buffer, sizeof(buffer),60); //This takes a bit longer
}

/*
 * Write a customer character to the display
 *
 * uint8_t location - character number 0 to 7
 */
void SerLCD::writeChar(uint8_t location)
{
  location &= 0x7; // we only have 8 locations 0-7

  command(35 + location);
}

/*
 * Write a uint8_t to the display.
 * Required for Print.
 */
size_t SerLCD::write(uint8_t b)
{
  esp_err_t ret;
  write(&b, 1, 10);
  // ret = (i2c_master_write_to_device(_i2cPort, _i2cAddr, &b, 1, 10 / portTICK_PERIOD_MS));
  // vTaskDelay(pdMS_TO_TICKS(20)); // test was 10. Not sure this is ever called
  // if (ret != ESP_OK) ESP_LOGI(TAG_SERLCD, "write(0x%02x) returned %i", b, ret);
  DELAY(10);
  return 1;
}
/*
 * Write a character buffer to the display.
 * Required for Print.
 */
size_t SerLCD::write(const uint8_t *buffer, size_t size)
{
  esp_err_t ret;
  write(buffer, size, 10);
  DELAY(20);
  // ret = (i2c_master_write_to_device(_i2cPort, _i2cAddr, buffer, size, 10 / portTICK_PERIOD_MS));
  // DELAY(80); // TEST 20
  // if (ret != ESP_OK) ESP_LOGI(TAG_SERLCD, "write({0x%02x, 0x%02x},0x%04x) returned %i", buffer[0], size >1 ? buffer[1] : 0, size, ret);
  return size;
} 
/*
 * Write a character buffer to the display and delay a specified amount.
 * Required for Print.
 */
size_t SerLCD::write(const uint8_t *buffer, size_t size, uint16_t ms)
{
  esp_err_t ret = ESP_OK;
  uint8_t i = 0;
  do {
    ret = (i2c_master_write_to_device(_i2cPort, _i2cAddr, buffer, size, 10 / portTICK_PERIOD_MS));
    DELAY(ms);
    // DELAY(100);
    i++;
    if (ret != ESP_OK) ESP_LOGI(TAG_SERLCD, "write({0x%02x, 0x%02x},0x%04x, 0x%04x) returned %i", buffer[0], buffer[1], size, ms, ret);
  } while (ret == ESP_ERR_TIMEOUT && i < 3);
  return size;
} //write

/*
 * Write a string to the display.
 * Required for Print.
 */
size_t SerLCD::write(const char *str)
{
  if (str == NULL)
    return 0;
  return write((const uint8_t *)str, strlen(str));
}

/*
  * Turn the display off quickly.
  */
void SerLCD::noDisplay()
{
  _displayControl &= ~LCD_DISPLAYON;
  specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // noDisplay

/*
 * Turn the display on quickly.
 */
void SerLCD::display()
{
  _displayControl |= LCD_DISPLAYON;
  specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // display

/*
  * Turn the underline cursor off.
  */
void SerLCD::noCursor()
{
  _displayControl &= ~LCD_CURSORON;
  specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // noCursor

/*
 * Turn the underline cursor on.
 */
void SerLCD::cursor()
{
  _displayControl |= LCD_CURSORON;
  specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // cursor

/*
  * Turn the blink cursor off.
  */
void SerLCD::noBlink()
{
  _displayControl &= ~LCD_BLINKON;
  specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // noBlink

/*
 * Turn the blink cursor on.
 */
void SerLCD::blink()
{
  _displayControl |= LCD_BLINKON;
  specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // blink

/*
 * Scroll the display one character to the left, without
 * changing the text
 */
void SerLCD::scrollDisplayLeft()
{
  specialCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
} // scrollDisplayLeft

/*
 * Scroll the display multiple characters to the left, without
 * changing the text
 *
 * count uint8_t - number of characters to scroll
 */
void SerLCD::scrollDisplayLeft(uint8_t count)
{
  specialCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, count);
} // scrollDisplayLeft

/*
 * Scroll the display one character to the right, without
 * changing the text
 */
void SerLCD::scrollDisplayRight()
{
  specialCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
} // scrollDisplayRight

/*
 * Scroll the display multiple characters to the right, without
 * changing the text
 *
 * count uint8_t - number of characters to scroll
 */
void SerLCD::scrollDisplayRight(uint8_t count)
{
  specialCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, count);
} // scrollDisplayRight

/*
 *  Move the cursor one character to the left.
 */
void SerLCD::moveCursorLeft()
{
  specialCommand(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVELEFT);
} // moveCursorLeft

/*
 *  Move the cursor multiple characters to the left.
 *
 *  count uint8_t - number of characters to move
 */
void SerLCD::moveCursorLeft(uint8_t count)
{
  specialCommand(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVELEFT, count);
} // moveCursorLeft

/*
 *  Move the cursor one character to the right.
 */
void SerLCD::moveCursorRight()
{
  specialCommand(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVERIGHT);
} // moveCursorRight

/*
 *  Move the cursor multiple characters to the right.
 *
 *  count uint8_t - number of characters to move
 */
void SerLCD::moveCursorRight(uint8_t count)
{
  specialCommand(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVERIGHT, count);
} // moveCursorRight

/*
 * Use a standard hex rgb value (0x00000000 to 0x00FFFFFF) to set
 * the backlight color.
 *
 * The encoded long value has form (0x00RRGGBB) where RR, GG and BB
 * are red, green, blue uint8_t values in hex.  The remaining two most
 * significant bytes of the long value are ignored.
 *
 * rgb - unsigned long hex encoded rgb value.
 */
void SerLCD::setBacklight(unsigned long rgb)
{
  // convert from hex triplet to uint8_t values
  uint8_t r = (rgb >> 16) & 0x0000FF;
  uint8_t g = (rgb >> 8) & 0x0000FF;
  uint8_t b = rgb & 0x0000FF;

  return setBacklight(r, g, b);
}

/*
 * Uses a standard rgb uint8_t triplit eg. (255, 0, 255) to
 * set the backlight color.
 */
void SerLCD::setBacklight(uint8_t r, uint8_t g, uint8_t b)
{
  // map the uint8_t value range to backlight command range
  uint8_t red = 128 + map(r, 0, 255, 0, 29);
  uint8_t green = 158 + map(g, 0, 255, 0, 29);
  uint8_t blue = 188 + map(b, 0, 255, 0, 29);

  //send commands to the display to set backlights
  //Turn display off to hide confirmation messages
  _displayControl &= ~LCD_DISPLAYON;
  uint8_t displayOn = _displayControl | LCD_DISPLAYON;
  const uint8_t buffer[] = { 
    (SPECIAL_COMMAND),  //Send special command character
    (uint8_t)(LCD_DISPLAYCONTROL | _displayControl),
    SETTING_COMMAND,
    red,
    SETTING_COMMAND,
    green,
    SETTING_COMMAND,
    blue,
    SPECIAL_COMMAND,
    (uint8_t)(LCD_DISPLAYCONTROL | displayOn),
    
    };
  _displayControl = displayOn;
  write(buffer, sizeof(buffer), 100); // TEST

  // //Turn display back on and end
  // _displayControl |= LCD_DISPLAYON;
  // transmit(SPECIAL_COMMAND);                      //Send special command character
  // transmit(LCD_DISPLAYCONTROL | _displayControl); //Turn display on as before
} // setBacklight

// New backlight function
void SerLCD::setFastBacklight(unsigned long rgb)
{
  // convert from hex triplet to uint8_t values
  uint8_t r = (rgb >> 16) & 0x0000FF;
  uint8_t g = (rgb >> 8) & 0x0000FF;
  uint8_t b = rgb & 0x0000FF;

  setFastBacklight(r, g, b);
}

//New command - set backlight with LCD messages or delays
void SerLCD::setFastBacklight(uint8_t r, uint8_t g, uint8_t b)
{
  //send commands to the display to set backlights
  const uint8_t buffer[] =
  {
    (SETTING_COMMAND), //Send special command character
    (SET_RGB_COMMAND), //Send the set RGB character '+' or plus
    (r),               //Send the red value
    (g),               //Send the green value
    (b)               //Send the blue value
  };
  write(buffer, sizeof(buffer));         //Stop transmission
} // setFastBacklight

//Enable system messages
//This allows user to see printing messages like 'UART: 57600' and 'Contrast: 5'
void SerLCD::enableSystemMessages()
{
  command(ENABLE_SYSTEM_MESSAGE_DISPLAY);
}

//Disable system messages
//This allows user to disable printing messages like 'UART: 57600' and 'Contrast: 5'
void SerLCD::disableSystemMessages()
{
  command(DISABLE_SYSTEM_MESSAGE_DISPLAY);
}

//Enable splash screen at power on
void SerLCD::enableSplash()
{
  command(ENABLE_SPLASH_DISPLAY);
}

//Disable splash screen at power on
void SerLCD::disableSplash()
{
  command(DISABLE_SPLASH_DISPLAY);
}

//Save the current display as the splash
void SerLCD::saveSplash()
{
  //Save whatever is currently being displayed into EEPROM
  //This will be displayed at next power on as the splash screen
  command(SAVE_CURRENT_DISPLAY_AS_SPLASH);
}

/*
 * Set the text to flow from left to right.  This is the direction
 * that is common to most Western languages.
 */
void SerLCD::leftToRight()
{
  _displayMode |= LCD_ENTRYLEFT;
  specialCommand(LCD_ENTRYMODESET | _displayMode);
} // leftToRight

/*
 * Set the text to flow from right to left.
 */
void SerLCD::rightToLeft()
{
  _displayMode &= ~LCD_ENTRYLEFT;
  specialCommand(LCD_ENTRYMODESET | _displayMode);
} //rightToLeft

/*
 * Turn autoscrolling on. This will 'right justify' text from
 * the cursor.
 */
void SerLCD::autoscroll()
{
  _displayMode |= LCD_ENTRYSHIFTINCREMENT;
  specialCommand(LCD_ENTRYMODESET | _displayMode);
} //autoscroll

/*
 * Turn autoscrolling off.
 */
void SerLCD::noAutoscroll()
{
  _displayMode &= ~LCD_ENTRYSHIFTINCREMENT;
  specialCommand(LCD_ENTRYMODESET | _displayMode);
} //noAutoscroll

/*
 * Change the contrast from 0 to 255. 120 is default.
 *
 * uint8_t new_val - new contrast value
 */
void SerLCD::setContrast(uint8_t new_val)
{
  //send commands to the display to set backlights
  const uint8_t buffer[] = 
  {
    (SETTING_COMMAND),  //Send contrast command
    (CONTRAST_COMMAND), //0x18
    (new_val)          //Send new contrast value
  };
  write(buffer, sizeof(buffer));
} //setContrast

/*
 * Change the I2C Address. 0x72 is the default.
 * Note that this change is persistent.  If anything
 * goes wrong you may need to do a hardware reset
 * to unbrick the display.
 *
 * uint8_t new_addr - new i2c address
 */
void SerLCD::setAddress(uint8_t new_addr)
{
  const uint8_t buffer[] = 
  {
    (SETTING_COMMAND),  //Send contrast command
    (ADDRESS_COMMAND), //0x18
    (new_addr)          //Send new contrast value
  };
  write(buffer, sizeof(buffer), 60);
  //Update our own address so we can still talk to the display
  _i2cAddr = new_addr;
} //setAddress

/*
 * getAddress
 * 
 * Returns private variable I2C address
 */
uint8_t SerLCD::getAddress()
{
  return _i2cAddr;
} //getAddress
