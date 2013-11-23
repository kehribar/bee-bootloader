bee-bootloader
==============

ENC28j60 based atmega328p bootloader.

Uses UDP broadcast messages, therefore there is no need for static IP configuration and dealing with IP conflicts.

Fits into the 4096 byte bootloader area. Might be possible to slim down to 2048 byte.

Software side is spaghetti code right now, but it's working ok. I'm going to fix it soon.

ihsan.

