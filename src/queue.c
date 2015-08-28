/*
 c-pthread-queue - c implementation of a bounded buffer queue using posix threads
 Copyright (C) 2008  Matthew Dickinson
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//
//  queue.c
//
//  Modified by Tung HOANG on 12/16/13.
//  Copyright (c) 2013 TungMacBook. All rights reserved.
//

#include <pthread.h>
#include <unistd.h>
#include "queue.h"

void queue_init(queue_t *queue, int cap, int data_size, pj_pool_t *p_mempool) {
    queue->capacity = cap;
    queue->buffer = pj_pool_zalloc(p_mempool, sizeof(void *) * cap);
    queue->size = 0;
    queue->in = 0;
    queue->out = 0;

//#define QEPOOL_SIZE(queue) (5*queue->capacity)
//    qepool_init(&(queue->qepool), QEPOOL_SIZE(queue), data_size, p_mempool);
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond_full, NULL);
    pthread_cond_init(&queue->cond_empty, NULL);
}

void queue_enqueue(queue_t *queue, void *value)
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == queue->capacity) {
        pj_thread_sleep(2);
        pthread_cond_wait(&(queue->cond_full), &(queue->mutex));
    }
    queue->buffer[queue->in] = value;
    ++ queue->size;
    ++ queue->in;
    queue->in %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_empty));
}

void *queue_dequeue(queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 0) {
        pj_thread_sleep(2);
        pthread_cond_wait(&(queue->cond_empty), &(queue->mutex));
    }
    void *value = queue->buffer[queue->out];
    //printf("dequeue %d\n", *(int *)value);
    -- queue->size;
    ++ queue->out;
    queue->out %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_full));
    return value;
}

int queue_size(queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    int size = queue->size;
    pthread_mutex_unlock(&(queue->mutex));
    return size;
}

