#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "msg_legacy.h"
#include "msg_checksum.h"

int msg_queue_get_status(struct msg_srv *srv)
{
    if (srv == NULL)
        return -MSG_ERR_PARAMETER;
        
    return srv->type;
}

int msg_queue_set_status(struct msg_srv *srv, enum msg_q_type status)
{
    if (srv == NULL)
        return -MSG_ERR_PARAMETER;
        
    srv->type = status;
    return MSG_ERR_OK;
}

int msg_queue_get_count(struct msg_srv *srv)
{
    if (srv == NULL)
        return -MSG_ERR_PARAMETER;
        
    return srv->q->count;
}

int msg_pull_internal(struct msg_srv *srv, struct msg_buff *msgb)
{
    struct msg_queue *msg_q = srv->q;

    if (msgb == NULL)
        return -MSG_ERR_PARAMETER;

    if (msg_q->head == NULL)
        msg_q->head = msg_q->tail = msgb;
    else {
        msg_q->tail->next = msgb;
        msgb->prev = msg_q->tail;
        msg_q->tail = msgb;
    }

    msg_q->count++;
    
    return MSG_ERR_OK;
}

int msg_select_queue(struct msg_srvs *srvs, struct msg_srv *srv, struct msg_buff *msgb)
{
    if (msgb == NULL)
        return -MSG_ERR_PARAMETER;

    if (msgb->hdr->priority < srvs->size && msgb->hdr->priority >= 0) {
        srv = &srvs->srvs[msgb->hdr->priority];
    } else {
        return -MSG_ERR_PARAMETER;
    }

    return MSG_ERR_OK;
}

int msg_pull(struct msg_srvs *srvs, struct msg_buff *msgb)
{
    struct msg_srv *srv = NULL;
    int ret;
    
    ret = msg_select_queue(srvs, srv, msgb);
    if (ret != 0)
        return ret;

    ret = msg_pull_internal(srv, msgb);
    if (ret != 0)
        return ret;

    return MSG_ERR_OK;
}

int msg_push_internal(struct msg_srv *srv, struct msg_buff *msgb)
{
    struct msg_queue *msg_q = srv->q;
    struct msg_buff *msgb_temp;

    if (!msg_q->count)
        return -MSG_ERR_QUEUE_IS_EMPTY;

    msgb_temp = msg_q->head->next;
    msgb_temp->prev = NULL;
    msg_q->head->next = NULL;
    msgb = msg_q->head;
    msg_q->head = msgb_temp;

    msg_q->count--;

    return MSG_ERR_OK;
}

int msg_block_create(struct msg_block *msg_blk, struct msg_header *msg_hdr, uint8_t *data)
{
    uint32_t all_blk_len = msg_hdr->paylaod_len;
    uint16_t prev_type = msg_hdr->next_block;
    struct msg_block *prev_blk = NULL;

    while (all_blk_len > 0) {
        struct msg_block *temp = (struct msg_block *)malloc(sizeof(struct msg_block));
        if (temp == NULL)
            return -MSG_ERR_NO_MEMORY;

        temp->type = prev_type;
        memcpy(&(temp->payload_len), (data + sizeof(uint16_t)), sizeof(uint16_t));
        temp->data = (uint8_t *)malloc(temp->payload_len);
        if (temp->data == NULL)
            return -MSG_ERR_NO_MEMORY;

        memcpy(temp->data, (data + sizeof(uint16_t) * 2), temp->payload_len);

        memcpy(&prev_type, data, sizeof(uint16_t));
        temp->next = prev_blk;
        prev_blk = temp;

        all_blk_len -= (sizeof(uint16_t) * 2 + temp->payload_len);
        data += (sizeof(uint16_t) * 2 + temp->payload_len);
    }

    msg_blk = prev_blk;

    return MSG_ERR_OK;
}

int msg_header_create(struct msg_header *msg_hdr, uint8_t *data)
{
    int ret;

    uint16_t check_sum;
    memcpy(&check_sum, (data + sizeof(struct msg_header) - sizeof(check_sum)), sizeof(check_sum));

    if (check_sum != crc_16((uint8_t *)data, sizeof(struct msg_header) - sizeof(check_sum)))
        return -MSG_ERR_CHECK_SUM_IS_INVALID;

    msg_hdr = (struct msg_header *)malloc(sizeof(struct msg_header));
    if (msg_hdr == NULL)
        return -MSG_ERR_NO_MEMORY;
    memcpy(msg_hdr, data, sizeof(struct msg_header));
    
    return MSG_ERR_OK;
}

int msg_buf_create(struct msg_buff *msgb, uint8_t *data)
{
    int ret;
    msgb = (struct msg_buff *)malloc(sizeof(struct msg_buff));
    if (msgb == NULL)
        return -MSG_ERR_NO_MEMORY;
    
    ret = msg_header_create(msgb->hdr, data);
    if (ret != 0)
        return ret;

    if (msgb->hdr->paylaod_len == 0)
        return MSG_ERR_OK;

    data = data + sizeof(struct msg_header);
    ret = msg_block_create(msgb->blk_head, msgb->hdr, data);
    if (ret != 0)
        return ret;

    return MSG_ERR_OK;
}

int msg_queue_init(struct msg_queue *msg_q)
{
    if (msg_q == NULL)
        return -MSG_ERR_PARAMETER;

    msg_q->head = NULL;
    msg_q->tail = NULL;
    msg_q->size = MSG_Q_SIZE;
    msg_q->count = 0;

    return MSG_ERR_OK;
}

int msg_srv_init(struct msg_queue *msg_q, struct msg_srv *msg_srv, int msg_id)
{
    msg_srv->q = msg_q;
    msg_srv->id = msg_id;
    msg_srv->type = MSG_Q_IDLE;

    return MSG_ERR_OK;
}

int msg_srvs_init(struct msg_srvs *msg_srvs)
{
    int i;
    struct msg_srvs *msg_srvs_temp = (struct msg_srvs *)malloc(sizeof(struct msg_srvs));
    if (msg_srvs_temp == NULL)
        return -MSG_ERR_NO_MEMORY;

    *msg_srvs = *msg_srvs_temp;
    struct msg_srv *msg_srv;
    printf("srvs addr: %p\r\n", *msg_srvs);

    msg_srvs->size = MSG_Q_CNT;

    // for (i = 0; i < msg_srvs->size; i++) {
    //     msg_srv = &msg_srvs->srvs[i];
    //     msg_srv->q = (struct msg_queue *)malloc(sizeof(struct msg_queue));
        
    //     msg_queue_init(msg_srv->q);
    //     msg_srv_init(msg_srv->q, msg_srv, i);
    // }

    return MSG_ERR_OK;
}

void msg_block_del(struct msg_block *msg_blk)
{
    if (msg_blk == NULL)
        return;

    if (msg_blk->data != NULL)
        free(msg_blk->data);

    free(msg_blk);
}

void msg_header_del(struct msg_header *msg_hdr)
{
    if (msg_hdr == NULL)
        return;
    
    free(msg_hdr);
}

void msg_buff_del(struct msg_buff *msgb)
{
    if (msgb == NULL)
        return;

    struct msg_block *msg_blk = msgb->blk_head;
    struct msg_block *next;

    while (msg_blk != NULL) {
        next = msg_blk->next;
        msg_block_del(msg_blk);
        msg_blk = next;
    }

    msg_header_del(msgb->hdr);
    free(msgb);
}

void msg_queue_deinit(struct msg_queue *msg_q)
{
    if (msg_q == NULL)
        return;

    struct msg_buff *msgb = msg_q->head;
    struct msg_buff *next;
    
    while (msgb != NULL) {
        next = msgb->next;
        msg_buff_del(msgb);
        msgb = next;
    }
    
    free(msg_q);
}

void msg_srv_deinit(struct msg_srv *msg_srv)
{
    if (msg_srv == NULL)
        return;

    msg_queue_deinit(msg_srv->q);
}

void msg_srvs_deinit(struct msg_srvs *msg_srvs)
{
    if (msg_srvs == NULL)
        return;

    int i;
    for (i = 0; i < msg_srvs->size; i++) {
        struct msg_srv *msg_srv = &msg_srvs->srvs[i];
        msg_srv_deinit(msg_srv);
    }

    free(msg_srvs);
}
