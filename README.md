# DiceFirmware

Firmware for the Bluetooth Pixel dice, based on Nordic's nRF5 SDK (available [here](https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs)).

## Building The Firmware

### Environment Setup on Windows

The requirements are the same than for building the dice _bootloader_.
Check out the instructions on the _bootloader_'s GitHub [page](https://github.com/GameWithPixels/DiceBootloader#readme).

### Building

Make sure that the _Makefile_ `SDK_ROOT` variable is pointing to the correct folder.

Open a command line and go the folder where this repository is cloned and run `make`.

The output files are placed in the `_builds` folder, by default those are debug files (not release). The one that we want to program to the flash memory is the `.hex` file (more about this format [here](https://en.wikipedia.org/wiki/Intel_HEX)) .

## Programming a die (with USB)

Using the provided _Makefile_ you may:

* `reset`: restart the device
* `erase`: entirely erase the flash memory
* `settings`: generate firmware_settings.hex

For debug builds:

* `firmware_debug` (default): produce a debug build of the firmware
* `flash`: program the firmware into the die's memory and reboot the device
* `flash_softdevice`: program the _SoftDevice_ into the die's memory and reboot the device
* `flash_bootloader`: program the bootloader into the die's memory and reboot the device
* `flash_board`: call `erase`, `flash_bootloader`, `flash_softdevice` and `flash` in a sequence
* `reflash`:call `erase`, `flash_softdevice` and `flash` in a sequence

For release builds:

* `firmware_release`: produce a release build of the firmware
* `flash_release`: program the firmware into the die's memory and reboot the device
* `reflash_release`: call `erase`, `flash_softdevice` and `flash_release` in a sequence
* `publish`: produce a zipped DFU package (also copied in the `binaries` folder)

The last command requires `nRF Util` to produce the DFU package (see documentation [here](https://infocenter.nordicsemi.com/topic/ug_nrfutil/UG/nrfutil/nrfutil_intro.html) about this tool). DFU packages can be pushed on dice via Bluetooth using Nordic's nRF Toolbox (available on mobile).

The _Makefile_ expects to find `nrfutil.exe`. Search for `NRFUTIL` to set a different path.
We're using the 5.2 build that can be downloaded from [GitHub](https://github.com/NordicSemiconductor/pc-nrfutil/releases/tag/v5.2.0).

The version of the firmware is defined by the variable `VERSION` in the _Makefile_.

_Note: when switching between debug and release builds, be sure to first delete the \_builds folder first._

## Output logs in Visual Studio Code

Install Arduino [extension](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino) from Microsoft.
It enables access to the serial port to the die's electronic board (through USB).

To connect to the die electronic board, run the following commands in VS Code:
* `Arduino: Select Port` and select SEGGER
* `Arduino: Open Serial Monitor`
