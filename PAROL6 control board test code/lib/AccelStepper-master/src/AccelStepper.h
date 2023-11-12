// AccelStepper.h
//
/// \mainpage AccelStepper library for Arduino
///
/// This is the Arduino AccelStepper library.
/// It provides an object-oriented interface for 2, 3 or 4 pin stepper motors and motor drivers.
///
/// The standard Arduino IDE includes the Stepper library
/// (http://arduino.cc/en/Reference/Stepper) for stepper motors. It is
/// perfectly adequate for simple, single motor applications.
///
/// AccelStepper significantly improves on the standard Arduino Stepper library in several ways:
/// \li Supports acceleration and deceleration
/// \li Supports multiple simultaneous steppers, with independent concurrent stepping on each stepper
/// \li Most API functions never delay() or block (unless otherwise stated)
/// \li Supports 2, 3 and 4 wire steppers, plus 3 and 4 wire half steppers.
/// \li Supports alternate stepping functions to enable support of AFMotor (https://github.com/adafruit/Adafruit-Motor-Shield-library)
/// \li Supports stepper drivers such as the Sparkfun EasyDriver (based on 3967 driver chip)
/// \li Very slow speeds are supported
/// \li Extensive API
/// \li Subclass support
///
/// The latest version of this documentation can be downloaded from 
/// http://www.airspayce.com/mikem/arduino/AccelStepper
/// The version of the package that this documentation refers to can be downloaded 
/// from http://www.airspayce.com/mikem/arduino/AccelStepper/AccelStepper-1.64.zip
///
/// Example Arduino programs are included to show the main modes of use.
///
/// You can also find online help and discussion at http://groups.google.com/group/accelstepper
/// Please use that group for all questions and discussions on this topic. 
/// Do not contact the author directly, unless it is to discuss commercial licensing.
/// Before asking a question or reporting a bug, please read 
/// - http://en.wikipedia.org/wiki/Wikipedia:Reference_desk/How_to_ask_a_software_question
/// - http://www.catb.org/esr/faqs/smart-questions.html
/// - http://www.chiark.greenend.org.uk/~shgtatham/bugs.html
///
/// Beginners to C++ and stepper motors in general may find this helpful:
/// - https://hackaday.io/project/183279-accelstepper-the-missing-manual
/// - https://hackaday.io/project/183713-using-the-arduino-accelstepper-library
///
/// Tested on Arduino Diecimila and Mega with arduino-0018 & arduino-0021 
/// on OpenSuSE 11.1 and avr-libc-1.6.1-1.15,
/// cross-avr-binutils-2.19-9.1, cross-avr-gcc-4.1.3_20080612-26.5.
/// Tested on Teensy http://www.pjrc.com/teensy including Teensy 3.1 built using Arduino IDE 1.0.5 with 
/// teensyduino addon 1.18 and later.
///
/// \par Installation
///
/// Install in the usual way: unzip the distribution zip file to the libraries
/// sub-folder of your sketchbook. 
///
/// \par Theory
///
/// This code uses speed calculations as described in 
/// "Generate stepper-motor speed profiles in real time" by David Austin 
/// http://fab.cba.mit.edu/classes/MIT/961.09/projects/i0/Stepper_Motor_Speed_Profile.pdf or
/// http://www.embedded.com/design/mcus-processors-and-socs/4006438/Generate-stepper-motor-speed-profiles-in-real-time or
/// http://web.archive.org/web/20140705143928/http://fab.cba.mit.edu/classes/MIT/961.09/projects/i0/Stepper_Motor_Speed_Profile.pdf
/// with the exception that AccelStepper uses steps per second rather than radians per second
/// (because we dont know the step angle of the motor)
/// An initial step interval is calculated for the first step, based on the desired acceleration
/// On subsequent steps, shorter step intervals are calculated based 
/// on the previous step until max speed is achieved.
///
/// \par Adafruit Motor Shield V2
///
/// The included examples AFMotor_* are for Adafruit Motor Shield V1 and do not work with Adafruit Motor Shield V2.
/// See https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library for examples that work with Adafruit Motor Shield V2.
/// 
/// \par Donations
///
/// This library is offered under a free GPL license for those who want to use it that way. 
/// We try hard to keep it up to date, fix bugs
/// and to provide free support. If this library has helped you save time or money, please consider donating at
/// http://www.airspayce.com or here:
///
/// \htmlonly <form action="https://www.paypal.com/cgi-bin/webscr" method="post"><input type="hidden" name="cmd" value="_donations" /> <input type="hidden" name="business" value="mikem@airspayce.com" /> <input type="hidden" name="lc" value="AU" /> <input type="hidden" name="item_name" value="Airspayce" /> <input type="hidden" name="item_number" value="AccelStepper" /> <input type="hidden" name="currency_code" value="USD" /> <input type="hidden" name="bn" value="PP-DonationsBF:btn_donateCC_LG.gif:NonHosted" /> <input type="image" alt="PayPal — The safer, easier way to pay online." name="submit" src="https://www.paypalobjects.com/en_AU/i/btn/btn_donateCC_LG.gif" /> <img alt="" src="https://www.paypalobjects.com/en_AU/i/scr/pixel.gif" width="1" height="1" border="0" /></form> \endhtmlonly
/// 
/// \par Trademarks
///
/// AccelStepper is a trademark of AirSpayce Pty Ltd. The AccelStepper mark was first used on April 26 2010 for
/// international trade, and is used only in relation to motor control hardware and software.
/// It is not to be confused with any other similar marks covering other goods and services.
///
/// \par Copyright
///
/// This software is Copyright (C) 2010-2021 Mike McCauley. Use is subject to license
/// conditions. The main licensing options available are GPL V3 or Commercial:
///
/// \par Open Source Licensing GPL V3
/// This is the appropriate option if you want to share the source code of your
/// application with everyone you distribute it to, and you also want to give them
/// the right to share who uses it. If you wish to use this software under Open
/// Source Licensing, you must contribute all your source code to the open source
/// community in accordance with the GPL Version 23 when your application is
/// distributed. See https://www.gnu.org/licenses/gpl-3.0.html
/// 
/// \par Commercial Licensing
/// This is the appropriate option if you are creating proprietary applications
/// and you are not prepared to distribute and share the source code of your
/// application. To purchase a commercial license, contact info@airspayce.com
///
/// \par Revision History
/// \version 1.0 Initial release
///
/// \version 1.1 Added speed() function to get the current speed.
/// \version 1.2 Added runSpeedToPosition() submitted by Gunnar Arndt.
/// \version 1.3 Added support for stepper drivers (ie with Step and Direction inputs) with _pins == 1
/// \version 1.4 Added functional contructor to support AFMotor, contributed by Limor, with example sketches.
/// \version 1.5 Improvements contributed by Peter Mousley: Use of microsecond steps and other speed improvements
///              to increase max stepping speed to about 4kHz. New option for user to set the min allowed pulse width.
///              Added checks for already running at max speed and skip further calcs if so. 
/// \version 1.6 Fixed a problem with wrapping of microsecond stepping that could cause stepping to hang. 
///              Reported by Sandy Noble.
///              Removed redundant _lastRunTime member.
/// \version 1.7 Fixed a bug where setCurrentPosition() did not always work as expected. 
///              Reported by Peter Linhart.
/// \version 1.8 Added support for 4 pin half-steppers, requested by Harvey Moon
/// \version 1.9 setCurrentPosition() now also sets motor speed to 0.
/// \version 1.10 Builds on Arduino 1.0
/// \version 1.11 Improvments from Michael Ellison:
///   Added optional enable line support for stepper drivers
///   Added inversion for step/direction/enable lines for stepper drivers
/// \version 1.12 Announce Google Group
/// \version 1.13 Improvements to speed calculation. Cost of calculation is now less in the worst case, 
///    and more or less constant in all cases. This should result in slightly beter high speed performance, and
///    reduce anomalous speed glitches when other steppers are accelerating. 
///    However, its hard to see how to replace the sqrt() required at the very first step from 0 speed.
/// \version 1.14 Fixed a problem with compiling under arduino 0021 reported by EmbeddedMan
/// \version 1.15 Fixed a problem with runSpeedToPosition which did not correctly handle
///    running backwards to a smaller target position. Added examples
/// \version 1.16 Fixed some cases in the code where abs() was used instead of fabs().
/// \version 1.17 Added example ProportionalControl
/// \version 1.18 Fixed a problem: If one calls the funcion runSpeed() when Speed is zero, it makes steps 
///    without counting. reported by  Friedrich, Klappenbach.
/// \version 1.19 Added MotorInterfaceType and symbolic names for the number of pins to use
///               for the motor interface. Updated examples to suit.
///               Replaced individual pin assignment variables _pin1, _pin2 etc with array _pin[4].
///               _pins member changed to _interface.
///               Added _pinInverted array to simplify pin inversion operations.
///               Added new function setOutputPins() which sets the motor output pins.
///               It can be overridden in order to provide, say, serial output instead of parallel output
///               Some refactoring and code size reduction.
/// \version 1.20 Improved documentation and examples to show need for correctly
///               specifying AccelStepper::FULL4WIRE and friends.
/// \version 1.21 Fixed a problem where desiredSpeed could compute the wrong step acceleration
///               when _speed was small but non-zero. Reported by Brian Schmalz.
///               Precompute sqrt_twoa to improve performance and max possible stepping speed
/// \version 1.22 Added Bounce.pde example
///               Fixed a problem where calling moveTo(), setMaxSpeed(), setAcceleration() more 
///               frequently than the step time, even
///               with the same values, would interfere with speed calcs. Now a new speed is computed 
///               only if there was a change in the set value. Reported by Brian Schmalz.
/// \version 1.23 Rewrite of the speed algorithms in line with 
///               http://fab.cba.mit.edu/classes/MIT/961.09/projects/i0/Stepper_Motor_Speed_Profile.pdf
///               Now expect smoother and more linear accelerations and decelerations. The desiredSpeed()
///               function was removed.
/// \version 1.24  Fixed a problem introduced in 1.23: with runToPosition, which did never returned
/// \version 1.25  Now ignore attempts to set acceleration to 0.0
/// \version 1.26  Fixed a problem where certina combinations of speed and accelration could cause
///                oscillation about the target position.
/// \version 1.27  Added stop() function to stop as fast as possible with current acceleration parameters.
///                Also added new Quickstop example showing its use.
/// \version 1.28  Fixed another problem where certain combinations of speed and acceleration could cause
///                oscillation about the target position.
///                Added support for 3 wire full and half steppers such as Hard Disk Drive spindle.
///                Contributed by Yuri Ivatchkovitch.
/// \version 1.29  Fixed a problem that could cause a DRIVER stepper to continually step
///                with some sketches. Reported by Vadim.
/// \version 1.30  Fixed a problem that could cause stepper to back up a few steps at the end of
///                accelerated travel with certain speeds. Reported and patched by jolo.
/// \version 1.31  Updated author and distribution location details to airspayce.com
/// \version 1.32  Fixed a problem with enableOutputs() and setEnablePin on Arduino Due that
///                prevented the enable pin changing stae correctly. Reported by Duane Bishop.
/// \version 1.33  Fixed an error in example AFMotor_ConstantSpeed.pde did not setMaxSpeed();
///                Fixed a problem that caused incorrect pin sequencing of FULL3WIRE and HALF3WIRE.
///                Unfortunately this meant changing the signature for all step*() functions.
///                Added example MotorShield, showing how to use AdaFruit Motor Shield to control
///                a 3 phase motor such as a HDD spindle motor (and without using the AFMotor library.
/// \version 1.34  Added setPinsInverted(bool pin1Invert, bool pin2Invert, bool pin3Invert, bool pin4Invert, bool enableInvert) 
///                to allow inversion of 2, 3 and 4 wire stepper pins. Requested by Oleg.
/// \version 1.35  Removed default args from setPinsInverted(bool, bool, bool, bool, bool) to prevent ambiguity with 
///                setPinsInverted(bool, bool, bool). Reported by Mac Mac.
/// \version 1.36  Changed enableOutputs() and disableOutputs() to be virtual so can be overridden.
///                Added new optional argument 'enable' to constructor, which allows you toi disable the 
///                automatic enabling of outputs at construction time. Suggested by Guido.
/// \version 1.37  Fixed a problem with step1 that could cause a rogue step in the 
///                wrong direction (or not,
///                depending on the setup-time requirements of the connected hardware). 
///                Reported by Mark Tillotson.
/// \version 1.38  run() function incorrectly always returned true. Updated function and doc so it returns true 
///                if the motor is still running to the target position.
/// \version 1.39  Updated typos in keywords.txt, courtesey Jon Magill.
/// \version 1.40  Updated documentation, including testing on Teensy 3.1
/// \version 1.41  Fixed an error in the acceleration calculations, resulting in acceleration of haldf the intended value
/// \version 1.42  Improved support for FULL3WIRE and HALF3WIRE output pins. These changes were in Yuri's original
///                contribution but did not make it into production.<br>
/// \version 1.43  Added DualMotorShield example. Shows how to use AccelStepper to control 2 x 2 phase steppers using the 
///                Itead Studio Arduino Dual Stepper Motor Driver Shield model IM120417015.<br>
/// \version 1.44  examples/DualMotorShield/DualMotorShield.ino examples/DualMotorShield/DualMotorShield.pde
///                was missing from the distribution.<br>
/// \version 1.45  Fixed a problem where if setAcceleration was not called, there was no default
///                acceleration. Reported by Michael Newman.<br>
/// \version 1.45  Fixed inaccuracy in acceleration rate by using Equation 15, suggested by Sebastian Gracki.<br>
///                Performance improvements in runSpeed suggested by Jaakko Fagerlund.<br>
/// \version 1.46  Fixed error in documentation for runToPosition().
///                Reinstated time calculations in runSpeed() since new version is reported 
///                not to work correctly under some circumstances. Reported by Oleg V Gavva.<br>
/// \version 1.48  2015-08-25
///                Added new class MultiStepper that can manage multiple AccelSteppers, 
///                and cause them all to move
///                to selected positions at such a (constant) speed that they all arrive at their
///                target position at the same time. Suitable for X-Y flatbeds etc.<br>
///                Added new method maxSpeed() to AccelStepper to return the currently configured maxSpeed.<br>
/// \version 1.49  2016-01-02
///                Testing with VID28 series instrument stepper motors and EasyDriver.
///                OK, although with light pointers
///                and slow speeds like 180 full steps per second the motor movement can be erratic, 
///                probably due to some mechanical resonance. Best to accelerate through this speed.<br>
///                Added isRunning().<br>
/// \version 1.50 2016-02-25
///                AccelStepper::disableOutputs now sets the enable pion to OUTPUT mode if the enable pin is defined.
///                Patch from Piet De Jong.<br>
///                Added notes about the fact that AFMotor_* examples do not work with Adafruit Motor Shield V2.<br>
/// \version 1.51 2016-03-24
///                Fixed a problem reported by gregor: when resetting the stepper motor position using setCurrentPosition() the 
///                stepper speed is reset by setting _stepInterval to 0, but _speed is not 
///                reset. this results in the stepper motor not starting again when calling 
///                setSpeed() with the same speed the stepper was set to before.
/// \version 1.52 2016-08-09
///                Added MultiStepper to keywords.txt.
///                Improvements to efficiency of AccelStepper::runSpeed() as suggested by David Grayson.
///                Improvements to speed accuracy as suggested by David Grayson.
/// \version 1.53 2016-08-14
///                Backed out Improvements to speed accuracy from 1.52 as it did not work correctly.
/// \version 1.54 2017-01-24
///                Fixed some warnings about unused arguments.
/// \version 1.55 2017-01-25
///                Fixed another warning in MultiStepper.cpp
/// \version 1.56 2017-02-03
///                Fixed minor documentation error with DIRECTION_CCW and DIRECTION_CW. Reported by David Mutterer.
///                Added link to Binpress commercial license purchasing.
/// \version 1.57 2017-03-28
///                _direction moved to protected at the request of Rudy Ercek.
///                setMaxSpeed() and setAcceleration() now correct negative values to be positive.
/// \version 1.58 2018-04-13
///                Add initialisation for _enableInverted in constructor.
/// \version 1.59 2018-08-28
///                Update commercial licensing, remove binpress.
/// \version 1.60 2020-03-07
///                Release under GPL V3
/// \version 1.61 2020-04-20
///                Added yield() call in runToPosition(), so that platforms like esp8266 dont hang/crash
///                during long runs.
/// \version 1.62 2022-05-22
///                Added link to AccelStepper - The Missing Manual.<br>
///                Fixed a problem when setting the maxSpeed to 1.0 due to incomplete initialisation.
///                Reported by Olivier Pécheux. <br>
/// \version 1.63 2022-06-30
///                Added virtual destructor at the request of Jan.<br>
/// \version 1.64 2022-10-31
///                Patch courtesy acwest: Changes to make AccelStepper more subclassable. These changes are
///                largely oriented to implementing new step-scheduling algorithms.
///
/// \author  Mike McCauley (mikem@airspayce.com) DO NOT CONTACT THE AUTHOR DIRECTLY: USE THE GOOGLE GROUP
// Copyright (C) 2009-2020 Mike McCauley
// $Id: AccelStepper.h,v 1.28 2020/04/20 00:15:03 mikem Exp mikem $

#ifndef AccelStepper_h
#define AccelStepper_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

// These defs cause trouble on some versions of Arduino
#undef round

// Use the system yield() whenever possoible, since some platforms require it for housekeeping, especially
// ESP8266
#if (defined(ARDUINO) && ARDUINO >= 155) || defined(ESP8266)
 #define YIELD yield();
#else
 #define YIELD
#endif

/////////////////////////////////////////////////////////////////////
/// \class AccelStepper AccelStepper.h <AccelStepper.h>
/// \brief Support for stepper motors with acceleration etc.
///
/// This defines a single 2 or 4 pin stepper motor, or stepper moter with fdriver chip, with optional
/// acceleration, deceleration, absolute positioning commands etc. Multiple
/// simultaneous steppers are supported, all moving 
/// at different speeds and accelerations. 
///
/// \par Operation
/// This module operates by computing a step time in microseconds. The step
/// time is recomputed after each step and after speed and acceleration
/// parameters are changed by the caller. The time of each step is recorded in
/// microseconds. The run() function steps the motor once if a new step is due.
/// The run() function must be called frequently until the motor is in the
/// desired position, after which time run() will do nothing.
///
/// \par Positioning
/// Positions are specified by a signed long integer. At
/// construction time, the current position of the motor is consider to be 0. Positive
/// positions are clockwise from the initial position; negative positions are
/// anticlockwise. The current position can be altered for instance after
/// initialization positioning.
///
/// \par Caveats
/// This is an open loop controller: If the motor stalls or is oversped,
/// AccelStepper will not have a correct 
/// idea of where the motor really is (since there is no feedback of the motor's
/// real position. We only know where we _think_ it is, relative to the
/// initial starting point).
///
/// \par Performance
/// The fastest motor speed that can be reliably supported is about 4000 steps per
/// second at a clock frequency of 16 MHz on Arduino such as Uno etc. 
/// Faster processors can support faster stepping speeds. 
/// However, any speed less than that
/// down to very slow speeds (much less than one per second) are also supported,
/// provided the run() function is called frequently enough to step the motor
/// whenever required for the speed set.
/// Calling setAcceleration() is expensive,
/// since it requires a square root to be calculated.
///
/// Gregor Christandl reports that with an Arduino Due and a simple test program, 
/// he measured 43163 steps per second using runSpeed(), 
/// and 16214 steps per second using run();
class AccelStepper
{
public:
    /// \brief Symbolic names for number of pins.
    /// Use this in the pins argument the AccelStepper constructor to 
    /// provide a symbolic name for the number of pins
    /// to use.
    typedef enum
    {
	FUNCTION  = 0, ///< Use the functional interface, implementing your own driver functions (internal use only)
	DRIVER    = 1, ///< Stepper Driver, 2 driver pins required
	FULL2WIRE = 2, ///< 2 wire stepper, 2 motor pins required
	FULL3WIRE = 3, ///< 3 wire stepper, such as HDD spindle, 3 motor pins required
        FULL4WIRE = 4, ///< 4 wire full stepper, 4 motor pins required
	HALF3WIRE = 6, ///< 3 wire half stepper, such as HDD spindle, 3 motor pins required
	HALF4WIRE = 8  ///< 4 wire half stepper, 4 motor pins required
    } MotorInterfaceType;

    /// Constructor. You can have multiple simultaneous steppers, all moving
    /// at different speeds and accelerations, provided you call their run()
    /// functions at frequent enough intervals. Current Position is set to 0, target
    /// position is set to 0. MaxSpeed and Acceleration default to 1.0.
    /// The motor pins will be initialised to OUTPUT mode during the
    /// constructor by a call to enableOutputs().
    /// \param[in] interface Number of pins to interface to. Integer values are
    /// supported, but it is preferred to use the \ref MotorInterfaceType symbolic names. 
    /// AccelStepper::DRIVER (1) means a stepper driver (with Step and Direction pins).
    /// If an enable line is also needed, call setEnablePin() after construction.
    /// You may also invert the pins using setPinsInverted().
    /// Caution: DRIVER implements a blocking delay of minPulseWidth microseconds (default 1us) for each step.
    /// You can change this with setMinPulseWidth().
    /// AccelStepper::FULL2WIRE (2) means a 2 wire stepper (2 pins required). 
    /// AccelStepper::FULL3WIRE (3) means a 3 wire stepper, such as HDD spindle (3 pins required). 
    /// AccelStepper::FULL4WIRE (4) means a 4 wire stepper (4 pins required). 
    /// AccelStepper::HALF3WIRE (6) means a 3 wire half stepper, such as HDD spindle (3 pins required)
    /// AccelStepper::HALF4WIRE (8) means a 4 wire half stepper (4 pins required)
    /// Defaults to AccelStepper::FULL4WIRE (4) pins.
    /// \param[in] pin1 Arduino digital pin number for motor pin 1. Defaults
    /// to pin 2. For a AccelStepper::DRIVER (interface==1), 
    /// this is the Step input to the driver. Low to high transition means to step)
    /// \param[in] pin2 Arduino digital pin number for motor pin 2. Defaults
    /// to pin 3. For a AccelStepper::DRIVER (interface==1), 
    /// this is the Direction input the driver. High means forward.
    /// \param[in] pin3 Arduino digital pin number for motor pin 3. Defaults
    /// to pin 4.
    /// \param[in] pin4 Arduino digital pin number for motor pin 4. Defaults
    /// to pin 5.
    /// \param[in] enable If this is true (the default), enableOutputs() will be called to enable
    /// the output pins at construction time.
    AccelStepper(uint8_t interface = AccelStepper::FULL4WIRE, uint8_t pin1 = 2, uint8_t pin2 = 3, uint8_t pin3 = 4, uint8_t pin4 = 5, bool enable = true);

    /// Alternate Constructor which will call your own functions for forward and backward steps. 
    /// You can have multiple simultaneous steppers, all moving
    /// at different speeds and accelerations, provided you call their run()
    /// functions at frequent enough intervals. Current Position is set to 0, target
    /// position is set to 0. MaxSpeed and Acceleration default to 1.0.
    /// Any motor initialization should happen before hand, no pins are used or initialized.
    /// \param[in] forward void-returning procedure that will make a forward step
    /// \param[in] backward void-returning procedure that will make a backward step
    AccelStepper(void (*forward)(), void (*backward)());
    
    /// Set the target position. The run() function will try to move the motor (at most one step per call)
    /// from the current position to the target position set by the most
    /// recent call to this function. Caution: moveTo() also recalculates the speed for the next step. 
    /// If you are trying to use constant speed movements, you should call setSpeed() after calling moveTo().
    /// \param[in] absolute The desired absolute position. Negative is
    /// anticlockwise from the 0 position.
    void    moveTo(long absolute); 

    /// Set the target position relative to the current position.
    /// \param[in] relative The desired position relative to the current position. Negative is
    /// anticlockwise from the current position.
    void    move(long relative);

    /// Poll the motor and step it if a step is due, implementing
    /// accelerations and decelerations to achieve the target position. You must call this as
    /// frequently as possible, but at least once per minimum step time interval,
    /// preferably in your main loop. Note that each call to run() will make at most one step, and then only when a step is due,
    /// based on the current speed and the time since the last step.
    /// \return true if the motor is still running to the target position.
    boolean run();

    /// Poll the motor and step it if a step is due, implementing a constant
    /// speed as set by the most recent call to setSpeed(). You must call this as
    /// frequently as possible, but at least once per step interval,
    /// \return true if the motor was stepped.
    boolean runSpeed();

    /// Sets the maximum permitted speed. The run() function will accelerate
    /// up to the speed set by this function.
    /// Caution: the maximum speed achievable depends on your processor and clock speed.
    /// The default maxSpeed is 1.0 steps per second.
    /// \param[in] speed The desired maximum speed in steps per second. Must
    /// be > 0. Caution: Speeds that exceed the maximum speed supported by the processor may
    /// Result in non-linear accelerations and decelerations.
    void    setMaxSpeed(float speed);

    /// Returns the maximum speed configured for this stepper
    /// that was previously set by setMaxSpeed();
    /// \return The currently configured maximum speed
    float   maxSpeed();

    /// Sets the acceleration/deceleration rate.
    /// \param[in] acceleration The desired acceleration in steps per second
    /// per second. Must be > 0.0. This is an expensive call since it requires a square 
    /// root to be calculated. Dont call more ofthen than needed
    void    setAcceleration(float acceleration);

    /// Returns the acceleration/deceleration rate configured for this stepper
    /// that was previously set by setAcceleration();
    /// \return The currently configured acceleration/deceleration
    float   acceleration();
    
    /// Sets the desired constant speed for use with runSpeed().
    /// \param[in] speed The desired constant speed in steps per
    /// second. Positive is clockwise. Speeds of more than 1000 steps per
    /// second are unreliable. Very slow speeds may be set (eg 0.00027777 for
    /// once per hour, approximately. Speed accuracy depends on the Arduino
    /// crystal. Jitter depends on how frequently you call the runSpeed() function.
    /// The speed will be limited by the current value of setMaxSpeed()
    void    setSpeed(float speed);

    /// The most recently set speed.
    /// \return the most recent speed in steps per second
    float   speed();

    /// The distance from the current position to the target position.
    /// \return the distance from the current position to the target position
    /// in steps. Positive is clockwise from the current position.
    long    distanceToGo();

    /// The most recently set target position.
    /// \return the target position
    /// in steps. Positive is clockwise from the 0 position.
    long    targetPosition();

    /// The current motor position.
    /// \return the current motor position
    /// in steps. Positive is clockwise from the 0 position.
    long    currentPosition();  

    /// Resets the current position of the motor, so that wherever the motor
    /// happens to be right now is considered to be the new 0 position. Useful
    /// for setting a zero position on a stepper after an initial hardware
    /// positioning move.
    /// Has the side effect of setting the current motor speed to 0.
    /// \param[in] position The position in steps of wherever the motor
    /// happens to be right now.
    void    setCurrentPosition(long position);  
    
    /// Moves the motor (with acceleration/deceleration)
    /// to the target position and blocks until it is at
    /// position. Dont use this in event loops, since it blocks.
    void    runToPosition();

    /// Executes runSpeed() unless the targetPosition is reached.
    /// This function needs to be called often just like runSpeed() or run().
    /// Will step the motor if a step is required at the currently selected
    /// speed unless the target position has been reached.
    /// Does not implement accelerations.
    /// \return true if it stepped
    boolean runSpeedToPosition();

    /// Moves the motor (with acceleration/deceleration)
    /// to the new target position and blocks until it is at
    /// position. Dont use this in event loops, since it blocks.
    /// \param[in] position The new target position.
    void    runToNewPosition(long position);

    /// Sets a new target position that causes the stepper
    /// to stop as quickly as possible, using the current speed and acceleration parameters.
    void stop();

    /// Disable motor pin outputs by setting them all LOW
    /// Depending on the design of your electronics this may turn off
    /// the power to the motor coils, saving power.
    /// This is useful to support Arduino low power modes: disable the outputs
    /// during sleep and then reenable with enableOutputs() before stepping
    /// again.
    /// If the enable Pin is defined, sets it to OUTPUT mode and clears the pin to disabled.
    virtual void    disableOutputs();

    /// Enable motor pin outputs by setting the motor pins to OUTPUT
    /// mode. Called automatically by the constructor.
    /// If the enable Pin is defined, sets it to OUTPUT mode and sets the pin to enabled.
    virtual void    enableOutputs();

    /// Sets the minimum pulse width allowed by the stepper driver. The minimum practical pulse width is 
    /// approximately 20 microseconds. Times less than 20 microseconds
    /// will usually result in 20 microseconds or so.
    /// \param[in] minWidth The minimum pulse width in microseconds. 
    void    setMinPulseWidth(unsigned int minWidth);

    /// Sets the enable pin number for stepper drivers.
    /// 0xFF indicates unused (default).
    /// Otherwise, if a pin is set, the pin will be turned on when 
    /// enableOutputs() is called and switched off when disableOutputs() 
    /// is called.
    /// \param[in] enablePin Arduino digital pin number for motor enable
    /// \sa setPinsInverted
    void    setEnablePin(uint8_t enablePin = 0xff);

    /// Sets the inversion for stepper driver pins
    /// \param[in] directionInvert True for inverted direction pin, false for non-inverted
    /// \param[in] stepInvert      True for inverted step pin, false for non-inverted
    /// \param[in] enableInvert    True for inverted enable pin, false (default) for non-inverted
    void    setPinsInverted(bool directionInvert = false, bool stepInvert = false, bool enableInvert = false);

    /// Sets the inversion for 2, 3 and 4 wire stepper pins
    /// \param[in] pin1Invert True for inverted pin1, false for non-inverted
    /// \param[in] pin2Invert True for inverted pin2, false for non-inverted
    /// \param[in] pin3Invert True for inverted pin3, false for non-inverted
    /// \param[in] pin4Invert True for inverted pin4, false for non-inverted
    /// \param[in] enableInvert    True for inverted enable pin, false (default) for non-inverted
    void    setPinsInverted(bool pin1Invert, bool pin2Invert, bool pin3Invert, bool pin4Invert, bool enableInvert);

    /// Checks to see if the motor is currently running to a target
    /// \return true if the speed is not zero or not at the target position
    bool    isRunning();

    /// Virtual destructor to prevent warnings during delete
    virtual ~AccelStepper() {};
protected:

    /// \brief Direction indicator
    /// Symbolic names for the direction the motor is turning
    typedef enum
    {
	DIRECTION_CCW = 0,  ///< Counter-Clockwise
        DIRECTION_CW  = 1   ///< Clockwise
    } Direction;

    /// Forces the library to compute a new instantaneous speed and set that as
    /// the current speed. It is called by
    /// the library:
    /// \li  after each step
    /// \li  after change to maxSpeed through setMaxSpeed()
    /// \li  after change to acceleration through setAcceleration()
    /// \li  after change to target position (relative or absolute) through
    /// move() or moveTo()
    /// \return the new step interval
    virtual unsigned long  computeNewSpeed();

    /// Low level function to set the motor output pins
    /// bit 0 of the mask corresponds to _pin[0]
    /// bit 1 of the mask corresponds to _pin[1]
    /// You can override this to impment, for example serial chip output insted of using the
    /// output pins directly
    virtual void   setOutputPins(uint8_t mask);

    /// Called to execute a step. Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default calls step1(), step2(), step4() or step8() depending on the
    /// number of pins defined for the stepper.
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step(long step);
    
    /// Called to execute a clockwise(+) step. Only called when a new step is
    /// required. This increments the _currentPos and calls step()
    /// \return the updated current position
    long   stepForward();

    /// Called to execute a counter-clockwise(-) step. Only called when a new step is
    /// required. This decrements the _currentPos and calls step()
    /// \return the updated current position
    long   stepBackward();

    /// Called to execute a step using stepper functions (pins = 0) Only called when a new step is
    /// required. Calls _forward() or _backward() to perform the step
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step0(long step);

    /// Called to execute a step on a stepper driver (ie where pins == 1). Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default sets or clears the outputs of Step pin1 to step, 
    /// and sets the output of _pin2 to the desired direction. The Step pin (_pin1) is pulsed for 1 microsecond
    /// which is the minimum STEP pulse width for the 3967 driver.
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step1(long step);

    /// Called to execute a step on a 2 pin motor. Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default sets or clears the outputs of pin1 and pin2
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step2(long step);

    /// Called to execute a step on a 3 pin motor, such as HDD spindle. Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default sets or clears the outputs of pin1, pin2,
    /// pin3
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step3(long step);

    /// Called to execute a step on a 4 pin motor. Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default sets or clears the outputs of pin1, pin2,
    /// pin3, pin4.
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step4(long step);

    /// Called to execute a step on a 3 pin motor, such as HDD spindle. Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default sets or clears the outputs of pin1, pin2,
    /// pin3
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step6(long step);

    /// Called to execute a step on a 4 pin half-stepper motor. Only called when a new step is
    /// required. Subclasses may override to implement new stepping
    /// interfaces. The default sets or clears the outputs of pin1, pin2,
    /// pin3, pin4.
    /// \param[in] step The current step phase number (0 to 7)
    virtual void   step8(long step);

    /// Current direction motor is spinning in
    /// Protected because some peoples subclasses need it to be so
    boolean _direction; // 1 == CW
    
    /// The current interval between steps in microseconds.
    /// 0 means the motor is currently stopped with _speed == 0
    unsigned long  _stepInterval;

private:
    /// Number of pins on the stepper motor. Permits 2 or 4. 2 pins is a
    /// bipolar, and 4 pins is a unipolar.
    uint8_t        _interface;          // 0, 1, 2, 4, 8, See MotorInterfaceType

    /// Arduino pin number assignments for the 2 or 4 pins required to interface to the
    /// stepper motor or driver
    uint8_t        _pin[4];

    /// Whether the _pins is inverted or not
    uint8_t        _pinInverted[4];

    /// The current absolution position in steps.
    long           _currentPos;    // Steps

    /// The target position in steps. The AccelStepper library will move the
    /// motor from the _currentPos to the _targetPos, taking into account the
    /// max speed, acceleration and deceleration
    long           _targetPos;     // Steps

    /// The current motos speed in steps per second
    /// Positive is clockwise
    float          _speed;         // Steps per second

    /// The maximum permitted speed in steps per second. Must be > 0.
    float          _maxSpeed;

    /// The acceleration to use to accelerate or decelerate the motor in steps
    /// per second per second. Must be > 0
    float          _acceleration;
    float          _sqrt_twoa; // Precomputed sqrt(2*_acceleration)

    /// The last step time in microseconds
    unsigned long  _lastStepTime;

    /// The minimum allowed pulse width in microseconds
    unsigned int   _minPulseWidth;

    /// Is the direction pin inverted?
    ///bool           _dirInverted; /// Moved to _pinInverted[1]

    /// Is the step pin inverted?
    ///bool           _stepInverted; /// Moved to _pinInverted[0]

    /// Is the enable pin inverted?
    bool           _enableInverted;

    /// Enable pin for stepper driver, or 0xFF if unused.
    uint8_t        _enablePin;

    /// The pointer to a forward-step procedure
    void (*_forward)();

    /// The pointer to a backward-step procedure
    void (*_backward)();

    /// The step counter for speed calculations
    long _n;

    /// Initial step size in microseconds
    float _c0;

    /// Last step size in microseconds
    float _cn;

    /// Min step size in microseconds based on maxSpeed
    float _cmin; // at max speed

};

/// @example Random.pde
/// Make a single stepper perform random changes in speed, position and acceleration

/// @example Overshoot.pde
///  Check overshoot handling
/// which sets a new target position and then waits until the stepper has 
/// achieved it. This is used for testing the handling of overshoots

/// @example MultipleSteppers.pde
/// Shows how to multiple simultaneous steppers
/// Runs one stepper forwards and backwards, accelerating and decelerating
/// at the limits. Runs other steppers at the same time

/// @example ConstantSpeed.pde
/// Shows how to run AccelStepper in the simplest,
/// fixed speed mode with no accelerations

/// @example Blocking.pde 
/// Shows how to use the blocking call runToNewPosition
/// Which sets a new target position and then waits until the stepper has 
/// achieved it.

/// @example AFMotor_MultiStepper.pde
/// Control both Stepper motors at the same time with different speeds
/// and accelerations. 

/// @example AFMotor_ConstantSpeed.pde
/// Shows how to run AccelStepper in the simplest,
/// fixed speed mode with no accelerations

/// @example ProportionalControl.pde
/// Make a single stepper follow the analog value read from a pot or whatever
/// The stepper will move at a constant speed to each newly set posiiton, 
/// depending on the value of the pot.

/// @example Bounce.pde
/// Make a single stepper bounce from one limit to another, observing
/// accelrations at each end of travel

/// @example Quickstop.pde
/// Check stop handling.
/// Calls stop() while the stepper is travelling at full speed, causing
/// the stepper to stop as quickly as possible, within the constraints of the
/// current acceleration.

/// @example MotorShield.pde
/// Shows how to use AccelStepper to control a 3-phase motor, such as a HDD spindle motor
/// using the Adafruit Motor Shield http://www.ladyada.net/make/mshield/index.html.

/// @example DualMotorShield.pde
/// Shows how to use AccelStepper to control 2 x 2 phase steppers using the 
/// Itead Studio Arduino Dual Stepper Motor Driver Shield
/// model IM120417015

#endif 
