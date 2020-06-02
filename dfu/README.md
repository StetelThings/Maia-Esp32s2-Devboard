# Maia ESP32-S2 Device Firmware Update

The Maia board can be updated 





---

## First findings 

We investigated a bit the new ESP32-S2 deeper features while developing the Maia board Most of the findings are now public, documented and offially usable. You can a video we did in that phase.

[ESP32-S2 USB Video on Youtube](https://www.example.com)


## Using our DFU Web flasher for ESP32-S2

Finding that you can flash the board directly from the native USB interface via DFU gave us the idea to build a flasher that is completely web based.

[ESP32-S2 Dfu Web Flasher](https://dfu.stetelthings.com)

The Web Flasher runs entirely in the browser (we suggest to use Chrome which has the best WebUSB support). 
** Your firmware gets never transmitted over the internet ** . Basically you load it from your PC to the browser and from the browser to the device.

Note that the firmware must be built using the new `idf.py dfu` command available in the ESP-IDF framework. 

### Platform specific suggestions:

#### OS X:

If you use Chrome on OS X everything should work without the need to tweak or install somthing

#### Linux:

Theoretically you could use also Chromium but on several distributions (like Ubuntu) it gets installed via Snap and is therefore heavely Sandboxed.
For this reason we suggest to download and install Chrome.
Once installed you have to grant access to the USB device for your user. Example for **Ubuntu**:

As superuser create a file: `/etc/udev/rules.d/50-yourdevicename.rules`

and add following content:

`SUBSYSTEM=="usb", ATTR{idVendor}=="3a30", ATTR{idProduct}="0002" MODE="0664", GROUP="plugdev"`

then run (as a superuser)  `udevadm control --reload-rules`

#### Windows 10

WHen you plug a USB device in windows it attempts to find a suitable driver otherwise the device isn't available. To make it available you have to run a .INF file that tells the system to use the WinUSB driver for our board so that Chrome can actually us it.
We have prepare the required file which is available [ESP32-S2BootUSB.inf](here).

