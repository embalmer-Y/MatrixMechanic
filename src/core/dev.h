#ifndef __DEV_H__
#define __DEV_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_ABILITY_NAME_LENGTH 128
#define MAX_DEVICE_NAME_LENGTH 128

enum ability_type {
    ABILITY_TYPE_NONE,
    ABILITY_TYPE_TRANSPORT,
    ABILITY_TYPE_CTRL,
};

enum ability_state {
    ABILITY_STATE_DISABLED,
    ABILITY_STATE_ENABLED,
    ABILITY_STATE_START,
    ABILITY_STATE_STOP,
    ABILITY_STATE_PAUSE,
    ABILITY_STATE_ERROR
};

struct ability_ops {
    int (*init)(int argc, void *argv[]);
    int (*deinit)(void);
    int (*read)(int argc, void *argv[]);
    int (*write)(int argc, void *argv[]);
    int (*callback)(int argc, void *argv[]);
};

struct ability {
    struct ability *next;
    struct ability *prev;
    struct ability *dependencies;

    uint16_t id;
    char name[MAX_ABILITY_NAME_LENGTH];
    char *desc;
    enum ability_type type;
    enum ability_state state;

    struct ability_ops ops;
};

struct ability_ctrl_block {
    struct ability* head;
    struct ability* tail;
    uint16_t count;
};

struct device {
    struct device *next;
    struct device *prev;

    uint16_t id;
    char name[MAX_ABILITY_NAME_LENGTH];
    struct ability_ctrl_block abilities;
};

struct device_ctrl_block {
    struct device* head;
    struct device* tail;
    uint16_t count;
};

#endif /* __DEV_H__ */
