/****
 * 
 * This file is a part of the GemRedAngle library. See GemRedAngle.h for details.
 * 
 * =====
 *
 *  @file     GemRedAngle.cpp 
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
#include "GemRedAngle.h"

enum msgType : byte {unknown , stop, angle, calibrate};

// Return the nth ';'-terminated field. Field numbering is zero-based.
String getField(String line, byte n) {
  unsigned int start = 0;
  int end = -1;
  for (byte i = 0; i <= n; i++) {
    start = end + 1;
    end = line.indexOf(";", start);
    if (end == -1) {
      return "";
    }
  }
  return line.substring(start, end);
}

// Return the message type of the passed message
msgType getType(String theMsg) {
  String theField = getField(theMsg, 0);
  if (theField.equals(GEMRED_MSG_ANGLE)) {
    return angle;
  }
  if (theField.equals(GEMRED_MSG_STOP)) {
    return stop;
  }
  if (theField.equals(GEMRED_MSG_CAL)) {
    return calibrate;
  }
  return unknown;
}

GemRedAngle::GemRedAngle(Stream &i, uint8_t p, bool a) {
  theDevice = &i;
  powerPin = p;
  powerOnHigh = a;
  curAngle = GEMRED_NOT_INIT;
  theLine = "";
  gotFirstMeasurement = false;
  lastMillis = millis();
}

void GemRedAngle::begin() {
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, powerOnHigh ? HIGH : LOW);
}

bool GemRedAngle::run() {
  unsigned long curMillis = millis();
  float absa = curAngle;

  if (theDevice->available()) {                       // If the GemRed has produced something, deal wit it
    unsigned char inChar = (char)theDevice->read();

    // NUL isn't interesting; nothing changed
    if (inChar == 0x00) {
      return false; 
    }
    // Interesting char; append it
    if (inChar < ' ') {                               // If it's a control char, convert it to hex and then append it
      theLine += 'x';
      if (inChar < 0x10) {
        theLine += '0';
      }
      theLine += String((byte)inChar, HEX);
    } else {                                          // otherwise just append it
      theLine += (char)inChar;
    }

    // The last char in a message is 0x0D (<LF>). If that's what we got, process the message
    if (inChar == 0x0D) {
      #ifdef GEMRED_DEBUG_VERBOSE
      Serial.print(F("GemRed new message: "));
      Serial.println(theLine);
      #endif

      // Deal with the completed message based on what its type is
      switch (getType(theLine)) {
        char quadrant;
        case unknown:                                 // unknown message type -- set up to return GEMRED_BAD_MSG
          #ifdef GEMRED_DEBUG
          Serial.print(F("GemRed unknown message type: "));
          Serial.println(theLine);
          #endif
          absa = GEMRED_BAD_MSG;
          break;
          
        case angle:                                   // angle message type. Handle it based on rotational quadrant and absolute angle measurement
          // When it powers up, GemRed sends several angle messages with absolute angle measurements == "----". Ignore them.
          if (!gotFirstMeasurement) {
            if (getField(theLine, GEMRED_ABSA_FIELD).equals("----")) {
              theLine = "";
              return false;
            }
            gotFirstMeasurement = true;
          }

          quadrant = getField(theLine, GEMRED_FLAGS_FIELD).charAt(GEMRED_FLAG_Y);
          absa = getField(theLine, GEMRED_ABSA_FIELD).toFloat();
                                                      // The first few angle messages have "----" in the absolute angle measurement (GEMRED_ABSA_FIELD) field
          // Deal with angle messages based on which rotational quadrant was reported
          switch (quadrant) {
            case '-':                                 //   Once we get going, pseudo-quadrant "-" indicates the device has been tipped on its face or back
              #ifdef GEMRED_DEBUG
              Serial.print(F("GemRed detected tipping. absa: "));
              Serial.println(absa, 1);
              #endif
              absa = GEMRED_TIPPED;
              break;

            case '1':                                 //   Right side up to on its left side
              absa = 360.0 - absa;
              break;

            case '2':                                 //   On its left side to upside down
              absa += 180.0;
              break;

            case '3':                                 //   Upside down to on its right side
              absa = 180.0 - absa;
              break;

            case '4':                                 //   On its rght side to right side up
              // no change
              break;

            default:
              #ifdef GEMRED_DEBUG                     //   Not supposed to be anything else
              Serial.print(F("Invalid angle message: "));
              Serial.println(theLine);
              #endif
              absa = GEMRED_BAD_QUADRANT;
            }
          break;
                  
        case calibrate:                               // Calibrate message type. Not equipped to deal with that
          #ifdef GEMRED_DEBUG
          Serial.print(F("GemRed calibration message: "));
          Serial.println(theLine);
          #endif
          absa = GEMRED_CAL_MSG;
          break;
                  
        case stop:                                    // Stop message type. The GemRed is shutting down. Power it off and then on to restart
          #ifdef GEMRED_DEBUG
          Serial.print(F("GemRed stop. Restarting. "));
          #endif
          gotFirstMeasurement = false;
          digitalWrite(powerPin, powerOnHigh ? LOW : HIGH);
          delay(GEMRED_OFF_MS);
          digitalWrite(powerPin, powerOnHigh ? HIGH : LOW);
          #ifdef GEMRED_DEBUG
          Serial.println(F("Done."));
          #endif
          break;
      }
      theLine = "";                                   // All done with the message. The result is in absa
    }
    lastMillis = curMillis;                           // Update when we last heard from the GemRed
  }

  // If it's been a long time since we heard from the GemRed, and we had been hearing from it, indicate GemRed is AWOL
  if (curMillis - lastMillis > GEMRED_AWOL_MS) {
    #ifdef GEMRED_DEBUG
    Serial.print(F("GemRed is AWOL. Last message: "));
    Serial.println(theLine);
    #endif
    if (gotFirstMeasurement) {
      absa = GEMRED_AWOL;
    }
    lastMillis = curMillis;                           // Reset the watchdog
  }

  // If there's new data, update the current data with the new and return true to indicate something happened else return false
  if (curAngle != absa) {
    #ifdef GEMRED_DEBUG
    Serial.print(F("GemRed new angle: "));
    Serial.println(absa, 1);
    #endif
    curAngle = absa;
    return true;
  }
  return false;
}

float GemRedAngle::getAngle() {
  return curAngle;
}