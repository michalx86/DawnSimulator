/* $Id$
||
|| @file	Button.cpp
|| @version 1.1
|| @author	Alexander Brevig	<alexanderbrevig@gmail.com>        
|| @url	http://alexanderbrevig.com
||
|| @description
|| | This is a Hardware Abstraction Library for Buttons
|| | It provides an easy way of handling buttons
|| |
|| #
||
|| @revisions
|| | 09_15_2018 v1.1 Added double-click callback method 
|| |                 by Ricardo Moreno https://github.com/rmorenojr/Button
|| #
||
|| @license LICENSE_REPLACE
||
*/

#include <Arduino.h>
#include "Button.h"

/*
|| @constructor
|| | Set the initial state of this button
|| #
|| 
|| @parameter buttonPin  sets the pin that this switch is connected to
|| @parameter buttonMode indicates BUTTON_PULLUP or BUTTON_PULLDOWN resistor
*/
Button::Button(uint8_t buttonPin, uint8_t buttonMode, bool _debounceMode, int _debounceDuration) : Button(buttonPin, buttonMode, _debounceMode, _debounceDuration, {1}) {
}

/*
|| @constructor
|| | Set the initial state of this button
|| #
|| 
|| @parameter buttonPin  sets the pin that this switch is connected to
|| @parameter buttonMode in case of BUTTON_MULTIKEY it is expected that _buttonLimits vector is defined
|| @parameter _debounceMode
|| @parameter _debounceDuration
|| @parameter _buttonLimits is used to determine witch of several buttons of keyboard module connected to this pin was pressed.
||             Specifies limits between readouts of pin values allowing to clasify an ANALOG readout to appropriated key.
||             If this vector is {v1, v2, .. vn}, where v1 < v2 < .. < vn then:
||            [   0..v1)  - 1-st button is pressed
||            [  v1..v2)  - 2-nd button is pressed
||            [vn-1..vn)  - n-th button is pressed
||            [  vn..inf) - All buttons are released. Keyboard module will pull up the pin in such case.
||
||            In case of single button connected to the pin (either  BUTTON_PULLUP or BUTTON_PULLDOWN) this vector should be defined as {1}. Then DIGITAL readout:
||            0 - button was pressed
||            1 - button was released
*/

Button::Button(uint8_t buttonPin, uint8_t buttonMode, bool _debounceMode, int _debounceDuration, std::vector<uint32_t> _buttonLimits) {
  pin=buttonPin;
  pinMode(pin,INPUT);
  
  switch (buttonMode) {
    case BUTTON_PULLDOWN:
      pulldown(); 
      break;
    case BUTTON_PULLUP:
    case BUTTON_PULLUP_INTERNAL:
      pullup(buttonMode);
      break;
    case BUTTON_MULTIKEY:
      mode = BUTTON_MULTIKEY;
      break;
    default:
      break;
  }

  debounceMode = _debounceMode;
  debounceDuration = _debounceDuration;
  doubleclickFound = false;
  pressedStartTime = -1;
  previouspressedStartTime = 0;
  releasedTime = 0;
  debounceStartTime = millis();
  cb_onPress = 0;
  cb_onRelease = 0;
  cb_onClick = 0;
  cb_onDoubleClick = 0;
  cb_onHold = 0;
  numberOfPresses = 0;
  triggeredHoldEvent = true;
  latchedKey = 0;
  buttonLimits = _buttonLimits;
  currentButtonState = buttonLimits.size();
  changed = false;
}



/*
|| @description
|| | Prepare logic for a pullup button.
|| | If mode is internal set pin HIGH as default
|| #
*/
void Button::pullup(uint8_t buttonMode)
{
  mode=BUTTON_PULLUP;
  if (buttonMode == BUTTON_PULLUP_INTERNAL) 
  {
	  digitalWrite(pin,HIGH);
  }
}

/*
|| @description
|| | Set pin LOW as default
|| #
*/
void Button::pulldown(void)
{
  mode=BUTTON_PULLDOWN;
}

/*
|| @description
|| | Read and write states; issue callbacks
|| #
|| 
|| @return true if button is pressed
*/
void Button::process(void)
{
  //save the previous value
  auto previousButtonState = currentButtonState;

  if (mode==BUTTON_MULTIKEY) {
    auto pinValue = analogRead(pin);
    for (currentButtonState = 0; currentButtonState < buttonLimits.size(); currentButtonState++) {
      if ( pinValue < buttonLimits[currentButtonState]) {
        break;
      }
    }
  } else {
    currentButtonState = (digitalRead(pin) == mode);
  }
  
  //handle state changes
  if (currentButtonState != previousButtonState)
  {
    // Serial.print("state changed: ");
    // Serial.println(currentButtonState);
    debounceStartTime = millis();
    return;  
  }
  else
  {
    unsigned int interval = millis() - debounceStartTime;
    // Debug Code:
    // if(debounceMode){
    //   Serial.print("debounceStartTime: ");
    //   Serial.print(debounceStartTime);
    //   Serial.print("\tdebounceDuration: ");
    //   Serial.print(debounceDuration);
    //   Serial.print("\tinterval: ");
    //   Serial.println(interval);
    // }
    if(debounceMode && (interval < debounceDuration)) {
      // not enough time has passed; ignore
      return;
    }
    
    if (currentButtonState < buttonLimits.size())  // is currently pressed...
    {
      if (pressedStartTime == -1) {  // ...and was NOT pressed up till now => the state changed to PRESSED
        numberOfPresses++;
        latchedKey = currentButtonState;
        if (cb_onPress) { cb_onPress(*this); }   //fire the onPress event
        pressedStartTime = millis();             //start timing
        triggeredHoldEvent = false;
        changed = true;
      } else {
        changed = false;
      }
    } 
    else // is NOT currently pressed...
    {
      if (pressedStartTime != -1) { // ...and was pressed up till now => the state changed to RELEASED
        if (cb_onRelease) { cb_onRelease(*this); } //fire the onRelease event
        if (cb_onClick) { cb_onClick(*this); }   //fire the onClick event AFTER the onRelease
        //reset states (for timing and for event triggering)
        
        //check for double-click
        unsigned interval = millis() - previouspressedStartTime;
        if (interval <= doubleclickThreshold) {
          //double-click occured
          doubleclickFound = true;
          if (cb_onDoubleClick) { cb_onDoubleClick(*this); } //fire the onDoubleClick event AFTER the onRelease
        } else {
          doubleclickFound = false;
        }
        previouspressedStartTime = pressedStartTime;              //store previous button press start time
        pressedStartTime = -1;
        changed = true;
      } else {
        changed = false;
      }
    }
    
    //should we trigger an onHold event?
    if (pressedStartTime!=-1 && !triggeredHoldEvent) 
    {
      if (millis()-pressedStartTime > holdEventThreshold) 
      { 
        if (cb_onHold) 
        { 
          cb_onHold(*this); 
          triggeredHoldEvent = true;
        }
      }
    }
  }
}

/*
|| @description
|| | Return the bitRead(state,CURRENT) of the switch
|| #
|| 
|| @return true if button is pressed
*/
bool Button::isPressed(bool proc)
{
  if(proc) process();
  return pressedStartTime != -1;
}

/*
|| @description
|| | Return the bitRead(state,CURRENT) of the switch
|| #
|| 
|| @return true if button is pressed
*/
bool Button::isDoubleClicked(bool proc)
{
  if(proc) process();
  return doubleclickFound;
}


/*
|| @description
|| | Return true if the button has been pressed
|| #
*/
bool Button::wasPressed(bool proc)
{
  return isPressed(proc);
}

/*
|| @description
|| | Return true if state has been changed
|| #
*/
bool Button::stateChanged(bool proc)
{
  if(proc) process();
  return changed;
}

/*
|| @description
|| | Return true if the button is pressed, and was not pressed before
|| #
*/
bool Button::uniquePress()
{
  process();
  return (isPressed(false) && stateChanged(false));
}

/*
|| @description
|| | onHold polling model
|| | Check to see if the button has been pressed for time ms
|| | This will clear the counter for next iteration and thus return true once
|| #
*/
bool Button::held(unsigned int time /*=0*/) 
{
  process();
  unsigned int threshold = time ? time : holdEventThreshold; //use holdEventThreshold if time == 0
	//should we trigger a onHold event?
  if (pressedStartTime!=-1 && !triggeredHoldEvent) 
  {
    if (millis()-pressedStartTime > threshold) 
    { 
      triggeredHoldEvent = true;
      return true;
    }
  }
  return false;
}

/*
|| @description
|| | Polling model for holding, this is true every check after hold time
|| | Check to see if the button has been pressed for time ms
|| #
*/
bool Button::heldFor(unsigned int time) 
{
  if (isPressed()) 
  {
    if (millis()-pressedStartTime > time) { return true; }
  }
  return false;
}

/*
|| @description
|| | Set the hold event time threshold
|| #
*/
void Button::setHoldThreshold(unsigned int holdTime) 
{ 
  holdEventThreshold = holdTime; 
}

/*
|| @description
|| | Set the double-click event time threshold
|| |  The time in ms when the first click starts and
|| |  second click ends
|| #
*/
void Button::setdoubleclickThreshold(unsigned int doublclickTime) 
{ 
  doubleclickThreshold = doublclickTime; 
}

/*
|| @description
|| | Register a handler for presses on this button
|| #
||
|| @parameter handler The function to call when this button is pressed
*/
void Button::pressHandler(buttonEventHandler handler)
{
  cb_onPress = handler;
}

/*
|| @description
|| | Register a handler for releases on this button
|| #
||
|| @parameter handler The function to call when this button is released
*/
void Button::releaseHandler(buttonEventHandler handler)
{
  cb_onRelease = handler;
}

/*
|| @description
|| | Register a handler for clicks on this button
|| #
||
|| @parameter handler The function to call when this button is clicked
*/
void Button::clickHandler(buttonEventHandler handler)
{
  cb_onClick = handler;
}

/*
|| @description
|| | Register a handler for double-clicks on this button
|| #
||
|| @parameter handler The function to call when this button is clicked
*/
void Button::doubleclickHandler(buttonEventHandler handler, unsigned int doublclickTime /*=900*/)
{
  cb_onDoubleClick = handler;
}

/*
|| @description
|| | Register a handler for when this button is held
|| #
||
|| @parameter handler The function to call when this button is held
*/
void Button::holdHandler(buttonEventHandler handler, unsigned int holdTime /*=0*/)
{
  if (holdTime>0) { setHoldThreshold(holdTime); }
  cb_onHold = handler;
}

/*
|| @description
|| | Get the time this button has been held
|| #
||
|| @return The time this button has been held
*/
unsigned int Button::holdTime() const { 
  if (pressedStartTime!=-1) { 
    return millis()-pressedStartTime; 
  } else return 0; 
}


/*
|| @description
|| | Compare a button object against this
|| #
|| 
|| @parameter  rhs the Button to compare against this Button
|| 
|| @return true if they are the same
*/
bool Button::operator==(Button &rhs) 
{
  return (this==&rhs);
}
