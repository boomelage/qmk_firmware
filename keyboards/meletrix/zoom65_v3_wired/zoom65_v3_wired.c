#include QMK_KEYBOARD_H
#include "quantum.h"
#include "uart.h"
#include "raw_hid.h"
#include "os_detection.h"

// Wired-only variant of the Zoom65 v3.
//
// All Bluetooth / 2.4GHz wireless support has been removed: the separate
// wireless co-processor (UART1) is left untouched and simply idles, and the
// main-MCU firmware no longer talks to it. Output is always USB.
//
// Everything non-wireless is preserved and behaves identically to the stock
// firmware: keys, encoder, RGB matrix, and the LCD screen module (driven over
// UART3, a different bus from the wireless link).

enum __layers { WIN_B, WIN_FN };

static SerialConfig serialConfig = {
    SERIAL_DEFAULT_BITRATE, UART_WRDLEN, UART_STPBIT, UART_PARITY, UART_ATFLCT,
};

void keyboard_post_init_kb(void) {
    // enable usb data pin (low = USB correctly recognized)
    gpio_set_pin_output(USB_POWER_EN_PIN);
    gpio_write_pin_low(USB_POWER_EN_PIN);

    // enable LED-V power circuit
    gpio_set_pin_output_open_drain(LED_POWER_EN_PIN);
    gpio_write_pin_low(LED_POWER_EN_PIN);

    // setup uart3 for serial communication with the screen module
    serialConfig.speed = 115200;
    palSetLineMode(SD3_TX_PIN, PAL_MODE_ALTERNATE(UART_TX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST);
    palSetLineMode(SD3_RX_PIN, PAL_MODE_ALTERNATE(UART_RX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST);
    sdStart(&SD3, &serialConfig);

    keyboard_post_init_user();
}

bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_user()) {
        return false;
    }

    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(CAPS_INDEX, 255, 255, 255);
    }
    return true;
}

bool uart3_command(uint8_t payload[], int len) {
    if (len == 0) return false;
    sdWrite(&SD3, payload, len);
    return (bool)sdGet(&SD3);
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }
    switch (keycode) {
        case QK_GRAVE_ESCAPE: {
            // GUI+Esc should type a bare backtick. macOS/iOS eat Cmd+grave (the
            // in-app window cycle), so the GUI modifier must be stripped there;
            // Windows breaks if GUI is stripped (a lone Win tap opens the Start
            // menu). This variant is USB-only, so the host OS is detected
            // directly (no Bluetooth/Mac-mode toggle needed). Ctrl/Alt combos
            // are left alone so Ctrl+Shift+Esc and Cmd+Opt+Esc still send Esc.
            static bool gui_grave_active = false;
            if (record->event.pressed) {
                uint8_t mods = get_mods();
                if ((mods & MOD_MASK_GUI) && !(mods & (MOD_MASK_CTRL | MOD_MASK_ALT))) {
                    os_variant_t os = detected_host_os();
                    if (os == OS_MACOS || os == OS_IOS) {
                        uint8_t gui = mods & MOD_MASK_GUI;
                        del_mods(gui);
                        send_keyboard_report();
                        tap_code(KC_GRV);
                        add_mods(gui);
                        send_keyboard_report();
                        gui_grave_active = true;
                        return false;
                    }
                }
            } else if (gui_grave_active) {
                gui_grave_active = false;
                return false;
            }
            return true;
        }
        case DF(WIN_B):
            if (record->event.pressed) {
                set_single_persistent_default_layer(WIN_B);
                layer_state_set(1 << WIN_B);
            }
            return false;
        case SC_UP:
            if (record->event.pressed) {
                uint8_t buf[3] = {165, 0, 34};
                uart3_command(*&buf, 3);
            }
            return false;
        case SC_DOWN:
            if (record->event.pressed) {
                uint8_t buf[3] = {165, 0, 33};
                uart3_command(*&buf, 3);
            }
            return false;
        case SC_SWCH:
            if (record->event.pressed) {
                uint8_t buf[3] = {165, 0, 32};
                uart3_command(*&buf, 3);
            }
            return false;
        case SC_TOGG:
            if (record->event.pressed) {
                uint8_t buf[3] = {165, 0, 16};
                uart3_command(*&buf, 3);
            }
            return false;
        default:
            return true;
    }
}

#ifdef RAW_ENABLE
// proxy raw HID commands (e.g. zoom-sync / screen software) to the screen module
bool via_command_kb(uint8_t *data, uint8_t length) {
    if (data[0] == 88 && data[1] <= 30) {
        uint8_t buf[32] = {0};
        buf[0]          = 88;
        buf[1]          = 1;
        buf[2]          = (uint8_t)uart3_command(data + 2, data[1]);
        raw_hid_send(buf, 32);
        return true;
    }

    return false;
}
#endif
