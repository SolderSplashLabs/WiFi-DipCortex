WiFi-Dipcortex
===============================

Codebase for the WiFi DipCortex - LPC1347

Creates a USB CDC (Serial) port, allowing control of the TI CC3000 Module 

Use a VT100 capable terminal like Putty. The .bin file found in the debug directory can be flashed using the built in USB bootloader

Toolchain
-------------------------------
LPCXpresso / Code_Red v6 - Free Compiler for PC, MAC and Linux from NXP

Examples
-------------------------------
You can use this code to learn how to do the following :

* WiFi DipCortex implementation of the TI CC3000 stack
* Smartconfig triggering
* Ping hostnames
* Scan for access points
* UDP Server, polling and non blocking. Used to locate the WiFi DipCortex on your network
* UDP Client, NTP to collect the time and DNS to resolve hostnames
* TCP Client, Post to a webserver. Xively CSV data or Prowl for iOS notifications

TODO
-------------------------------
* TCP Server
* Removing more blocking code where possible


Notes
-------------------------------
This is an example to demonstrate the use of the CC3000 module. For more detailed info on the CC3000 module see the TI wiki http://processors.wiki.ti.com/index.php/CC3000

License
-------------------------------

See individual files for license info, mostly BSD
Uses ARM/Code Red supplied CMSIS libraries please see their license terms


Pin List
-------------------------------

* PIO0_0  --> Led, Low - Output - On (Alpha Board Only)
* PIO0_6  --> USB Soft Connect - Output
* PIO1_15 --> CC3000 - Output - Vbat Enable - CC Module powered when high
* PIO1_16 <-- CC3000 - Input - IRQ - Raised when CC wants to be talked too
* PIO1_19 --> CC3000 - Output - CS - Active low
* PIO1_20 --> CC3000 - Output - SPI CLK - 12MHz max for CC3000
* PIO1_21 <-- CC3000 - Input - SPI MISO
* PIO1_22 --> CC3000 - Output - SPI MOSI
