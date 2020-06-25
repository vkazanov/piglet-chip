#ifndef IO_EVDEV_H
#define IO_EVDEV_H

#include <stdbool.h>

enum key_evdev_status {
    KEY_EVDEV_SUCCESS,
    KEY_EVDEV_FAIL,
};

#define CHIP8_KEY_1 0x1
#define CHIP8_KEY_2 0x2
#define CHIP8_KEY_3 0x3
#define CHIP8_KEY_C 0xC

#define CHIP8_KEY_4 0x4
#define CHIP8_KEY_5 0x5
#define CHIP8_KEY_6 0x6
#define CHIP8_KEY_D 0xD

#define CHIP8_KEY_7 0x7
#define CHIP8_KEY_8 0x8
#define CHIP8_KEY_9 0x9
#define CHIP8_KEY_E 0xE

#define CHIP8_KEY_A 0xA
#define CHIP8_KEY_0 0x0
#define CHIP8_KEY_B 0xB
#define CHIP8_KEY_F 0xF

typedef struct key_evdev key_evdev;

int key_evdev_new(const char *path, key_evdev **ke_ptr);

void key_evdev_free(key_evdev *ke);

int key_evdev_wait_for_key(key_evdev *ke, int *key_pressed);

int key_evdev_is_key_pressed(key_evdev *ke, int key_to_check, bool *is_key_pressed);

int key_evdev_flush(key_evdev *ke);

#endif /* IO_EVDEV_H */
