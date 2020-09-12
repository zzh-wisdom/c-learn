#ifndef THREAD_POOL_CM_H
#define THREAD_POOL_CM_H

#include <pthread.h>
#include <stdlib.h>

#include "list.h"


enum futureStatus {
    Ready,
    Timeout,
    Deferred
};

typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_shutdown       = -3,
    threadpool_thread_failure = -4
} threadpool_error_t;

typedef struct Future_C {
    enum futureStatus *state;
    pthread_mutex_t lock;
    pthread_cond_t signal;
} Future_S;

typedef struct thread_task {
    struct list_head entry;     // 对应thread_pool_t的task_queue
    void (*func)(void *);
    void *arg;
} thread_task_t;

typedef struct thread_pool {
    pthread_mutex_t lock;         // 互斥领取任务 和 访问shutdown
    pthread_cond_t signal;        // 线程等待的信号量，和lock配合使用
    pthread_t *threads;           // 线程句柄数组，初始化大小为最大值（参数设置的）
    struct list_head task_queue;  //任务等待队列
    int thread_count;             // 线程个数，标志数组threads的大小
    int shutdown;
} thread_pool_t;

static inline thread_pool_t *create_thread_pool(int thread_count);
static inline int destory_thread_pool(thread_pool_t *pool);
static inline int threadpool_add_task(thread_pool_t *pool, void (*function)(void *), void *argument);
static inline void *threadpool_thread(void *threadpool);

static inline thread_pool_t *create_thread_pool(int thread_count) {
    thread_pool_t *pool = NULL;
    int i = 0;
    if (thread_count <= 0) {
        return NULL;
    }
    
    if ((pool = (thread_pool_t *)malloc(sizeof(thread_pool_t))) == NULL) {
        return NULL;
    }
    pool->thread_count = 0;
    pool->shutdown = 0;
    
    pool->threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    if (pool->threads == NULL) {
        free(pool);
        return NULL;
    }
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->signal, NULL);
    INIT_LIST_HEAD(&pool->task_queue);
    
    for (i = 0; i < thread_count; i ++) {
        if (pthread_create(&(pool->threads[i]), NULL, threadpool_thread, pool) != 0) {
            destory_thread_pool(pool);
            return NULL;
        }
        pool->thread_count ++;
    }
    
    return pool;
}

static inline void free_thread_pool(thread_pool_t *pool) {
    if (pool == NULL) {
        return ;
    }
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->signal);
    free(pool->threads);
    free(pool);
}

// 销毁线程池
// 先将shutdown设置为1
// 然后等待所有线程的正常结束，保证队列中剩余的任务均完成
static inline int destory_thread_pool(thread_pool_t *pool) {
    int i = 0;
    int err = 0;
    if (pool == NULL) {
        return threadpool_invalid;
    }
    
    pthread_mutex_lock(&pool->lock);
    
    do {
        if (pool->shutdown) { // 未调用destory_thread_pool便设置shutdown为1,说明已经关闭
            err = threadpool_shutdown;
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        
        pool->shutdown = 1;
        pthread_cond_broadcast(&pool->signal);
        pthread_mutex_unlock(&pool->lock);
        
        for (int i = 0; i < pool->thread_count; i ++) {  //等待所有线程结束
            pthread_join(pool->threads[i], NULL);
        }
    } while (0);
    
    if (!err) {
        free_thread_pool(pool);
    }
    return err;
}

static inline thread_task_t *new_thread_task(void (*func)(void *), void *arg) {
    thread_task_t *task = (thread_task_t *)malloc(sizeof(thread_task_t));
    if (task == NULL) {
        return NULL;
    }
    
    task->func = func;
    task->arg = arg;
    
    return task;
}

static inline void delete_thread_task(thread_task_t *task) {
    free(task);
}

// 这里应该要判断线程池是否已经关闭吧，若关闭便不能增加任务
static inline int threadpool_add_task(thread_pool_t *pool, void (*function)(void *), void *argument) {
    int res = 0;
    int should_singal_one = 0;
    thread_task_t *task = NULL;
    if (pool == NULL || function == NULL) {
        return threadpool_invalid;
    }
    
    task = new_thread_task(function, argument);
    if (task == NULL) {
        return threadpool_invalid;
    }
    pthread_mutex_lock(&pool->lock);
    
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->lock);
        return threadpool_shutdown;
    }
    
    if (list_empty(&pool->task_queue)) {
        should_singal_one = 1;
    }
    list_add_tail(&task->entry, &pool->task_queue);

    int num = list_entry_number(&pool->task_queue);
    //printf("task num: %d\n", num);
    
    if (should_singal_one) {
        // 第一次，则只用唤醒一个处于阻塞的线程
        pthread_cond_signal(&pool->signal);
    } else {
        // 有多个任务，广播信号，唤醒所有阻的线程
        pthread_cond_broadcast(&pool->signal);
    }
    
    pthread_mutex_unlock(&pool->lock);
    
    return res;
}

/**
 * 使用无限循环来不断从队列中接收任务，并执行
 **/
static inline void *threadpool_thread(void *threadpool) {
    thread_pool_t *pool = (thread_pool_t *)threadpool;
    thread_task_t *task = NULL;
    
    for (;;) {
        pthread_mutex_lock(&pool->lock);
        while (list_empty(&pool->task_queue) && (!pool->shutdown)) {
            pthread_cond_wait(&pool->signal, &pool->lock);
        }
        
        //如果还有任务，必须执行完任务才能退出
        if (pool->shutdown && list_empty(&pool->task_queue)) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        task = list_first_entry_or_null(&pool->task_queue, thread_task_t, entry);
        if (task != NULL) {
            list_del(&task->entry);
        }
        pthread_mutex_unlock(&pool->lock);
        
        if (task != NULL) {
            task->func(task->arg);
            delete_thread_task(task); // 释放task结构体本身占的空间
        }
    }
    
    // 显式退出线程
    pthread_exit(NULL);
    return NULL;
}

/***
 * function: is_threadpool_busy
 *     simple judge the pool is busy or not.
 * @pool thread_pool_t
 * return  true:busy,
 * **/
// 通过简单判断任务队列是否为空
static inline int is_threadpool_busy(thread_pool_t *pool) {
    // 粗粒度判读，只是简单判断task队列是否为空
    pthread_mutex_lock(&pool->lock);
    int busy = !list_empty(&pool->task_queue);
    pthread_mutex_unlock(&pool->lock);
    return busy;
}

#endif