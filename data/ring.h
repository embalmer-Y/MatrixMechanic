#ifndef __RING_H__
#define __RING_H__

#include "data.h"
#include "proto.h"
#include "../common/common.h"


struct msg_data {
    struct msg_data *next;
    struct msg_data *prev;
    uint8_t is_raw; // 1: raw data, 0: parsed data
    void *data;
};

void msg_data_free(struct msg_data *data);
struct msg_data *msg_data_alloc(void);
int msg_data_ring_init(struct msg_data **head, struct msg_data **tail, uint32_t cnt);
void msg_data_ring_deinit(struct msg_data *head, uint32_t cnt);
int msg_data_ring_push(struct msg_data **tail, void *data, uint8_t is_raw);
void *msg_data_ring_pull(struct msg_data **head, uint8_t *is_raw);
int msg_data_ring_extend(struct msg_data **head, uint32_t cnt);
int msg_data_ring_decrease(struct msg_data **head, uint32_t cnt);

#endif /* __RING_H__ */
