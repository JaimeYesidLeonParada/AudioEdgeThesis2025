/* Sample output:

Starting TwoButtons...
Button 1 click.
Button 2 click.
Button 1 doubleclick.
Button 2 doubleclick.
Button 1 longPress start
Button 1 longPress...
Button 1 longPress...
Button 1 longPress...
Button 1 longPress stop
Button 2 longPress start
Button 2 longPress...
Button 2 longPress...
Button 2 longPress stop

*/

#include "OneButton.h"

#define PIN_INPUT1 9
#define PIN_INPUT2 18


// Setup a new OneButton on pin PIN_INPUT2.
OneButton button1(PIN_INPUT1, true);
// Setup a new OneButton on pin PIN_INPUT2.
OneButton button2(PIN_INPUT2, true);


// setup code here, to run once:
void setup() {
  // Setup the Serial port. see http://arduino.cc/en/Serial/IfSerial
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Starting TwoButtons...");

  // link the button 1 functions.
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);

  // link the button 2 functions.
  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);

}  // setup


// main code here, to run repeatedly:
void loop() {
  // keep watching the push buttons:
  button1.tick();
  button2.tick();

  // You can implement other code in here or just wait a while
  delay(10);
}  // loop


// ----- button 1 callback functions

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1() {
  Serial.println("BOOT Button click.");
}  // click1


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("BOOT Button doubleclick.");
}  // doubleclick1


// This function will be called once, when the button1 is pressed for a long time.
void longPressStart1() {
  Serial.println("BOOT Button longPress start");
}  // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
  Serial.println("BOOT Button longPress...");
}  // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  Serial.println("BOOT Button longPress stop");
}  // longPressStop1


// ... and the same for button 2:

void click2() {
  Serial.println("PWR Button click.");
}  // click2


void doubleclick2() {
  Serial.println("PWR Button doubleclick.");
}  // doubleclick2


void longPressStart2() {
  Serial.println("PWR Button longPress start");
}  // longPressStart2


void longPress2() {
  Serial.println("PWR Button longPress...");
}  // longPress2

void longPressStop2() {
  Serial.println("PWR Button longPress stop");
}  // longPressStop2


// End
