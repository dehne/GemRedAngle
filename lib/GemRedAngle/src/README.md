# GemRedAngle: an Arduino Library for Reading GemRed Mini Angle Gauges

The GemRedAngle library makes reading the angle measured by a hacked GemRed 82421 Mini Angle 
Gauge (e.g., https://amazon.com/dp/B078JNS7V6) easy. 

The hack consists of running three wires, GND, +1.5V, and TX from exposed pads on the GemRed's 
PCB to the outside world, where they can be interfaced with an Arduino. To use a hacked 
GemRedAngle, two things are required. First, the TX signal, which is a 3.3V serial ASCII data 
stream at 9600 baud, needs to be attached to a pin that is the RX line for some concrete subclass 
of Stream, a HardwareSerial object like Serial1, for instance. And second, there needs to be a 
digital pin that can be used to turn the power to the GemRed device on and off. There are lots of 
ways to accomplish this, for example by building a crude Vcc-powered linear regulator made from a 
small general purpose NPN transistor and two resistors controlled from a GPIO pin. That's needed 
because after a while, when there's no change to the measured angle, the GemRed switches itself 
off to conserve power. Turning the power off and then back on restarts things.

The library itself may be found in the repository's lib folder. The hack is more fully documented 
in docs folder of the repository. There is a simple example of how to use GemRedAngle in the src 
folder. GemRedAngle member functions are documented in the library's GemRedAngle.h file.

NB: Concrete classes derived from Stream may require class-specific initialization. For 
example, HardwareSerial objects require initialization through their begin() member function. 
This sort of initialization needs to be done before initializing GemRedAngle objects through 
their begin() member function. Typically this is done in the sketch's setup() function. 
