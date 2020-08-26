# Dash Button example for Maia DevBoard


This example shows how to implement a dash button using Maia DevBoard.

You just have to set your WiFi SSID and Password into menuconfig (Example Connection Configuration). 
HOST_NAME is defined in dash_button_example.c file.

Like Amazon's Dash Buttons, it calls an URL and gets a response. 

When you push USER button, yellow led indicates the example is running. When it got a response, it turns on green led if this is OK, red led otherwise.
Lastly, it turns off the board (not deep sleep, really power off: no power consumption).

Pushing USER button again, the example restarts.


