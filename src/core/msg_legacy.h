#include <stdint.h>
#include <time.h>

#define MSG_Q_SIZE 1000
#define MSG_Q_CNT  5

enum msg_errno {
    MSG_ERR_OK,
    MSG_ERR_PARAMETER,
    MSG_ERR_NO_MEMORY,
    MSG_ERR_QUEUE_IS_EMPTY,
    MSG_ERR_QUEUE_IS_FULL,
    MSG_ERR_DATA_IS_INVALID,
    MSG_ERR_CHECK_SUM_IS_INVALID,
};

enum msg_q_type {
    MSG_Q_IDLE,
    MSG_Q_RUNNING,
    MSG_Q_STOPPED,
};

struct msg_block {
    struct msg_block *next;
    uint16_t type;
    uint32_t payload_len;
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

struct msg_srv {
    struct msg_queue *q;
    uint32_t id;
    enum msg_q_type type;
};

struct msg_srvs {
    uint8_t size;
    struct msg_srv srvs[MSG_Q_CNT];
};


int msg_queue_get_status(struct msg_srv *srv);
int msg_queue_set_status(struct msg_srv *srv, enum msg_q_type status);
int msg_queue_get_count(struct msg_srv *srv);
int msg_pull_internal(struct msg_srv *srv, struct msg_buff *msgb);
int msg_select_queue(struct msg_srvs *srvs, struct msg_srv *srv, struct msg_buff *msgb);
int msg_pull(struct msg_srvs *srvs, struct msg_buff *msgb);
int msg_push_internal(struct msg_srv *srv, struct msg_buff *msgb);
int msg_block_create(struct msg_block *msg_blk, struct msg_header *msg_hdr, uint8_t *data);
int msg_header_create(struct msg_header *msg_hdr, uint8_t *data);
int msg_buf_create(struct msg_buff *msgb, uint8_t *data);
int msg_queue_init(struct msg_queue *msg_q);
int msg_srv_init(struct msg_queue *msg_q, struct msg_srv *msg_srv, int msg_id);
int msg_srvs_init(struct msg_srvs *msg_srvs);
void msg_block_del(struct msg_block *msg_blk);
void msg_buff_del(struct msg_buff *msgb);
void msg_header_del(struct msg_header *msg_hdr);
void msg_queue_deinit(struct msg_queue *msg_q);
void msg_srv_deinit(struct msg_srv *msg_srv);
void msg_srvs_deinit(struct msg_srvs *msg_srvs);
