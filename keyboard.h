#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#include "common.h"

enum keyboard_status {
    KEYBOARD_SUCCESS,
    KEYBOARD_FAIL,
};

typedef struct keyboard keyboard;

int keyboard_new(const char *path, keyboard **ke_ptr);

void keyboard_free(keyboard *ke);

int keyboard_wait_for_key(keyboard *ke, int *key_pressed);

int keyboard_is_key_pressed(keyboard *ke, int key_to_check, bool *is_key_pressed);

int keyboard_get_key_state(keyboard *ke, bool key_state[CHIP8_KEY_COUNT]);

int keyboard_flush(keyboard *ke);

#endif /* KEYBOARD_H */
