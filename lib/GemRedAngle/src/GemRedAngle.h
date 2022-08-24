/****
 * 
 * This file is a part of the GemRedAngle library which is designed to read the angle measured 
 * by a hacked GemRed 82421 Mini Angle Gauge (e.g., https://amazon.com/dp/B078JNS7V6). 
 * 
 * The hack consists of running three wires, GND, +1.5V, and TX from exposed pads on the GemRed's 
 * PCB to the outside world, where they can be interfaced with an Arduino. The hack is more fully 
 * documented in other parts of the repository. To use a hacked GemRedAngle, two things are 
 * required. First, the TX signal, which is a 3.3V serial ASCII data stream at 9600 baud, needs to 
 * be attached to a pin that is the RX line for some concrete subclass of Stream, a HardwareSerial 
 * object like Serial1, for instance. And second, there needs to be a digital pin that can be used 
 * to turn the power to the GemRed on and off. There are lots of ways to accomplish this, for 
 * example by building a crude Vcc-powered linear regulator made from a small general purpose NPN 
 * transistor and two resistors. That's needed because after a while when there's no change to the 
 * measured angle, the GemRed switches itself off automatically to conserve power. Turning the 
 * power off and then back on restarts things.
 * 
 * =====
 *
 *  @file     GemRedAngle.h 
 * 
 *  @version  Version 1.0.0, August 2022
 *
 *  @author   D. L. Ehnebuske
 *
 *  @section  license
 *
 *  Software License Agreement (BSD License)
 *
 *  Copyright (c) 2022 by D. L. Ehnebuke All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 *    3. Neither the name of the copyright holders nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 ****/
#pragma once
#ifndef Arduino_h
#include <Arduino.h>
#endif

//#define GEMRED_DEBUG                    // Uncomment to enable debug printing

#define GEMRED_OFF_MS       (1000UL)    // How long (ms) to hold GemRed power off to reset it.
#define GEMRED_AWOL_MS      (3000UL)    // How long after hearing from GemRed we decide its AWOL

// GemRead message types (the 0th field of a message)
#define GEMRED_MSG_STOP     "*9;"       // Stop message
#define GEMRED_MSG_ANGLE    "*25;"      // Angle reading message
#define GEMRED_MSG_CAL      "*30;"      // Calibrate message

// For angle eading messages from the GemRed:
#define GEMRED_ABSA_FIELD   (2)         // The number of the field containing the absolute angle
#define GEMRED_FLAGS_FIELD  (1)         // The number of the flags field
#define GEMRED_FLAG_Y       (4)         // The flag for rotation quadrant. '-' ==> device tipped over

// Device errors returned by getAngle()
#define GEMRED_NOT_INIT     (-1.0)      // GemRed device is not yet initialized
#define GEMRED_TIPPED       (-2.0)      // GemRed device is tipped too far forward or backward to work
#define GEMRED_BAD_QUADRANT (-3.0)      // GemRed device sent an undefined quadrant number
#define GEMRED_BAD_MSG      (-4.0)      // GemRed device sent a  message with an undefined message type
#define GEMRED_CAL_MSG      (-5.0)      // GemRed device unexpectedly went into calibration mode
#define GEMRED_AWOL         (-6.0)      // GemRed stopped sending messages for longer than is reasonable

class GemRedAngle {
  public:
    /****
     * 
     * GemRedAngle(i, p[, a])
     *    Instantiate a new GemRedAngle object attached to a GemRed Mini Angle Gauge by way of the 
     *    object i, a concrete implementation of the Stream class and whose power can be turned on 
     *    and off by switching GPIO pin p. The parameter a can be omitted if GemRed power on is 
     *    active HIGH, or set to false if it's active LOW.
     * 
     ****/
    GemRedAngle(Stream &i, uint8_t p, bool a = true);

    /****
     * 
     * begin()
     *  Start the GemRedAngle object going.
     * 
     ****/
    void begin();

    /****
     * 
     * bool run()
     *    Let the GemRedAngle object do its thing. Returns true if the re is a new angle reading 
     *    available, flase otherwise
     *  
     ****/
    bool run();

    /****
     * 
     * float getAngle()
     *    Return the current reading for the angle in degrees (with a resolution of 0.1 degrees) or 
     *    a number less than 0 if something went wrong. Angle values range from 0.0 to 359.9, where 
     *    0.0 is level with the GemRed device base pointing down. The angle increases as the GemRed 
     *    device rotates clockwise while viewing it from the front.
     * 
     ****/
    float getAngle();

  private:
    Stream* theDevice;
    uint8_t powerPin;
    bool powerOnHigh;
    float curAngle;
    String theLine;
    bool gotFirstMeasurement;
    unsigned long lastMillis;
};