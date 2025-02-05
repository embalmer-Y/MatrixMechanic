#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "data.h"
#include <time.h>

enum data_net_type {
    DATA_NET_TYPE_NONE = 0,
    DATA_NET_TYPE_UART,
    DATA_NET_TYPE_SPI,
    DATA_NET_TYPE_I2C,
    DATA_NET_TYPE_CAN,
    DATA_NET_TYPE_USB,
    DATA_NET_TYPE_ETHERNET,
    DATA_NET_TYPE_WIFI,
    DATA_NET_TYPE_BLE,
    DATA_NET_TYPE_LPBLE,
    DATA_NET_TYPE_LORA,
    DATA_NET_TYPE_ZIGBEE,
    DATA_NET_TYPE_SLE,
    DATA_NET_TYPE_NFC,
};

enum data_link_state {
    DATA_LINK_STATE_NONE = 0,
    DATA_LINK_STATE_CONNECTED,
    DATA_LINK_STATE_ACTIVE,
    DATA_LINK_STATE_DISCONNECTED,
};

struct data_net_ability_cfg {
    uint16_t ability_id;
    uint16_t hop_count;
    uint16_t mtu;
    time_t delay_time;
    time_t alive_time;
    enum data_net_type net_type;
    enum data_link_state link_state;
    uint8_t net_args_num;
    void **net_args;
};

struct data_net_ability_info {
    struct data_net_ability_info *next;
    struct data_net_ability_info *prev;
    struct data_net_dev_info *dev_info;
    struct data_net_ability_cfg *cfg;
};

struct data_net_dev_info {
    struct data_net_dev_info *next;
    struct data_net_dev_info *prev;
    struct data_net_ability_info *first;
    uint8_t is_micro;
    uint16_t device_id;
    uint16_t ability_num;
};

struct data_net_srv {
    struct data_net_dev_info *first;
    uint32_t count;
};

struct data_net_ability_info *data_net_ability_info_alloc(void);
struct data_net_dev_info *data_net_dev_info_alloc(void);
struct data_net_srv *data_net_srv_alloc(void);
void data_net_ability_info_free(struct data_net_ability_info *info);
void data_net_dev_info_free(struct data_net_dev_info *info);
void data_net_srv_free(struct data_net_srv *srv);
struct data_net_ability_info *data_net_ability_find_last(struct data_net_ability_info *first);
struct data_net_dev_info *data_net_dev_find_last(struct data_net_dev_info *first);
int data_net_ability_info_init(struct data_net_ability_info **info,
                               struct data_net_ability_cfg **cfg);
int data_net_dev_info_init(struct data_net_dev_info **info, 
                           uint16_t id, uint8_t is_micro);
int data_net_srv_init(struct data_net_srv **srv,
                      struct data_net_dev_info **dev_info);
int data_net_ability_info_deinit(struct data_net_ability_info *info);
int data_net_dev_info_deinit(struct data_net_dev_info *info);
int data_net_srv_deinit(struct data_net_srv *srv);
int data_net_ability_info_add(struct data_net_dev_info **dev_info, 
                              struct data_net_ability_info **a_info);
int data_net_dev_info_add(struct data_net_srv **srv,
                          struct data_net_dev_info **dev_info);
int data_net_ability_find_by_id(struct data_net_ability_info **first,
                                struct data_net_ability_info **target, 
                                uint16_t id);
int data_net_dev_find_by_id(struct data_net_dev_info **first,
                            struct data_net_dev_info **target,
                            uint16_t id);
int data_net_ability_info_remove(struct data_net_dev_info **dev_info,
                                 uint16_t id);
int data_net_dev_info_remove(struct data_net_srv **srv, uint16_t id);

#endif /* __ROUTE_H__ */
