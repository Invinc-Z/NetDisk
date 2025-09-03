#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "block_queue.h"
#include "commons.h"

typedef struct threadpool_s{
    pthread_t * p_threads;
    int thread_count;
    block_queue_t queue;
}threadpool_t;

int threadpool_init(threadpool_t * p_threadpool, int num); // 初始化线程池
int threadpool_destroy(threadpool_t * p_threadpool); // 销毁线程池
int threadpool_start(threadpool_t * p_threadpool); // 启动线程池
int threadpool_stop(threadpool_t * p_threadpool); // 停止线程池

#endif /* __THREADPOOL_H__ */



