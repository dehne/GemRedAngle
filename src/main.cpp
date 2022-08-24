/****
 * 
 * This sketch is an example showing how to use the GemRedAngle library to read the angle measured 
 * by a hacked GemRed 82421 Mini Angle Gauge (e.g., https://amazon.com/dp/B078JNS7V6). 
 * 
 * The hack consists of running three wires, GND, +1.5V, and TX from exposed pads on the GemRed's 
 * PCB to the outside world, where they can be interfaced with an Arduino. For the Arduino, I used 
 * Sparkfun's 5V proMicro because I had one in my bin of stuff and because it has Serial1 
 * implemented in hardware. The library requires that the power to the GemRed be controlled by a 
 * GPIO pin. That's needed because if there's no change to the measured angle for a while, the 
 * GemRed switches itself off automatically to conserve power. Cycling the power restarts things. 
 * 
 * An easy way to do this is to make a crude linear regulator from a 2N3904 with its collector 
 * connected to one of the Arduino's GPIO pins, and its emitter connected to the GemRed's 1.5V 
 * power rail. The 2N3904's base is attached to a voltage divider made from a 1K2 resistor 
 * attached to the collector and a 1K resistor to GND. This delivers about 1.3V to the GemRed when 
 * the GPIO pin is HIGH and 0V when it's LOW, letting the library turn the GemRed on and off. The 
 * 16mA the GemRed uses is well within the recommended maximum current of 20mA for a GPIO pin.
 * 
 * When it's switched on and gets going, the GemRed delivers a serial ASCII stream at 9600 baud 
 * on its 3.3V TX pad. For this sketch, the GemRed TX line is connected to the proMicro's RX1 line 
 * (the receive pin for Serial1) that the library uses to read the datastream. 
 * 
 * GND on the GemRed is connected to GND on the proMicro.
 * 
 * It's important to note that Serial1 is a HardwareSerial object. HardwareSerial is a derived 
 * class that inherits from Stream. The GemRedAngle library uses a Stream object to read from the 
 * GemRed device, so Serial1 fits the bill. But HardwareSerial objects need to be initialized via 
 * the begin() member function which is not a part of the Stream definition. That means it's 
 * necessary to invoke Serial1.begin(9600) in setup() before invoking begin() on the RemRedAngle 
 * device. Other kinds of Stream objects might have different (or no) special initialization.
 * 
 * =====
 *
 *  @file     main.cpp 
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
#include <Arduino.h>
#include <GemRedAngle.h>

#define BANNER          F("GemRedExample v1.0.0 August 2022")

#define GR_PWR_PIN      (5)               // The GemRed power control is attached (active HIGH) to this GPIO pin

GemRedAngle angleGauge(Serial1, GR_PWR_PIN);
const char errorString[][50] = {
  {"Not init"},
  {"Tipped over"},
  {"Internal error -- Undefined quadrant reported"},
  {"Internal error -- Undefined message type sent"},
  {"Unexpectedly entered calibration mode"},
  {"Device timeout."}};

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println(BANNER);
  Serial1.begin(9600);
  angleGauge.begin();
}

void loop() {
  if (angleGauge.run()) {
    float reading = angleGauge.getAngle();
    if (reading >= 0) {
    Serial.print(F("angleGauge reading: "));
      Serial.println(reading, 1);
    } else {
      Serial.print(F("angleGauge reports an error: "));
      Serial.println(errorString[-((int16_t)reading) - 1]);
    }
  }
}