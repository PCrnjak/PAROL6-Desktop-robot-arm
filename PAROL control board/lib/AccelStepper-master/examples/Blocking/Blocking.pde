// Blocking.pde
// -*- mode: C++ -*-
//
// Shows how to use the blocking call runToNewPosition
// Which sets a new target position and then waits until the stepper has 
// achieved it.
//
// Copyright (C) 2009 Mike McCauley
// $Id: Blocking.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

void setup()
{  
    stepper.setMaxSpeed(200.0);
    stepper.setAcceleration(100.0);
}

void loop()
{    
    stepper.runToNewPosition(0);
    stepper.runToNewPosition(500);
    stepper.runToNewPosition(100);
    stepper.runToNewPosition(120);
}
