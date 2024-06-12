#ifndef __DATA_H__
#define __DATA_H__

#include <stdint.h>
#include <time.h>

#define MSG_RXQ_SIZE 1024
#define MSG_TXQ_SIZE 1024
#define MSG_RXQ_RAW_SIZE 1024
#define MSG_TXQ_RAW_SIZE 1024
#define MSG_RXQ_CNT  5
#define MSG_TXQ_CNT  2
#define MSG_RXQ_RAW_CNT 2
#define MSG_TXQ_RAW_CNT 2

enum data_errno {
    DATA_ERR_OK,
    DATA_ERR_PARAMETER,
    DATA_ERR_NO_MEMORY,
    DATA_ERR_QUEUE_NO_MATCH,
    DATA_ERR_QUEUE_IS_EMPTY,
    DATA_ERR_QUEUE_IS_FULL,
    DATA_ERR_DATA_IS_INVALID,
    DATA_ERR_CHECK_SUM_IS_INVALID,
};

enum msg_priority {
    MSG_PRIO_LOW = 0,
    MSG_PRIO_LEVEL1,
    MSG_PRIO_LEVEL2,
    MSG_PRIO_LEVEL3,
    MSG_PRIO_LEVEL4,
};

enum msg_q_type {
    MSG_Q_IDLE,
    MSG_Q_RUNNING,
    MSG_Q_STOPPED,
};

struct msg_block {
    struct msg_block *next;
    uint16_t type;
    uint16_t payload_len;
    uint8_t *data;
};

struct msg_header {
    uint16_t hop_limit;
    struct {
        uint16_t earmark: 13;
        uint16_t priority: 3;
    };
    uint16_t flow_label;
    uint16_t heart_rate;
    uint32_t src_id;
    uint32_t dst_id;
    uint32_t paylaod_len;
    uint16_t next_block;
    uint16_t check_sum;
};

struct msg_buff {
    struct msg_buff *next;
    struct msg_buff *prev;
    struct msg_header *hdr;
    struct msg_block *blk_head;

    time_t tstamp;
};

struct msg_queue {
    struct msg_buff *head;
    struct msg_buff *tail;
    uint32_t size;
    uint32_t count;
};

struct msg_raw_buff {
    struct msg_raw_buff *next;
    struct msg_raw_buff *prev;
    uint8_t *data;
    uint16_t len;
};

struct msg_raw_queue {
    struct msg_raw_buff *head;
    struct msg_raw_buff *tail;
    uint32_t size;
    uint32_t count;
};  

struct msg_rx_srv {
    struct msg_queue *q;
    uint32_t id;
    enum msg_q_type type;
};

struct msg_rx_srvs {
    uint8_t size;
    struct msg_rx_srv srvs[MSG_RXQ_CNT];
};

struct msg_tx_srv {
    struct msg_queue *q;
    uint32_t id;
    enum msg_q_type type;
};

struct msg_tx_srvs {
    uint8_t size;
    struct msg_tx_srv srvs[MSG_TXQ_CNT];
};

struct msg_rx_raw_srv {
    struct msg_raw_queue *q;
    uint32_t id;
    enum msg_q_type type;
};

struct msg_rx_raw_srvs {
    uint8_t size;
    struct msg_rx_raw_srv srvs[MSG_RXQ_RAW_CNT];
};

struct msg_tx_raw_srv {
    struct msg_raw_queue *q;
    uint32_t id;
    enum msg_q_type type;
};

struct msg_tx_raw_srvs {
    uint8_t size;
    struct msg_tx_raw_srv srvs[MSG_TXQ_CNT];
};

struct data_ctrl_block {
    struct msg_tx_srvs *tx_srvs;
    struct msg_rx_srvs *rx_srvs;
    struct msg_rx_raw_srvs *rx_raw_srvs;
    struct msg_tx_raw_srvs *tx_raw_srvs;
};


int data_init(struct data_ctrl_block *dcb);
struct data_ctrl_block *data_alloc(void);
void data_deinit(struct data_ctrl_block *dcb);
void data_free(struct data_ctrl_block *dcb);

int msg_buff_dump(struct msg_buff *msgb, uint8_t *data);
int msg_buff_load(struct msg_buff *msgb, uint8_t *data);

struct msg_buff *msg_buff_alloc(void);
void msg_buff_free(struct msg_buff *msgb);

int msg_rxq_select(struct msg_rx_srvs *rx_srvs, struct msg_buff *msgb);
int msg_txq_select(struct msg_tx_srvs *tx_srvs, struct msg_buff *msgb);
int msg_queue_del(struct msg_queue *q, struct msg_buff *msgb);
int msg_queue_add(struct msg_queue *q, struct msg_buff *msgb);

#endif /* __DATA_H__ */
