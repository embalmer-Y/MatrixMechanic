#include "queue.h"
#include "data.h"
#include <stdint.h>
#include <stdlib.h>

static void msg_queue_free(struct msg_queue *q)
{
    if (q == NULL)
        return;

    free(q);
}

static struct msg_queue *msg_queue_alloc(void)
{
    struct msg_queue *q = malloc(sizeof(struct msg_queue));
    if (q == NULL)
        return NULL;

    return q;
}

static void msg_queue_srv_free(struct msg_queue_srv *srv)
{
    if (srv == NULL)
        return;

    free(srv);
}

static struct msg_queue_srv *msg_queue_srv_alloc(void)
{
    struct msg_queue_srv *srv = malloc(sizeof(struct msg_queue_srv));
    if (srv == NULL)
        return NULL;

    return srv;
}

static void msg_mcb_free(struct msg_ctrl_blk *mcb)
{
    if (mcb == NULL)
        return;

    free(mcb);
}

static struct msg_ctrl_blk *msg_mcb_alloc(void)
{
    struct msg_ctrl_blk *mcb = malloc(sizeof(struct msg_ctrl_blk));
    if (mcb == NULL)
        return NULL;

    return mcb;
}

int msg_queue_init(struct msg_queue *q, struct msg_queue_cfg *cfg)
{
    int ret;

    if (q == NULL || cfg == NULL)
        return -ERR_INVALID;

    q->cfg.max_msg_cnt = cfg->max_msg_cnt;
    q->cfg.priority = cfg->priority;
    q->cfg.run_state = cfg->run_state;

    memset(&q->info, 0, sizeof(struct msg_queue_info));

    ret = msg_data_ring_init(&q->head, &q->tail, q->cfg.max_msg_cnt);
    if (ret != ERR_OK)
        return ret;

    return ERR_OK;
}

int msg_queue_srv_init(struct msg_queue_srv *srv, uint8_t q_cnt)
{
    int i;

    if (srv == NULL)
        return -ERR_INVALID;

    srv->q = (struct msg_queue **)malloc(sizeof(struct msg_queue) * q_cnt);

    for (i = 0; i < q_cnt; i++) {
        srv->q[i] = msg_queue_alloc();
        if (srv->q[i] == NULL)
            goto _free;

        srv->q_cnt++;
        msg_queue_set_id(srv->q[i], i);
    }

    return ERR_OK;
_free:
    for (i = 0; i < srv->q_cnt; i++) {
        msg_queue_free(srv->q[i]);
    }

    return -ERR_NOMEM;
}

int msg_mcb_init(struct msg_ctrl_blk *mcb, int srvs_cnt)
{
    int i;

    if (mcb == NULL)
        return -ERR_INVALID;

    mcb->srvs = (struct msg_queue_srv **)malloc(sizeof(struct msg_queue_srv) * srvs_cnt);
    
    for (i = 0; i < srvs_cnt; i++) {
        mcb->srvs[i] = msg_queue_srv_alloc();
        if (mcb->srvs[i] == NULL) 
            goto _free;            

        mcb->srvs_cnt++;
        msg_srv_set_id(mcb->srvs[i], i);
    }

    return ERR_OK;

_free:
    for (i = 0; i < mcb->srvs_cnt; i++) {
        msg_queue_srv_free(mcb->srvs[i]);
    }

    return -ERR_NOMEM;
}

int msg_queue_deinit(struct msg_queue *q)
{
    if (q == NULL)
        return -ERR_INVALID;

    msg_data_ring_deinit(q->head, q->cfg.max_msg_cnt);

    msg_queue_free(q);

    return ERR_OK;
}

int msg_queue_srv_deinit(struct msg_queue_srv *srv)
{
    int ret;
    int i;

    if (srv == NULL)
        return -ERR_INVALID;

    for (i = 0; i < srv->q_cnt; i++) {
        ret = msg_queue_deinit(srv->q[i]);
        if (ret != ERR_OK)
            return ret;
    }

    msg_queue_srv_free(srv);

    return ERR_OK;
}

int msg_mcb_deinit(struct msg_ctrl_blk *mcb)
{
    int ret;
    int i;

    if (mcb == NULL)
        return -ERR_INVALID;

    for (i = 0; i < mcb->srvs_cnt; i++) {
        ret = msg_queue_srv_deinit(mcb->srvs[i]);
        if (ret != ERR_OK)
            return ret;
    }

    msg_mcb_free(mcb);

    return ERR_OK;
}

int msg_queue_get_id(struct msg_queue *q, uint32_t *q_id)
{
    if (q == NULL || q_id == NULL)
        return -ERR_INVALID;

    *q_id = q->cfg.q_id;

    return ERR_OK;
}

int msg_queue_set_id(struct msg_queue *q, uint32_t q_id)
{
    if (q == NULL)
        return -ERR_INVALID;

    q->cfg.q_id = q_id;

    return ERR_OK;
}

int msg_queue_get_limit(struct msg_queue *q, uint32_t *msg_cnt)
{
    if (q == NULL || msg_cnt == NULL)
        return -ERR_INVALID;

    *msg_cnt = q->cfg.max_msg_cnt;

    return ERR_OK;
}

int msg_queue_set_limit(struct msg_queue *q, uint32_t msg_cnt)
{
    int ret;

    if (q == NULL)
        return -ERR_INVALID;

    if (q->cfg.max_msg_cnt == msg_cnt)
        return ERR_OK;

    if (q->cfg.run_state == MSG_QUEUE_IS_DEINIT) {
        q->cfg.max_msg_cnt = msg_cnt;
        return ERR_OK;
    }

    if (q->cfg.max_msg_cnt < msg_cnt) {
        ret = msg_data_ring_extend(&q->head, msg_cnt);
        if (ret != ERR_OK)
            return ret;
    } else {
        if (q->info.msg_cnt > q->cfg.max_msg_cnt - msg_cnt)
            return -ERR_DATA_QUEUE_SIZE_INSUFF;

        if (q->cfg.run_state == MSG_QUEUE_IS_RUN)
            return -ERR_DATA_QUEUE_RUNNING;

        ret = msg_data_ring_decrease(&q->head, msg_cnt);
        if (ret != ERR_OK)
            return ret;
    }

    return ERR_OK;
}

int msg_queue_get_info(struct msg_queue *q, struct msg_queue_info *info)
{
    if (q == NULL || info == NULL)
        return -ERR_INVALID;

    memcpy(info, &q->info, sizeof(struct msg_queue_info));

    return ERR_OK;
}

int msg_queue_get_priority(struct msg_queue *q, uint8_t *priority)
{
    if (q == NULL || priority == NULL)
        return -ERR_INVALID;

    *priority = q->cfg.priority;

    return ERR_OK;
}

int msg_queue_set_priority(struct msg_queue *q, uint8_t priority)
{
    if (q == NULL)
        return -ERR_INVALID;

    q->cfg.priority = priority;

    return ERR_OK;
}

int msg_queue_get_running_state(struct msg_queue *q, 
                                enum msg_queue_running_state *run_state)
{
    if (q == NULL || run_state == NULL)
        return -ERR_INVALID;

    *run_state = q->cfg.run_state;

    return ERR_OK;
}

int msg_queue_set_running_state(struct msg_queue *q, 
                                enum msg_queue_running_state run_state)
{
    if (q == NULL)
        return -ERR_INVALID;

    q->cfg.run_state = run_state;

    return ERR_OK;
}

int msg_queue_push(struct msg_queue *q, void *data, uint8_t is_raw)
{
    int ret;

    if (q == NULL || data == NULL)
        return -ERR_INVALID;

    if (q->cfg.run_state == MSG_QUEUE_IS_STOP)
        return -ERR_DATA_QUEUE_STOPPED;

    if (q->info.status == MSG_QUEUE_FULL)
        return -ERR_DATA_QUEUE_FULL;

    ret = msg_data_ring_push(&q->head, data, is_raw);
    if (ret != ERR_OK) {
        return ret;
    }

    q->info.rx_cnt++;
    q->info.msg_cnt++;

    if (q->info.msg_cnt == q->cfg.max_msg_cnt) {
        q->info.status = MSG_QUEUE_FULL;
    } else if (q->cfg.max_msg_cnt / q->info.msg_cnt >= 2) {
        q->info.status = MSG_QUEUE_HALF_FULL;
    }

    return ERR_OK;
}

int msg_queue_pull(struct msg_queue *q, void **data, uint8_t *is_raw)
{
    if (q == NULL || data == NULL || is_raw == NULL)
        return -ERR_INVALID;

    if (q->info.status == MSG_QUEUE_EMPTY)
        return -ERR_DATA_QUEUE_EMPTY;

    *data = msg_data_ring_pull(&q->head, is_raw);
    if (*data == NULL)
        return -ERR_DATA_QUEUE_EMPTY;

    q->info.tx_cnt++;
    q->info.msg_cnt--;

    if (q->info.msg_cnt == 0)
        q->info.status = MSG_QUEUE_EMPTY;

    return ERR_OK;
}

int msg_srv_get_id(struct msg_queue_srv *srv, uint8_t *id)
{
    if (srv == NULL || id == NULL)
        return -ERR_INVALID;

    *id = srv->id;

    return ERR_OK;
}

int msg_srv_set_id(struct msg_queue_srv *srv, uint8_t id)
{
    if (srv == NULL)
        return -ERR_INVALID;

    srv->id = id;

    return ERR_OK;
}

int msg_srv_get_q_cnt(struct msg_queue_srv *srv, uint8_t *q_cnt)
{
    if (srv == NULL || q_cnt == NULL)
        return -ERR_INVALID;

    *q_cnt = srv->q_cnt;

    return ERR_OK;
}

int msg_srv_get_queue(struct msg_queue_srv *srv, uint8_t id, struct msg_queue *q)
{
    if (srv == NULL || q == NULL)
        return -ERR_INVALID;

    if (id >= srv->q_cnt)
        return -ERR_INVALID;

    q = srv->q[id];

    return ERR_OK;
}

int msg_mcb_get_srvs_cnt(struct msg_ctrl_blk *mcb, uint8_t *srvs_cnt)
{
    if (mcb == NULL || srvs_cnt == NULL)
        return -ERR_INVALID;

    *srvs_cnt = mcb->srvs_cnt;

    return ERR_OK;
}

int msg_mcb_get_srv(struct msg_ctrl_blk *mcb, uint8_t id, struct msg_queue_srv *srv)
{
    if (mcb == NULL || srv == NULL)
        return -ERR_INVALID;

    if (id >= mcb->srvs_cnt)
        return -ERR_INVALID;

    srv = mcb->srvs[id];

    return ERR_OK;
}
