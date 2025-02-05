#include "ring.h"
#include <stdint.h>


void msg_data_free(struct msg_data *data)
{
    if (data == NULL)
        return;

    free(data);
}

struct msg_data *msg_data_alloc(void)
{
    struct msg_data *data = (struct msg_data *)malloc(sizeof(struct msg_data));
    if (data == NULL)
        return NULL;

    return data;
}

int msg_data_ring_init(struct msg_data **head, struct msg_data **tail, uint32_t cnt)
{
    struct msg_data *temp;
    uint32_t i, j;

    (*head) = temp;
    (*head) = msg_data_alloc();

    for (i = 0; i < cnt - 1; i++) {
        temp->next = msg_data_alloc();
        if (temp->next == NULL)
            goto _free;
        temp->next->prev = temp;

        temp = temp->next;
    }

    temp = temp->next;
    temp->next = *head;
    (*head)->prev = temp;
    *tail = *head;

    return ERR_OK;

_free:
    for (j = 0; j < i; j++) {
        temp = temp->prev;
        msg_data_free(temp->next);
    }
    msg_data_free(temp);

    return -ERR_NOMEM;
}

void msg_data_ring_deinit(struct msg_data *head, uint32_t cnt)
{
    struct msg_data *temp;
    uint32_t i;

    if (head == NULL)
        return;

    temp = head;
    for (i = 0; i < cnt - 1; i++) {
        temp = temp->next;
        if (temp->prev->data != NULL && temp->prev->is_raw == 1) {
            free(temp->prev->data);
        } else if (temp->prev->data != NULL && temp->prev->is_raw == 0) {
            msg_buff_free(temp->prev->data);
        }
           
        msg_data_free(temp->prev);
    }

    msg_data_free(temp);
}

int msg_data_ring_push(struct msg_data **tail, void *data, uint8_t is_raw)
{
    if (*tail == NULL || data == NULL)
        return -ERR_INVALID;

    (*tail)->next->data = data;
    (*tail)->next->is_raw = is_raw;
    *tail = (*tail)->next;

    return ERR_OK;
}

void *msg_data_ring_pull(struct msg_data **head, uint8_t *is_raw)
{
    void *data = NULL;

    if ((*head) == NULL)
        return NULL;

    data = (*head)->data;
    *is_raw = (*head)->is_raw;
    (*head)->data = NULL;
    *head = (*head)->next;

    return data;
}

int msg_data_ring_extend(struct msg_data **head, uint32_t cnt)
{
    struct msg_data *temp_head, *temp_tail;
    uint32_t i, j;

    if ((*head) == NULL)
        return -ERR_INVALID;

    temp_head = msg_data_alloc();
    temp_tail = temp_head;

    for (i = 0; i < cnt - 1; i++) {
        temp_tail->next = msg_data_alloc();
        if (temp_tail->next == NULL)
            goto _free;

        temp_tail->next->prev = temp_tail;
        temp_tail = temp_tail->next;
    }

    (*head)->prev->next = temp_head;
    temp_head->prev = (*head)->prev;
    temp_tail->next = *head;
    (*head)->prev = temp_tail;

    return ERR_OK;

_free:
    for (j = 0; j < i; j++) {
        temp_tail = temp_tail->prev;
        msg_data_free(temp_tail->next);
    }
    msg_data_free(temp_tail);

    return -ERR_NOMEM;
}

int msg_data_ring_decrease(struct msg_data **head, uint32_t cnt)
{
    struct msg_data *temp;
    uint32_t i;

    if ((*head) == NULL)
        return -ERR_INVALID;

    temp = (*head)->prev;

    for (i = 0; i < cnt; i++) {
        temp = temp->prev;
        msg_data_free(temp->next);
    }

    (*head)->prev = temp;
    temp->next = *head;

    return ERR_OK;
}
