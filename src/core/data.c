#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "data.h"
#include "msg_checksum.h"

int msg_block_dump(struct msg_block *msg_blk, uint8_t *data)
{
    struct msg_block *temp;

    if (msg_blk == NULL || data == NULL) {
        return -DATA_ERR_PARAMETER;
    }

    temp = msg_blk;
    while (temp != NULL) {
        if (temp->next == NULL) {
            *(uint16_t *)data = 0;
        } else {
            *(uint16_t *)data = temp->next->type;
        }

        data += sizeof(uint16_t);
        *(uint16_t *)data = temp->payload_len;
        data += sizeof(uint16_t);
        memcpy(data, temp->data, temp->payload_len);
        data += temp->payload_len;
        temp = temp->next;
    }
    
    return DATA_ERR_OK;
}

int msg_header_dump(struct msg_header *msg_hdr, uint8_t *data)
{
    if (msg_hdr == NULL || data == NULL) {
        return -DATA_ERR_PARAMETER;
    }

    memcpy(data, msg_hdr, sizeof(struct msg_header));

    return DATA_ERR_OK;
}

int msg_buff_dump(struct msg_buff *msgb, uint8_t *data)
{
    int ret;
    if (msgb == NULL|| data == NULL) {
        return -DATA_ERR_PARAMETER;
    }

    ret = msg_header_dump(msgb->hdr, data);
    if (ret != DATA_ERR_OK) {
        return ret;
    }

    if (!msgb->hdr->paylaod_len) {
        return DATA_ERR_OK;
    }

    ret = msg_block_dump(msgb->blk_head, data + sizeof(struct msg_header));
    if (ret != DATA_ERR_OK) {
        return ret;
    }

    return DATA_ERR_OK;
}

int msg_block_load(struct msg_buff *msgb, uint8_t *data)
{
    if (msgb == NULL || data == NULL) {
        return -DATA_ERR_PARAMETER;
    }

    uint32_t all_blk_len = msgb->hdr->paylaod_len;
    uint16_t prev_type = msgb->hdr->next_block;
    msgb->blk_head = (struct msg_block *)malloc(sizeof(struct msg_block));
    if (msgb->blk_head == NULL)
        return -DATA_ERR_NO_MEMORY;

    struct msg_block *prev_blk = msgb->blk_head;

    while (all_blk_len > 0) {
        prev_blk->type = prev_type;
        prev_type = *(uint16_t *)(data);
        prev_blk->payload_len = *(uint16_t *)(data + sizeof(uint16_t));
        prev_blk->data = (uint8_t *)malloc(prev_blk->payload_len);
        if (prev_blk->data == NULL)
            return -DATA_ERR_NO_MEMORY;

        memcpy(prev_blk->data, (data + sizeof(uint16_t) * 2), prev_blk->payload_len);

        if (prev_type == 0) {
            prev_blk->next = NULL;
            break;
        } else {
            prev_blk->next = (struct msg_block *)malloc(sizeof(struct msg_block));
            if (prev_blk->next == NULL)
                return -DATA_ERR_NO_MEMORY;
        }
        
        all_blk_len -= (sizeof(uint16_t) * 2 + prev_blk->payload_len);
        data += (sizeof(uint16_t) * 2 + prev_blk->payload_len);
        prev_blk = prev_blk->next;
    }
    
    return DATA_ERR_OK;
}

int msg_header_load(struct msg_header *msg_hdr, uint8_t *data)
{
    uint16_t check_sum;

    if (msg_hdr == NULL || data == NULL) {
        return -DATA_ERR_PARAMETER;
    }

    check_sum = crc_16((uint8_t *)data, sizeof(struct msg_header) - sizeof(check_sum));
    if (check_sum != *(uint16_t *)(data + sizeof(struct msg_header) - sizeof(check_sum)))
        return -DATA_ERR_CHECK_SUM_IS_INVALID;

    memcpy(msg_hdr, data, sizeof(struct msg_header));
    
    return DATA_ERR_OK;
}

int msg_buff_load(struct msg_buff *msgb, uint8_t *data)
{
    int ret;
    
    if (msgb == NULL || data == NULL) {
        return -DATA_ERR_PARAMETER;
    }
    
    ret = msg_header_load(msgb->hdr, data);
    if (ret != 0)
        return ret;

    if (msgb->hdr->paylaod_len == 0)
        return DATA_ERR_OK;

    data = data + sizeof(struct msg_header);
    ret = msg_block_load(msgb, data);
    if (ret != 0)
        return ret;

    return DATA_ERR_OK;
}

struct msg_block *msg_block_alloc(void)
{
    struct msg_block *msg_blk = (struct msg_block *)malloc(sizeof(struct msg_block));
    if (msg_blk == NULL)
        return NULL;

    msg_blk->next = NULL;
    msg_blk->data = NULL;
    msg_blk->payload_len = 0;

    return msg_blk;
}

struct msg_header *msg_header_alloc(void)
{
    struct msg_header *msg_hdr = (struct msg_header *)malloc(sizeof(struct msg_header));
    if (msg_hdr == NULL)
        return NULL;

    return msg_hdr;
}

struct msg_buff *msg_buff_alloc(void)
{
    struct msg_buff *msgb = (struct msg_buff *)malloc(sizeof(struct msg_buff));
    if (msgb == NULL)
        goto _free;

    msgb->hdr = msg_header_alloc();
    if (msgb->hdr == NULL)
        goto _free;

    msgb->blk_head = msg_block_alloc();
    if (msgb->blk_head == NULL)
        goto _free;

    return msgb;

_free:
    free(msgb);
    return NULL;
}

void msg_block_free(struct msg_block *msg_blk)
{
    if (msg_blk == NULL)
        return;

    if (msg_blk->data != NULL)
        free(msg_blk->data);

    if (msg_blk->next != NULL)
        msg_block_free(msg_blk->next);

    free(msg_blk);
}

void msg_header_free(struct msg_header *msg_hdr)
{
    if (msg_hdr == NULL)
        return;

    free(msg_hdr);
}

void msg_buff_free(struct msg_buff *msgb)
{
    if (msgb == NULL)
        return;

    msg_header_free(msgb->hdr);
    msg_block_free(msgb->blk_head);
    free(msgb);
}

int msg_queue_del(struct msg_queue *q, struct msg_buff *msgb)
{
    if (q == NULL || msgb == NULL)
        return -DATA_ERR_PARAMETER;

    if (q->count == 0)
        return -DATA_ERR_QUEUE_IS_EMPTY;
        
    if (q->head == q->tail) {
        memcpy(msgb, q->head, sizeof(struct msg_buff));
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = q->head->next;
        q->head->prev = NULL;
    }

    q->count--;

    return DATA_ERR_OK;
}

int msg_queue_add(struct msg_queue *q, struct msg_buff *msgb)
{
    if (q == NULL || msgb == NULL)
        return -DATA_ERR_PARAMETER;

    if (q->count >= q->size)
        return -DATA_ERR_QUEUE_IS_FULL;

    if (q->head == NULL) {
        q->head = msgb;
        q->tail = msgb;
    } else {
        q->tail->next = msgb;
        msgb->prev = q->tail;
        q->tail = msgb;
    }

    q->count++;

    return DATA_ERR_OK;
}

int msg_raw_queue_del(struct msg_raw_queue *q, struct msg_raw_buff *msgb_raw)
{
    if (q == NULL || msgb_raw == NULL)
        return -DATA_ERR_PARAMETER;

    if (q->count == 0)
        return -DATA_ERR_QUEUE_IS_EMPTY;
        
    if (q->head == q->tail) {
        memcpy(msgb_raw, q->head, sizeof(struct msg_raw_buff));
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = q->head->next;
        q->head->prev = NULL;
    }

    q->count--;

    return DATA_ERR_OK;
}

int msg_raw_queue_add(struct msg_raw_queue *q, struct msg_raw_buff *msgb_raw)
{
    if (q == NULL || msgb_raw == NULL)
        return -DATA_ERR_PARAMETER;

    if (q->count >= q->size)
        return -DATA_ERR_QUEUE_IS_FULL;

    if (q->head == NULL) {
        q->head = msgb_raw;
        q->tail = msgb_raw;
    } else {
        q->tail->next = msgb_raw;
        msgb_raw->prev = q->tail;
        q->tail = msgb_raw;
    }

    q->count++;

    return DATA_ERR_OK;
}

int msg_rxq_select(struct msg_rx_srvs *rx_srvs, struct msg_buff *msgb)
{
    int i;
    if (rx_srvs == NULL || msgb == NULL)
        return -DATA_ERR_PARAMETER;

    for (i = 0; i < rx_srvs->size; i++) {
        if (rx_srvs->srvs[i].id == msgb->hdr->priority) {   
            return i;
        }
    }

    return -DATA_ERR_QUEUE_NO_MATCH;
}

int msg_txq_select(struct msg_tx_srvs *tx_srvs, struct msg_buff *msgb)
{
    if (tx_srvs == NULL || msgb == NULL)
        return -DATA_ERR_PARAMETER;
    
    if (msgb->hdr->priority > MSG_PRIO_LEVEL4)
        return -DATA_ERR_QUEUE_NO_MATCH;

    if (msgb->hdr->priority == MSG_PRIO_LOW) {
        return 0;
    } else {
        return 1;
    }
}

int msg_rxq_raw_select(struct msg_rx_raw_srvs *rx_raw_srvs, struct msg_raw_buff *msgb_raw)
{
    int i;
    int ret;
    struct msg_header *msg_hdr;
    if (rx_raw_srvs == NULL || msgb_raw == NULL)
        return -DATA_ERR_PARAMETER;

    ret = msg_header_load(msg_hdr, msgb_raw->data);
    if (ret != DATA_ERR_OK)
        return ret;

    for (i = 0; i < rx_raw_srvs->size; i++) {
        if (rx_raw_srvs->srvs[i].id == msg_hdr->priority) {   
            return i;
        }
    }

    return -DATA_ERR_QUEUE_NO_MATCH;
}

int msg_rxq_init(struct msg_rx_srv *rx_srv)
{
    if (rx_srv == NULL)
        return -DATA_ERR_PARAMETER;

    rx_srv->q = (struct msg_queue *)malloc(sizeof(struct msg_queue));
    if (rx_srv->q == NULL)
        return -DATA_ERR_NO_MEMORY;

    rx_srv->q->head = NULL;
    rx_srv->q->tail = NULL;
    rx_srv->q->size = MSG_RXQ_SIZE;
    rx_srv->q->count = 0;

    return DATA_ERR_OK;
}

int msg_rx_srv_init(struct msg_rx_srv *rx_srv, uint32_t srv_id)
{
    int ret;
    if (rx_srv == NULL)
        return -DATA_ERR_PARAMETER;

    rx_srv->id = srv_id;
    rx_srv->type = MSG_Q_IDLE;

    ret = msg_rxq_init(rx_srv);
    if (ret != DATA_ERR_OK)
        return ret;

    return DATA_ERR_OK;
}

int msg_rx_srvs_init(struct data_ctrl_block *dcb)
{
    int i;
    int ret;
    dcb->rx_srvs = (struct msg_rx_srvs *)malloc(sizeof(struct msg_rx_srvs));
    if (dcb->rx_srvs == NULL) {
        return -DATA_ERR_NO_MEMORY;
    }
    memset(dcb->rx_srvs, 0, sizeof(struct msg_rx_srv));

    dcb->rx_srvs->size = MSG_RXQ_CNT;

    for (i = 0; i < MSG_RXQ_CNT; i++) {
        ret = msg_rx_srv_init(&dcb->rx_srvs->srvs[i], i);
        if (ret != DATA_ERR_OK) {
            return ret;
        }
    }

    return DATA_ERR_OK;
}

int msg_txq_init(struct msg_tx_srv *tx_srv)
{
    if (tx_srv == NULL)
        return -DATA_ERR_PARAMETER;

    tx_srv->q = (struct msg_queue *)malloc(sizeof(struct msg_queue));
    tx_srv->q->head = NULL;
    tx_srv->q->tail = NULL;
    tx_srv->q->size = MSG_TXQ_SIZE;
    tx_srv->q->count = 0;

    return DATA_ERR_OK;
}

int msg_tx_srv_init(struct msg_tx_srv *tx_srv, uint32_t srv_id)
{
    int ret;
    if (tx_srv == NULL)
        return -DATA_ERR_PARAMETER;

    ret = msg_txq_init(tx_srv);
    if (ret != DATA_ERR_OK)
        return ret;

    tx_srv->id = srv_id;
    tx_srv->type = MSG_Q_IDLE;

    return DATA_ERR_OK;
}

int msg_tx_srvs_init(struct data_ctrl_block *dcb)
{
    int i;
    int ret;
    dcb->tx_srvs = (struct msg_tx_srvs *)malloc(sizeof(struct msg_tx_srvs));
    if (dcb->tx_srvs == NULL) {
        return -DATA_ERR_NO_MEMORY;
    }
    memset(dcb->tx_srvs, 0, sizeof(struct msg_tx_srv));

    dcb->tx_srvs->size = MSG_TXQ_CNT;

    for (i = 0; i < MSG_TXQ_CNT; i++) {
        ret = msg_tx_srv_init(&dcb->tx_srvs->srvs[i], i);
        if (ret != DATA_ERR_OK) {
            return ret;
        }
    }

    return DATA_ERR_OK;
}

int msg_rxq_raw_init(struct msg_rx_raw_srv *rx_raw_srv)
{
    if (rx_raw_srv == NULL)
        return -DATA_ERR_PARAMETER;

    rx_raw_srv->q = (struct msg_raw_queue *)malloc(sizeof(struct msg_raw_queue));
    if (rx_raw_srv->q == NULL)
        return -DATA_ERR_NO_MEMORY;

    rx_raw_srv->q->head = NULL;
    rx_raw_srv->q->tail = NULL;
    rx_raw_srv->q->size = MSG_RXQ_RAW_SIZE;
    rx_raw_srv->q->count = 0;

    return DATA_ERR_OK;
}

int msg_rx_raw_srv_init(struct msg_rx_raw_srv *rx_raw_srv, uint32_t srv_id)
{
    int ret;
    if (rx_raw_srv == NULL)
        return -DATA_ERR_PARAMETER;

    ret = msg_rxq_raw_init(rx_raw_srv);
    if (ret != DATA_ERR_OK)
        return ret;

    rx_raw_srv->id = srv_id;
    rx_raw_srv->type = MSG_Q_IDLE;

    return DATA_ERR_OK;
}

int msg_rx_raw_srvs_init(struct data_ctrl_block *dcb)
{
    int i;
    int ret;
    dcb->rx_raw_srvs = (struct msg_rx_raw_srvs *)malloc(sizeof(struct msg_rx_raw_srvs));
    if (dcb->rx_raw_srvs == NULL) {
        return -DATA_ERR_NO_MEMORY;
    }
    memset(dcb->rx_raw_srvs, 0, sizeof(struct msg_rx_raw_srv));

    dcb->rx_raw_srvs->size = MSG_RXQ_RAW_CNT;

    for (i = 0; i < MSG_RXQ_RAW_CNT; i++) {
        ret = msg_rx_raw_srv_init(&dcb->rx_raw_srvs->srvs[i], i);
        if (ret != DATA_ERR_OK) {
            return ret;
        }
    }

    return DATA_ERR_OK;
}

int data_init(struct data_ctrl_block *dcb)
{
    int ret;

    ret = msg_rx_srvs_init(dcb);
    if (ret != DATA_ERR_OK) {
        return ret;
    }

    ret = msg_tx_srvs_init(dcb);
    if (ret != DATA_ERR_OK) {
        return ret;
    }

    return DATA_ERR_OK;
}

struct data_ctrl_block *data_alloc(void)
{
    return (struct data_ctrl_block *)malloc(sizeof(struct data_ctrl_block));
}

void msg_rxq_deinit(struct msg_rx_srv *rx_srv)
{
    free(rx_srv->q);
}

void msg_rx_srv_deinit(struct msg_rx_srv *rx_srv)
{
    msg_rxq_deinit(rx_srv);
}

void msg_rx_srvs_deinit(struct data_ctrl_block *dcb)
{
    for (int i = 0; i < MSG_RXQ_CNT; i++) {
        msg_rxq_deinit(&dcb->rx_srvs->srvs[i]);
    }

    free(dcb->rx_srvs);
}

void msg_txq_deinit(struct msg_tx_srv *tx_srv)
{
    free(tx_srv->q);
}

void msg_tx_srv_deinit(struct msg_tx_srv *tx_srv)
{
    msg_txq_deinit(tx_srv);
}

void msg_tx_srvs_deinit(struct data_ctrl_block *dcb)
{
    for (int i = 0; i < MSG_TXQ_CNT; i++) {
        msg_txq_deinit(&dcb->tx_srvs->srvs[i]);
    }

    free(dcb->tx_srvs);
}

void msg_rxq_raw_deinit(struct msg_rx_raw_srv *rx_raw_srv)
{
    free(rx_raw_srv->q);
}

void msg_rx_raw_srv_deinit(struct msg_rx_raw_srv *rx_raw_srv)
{
    msg_rxq_raw_deinit(rx_raw_srv);
}

void msg_rx_raw_srvs_deinit(struct data_ctrl_block *dcb)
{
    for (int i = 0; i < MSG_RXQ_RAW_CNT; i++) {
        msg_rxq_raw_deinit(&dcb->rx_raw_srvs->srvs[i]);
    }

    free(dcb->rx_raw_srvs);
}

void data_deinit(struct data_ctrl_block *dcb)
{
    msg_rx_srvs_deinit(dcb);
    msg_tx_srvs_deinit(dcb);
}

void data_free(struct data_ctrl_block *dcb)
{
    free(dcb);
}
