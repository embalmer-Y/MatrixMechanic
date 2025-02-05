#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proto.h"

int msg_checksum_calc(uint8_t *data)
{
    return crc_16((uint8_t *)data, sizeof(struct msg_header) - sizeof(uint16_t));
}

int msg_buff_is_micro(uint8_t *data)
{
    return *(data + sizeof(uint8_t)) & MSG_MICRO_BIT_MASK;
}

int msg_obj_is_micro(uint8_t *obj)
{
    return *(uint8_t *)obj;
}

static struct msg_block *do_msg_block_alloc(void)
{
    struct msg_block *msg_blk = (struct msg_block *)malloc(sizeof(struct msg_block));
    if (msg_blk == NULL)
        return NULL;

    msg_blk->next = NULL;
    msg_blk->data = NULL;
    msg_blk->len = 0;

    return msg_blk;
}

static struct msg_header *do_msg_header_alloc(void)
{
    struct msg_header *msg_hdr = (struct msg_header *)malloc(sizeof(struct msg_header));
    if (msg_hdr == NULL)
        return NULL;

    return msg_hdr;
}

static struct msg_buff *do_msg_buff_alloc(void)
{
    struct msg_buff *msgb = (struct msg_buff *)malloc(sizeof(struct msg_buff));
    if (msgb == NULL)
        goto _free;

    msgb->hdr = do_msg_header_alloc();
    if (msgb->hdr == NULL)
        goto _free;

    return msgb;

_free:
    free(msgb);
    return NULL;
}

static void do_msg_block_free(struct msg_block *msg_blk)
{
    if (msg_blk == NULL)
        return;

    if (msg_blk->data != NULL)
        free(msg_blk->data);

    if (msg_blk->next != NULL)
        do_msg_block_free(msg_blk->next);

    free(msg_blk);
}

static void do_msg_header_free(struct msg_header *msg_hdr)
{
    if (msg_hdr == NULL)
        return;

    free(msg_hdr);
}

static void do_msg_buff_free(struct msg_buff *msgb)
{
    if (msgb == NULL)
        return;

    do_msg_header_free(msgb->hdr);
    do_msg_block_free(msgb->blk);
    free(msgb);
}

static struct msg_micro_block *do_msg_micro_block_alloc(void)
{
    struct msg_micro_block *msg_blk = (struct msg_micro_block *)malloc(sizeof(struct msg_micro_block));
    if (msg_blk == NULL)
        return NULL;

    msg_blk->next = NULL;
    msg_blk->data = NULL;
    msg_blk->len = 0;

    return msg_blk;
}

static struct msg_micro_header *do_msg_micro_header_alloc(void)
{
    struct msg_micro_header *msg_hdr = (struct msg_micro_header *)malloc(sizeof(struct msg_micro_header));
    if (msg_hdr == NULL)
        return NULL;

    return msg_hdr;
}

static struct msg_micro_buff *do_msg_micro_buff_alloc(void)
{
    struct msg_micro_buff *msgb = (struct msg_micro_buff *)malloc(sizeof(struct msg_micro_buff));
    if (msgb == NULL)
        goto _micro_free;

    msgb->hdr = do_msg_micro_header_alloc();
    if (msgb->hdr == NULL)
        goto _micro_free;

    return msgb;

_micro_free:
    free(msgb);
    return NULL;
}

static void do_msg_micro_block_free(struct msg_micro_block *msg_blk)
{
    if (msg_blk == NULL)
        return;

    if (msg_blk->data != NULL)
        free(msg_blk->data);

    if (msg_blk->next != NULL)
        do_msg_micro_block_free(msg_blk->next);

    free(msg_blk);
}

static void do_msg_micro_header_free(struct msg_micro_header *msg_hdr)
{
    if (msg_hdr == NULL)
        return;

    free(msg_hdr);
}

static void do_msg_micro_buff_free(struct msg_micro_buff *msgb)
{
    if (msgb == NULL)
        return;

    do_msg_micro_header_free(msgb->hdr);
    do_msg_micro_block_free(msgb->blk);
    free(msgb);
}

void *msg_block_alloc(uint8_t is_micro)
{
    return is_micro ? (void *)do_msg_micro_block_alloc() : (void *)do_msg_block_alloc();
}

void *msg_header_alloc(uint8_t is_micro)
{
    return is_micro ? (void *)do_msg_micro_header_alloc() : (void *)do_msg_header_alloc();
}

void *msg_buff_alloc(uint8_t is_micro)
{
    return is_micro ? (void *)do_msg_micro_buff_alloc() : (void *)do_msg_buff_alloc();
}

void msg_block_free(void *msg_blk, uint8_t is_micro)
{
    if (msg_blk == NULL)
        return;

    if (is_micro) {
        do_msg_micro_block_free((struct msg_micro_block *)msg_blk);
    } else {
        do_msg_block_free((struct msg_block *)msg_blk);
    }
}

void msg_header_free(void *msg_hdr, uint8_t is_micro)
{
    if (msg_hdr == NULL)
        return;

    if (is_micro) {
        do_msg_micro_header_free((struct msg_micro_header *)msg_hdr);
    } else {
        do_msg_header_free((struct msg_header *)msg_hdr);
    }
}

void msg_buff_free(void *msgb)
{
    if (msgb == NULL)
        return;

    if (msg_obj_is_micro(msgb)) {
        do_msg_micro_buff_free((struct msg_micro_buff *)msgb);
    } else {
        do_msg_buff_free((struct msg_buff *)msgb);
    }
}

static int do_msg_block_dump(struct msg_block *msg_blk, uint8_t *data)
{
    struct msg_block *temp;

    if (msg_blk == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    temp = msg_blk;
    while (temp != NULL) {
        if (temp->next == NULL) {
            *(uint16_t *)data = 0;
        } else {
            *(uint16_t *)data = temp->next->type;
        }

        data += sizeof(uint16_t);
        *(uint16_t *)data = temp->len;
        data += sizeof(uint16_t);
        memcpy(data, temp->data, temp->len);
        data += temp->len;
        temp = temp->next;
    }
    
    return ERR_OK;
}

static int do_msg_header_dump(struct msg_header *msg_hdr, uint8_t *data)
{
    if (msg_hdr == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    memcpy(data, msg_hdr, sizeof(struct msg_header));

    return ERR_OK;
}

static int do_msg_buff_dump(struct msg_buff *msgb, uint8_t *data)
{
    int ret;

    ret = do_msg_header_dump(msgb->hdr, data);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!msgb->hdr->len) {
        return ERR_OK;
    }

    ret = do_msg_block_dump(msgb->blk, data + sizeof(struct msg_header));
    if (ret != ERR_OK) {
        return ret;
    }

    return ERR_OK;
}

static int do_msg_block_load(struct msg_buff *msgb, uint8_t *data)
{
    struct msg_block *prev_blk;
    uint32_t all_blk_len;
    uint16_t prev_type;

    if (msgb == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    all_blk_len = msgb->hdr->len;
    prev_type = msgb->hdr->next_block;

    msgb->blk = (struct msg_block *)msg_block_alloc(0);
    if (msgb->blk == NULL)
        return -ERR_NOMEM;

    prev_blk = msgb->blk;

    while (all_blk_len > 0) {
        prev_blk->type = prev_type;
        prev_type = *(uint16_t *)(data);
        prev_blk->len = *(uint16_t *)(data + sizeof(uint16_t));
        prev_blk->data = (uint8_t *)malloc(prev_blk->len);
        if (prev_blk->data == NULL)
            return -ERR_NOMEM;

        memcpy(prev_blk->data, (data + sizeof(uint16_t) * 2), prev_blk->len);

        if (prev_type == 0) {
            prev_blk->next = NULL;
            break;
        } else {
            prev_blk->next = (struct msg_block *)msg_block_alloc(0);
            if (prev_blk->next == NULL)
                return -ERR_NOMEM;
        }
        
        all_blk_len -= (sizeof(uint16_t) * 2 + prev_blk->len);
        data += (sizeof(uint16_t) * 2 + prev_blk->len);
        prev_blk = prev_blk->next;
    }
    
    return ERR_OK;
}

static int do_msg_header_load(struct msg_header *msg_hdr, uint8_t *data)
{
    uint16_t check_sum;

    if (msg_hdr == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    check_sum = msg_checksum_calc(data);
    if (check_sum != *(uint16_t *)(data + sizeof(struct msg_header) - sizeof(check_sum)))
        return -ERR_DATA_BROKEN;

    memcpy(msg_hdr, data, sizeof(struct msg_header));
    
    return ERR_OK;
}

static int do_msg_buff_load(struct msg_buff *msgb, uint8_t *data)
{
    int ret;
    
    ret = do_msg_header_load(msgb->hdr, data);
    if (ret != 0)
        return ret;

    if (msgb->hdr->len == 0)
        return ERR_OK;

    data = data + sizeof(struct msg_header);
    ret = do_msg_block_load(msgb, data);
    if (ret != 0)
        return ret;

    return ERR_OK;
}

/* Do Micro proto param */

static int do_msg_micro_block_dump(struct msg_micro_block *msg_blk, uint8_t *data)
{
    struct msg_micro_block *temp;

    if (msg_blk == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    temp = msg_blk;
    while (temp != NULL) {
        if (temp->next == NULL) {
            *(uint8_t *)data = 0;
        } else {
            *(uint8_t *)data = temp->next->type;
        }

        data += sizeof(uint8_t);
        *(uint8_t *)data = temp->len;
        data += sizeof(uint8_t);
        memcpy(data, temp->data, temp->len);
        data += temp->len;
        temp = temp->next;
    }
    
    return ERR_OK;
}

static int do_msg_micro_header_dump(struct msg_micro_header *msg_hdr, uint8_t *data)
{
    if (msg_hdr == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    memcpy(data, msg_hdr, sizeof(struct msg_micro_header));

    return ERR_OK;
}

static int do_msg_micro_buff_dump(struct msg_micro_buff *msgb, uint8_t *data)
{
    int ret;

    ret = do_msg_micro_header_dump(msgb->hdr, data);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!msgb->hdr->len) {
        return ERR_OK;
    }

    ret = do_msg_micro_block_dump(msgb->blk, data + sizeof(struct msg_header));
    if (ret != ERR_OK) {
        return ret;
    }

    return ERR_OK;
}

static int do_msg_micro_block_load(struct msg_micro_buff *msgb, uint8_t *data)
{
    struct msg_micro_block *prev_blk;
    uint16_t all_blk_len;
    uint8_t prev_type;

    if (msgb == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    all_blk_len = msgb->hdr->len;
    prev_type = msgb->hdr->next_block;
    msgb->blk = (struct msg_micro_block *)msg_block_alloc(1);
    if (msgb->blk == NULL)
        return -ERR_NOMEM;

    prev_blk = msgb->blk;

    while (all_blk_len > 0) {
        prev_blk->type = prev_type;
        prev_type = *(uint8_t *)(data);
        prev_blk->len = *(uint8_t *)(data + sizeof(uint8_t));
        prev_blk->data = (uint8_t *)malloc(prev_blk->len);
        if (prev_blk->data == NULL)
            return -ERR_NOMEM;

        memcpy(prev_blk->data, (data + sizeof(uint8_t) * 2), prev_blk->len);

        if (prev_type == 0) {
            prev_blk->next = NULL;
            break;
        } else {
            prev_blk->next = (struct msg_micro_block *)msg_block_alloc(1);
            if (prev_blk->next == NULL)
                return -ERR_NOMEM;
        }
        
        all_blk_len -= (sizeof(uint8_t) * 2 + prev_blk->len);
        data += (sizeof(uint8_t) * 2 + prev_blk->len);
        prev_blk = prev_blk->next;
    }
    
    return ERR_OK;
}

static int do_msg_micro_header_load(struct msg_micro_header *msg_hdr, uint8_t *data)
{
    uint16_t check_sum;

    if (msg_hdr == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    check_sum = msg_checksum_calc(data);
    if (check_sum != *(uint16_t *)(data + sizeof(struct msg_micro_header) - sizeof(check_sum)))
        return -ERR_DATA_BROKEN;

    memcpy(msg_hdr, data, sizeof(struct msg_micro_header));
    
    return ERR_OK;
}

static int do_msg_micro_buff_load(struct msg_micro_buff *msgb, uint8_t *data)
{
    int ret;
    
    ret = do_msg_micro_header_load(msgb->hdr, data);
    if (ret != 0)
        return ret;

    if (msgb->hdr->len == 0)
        return ERR_OK;

    data = data + sizeof(struct msg_micro_header);
    ret = do_msg_micro_block_load(msgb, data);
    if (ret != 0)
        return ret;

    return ERR_OK;
}

int msg_buff_dump(void *msgb, uint8_t *data)
{
    uint8_t is_micro;

    if (msgb == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    is_micro = *(uint8_t *)msgb;

    if (is_micro)
        return do_msg_micro_buff_dump(msgb, data);

    return do_msg_buff_dump(msgb, data);
}

int msg_buff_load(void *msgb, uint8_t *data)
{
    uint8_t is_micro;

    if (msgb == NULL || data == NULL) {
        return -ERR_INVALID;
    }

    is_micro = msg_buff_is_micro(data);

    if (is_micro)
        return do_msg_micro_buff_load(msgb, data);

    return do_msg_buff_load(msgb, data);
}

int msg_buff_get_hop_limit(uint8_t *data, uint8_t *hop_limit)
{
    if (data == NULL || hop_limit == NULL)
        return -ERR_INVALID;

    *hop_limit = *data;

    return ERR_OK;
}

int msg_buff_add_hop_limit(uint8_t *data)
{
    if (data == NULL)
        return -ERR_INVALID;

    (*data)++;

    return ERR_OK;
}

int msg_buff_get_prority(uint8_t *data, enum msg_priority *priority)
{
    if (data == NULL || priority == NULL)
        return -ERR_INVALID;

    *priority = GET_BIT(*(data + 1), MSG_PRIORITY_BIT_MASK);

    return ERR_OK;
}

int msg_buff_set_priority(uint8_t *data, enum msg_priority priority)
{
    if (data == NULL)
        return -ERR_INVALID;

    *(data + 1) = CLEAR_BYTE(*(data + 1), MSG_PRIORITY_BIT_MASK);
    *(data + 1) = SET_BIT(*(data + 1), priority);

    return ERR_OK;
}

int msg_buff_get_heart_rate(uint8_t *data, void *heart_rate)
{
    if (data == NULL || heart_rate == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data)) 
        *(uint8_t *)heart_rate = *(data + 2);
    else
        *(uint16_t *)heart_rate = *(uint16_t *)(data + 2);

        
    return ERR_OK;
}

int msg_buff_set_heart_rate(uint8_t *data, uint16_t heart_rate)
{
    if (data == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint8_t *)(data + 2) = heart_rate;
    else
        *(uint16_t *)(data + 2) = heart_rate;

    return ERR_OK;
}

int msg_buff_get_src(uint8_t *data, void *src_id)
{
    if (data == NULL || src_id == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint16_t *)src_id = *(uint16_t *)(data + 3);
    else
        *(uint32_t *)src_id = *(uint32_t *)(data + 4);

    return ERR_OK;
}

int msg_buff_set_src(uint8_t *data, uint32_t src_id)
{
    if (data == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint16_t *)(data + 3) = src_id;
    else
        *(uint32_t *)(data + 4) = src_id;

    return ERR_OK;
}

int msg_buff_get_dst(uint8_t *data, void *dst_id)
{
    if (data == NULL || dst_id == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint16_t *)dst_id = *(uint16_t *)(data + 5);
    else
        *(uint32_t *)dst_id = *(uint32_t *)(data + 8);

    return ERR_OK;
}

int msg_buff_set_dst(uint8_t *data, uint32_t dst_id)
{
    if (data == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint16_t *)(data + 5) = dst_id;
    else
        *(uint32_t *)(data + 8) = dst_id;

    return ERR_OK;
}

int msg_buff_get_len(uint8_t *data, void *len)
{
    if (data == NULL || len == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint8_t *)len = *(data + 7);
    else
        *(uint16_t *)len = *(uint16_t *)(data + 12);

    return ERR_OK;
}

int msg_buff_set_len(uint8_t *data, uint16_t len)
{
    if (data == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint8_t *)(data + 7) = len;
    else
        *(uint16_t *)(data + 12) = len;

    return ERR_OK;
}

int msg_buff_get_next_block(uint8_t *data, void *next_block)
{
    if (data == NULL || next_block == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint8_t *)next_block = *(data + 8);
    else
        *(uint16_t *)next_block = *(uint16_t *)(data + 14);

    return ERR_OK;
}

int msg_buff_set_next_block(uint8_t *data, uint16_t next_block)
{
    if (data == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint8_t *)(data + 8) = next_block;
    else
        *(uint16_t *)(data + 14) = next_block;

    return ERR_OK;
}

int msg_buff_get_checksum(uint8_t *data, uint16_t *checksum)
{
    if (data == NULL || checksum == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *checksum = *(uint16_t *)(data + 9);
    else
        *checksum = *(uint16_t *)(data + 16);

    return ERR_OK;
}

int msg_buff_set_checksum(uint8_t *data, uint16_t checksum)
{
    if (data == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *(uint16_t *)(data + 9) = checksum;
    else
        *(uint16_t *)(data + 16) = checksum;

    return ERR_OK;
}

int msg_buff_get_payload(uint8_t *data, uint8_t *payload)
{
    if (data == NULL || payload == NULL)
        return -ERR_INVALID;

    if (msg_buff_is_micro(data))
        *payload = *(data + sizeof(struct msg_micro_header));
    else
        *payload = *(data + sizeof(struct msg_header));

    return ERR_OK;
}

int msg_buff_get_block_next(uint8_t *block, void *next_type, uint8_t is_micro)
{
    if (block == NULL || next_type == NULL)
        return -ERR_INVALID;

    if (is_micro)
        *(uint8_t *)next_type = *(uint8_t *)(block + 0);
    else
        *(uint16_t *)next_type = *(uint16_t *)(block + 0);

    return ERR_OK;
}

int msg_buff_get_block_len(uint8_t *block, void *len, uint8_t is_micro)
{
    if (block == NULL || len == NULL)
        return -ERR_INVALID;

    if (is_micro)
        *(uint8_t *)len = *(uint8_t *)(block + 1);
    else
        *(uint16_t *)len = *(uint16_t *)(block + 2);

    return ERR_OK;
}

int msg_buff_get_block_data(uint8_t *block, uint8_t *data, uint8_t is_micro)
{
    if (block == NULL || data == NULL)
        return -ERR_INVALID;

    if (is_micro)
        *data = *(block + 2);
    else
        *data = *(block + 4);

    return ERR_OK;
}
