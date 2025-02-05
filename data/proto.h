#ifndef __PROTO_H__
#define __PROTO_H__

#include "data.h"

#define MSG_MICRO_BIT_MASK 0x8
#define MSG_PRIORITY_BIT_MASK 0x7

enum msg_priority {
    MSG_PRIO_LOW = 0,
    MSG_PRIO_LEVEL1,
    MSG_PRIO_LEVEL2,
    MSG_PRIO_LEVEL3,
    MSG_PRIO_LEVEL4,
};

/*
 micro protocol block schematic:
 32 bits                                   16 bits                                  0 bits
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |   type(next block) |        len         |                data...                 |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
*/
struct msg_micro_block {
    struct msg_micro_block *next;
    uint8_t type;
    uint8_t len;
    uint8_t *data;
};

/*
 micro protocol header schematic:
 32 bits                                   16 bits                                  0 bits
 +---8bits---+--4bits--+--1bit--+--3bits---+-------8bits-------+--------8bits-------+
 | hop_limit | earmark | micro  | priority |     heart_rate    |      src_id(h)     |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |      src_id(l)     |      dst_id(h)     |     dst_id(l)     |        len         |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |     next_block     |     check_sum(h)   |    check_sum(l)   |                    |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
*/
struct msg_micro_header {
    uint8_t hop_limit;
    struct {
        uint8_t earmark: 4;
        uint8_t micro: 1;
        uint8_t priority: 3;
    };
    uint8_t heart_rate;
    uint16_t src_id;
    uint16_t dst_id;
    uint16_t len;
    uint8_t next_block;
    uint16_t check_sum;
};


/*
 protocol block schematic:
 32 bits                                   16 bits                                  0 bits
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |            type(next block)             |                  len                   |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                  data...                |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
*/
struct msg_block {
    struct msg_block *next;
    uint16_t type;
    uint16_t len;
    uint8_t *data;
};

/*
 protocol header schematic:
 32 bits                                   16 bits                                  0 bits
 +---8bits---+--4bits--+--1bit--+--3bits---+-------8bits-------+--------8bits-------+
 | hop_limit | earmark | micro  | priority |                heart_rate              |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                                      src_id                                      |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                                      dst_id                                      |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                                        len                                       |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                next_block               |    check_sum(h)   |    check_sum(l)    |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
*/
struct msg_header {
    uint8_t hop_limit;
    struct {
        uint8_t earmark: 4;
        uint8_t micro: 1;
        uint8_t priority: 3;
    };
    uint16_t heart_rate;
    uint32_t src_id;
    uint32_t dst_id;
    uint32_t len;
    uint16_t next_block;
    uint16_t check_sum;
};

struct msg_micro_buff {
    uint8_t is_micro;
    time_t tstamp;

    struct msg_micro_header *hdr;
    struct msg_micro_block *blk;
};

struct msg_buff {
    uint8_t is_micro;
    time_t tstamp;

    struct msg_header *hdr;
    struct msg_block *blk;
};

int msg_checksum_calc(uint8_t *data);

int msg_buff_dump(void *msgb, uint8_t *data);
int msg_buff_load(void *msgb, uint8_t *data);

void *msg_block_alloc(uint8_t is_micro);
void *msg_header_alloc(uint8_t is_micro);
void *msg_buff_alloc(uint8_t is_micro);

void msg_block_free(void *msg_blk, uint8_t is_micro);
void msg_header_free(void *msg_hdr, uint8_t is_micro);
void msg_buff_free(void *msgb);

int msg_buff_is_micro(uint8_t *data);
int msg_buff_get_hop_limit(uint8_t *data, uint8_t *hop_limit);
int msg_buff_add_hop_limit(uint8_t *data);
int msg_buff_get_prority(uint8_t *data, enum msg_priority *priority);
int msg_buff_set_priority(uint8_t *data, enum msg_priority priority);
int msg_buff_get_heart_rate(uint8_t *data, void *heart_rate);
int msg_buff_set_heart_rate(uint8_t *data, uint16_t heart_rate);
int msg_buff_get_src(uint8_t *data, void *src_id);
int msg_buff_set_src(uint8_t *data, uint32_t src_id);
int msg_buff_get_dst(uint8_t *data, void *dst_id);
int msg_buff_set_dst(uint8_t *data, uint32_t dst_id);
int msg_buff_get_len(uint8_t *data, void *len);
int msg_buff_set_len(uint8_t *data, uint16_t len);
int msg_buff_get_next_block(uint8_t *data, void *next_block);
int msg_buff_set_next_block(uint8_t *data, uint16_t next_block);
int msg_buff_get_checksum(uint8_t *data, uint16_t *checksum);
int msg_buff_set_checksum(uint8_t *data, uint16_t checksum);
int msg_buff_get_payload(uint8_t *data, uint8_t *payload);
int msg_buff_get_block_next(uint8_t *block, void *next_type, uint8_t is_micro);
int msg_buff_get_block_len(uint8_t *block, void *len, uint8_t is_micro);
int msg_buff_get_block_data(uint8_t *block, uint8_t *data, uint8_t is_micro);

#endif /* __PROTO_H__ */
