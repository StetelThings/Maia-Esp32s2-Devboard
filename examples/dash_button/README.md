# RainMaker template for Maia DevBoard

## Build firmware

This template needs Espressif's [ESP-IDF](https://github.com/espressif/esp-idf) and [ESP RainMaker](https://github.com/espressif/esp-rainmaker) to be build.
It is based in part on the code of the examples in these frameworks.

Please set development enviroment up as per the instructions in [RainMaker documentation](https://rainmaker.espressif.com/docs/get-started.html).

## What to expect in this example?

This example shows how to use ESP RainMaker with Maia DevBoard.
It creates two devices to get temperature from ESP32-S2 core sensor and to pilot RGB LED from your phone.

Configuration parameters are editable within the menuconfig, if you want to expand this example using IO headers.

You can also take advantage of the [OTA upgade service](https://rainmaker.espressif.com/docs/ota.html), if you wish.


### Reset to Factory

Press and hold Prg Button (PB1) for more than 3 seconds to reset the board to factory defaults. You will have to provision the board again to use it.


