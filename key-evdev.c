#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/input.h>

#include <libevdev/libevdev.h>

#include "key-evdev.h"

struct key_evdev {
    struct libevdev *dev;
};

static const int keys_used[] = {
    KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_Q, KEY_W, KEY_E, KEY_R,
    KEY_A, KEY_S, KEY_D, KEY_F,
    KEY_Z, KEY_X, KEY_C, KEY_V,
};

static const int key_to_chip8_key[KEY_CNT] = {
    [KEY_1] = CHIP8_KEY_1,
    [KEY_2] = CHIP8_KEY_2,
    [KEY_3] = CHIP8_KEY_3,
    [KEY_4] = CHIP8_KEY_C,

    [KEY_Q] = CHIP8_KEY_4,
    [KEY_W] = CHIP8_KEY_5,
    [KEY_E] = CHIP8_KEY_6,
    [KEY_R] = CHIP8_KEY_D,

    [KEY_A] = CHIP8_KEY_7,
    [KEY_S] = CHIP8_KEY_8,
    [KEY_D] = CHIP8_KEY_9,
    [KEY_F] = CHIP8_KEY_E,

    [KEY_Z] = CHIP8_KEY_A,
    [KEY_X] = CHIP8_KEY_0,
    [KEY_C] = CHIP8_KEY_B,
    [KEY_V] = CHIP8_KEY_F,
};

static const uint8_t chip8_key_to_key[] = {
    [CHIP8_KEY_1] = KEY_1,
    [CHIP8_KEY_2] = KEY_2,
    [CHIP8_KEY_3] = KEY_3,
    [CHIP8_KEY_C] = KEY_4,

    [CHIP8_KEY_4] = KEY_Q,
    [CHIP8_KEY_5] = KEY_W,
    [CHIP8_KEY_6] = KEY_E,
    [CHIP8_KEY_D] = KEY_R,

    [CHIP8_KEY_7] = KEY_A,
    [CHIP8_KEY_8] = KEY_S,
    [CHIP8_KEY_9] = KEY_D,
    [CHIP8_KEY_E] = KEY_F,

    [CHIP8_KEY_A] = KEY_Z,
    [CHIP8_KEY_0] = KEY_X,
    [CHIP8_KEY_B] = KEY_C,
    [CHIP8_KEY_F] = KEY_V,
};

static bool is_suitable_device(struct libevdev *dev);
static void evdev_resync(key_evdev *ke);

int key_evdev_new(const char *path, key_evdev **ke_ptr)
{
    int rc = 1;
    int fd = 0;
    struct libevdev *dev = NULL;
    key_evdev *ke = NULL;

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open device");
        return rc;
    }

    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
        fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
        goto err2;
    }

    if (!is_suitable_device(dev)) {
        fprintf(stderr, "Not a suitable keyboard: %s\n", path);
        goto err1;
    }

    ke = calloc(1, sizeof(key_evdev));
    if (ke == NULL) {
        fprintf(stderr, "Calloc failure\n");
        goto err1;
    }

    ke->dev = dev;

    *ke_ptr = ke;

    rc = 0;

    return rc;
err1:
    libevdev_free(dev);
err2:
    return KEY_EVDEV_FAIL;
}

void key_evdev_free(key_evdev *ke)
{
    if (!ke)
        return;

    fd = libevdev_get_fd(ke->dev);
    if (fd != -1)
        close(fd);

    libevdev_free(ke->dev);
    free(ke);
}

static bool is_key_code_defined(int code)
{
    for (size_t i = 0; i < sizeof(keys_used) / sizeof(keys_used[0]); i++)
        if (keys_used[i] == code)
            return true;
    return false;
}

int key_evdev_wait_for_key(key_evdev *ke, int *key_pressed)
{
    int rc = -1;

    for (;;) {
        struct input_event ev;
        rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SYNC) {
            evdev_resync(ke);
        } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            /* Done waiting? */
            if (ev.type == EV_KEY && ev.value == 1 && is_key_code_defined(ev.code)) {
                *key_pressed = key_to_chip8_key[ev.code];
                return KEY_EVDEV_SUCCESS;
            }
            /* Just ignore non-interesting ones */
        } else if (rc == -EAGAIN) {
            /* No more events, let's keep waiting */
        } else {
            /* Error? propagate the error */
            return rc;
        }

    };
}

int key_evdev_is_key_pressed(key_evdev *ke, int key_to_check, bool *is_key_pressed)
{
    int rc = -1;

    for (;;) {
        struct input_event ev;
        rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SYNC) {
            evdev_resync(ke);
        } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            /* Just flush all the events and update the internal libevdev
             * state */
        } else if (rc == -EAGAIN) {
            /* No more events, so let's check keyboard state  */
            int value = libevdev_get_event_value(
                ke->dev, EV_KEY, chip8_key_to_key[key_to_check]
            );
            *is_key_pressed = (value != 0);
            return KEY_EVDEV_SUCCESS;
        } else {
            /* Error?  */
            return KEY_EVDEV_FAIL;
        }
    }

}

int key_evdev_get_key_state(key_evdev *ke, bool keyboard_state[CHIP8_KEY_COUNT])
{
    for (size_t i = 0; i < CHIP8_KEY_COUNT; i++ ){
        int rc = key_evdev_is_key_pressed(ke, i, &keyboard_state[i]);
        if (rc != KEY_EVDEV_SUCCESS)
            return rc;
    }
    return KEY_EVDEV_SUCCESS;
}

int key_evdev_flush(key_evdev *ke)
{
    int rc = -1;

    for (;;) {
        struct input_event ev;
        rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SYNC) {
            evdev_resync(ke);
        } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            /* Just flush events */
        } else if (rc == -EAGAIN) {
            /* No more events so we're done  */
            return KEY_EVDEV_SUCCESS;
        } else {
            /* Error?  */
            return KEY_EVDEV_FAIL;
        }
    }
}

static void evdev_resync(key_evdev *ke)
{
    int rc = -1;
    do {
        struct input_event ev;
        rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
    } while (rc == LIBEVDEV_READ_STATUS_SYNC);
}

static bool is_suitable_device(struct libevdev *dev)
{
    if (!libevdev_has_event_type(dev, EV_KEY))
        return false;

    for (size_t i = 0; i < sizeof(keys_used) / sizeof(keys_used[0]); i++) {
        if (!libevdev_has_event_code(dev, EV_KEY, keys_used[i]))
            return false;
    }

    return true;
}
