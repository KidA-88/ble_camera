/*
  Saving camera stills over UART on SparkFun Edge board
  By: Owen Lyke
  SparkFun Electronics
  Date: November 18th 2019
  This example code is in the public domain

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15170

  This example shows how to use the HM01B0 camera. It will stream frames captured by the
  camera over the UART.

  To see images:
  1. upload this sketch (using baud 460800 is recommended)
  2. navigate to the library folder (~/.../Arduino/libraries/SparkFun_Himax_HM01B0_Camera)
  3. run the command 'python3 utils/Example1_StopMotion.py -p COM_PORT (COM_PORT is the same as the port used for upload in Arduino)
  4. say "cheese"
*/
//BLE
#include <ArduinoBLE.h>
#include "utility/HCI.h"    // needed for sendcommand
// MAX Up to 29 characters for BLE name
const char BLE_PERIPHERAL_NAME[] = "Peripheral Hello World BLE";
BLEService Test("83436682-10c6-4dad-acb1-133927f3f080");
#define SERIAL_PORT Serial
//BLEStringCharacteristic HelloWorlds("83436682-10c6-4dad-acb1-133927f3f080", BLEBroadcast, 100);
BLEStringCharacteristic HelloWorlds("83436682-10c6-4dad-acb1-133927f3f080", BLERead | BLENotify, 100);
//BLE


#include "hm01b0_arduino.h"
//#include "String.h"

///////////////////
// Begin User Setup

//#define SERIAL_PORT Serial
#define BAUD_RATE   115200 //(originally, 460800)

//#define DEMO_HM01B0_TEST_MODE_ENABLE          // Uncomment to enable test pattern generation
//#define DEMO_HM01B0_FRAMEBUFFER_DUMP_ENABLE   // Uncomment to enable frame output
#define DEMO_HM01B0_PYTHON                    // Uncomment to use Python script to visualize data
                                                // (go to the sketch directory and use 'python3 utils/Example1_StopMotion.py -p {COM_PORT}'

// End User Setup
/////////////////

HM01B0 myCamera;            // Declare an HM01B0 object called 'myCamera'
                            // The camera will try to specialize for the host architecture
                            // however it will fall back to a slow generic interface if no
                            // specialization is available.
                            // The default is not guaranteed to work due to the high amount
                            // of data the camera needs to transfer


// Auto-configure for python if requested
#ifdef DEMO_HM01B0_PYTHON
#define DEMO_HM01B0_FRAMEBUFFER_DUMP_ENABLE
#undef  DEMO_HM01B0_TEST_MODE_ENABLE
#endif // DEMO_HM01B0_PYTHON

// Forward declarations
void printWord(uint32_t num);
void printByte(uint8_t num);

void setup() {

  delay(2000);
  // Start up serial monitor
  SERIAL_PORT.begin(BAUD_RATE);
  SERIAL_PORT.println("start setup");
  do {
    delay(500);
  }while(!SERIAL_PORT);
  
//BLE part begin
#ifdef BLE_Debug
  BLE.debug(SERIAL_PORT);         // enable display HCI commands
#endif
  BLE.setLocalName(BLE_PERIPHERAL_NAME);
  BLE.begin();
  
    // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(Test);

  Test.addCharacteristic(HelloWorlds);

  BLE.addService(Test);
  BLE.advertise();
//BLE part end
  
  // Turn on camera regulator if using Edge board
#if defined (AM_BSP_GPIO_CAMERA_HM01B0_DVDDEN)
  // D10 is NOT defined on EDGE by default. see README
  //pinMode(AM_BSP_GPIO_CAMERA_HM01B0_DVDDEN, OUTPUT);
  //digitalWrite(AM_BSP_GPIO_CAMERA_HM01B0_DVDDEN, HIGH);
  am_hal_gpio_pinconfig(AM_BSP_GPIO_CAMERA_HM01B0_DVDDEN, g_AM_HAL_GPIO_OUTPUT);
  am_hal_gpio_state_write(AM_BSP_GPIO_CAMERA_HM01B0_DVDDEN, AM_HAL_GPIO_OUTPUT_SET);
  SERIAL_PORT.println("Turned on camera regulator");
#endif

  // Start the camera
  if(myCamera.begin() != HM01B0_ERR_OK){
    SERIAL_PORT.print("Camera.begin() failed with code: " + String(myCamera.status) + "\n");
  }else{
    SERIAL_PORT.print("Camera started successfully\n");
  }

  // Calibrate Autoexposure
  SERIAL_PORT.println("Calibrating Auto Exposure...");
  myCamera.calibrateAutoExposure();
  if(myCamera.status != HM01B0_ERR_OK){
    SERIAL_PORT.println("\tnot converged");
  }else{
    SERIAL_PORT.println("\tconverged!");
  }

#ifdef DEMO_HM01B0_TEST_MODE_ENABLE
  // Enable test mode (generates a 'walking 1s' pattern to verify interface function
  SERIAL_PORT.print("Enabling test mode...\n");
  myCamera.enableTestMode();
  if(myCamera.status != HM01B0_ERR_OK){
    SERIAL_PORT.print("\tfailed\n");
  }else{
    SERIAL_PORT.print("\tsucceeded!\n");
  }

  // In test mode capturing a frame fills the buffer with the test pattern
  myCamera.capture();

  uint32_t mismatches = myCamera.countTestMismatches();
  SERIAL_PORT.print("Self-test mismatches: 0x");
  printWord(mismatches);
  SERIAL_PORT.print("\n");
#endif

  SERIAL_PORT.write(0x55);                                                    // Special character to sync Python script
  SERIAL_PORT.print("\n\n");                                                  // Newlines allow Python script to find frame start
}

void loop() {
  //BLE
  //delay(2000);
  SERIAL_PORT.print("before BLE");

  //BLE
  SERIAL_PORT.print("before BLE");

  // Take an image
  myCamera.capture();

#ifdef DEMO_HM01B0_FRAMEBUFFER_DUMP_ENABLE
  // Print out a frame for the Python script to pick up
  framebuffer_dump();
#else
  // Print auto exposure state
  SERIAL_PORT.print("AE convergance(0x");
  printByte(myCamera.aeConvergenceStatus);
  SERIAL_PORT.print(") TargetMean 0x");
  printByte(myCamera.aecfg.ui8AETargetMean);
  SERIAL_PORT.print(", ConvergeInTh 0x");
  printByte(myCamera.aecfg.ui8ConvergeInTh);
  SERIAL_PORT.print(", AEMean 0x");
  printByte(myCamera.aecfg.ui8AEMean);
  SERIAL_PORT.print("\n");
#endif


//  // Wait a second
//  delay(1000);
}

//
// Utility functions

// hex formating
// Thanks to bootsector on the Arduino forums:
// https://forum.arduino.cc/index.php?topic=38107.msg282336#msg282336
void printWord(uint32_t num) {
  char tmp[9];                  // 8 hex digits + null terminator
  sprintf(tmp, "%08X", num);
  SERIAL_PORT.print(tmp);
}

void printByte(uint8_t num) {
  char tmp[3];                  // 2 hex digits + null terminator
  sprintf(tmp, "%02X", num);
  SERIAL_PORT.print(tmp);
}

// frame buffer dump (formatted for python script)
void framebuffer_dump( void ){
  SERIAL_PORT.print("+++ frame +++");                                         // Mark frame start
  for (uint32_t ui32Idx = 0; ui32Idx < myCamera.frameBufferSize; ui32Idx++){  // Process all bytes in frame
    if ((ui32Idx & 0xF) == 0x00){                                             // Print address every 16 bytes
      SERIAL_PORT.print("\n0x");
      printWord(ui32Idx);
      SERIAL_PORT.print(" ");
      //BLE send command every 16byte step
      BLE.poll();
      HelloWorlds.writeValue("16byte step");
    }
    printByte(myCamera.frameBuffer[ui32Idx]);                                 // Print byte value
    SERIAL_PORT.print(" ");
    //BLE sned out byte values
    BLE.poll();
    HelloWorlds.writeValue(String(myCamera.frameBuffer[ui32Idx]));
  }
  SERIAL_PORT.print("\n--- frame ---\n");
  memset(myCamera.frameBuffer, 0x00, sizeof(myCamera.frameBufferSize));       // Zero out frame buffer for help identifying errors
}
