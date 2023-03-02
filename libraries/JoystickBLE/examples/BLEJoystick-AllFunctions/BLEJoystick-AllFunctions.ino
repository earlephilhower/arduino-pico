/* Benjamin Aigner, 2022 <beni@asterics-foundation.org> */
/* Public domain / CC 0 */

/** example code using all possibilities of the Joystick class
   for the RP2040.
*/

#include <JoystickBLE.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Use BOOTSEL to start the Joystick demo.");
  JoystickBLE.begin();
}

void loop() {
  if (BOOTSEL) {
    Serial.println("Joystick buttons");
    for (uint8_t i = 1; i <= 32; i++) {
      JoystickBLE.button(i, true);
      delay(250);
      JoystickBLE.button(i, false);
      delay(10); //we need a short delay here, sending packets with less than 1ms leads to packet loss!
    }
    //alternativ with manual send:
    JoystickBLE.useManualSend(true);
    Serial.println("Joystick buttons - manual send");
    for (uint8_t i = 1; i <= 32; i++) {
      JoystickBLE.button(i, true);
      JoystickBLE.send_now();
      delay(250);
      JoystickBLE.button(i, false);
    }
    JoystickBLE.useManualSend(false);

    //iterate all joystick axis
    //Note: although you can use 0-1023 here (10bit), internally 8bits are used (-127 to 127)
    Serial.println("Joystick X");
    for (uint16_t i = 0; i < 1023; i++) {
      JoystickBLE.X(i);
      delay(2);
    } JoystickBLE.X(512);
    Serial.println("Joystick Y");
    for (uint16_t i = 0; i < 1023; i++) {
      JoystickBLE.Y(i);
      delay(2);
    } JoystickBLE.Y(512);
    Serial.println("Joystick Z");
    for (uint16_t i = 0; i < 1023; i++) {
      JoystickBLE.Z(i);
      delay(2);
    } JoystickBLE.Z(512);
    Serial.println("Joystick Zrotate");
    for (uint16_t i = 0; i < 1023; i++) {
      JoystickBLE.Zrotate(i);
      delay(2);
    } JoystickBLE.Zrotate(512);
    Serial.println("Joystick sliderLeft");
    for (uint16_t i = 0; i < 1023; i++) {
      JoystickBLE.sliderLeft(i);
      delay(2);
    } JoystickBLE.sliderLeft(0);
    Serial.println("Joystick sliderRight");
    for (uint16_t i = 0; i < 1023; i++) {
      JoystickBLE.sliderRight(i);
      delay(2);
    } JoystickBLE.sliderRight(0);
    Serial.println("Joystick hat");
    for (uint16_t i = 0; i < 360; i++) {
      JoystickBLE.hat(i);
      delay(20);
    } JoystickBLE.hat(-1);

    //use int8 mode for the axis.
    //Note: hat is not used differently.
    Serial.println("Now all axis in 8bit mode, -127 to 127");
    JoystickBLE.use8bit(true);
    Serial.println("Joystick X");
    for (int16_t i = -127; i < 128; i++) {
      JoystickBLE.X(i);
      delay(2);
    } JoystickBLE.X(0);
    Serial.println("Joystick Y");
    for (int16_t i = -127; i < 128; i++) {
      JoystickBLE.Y(i);
      delay(2);
    } JoystickBLE.Y(0);
    Serial.println("Joystick Z");
    for (int16_t i = -127; i < 128; i++) {
      JoystickBLE.Z(i);
      delay(2);
    } JoystickBLE.Z(0);
    Serial.println("Joystick Zrotate");
    for (int16_t i = -127; i < 128; i++) {
      JoystickBLE.Zrotate(i);
      delay(2);
    } JoystickBLE.Zrotate(0);
    Serial.println("Joystick sliderLeft");
    for (int16_t i = -127; i < 128; i++) {
      JoystickBLE.sliderLeft(i);
      delay(2);
    } JoystickBLE.sliderLeft(0);
    Serial.println("Joystick sliderRight");
    for (int16_t i = -127; i < 128; i++) {
      JoystickBLE.sliderRight(i);
      delay(2);
    } JoystickBLE.sliderRight(0);
    JoystickBLE.use8bit(false);
  }
}
