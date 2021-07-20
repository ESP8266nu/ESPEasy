# Library for modular scrolling LED matrix text displays

[![arduino-library-badge](https://www.ardu-badge.com/badge/MD_Parola.svg?)](https://www.ardu-badge.com/MD_Parola)

[Version 1.0 Video](http://www.youtube.com/watch?v=JgzVCSFaz3I)

[Version 2.0 Video](http://www.youtube.com/watch?v=u1iELyROjW8)

[Sprites Animation Video](http://www.youtube.com/watch?v=tfwAHx0MTxU)

[Library Documentation](https://majicdesigns.github.io/MD_Parola/)

Parola is a modular scrolling text display using MAX7219 or MAX7221 LED matrix display controllers using Arduino. The display is made up of any number of identical modules that are plugged together to create a wider/longer display.
* Text left, right or center justification in the display
* Text scrolling, entry and exit effects
* Control display parameters and animation speed
* Support for hardware SPI interface
* Multiple virtual displays (zones) in each string of LED modules
* User defined fonts and/or individual characters substitutions
* Support for double height displays
* Support for mixing text and graphics on the same display

The aim was to create a 'lego-like' LED matrix display, using standard 8x8 LED matrices. The software supports this flexibility through a scalable approach that only requires the definition of the number of modules to adapt existing software to a new configuration.

If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)

The Parola software has a dependency on the [MD_MAX72xx Arduino library](https://github.com/MajicDesigns/MD_MAX72xx) which implements hardware functions of the LED matrix. The sketched need to be configured for the type of matrices being used - please refer to the hardware section of documentation for the [MD_MAX72xx library](https://majicdesigns.github.io/MD_MAX72XX/page_hardware.html).

Parola discussion on the [Arduino forum](http://forum.arduino.cc/index.php?topic=171056.0) and kits available from [ElectroDragon](http://www.electrodragon.com/product/dot-matrix-chain-display-kit-max7219-v2).

Additional information also at [my blog](http://arduinoplusplus.wordpress.com), search "Parola A to Z".
