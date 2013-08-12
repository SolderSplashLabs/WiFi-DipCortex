WiFi-Dipcortex
===============================

Codebase for the WiFi DipCortex

Not Ready just yet!

Toolchain
-------------------------------

LPCXpresso / Code_Red v5.2.4


License
-------------------------------

See individual files for license info, mostly BSD
Uses ARM/Code Red supplied CMSIS libraries please see their license terms



Pin List
-------------------------------

PIO0_0  --> Led, Low - Output - On (Alpha Board Only)
PIO0_6  --> USB Soft Connect - Output
PIO1_15 --> CC3000 - Output - Vbat Enable - CC Module powered when high
PIO1_16 <-- CC3000 - Input - IRQ - Raised when CC wants to be talked too
PIO1_19 --> CC3000 - Output - CS - Active low
PIO1_20 --> CC3000 - Output - SPI CLK - 12MHz max for CC3000
PIO1_21 <-- CC3000 - Input - SPI MISO
PIO1_22 --> CC3000 - Output - SPI MOSI