# N64 -> Arduino Uno -> Nintendo Switch

## Description
By connecting an original N64 controller to an Arduino UNO R3 running this code, and plugging the USB from the Arduino into a Nintendo Switch, you can use your original N64 controller as a Switch controller! The button mappings were made using The Legend of Zelda: Ocarina of Time on Nintendo Switch Online and the buttons are mapped 1:1.

## Inspiration
Due to:
- the unavailability of Wireless Nintendo Switch N64 controllers
- no restocks in sight
- a growing library of nostaligic N64 games on Nintendo switch online

I wanted to try to somehow interface my original N64 controllers with my Nintendo Switch. From creating several macro programs in the past based on the Splatoon pixel art drawer by [shinyquagsire23](https://github.com/shinyquagsire23/Switch-Fightstick), I knew I could send button presses to the Switch. I just had to figure out how to get N64 controller data from the controller and to the Atmega16u2 on the UNO. Then I found the project by twinone, that allowed you to use an N64 controller with an emulator, but not a switch. So I Frankenstein Monster'd those projects together and made this!


## Setup
To program the Atmega328P
 1. Plug in your Arduino to a PC and open 'N64Simplified.ino' in the Arduino IDE.
 2. Install the [Arduino N64 Controller Interface Library](https://github.com/pothos/arduino-n64-controller-library)
 3. Upload to your Arduino
To program the Atmega16uP (dfu-programmer required on linux/osx, flip on windows)
 1. Plug in your Arduino to your PC if it's not already
 2. Short the Reset and Ground pins for the [USB-Serial Processor](https://support.arduino.cc/hc/en-us/articles/4410804625682-Set-a-board-to-DFU-mode)
 3. In the root directory of this project, run `./flash.sh Joystick.hex`. This will make it so you will no longer be able to program your Arduino's Atmega328P. If you want to restore that ability, you can run `./flash.sh uno.hex` and you should be back in business.

 Once both these steps are done, you should be able to [wire your N64 controller](https://github.com/pothos/arduino-n64-controller-library/blob/master/README.md#wireing) with your data wire on pin 2, then plug your USB cable from the Arduino into a Switch and play some games!

## Notes
While looking into converting the joystick x/y axis, I found an [article](https://n64squid.com/n64-joystick-360-degrees/) talking about how joysticks rarely have the full range and how developers should only rely on about 1/2 of the full axis range. To fix this, when initially started, I added logic so the joystick will start at 2x sensitivity and auto-scale sensitivity per-side of each axis to allow even old joysticks to get full range.

Many thanks to the work of shinyquagsire, twinone, MickMad, Andrew Brown, Peter Den Hartog, Kai Luke, pothos, Dean Camera, Darran Hunt and many more!