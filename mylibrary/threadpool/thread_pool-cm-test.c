/**
 * 产生并打印连续的非负数
 **/
#include<stdio.h>
#include<string.h>
#include <time.h>
#include <unistd.h>

#include"thread_pool-cm.h"

typedef struct nums_print {
    pthread_mutex_t lock;
    pthread_cond_t signal;
    int *nums;
    int count;
    int print_cur;
} nums_print_t;

nums_print_t *create_nums_print(int count) {
    if (count <= 0) {
        return 0;
    }
    nums_print_t *np = (nums_print_t *)malloc(sizeof(nums_print_t));
    if (!np) {
        return NULL;
    }
    
    np->print_cur = 0;
    np->count = count;
    pthread_mutex_init(&np->lock, NULL);
    pthread_cond_init(&np->signal, NULL);
    
    np->nums = (int *)malloc(sizeof(int) * count);
    if (!np->nums) {
        free(np);
        return NULL;
    }
    
    memset((void*)np->nums, -1, sizeof(int) * count);
    return np;
}

void destroy_nums_print(nums_print_t *np) {
    if (np == NULL) {
        return;
    }
    if (np->nums) {
        free(np->nums);
        np->nums = NULL;
    }
    free(np);
}

void print_nums(void *arg) {
    nums_print_t *np = (nums_print_t *) arg;
    int num, pos;
    while (1) {
        pthread_mutex_lock(&np->lock);
        while (np->print_cur < np->count && np->nums[np->print_cur] == -1) {
            // printf("wait.\n");
            pthread_cond_wait(&np->signal, &np->lock);
        }
        if (np->print_cur == np->count) {
            pthread_cond_signal(&np->signal);
            pthread_mutex_unlock(&np->lock);
            break;
        }
        
        pos = np->print_cur;
        num = np->nums[np->print_cur++];
        pthread_mutex_unlock(&np->lock);
        
        printf("%3d:%d\n", pos, num);
    }
}

typedef struct produce_nums {
    nums_print_t *np;
    int start;
    int end;
} produce_nums_t;

produce_nums_t *create_produce_nums(nums_print_t *np, int start, int end) {
    produce_nums_t *pn = (produce_nums_t *) malloc(sizeof(produce_nums_t));
    if (!pn) {
        return NULL;
    }
    
    pn->np = np;
    pn->start = start;
    pn->end = end;
    
    return pn;
}

void produce_nums(void *arg) {
    produce_nums_t *pn = (produce_nums_t *) arg;
    nums_print_t *np = pn->np;
    int num, pos;
    int ms = rand() % 500 + 500;
    // printf("produce_nums start\n");
    usleep(ms*1000);
    
    for (int i = pn->start; i < pn->end; i++) {
        np->nums[i] = i;
    }
    // for(int i = 0; i < np->count; i++) {
    //     printf("%d ", np->nums[i]);
    // }
    // printf("\n");

    pthread_cond_signal(&np->signal);
    free(arg);
}

int main() {
    srand(time(NULL));
    thread_pool_t *pool = create_thread_pool(5);
    if (!pool) {
        exit(-1);
    }
    
    nums_print_t *np = create_nums_print(100);
    if (!np) {
        exit(-1);
    }

    printf("start..\n");
    
    threadpool_add_task(pool, print_nums, (void *)np);
    threadpool_add_task(pool, print_nums, (void *)np);
    
    int step = 3;
    int start = 0;
    int end = 100;
    int num_count = end - start;
    int step_count = (num_count + step - 1) / step;
    for (int i = 0; i < step_count - 1; i++) {
        produce_nums_t *pn = create_produce_nums(np, start, start + step);
        if (!pn) {
            exit(-1);
        }
        threadpool_add_task(pool, produce_nums, (void *)pn);
        
        start += step;
    }
    produce_nums_t *pn = create_produce_nums(np, start, end);
    if (!pn) {
        exit(-1);
    }
    threadpool_add_task(pool, produce_nums, (void *)pn);

    printf("threadpool_add_task over\n");

    thread_pool_t *pool2 = create_thread_pool(1);
    if (!pool2) {
        exit(-1);
    }
    threadpool_add_task(pool2, destory_thread_pool, pool);

    sleep(1);
    //pn = create_produce_nums(np, start, end);
    int res = threadpool_add_task(pool, produce_nums, (void *)pn);
    
    destory_thread_pool(pool2);
    destroy_nums_print(np);

    printf("add task res:%d\n", res);
    
    return 0;
}