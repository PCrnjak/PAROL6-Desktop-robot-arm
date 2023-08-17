This fork follows the [upstream version](http://www.airspayce.com/mikem/arduino/AccelStepper/). Files are slightly reorganized to follow Arduino library conventions to allow for inclusion into the Arduino IDE library manager.

Please direct questions and discussion to http://groups.google.com/group/accelstepper

---

This is the Arduino AccelStepper library. It provides an object-oriented interface for 2, 3 or 4 pin stepper motors and motor drivers.

The standard Arduino IDE includes the Stepper library (http://arduino.cc/en/Reference/Stepper) for stepper motors. It is perfectly adequate for simple, single motor applications.

AccelStepper significantly improves on the standard Arduino Stepper library in several ways:

  - Supports acceleration and deceleration
  - Supports multiple simultaneous steppers, with independent concurrent stepping on each stepper
  - API functions never delay() or block
  - Supports 2, 3 and 4 wire steppers, plus 3 and 4 wire half steppers.
  - Supports alternate stepping functions to enable support of AFMotor (https://github.com/adafruit/Adafruit-Motor-Shield-library)
  - Supports stepper drivers such as the Sparkfun EasyDriver (based on 3967 driver chip)
  - Very slow speeds are supported
  - Extensive API
  - Subclass support

