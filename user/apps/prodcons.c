/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <sync/buf.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <chcore/assert.h>
#include <stdio.h>

/* Test setting */
#define PROD_THD_CNT  10
#define PROD_ITEM_CNT 30
#define CONS_THD_CNT  10
#define COSM_ITEM_CNT (PROD_THD_CNT * PROD_ITEM_CNT) / CONS_THD_CNT
#define PLAT_CPU_NUM  4

#define PRIO 255

volatile int global_exit = 0;
s32 empty_slot;
s32 filled_slot;

int rand_time[5] = {2, 8, 10, 1, 4};
int sleep(int time)
{
        for (int i = 0; i < time * 1000; i++) {
                __chcore_sys_yield();
        }
        return 0;
}
int producer_cnt = 0;
int produce_new()
{
        sleep(rand_time[producer_cnt % 5]);
        return __sync_fetch_and_add(&producer_cnt, 1);
}

int msg_map[PROD_THD_CNT * PROD_ITEM_CNT] = {0};
int consume_msg(int msg)
{
        chcore_assert(msg_map[msg] == 0);
        msg_map[msg] = 1;
        int pct = (int)100 * msg / (PROD_THD_CNT * PROD_ITEM_CNT);
        int part = (PROD_THD_CNT * PROD_ITEM_CNT) / 10;
        if (msg % part == 0)
                printf("%d%%==", pct);
        else if (msg == PROD_THD_CNT * PROD_ITEM_CNT - 1)
                printf("100%%\n");
        return 0;
}

void *producer(void *arg)
{
        int new_msg;
        int i = 0;

        while (i < PROD_ITEM_CNT) {
                __chcore_sys_wait_sem(empty_slot, true);
                new_msg = produce_new();
                buffer_add_safe(new_msg);
                __chcore_sys_signal_sem(filled_slot);
                i++;
        }
        __sync_fetch_and_add(&global_exit, 1);
        return 0;
}

void *consumer(void *arg)
{
        int cur_msg;
        int i = 0;

        while (i < COSM_ITEM_CNT) {
                __chcore_sys_wait_sem(filled_slot, true);
                cur_msg = buffer_remove_safe();
                __chcore_sys_signal_sem(empty_slot);
                consume_msg(cur_msg);
                i++;
        }
        __sync_fetch_and_add(&global_exit, 1);
        return 0;
}

/* Test program for prodcons */
int main(int argc, char *argv[], char *envp[])
{
        int i = 0;
        int thread_cap;

        /* init */
        empty_slot = BUF_SIZE;
        filled_slot = 0;
        empty_slot = __chcore_sys_create_sem();
        filled_slot = __chcore_sys_create_sem();

        printf("Begin Producer/Consumer Test!\n");
        for (i = 0; i < BUF_SIZE; i++) {
                __chcore_sys_signal_sem(empty_slot);
        }
        printf("Progress:");
        for (i = 0; i < PROD_THD_CNT; i++) {
                thread_cap =
                        chcore_thread_create(producer, i + 1, PRIO, TYPE_USER);
                /* set affinity */
                __chcore_sys_set_affinity(thread_cap, i % PLAT_CPU_NUM);
        }
        for (i = 0; i < CONS_THD_CNT; i++) {
                thread_cap =
                        chcore_thread_create(consumer, i + 1, PRIO, TYPE_USER);
                /* set affinity */
                __chcore_sys_set_affinity(thread_cap, i % PLAT_CPU_NUM);
        }
        while (global_exit < PROD_THD_CNT + CONS_THD_CNT)
                __chcore_sys_yield();
        printf("Producer/Consumer Test Finish!\n");
        return 0;
}
