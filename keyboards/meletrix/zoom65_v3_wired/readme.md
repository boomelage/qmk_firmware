# Zoom65 v3 (wired-only)

A wired-only variant of the Zoom65 v3. All Bluetooth / 2.4GHz wireless support
has been removed: the separate wireless co-processor is left untouched and
simply idles, and the main-MCU firmware no longer talks to it. Output is always
USB. Keys, encoder, RGB matrix, and the LCD screen module (driven over UART3)
behave identically to the wireless firmware.

This is a personal derivative of `meletrix/zoom65_v3` (maintainer @ozwaldorf).

* Hardware Supported: Zoom65 v3 1.6mm ANSI PCB (non-taillight)
* Hardware Availability: [meletrix](https://meletrix.com/)

Make example for this keyboard (after setting up your build environment):

    ```
    make meletrix/zoom65_v3_wired/ansi:via
    ```

Flashing example for this keyboard:

    ```
    make meletrix/zoom65_v3_wired/ansi:via:flash
    ```

**Reset Key**: Hold down the key located at *K00*, which is programmed as *Esc*, while plugging in the keyboard.

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
