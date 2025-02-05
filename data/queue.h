#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "data.h"
#include "proto.h"
#include "ring.h"
#include "route.h"
#include "../common/common.h"

enum msg_queue_status {
    MSG_QUEUE_EMPTY = 0,
    MSG_QUEUE_FULL,
    MSG_QUEUE_HALF_FULL,
};

enum msg_queue_running_state {
    MSG_QUEUE_IS_RUN = 0,
    MSG_QUEUE_IS_STOP,
    MSG_QUEUE_IS_DEINIT,
};

struct msg_queue_cfg {
    uint8_t q_id;
    struct {
        enum msg_priority priority: 3;
        enum msg_queue_running_state run_state: 2;
        uint8_t type: 3;
    };
    uint32_t max_msg_cnt;
};

struct msg_queue_info {
    uint32_t msg_cnt;
    uint32_t rx_cnt;
    uint32_t tx_cnt;
    enum msg_queue_status status;
};

struct msg_queue {
    struct msg_queue_cfg cfg;
    struct msg_queue_info info;
    struct msg_data *head;
    struct msg_data *tail;
};

struct msg_queue_srv {
    uint8_t id;
    uint8_t q_cnt;
    struct data_net_info *net_info;
    struct msg_queue **q;
};

struct msg_ctrl_blk {
    uint8_t srvs_cnt;
    struct msg_queue_srv **srvs;
};

int msg_queue_init(struct msg_queue *q, struct msg_queue_cfg *cfg);
int msg_queue_srv_init(struct msg_queue_srv *srv, uint8_t q_cnt);
int msg_mcb_init(struct msg_ctrl_blk *mcb, int srvs_cnt);
int msg_queue_deinit(struct msg_queue *q);
int msg_queue_srv_deinit(struct msg_queue_srv *srv);
int msg_mcb_deinit(struct msg_ctrl_blk *mcb);

int msg_queue_get_id(struct msg_queue *q, uint32_t *q_id);
int msg_queue_set_id(struct msg_queue *q, uint32_t q_id);
int msg_queue_get_limit(struct msg_queue *q, uint32_t *msg_cnt);
int msg_queue_set_limit(struct msg_queue *q, uint32_t msg_cnt);
int msg_queue_get_info(struct msg_queue *q, struct msg_queue_info *info);
int msg_queue_get_priority(struct msg_queue *q, uint8_t *priority);
int msg_queue_set_priority(struct msg_queue *q, uint8_t priority);
int msg_queue_get_running_state(struct msg_queue *q, 
                                enum msg_queue_running_state *run_state);
int msg_queue_set_running_state(struct msg_queue *q, 
                                enum msg_queue_running_state run_state);
int msg_queue_push(struct msg_queue *q, void *data, uint8_t is_raw);
int msg_queue_pull(struct msg_queue *q, void **data, uint8_t *is_raw);

int msg_srv_get_id(struct msg_queue_srv *srv, uint8_t *id);
int msg_srv_set_id(struct msg_queue_srv *srv, uint8_t id);
int msg_srv_get_q_cnt(struct msg_queue_srv *srv, uint8_t *q_cnt);

int msg_mcb_get_srvs_cnt(struct msg_ctrl_blk *mcb, uint8_t *srvs_cnt);
int msg_mcb_set_srvs_cnt(struct msg_ctrl_blk *mcb, uint8_t srvs_cnt);
int msg_mcb_get_srv(struct msg_ctrl_blk *mcb, uint8_t id, struct msg_queue_srv *srv);

#endif /* __QUEUE_H_ */
