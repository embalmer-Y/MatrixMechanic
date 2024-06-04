#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "data.h"
#include "msg_checksum.h"

void ut_msg_buff_trace(struct msg_buff *msg)
{
    int i = 0;
    int j = 0;
    
    printf("-------------------HDR-----------------\n");
    printf("msg->hdr->hop_limit:0x%X\n", msg->hdr->hop_limit);
    printf("msg->hdr->priority:0x%X\n", msg->hdr->priority);
    printf("msg->hdr->flow_label:0x%X\n", msg->hdr->flow_label);
    printf("msg->hdr->heart_rate:0x%X\n", msg->hdr->heart_rate);
    printf("msg->hdr->src_id:0x%X\n", msg->hdr->src_id);
    printf("msg->hdr->dst_id:0x%X\n", msg->hdr->dst_id);
    printf("msg->hdr->paylaod_len:0x%X\n", msg->hdr->paylaod_len);
    printf("msg->hdr->next_block:0x%X\n", msg->hdr->next_block);
    printf("msg->hdr->check_sum:0x%X\n", msg->hdr->check_sum);
    printf("-------------------HDR-----------------\n");
    printf("hdr:\n");
    uint8_t *data = (uint8_t *)msg->hdr;
    for (i = 0; i < sizeof(struct msg_header); i++) {
        printf("%2x ", *(data + i));
        if (i % 16 == 15) 
            printf("\n");
    }

    printf("\n");
    printf("-------------------HDR-----------------\n\n");


    struct msg_block *temp;
    temp = msg->blk_head;
    i = 0;
    while(temp != NULL) {
        printf("-------------------BLK%u-----------------\n", i);
        printf("block[%u]->type:0x%X\n", i, temp->type);
        printf("block[%u]->payload_len:0x%X\n", i, temp->payload_len);
        printf("block[%u]->data:\n", i);
        for (j = 0; j < temp->payload_len; j++) {
            printf("%2x ", *(temp->data + j));
            if (j % 16 == 15) 
                printf("\n");
        }
        printf("\n");
        printf("-------------------BLK%u-----------------\n\n", i);
        temp = temp->next;
        i++;
    }
}

void ut_msg_data_trace(uint8_t *data, int len)
{
    int i = 0;
    printf("-------------------DATA-----------------\n");
    printf("data:\n");
    for (i = 0; i < len; i++) {
        printf("%2x ", *(data + i));
        if (i % 16 == 15) 
            printf("\n");
    }

    printf("\n");
    printf("-------------------DATA-----------------\n");
}

struct msg_buff *ut_create_msg(uint8_t prio)
{
    struct msg_buff *msg = malloc(sizeof(struct msg_buff));
    if (msg == NULL) {
        printf("malloc failed\n");
        return NULL;
    }

    struct msg_header *header = malloc(sizeof(struct msg_header));
    if (header == NULL) {
        printf("malloc failed\n");
        return NULL;
    }

    struct msg_block *block = malloc(sizeof(struct msg_block));
    if (block == NULL) {
        printf("malloc failed\n");
        return NULL;
    }

    struct msg_block *block_next = malloc(sizeof(struct msg_block));
    if (block_next == NULL) {
        printf("malloc failed\n");
        return NULL;
    }

    uint8_t *data1 = malloc(64);
    if (data1 == NULL) {
        printf("malloc failed\n");
        return NULL;
    }

    memset(data1, 0, 64);
    data1[0] = 1;
    data1[1] = 2;
    data1[2] = 3;

    uint8_t *data2 = malloc(64);
    if (data2 == NULL) {
        printf("malloc failed\n");
        return NULL;
    }

    memset(data2, 0, 64);
    data2[0] = 4;
    data2[1] = 5;
    data2[2] = 6;

    block->data = data1;
    block->next = block_next;
    block->type = 1;
    block->payload_len = 64;

    block_next->data = data2;
    block_next->next = NULL;
    block_next->type = 2;
    block_next->payload_len = 64;

    header->hop_limit = 1;
    header->priority = prio;
    header->src_id = 1;
    header->dst_id = 2;
    header->flow_label = 1;
    header->heart_rate = 60;
    header->paylaod_len = 128 + sizeof(uint16_t) * 4;
    header->next_block = 1;
    header->check_sum = crc_16((uint8_t *)header, sizeof(struct msg_header) - sizeof(uint16_t));

    msg->blk_head =block;
    msg->hdr = header;
    msg->next = NULL;
    msg->prev = NULL;

    return msg;
}

int main()
{
    int ret;
    struct data_ctrl_block *dcb;
    
    dcb = data_alloc();
    ret = data_init(dcb);
    if (ret != 0) {
        printf("data_init failed\n");
        return -1;
    }

    printf("dcb->tx_srvs->size:%u\n", dcb->tx_srvs->size);
    printf("dcb->rx_srvs->size:%u\n", dcb->rx_srvs->size);

    printf("dcb->tx_srvs->data[1].id:%d\n", dcb->tx_srvs->srvs[1].id);

    printf("dcb->rx_srvs->data[2].id:%d\n", dcb->rx_srvs->srvs[2].id);

    data_deinit(dcb);
    data_free(dcb);

    /* ------------------------------- */
    uint8_t *data = malloc(sizeof(struct msg_header) + 128 + sizeof(uint16_t) * 4);
    if (data == NULL) {
        printf("malloc failed\n");
        return -1;
    }
    memset(data, 0, sizeof(struct msg_header) + 128 + sizeof(uint16_t) * 4);

    struct msg_buff *msg = ut_create_msg(0);
    if (msg == NULL) {
        printf("create_msg failed\n");
        return -1;
    }
    ut_msg_buff_trace(msg);

    struct msg_buff *msgb_out = msg_buff_alloc();

    printf("------------------DUMP-----------------\n");
    msg_buff_dump(msg, data);
    ut_msg_data_trace(data, sizeof(struct msg_header) + 128 + sizeof(uint16_t) * 4);
    msg_buff_free(msg);
    printf("------------------DUMP-----------------\n\n");

    printf("------------------LOAD-----------------\n");
    msg_buff_load(msgb_out, data);
    ut_msg_buff_trace(msgb_out);
    msg_buff_free(msgb_out);
    free(data);
    printf("------------------LOAD-----------------\n\n");

    /* --------------------------------- */

    printf("------------------TXQ & RXQ-----------------\n");
    uint8_t index1, index2;
    struct data_ctrl_block *dcb1 = data_alloc();
    ret = data_init(dcb1);
    if (ret != 0) {
        printf("data_init failed\n");
        return -1;
    }

    struct msg_buff *msgb1 = ut_create_msg(1);
    if (msgb1 == NULL) {
        printf("create_msg failed\n");
        return -1;
    }

    struct msg_buff *msgb2 = ut_create_msg(2);
    if (msgb2 == NULL) {
        printf("create_msg failed\n");
        return -1;
    }

    index1 = msg_txq_select(dcb1->tx_srvs, msgb1);
    index2 = msg_txq_select(dcb1->tx_srvs, msgb2);

    printf("tx index1:%d\n", index1);
    printf("tx index2:%d\n", index2);

    index1 = msg_rxq_select(dcb1->rx_srvs, msgb1);
    index2 = msg_rxq_select(dcb1->rx_srvs, msgb2);

    printf("rx index1:%d\n", index1);
    printf("rx index2:%d\n", index2);

    struct msg_rx_srv *rx_srv = dcb1->rx_srvs[2].srvs;
    if (rx_srv == NULL)
        printf("rx_srvs is NULL\n");

    if (rx_srv->q == NULL) {
        printf("rxq1 is NULL\n");
        return -1;
    }

    msg_queue_add(dcb1->rx_srvs->srvs[index1].q, msgb1);
    msg_queue_add(dcb1->rx_srvs->srvs[index2].q, msgb2);

    printf("rxq1 size:%d\n", dcb1->rx_srvs->srvs[index1].q->count);
    printf("rxq2 size:%d\n", dcb1->rx_srvs->srvs[index2].q->count);

    struct msg_buff *msgb3 = msg_buff_alloc();
    struct msg_buff *msgb4 = msg_buff_alloc();
    msg_queue_del(dcb1->rx_srvs->srvs[index1].q, msgb3);
    msg_queue_del(dcb1->rx_srvs->srvs[index2].q, msgb4);

    ut_msg_buff_trace(msgb3);
    ut_msg_buff_trace(msgb4);
    printf("rxq1 size:%d\n", dcb1->rx_srvs->srvs[index1].q->count);
    printf("rxq2 size:%d\n", dcb1->rx_srvs->srvs[index2].q->count);

    data_deinit(dcb1);
    data_free(dcb1);
    msg_buff_free(msgb1);
    msg_buff_free(msgb2);

    printf("------------------TXQ & RXQ-----------------\n\n");

    return 0;
}
