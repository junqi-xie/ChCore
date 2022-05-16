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

#include <common/types.h>
#include <common/errno.h>
#include <common/macro.h>
#include <common/lock.h>
#include <common/kprint.h>
#include <arch/sync.h>

#include "ticket.h"

int lock_init(struct lock *l)
{
        struct lock_impl *lock = (struct lock_impl *)l;
        BUG_ON(!lock);
        /* Initialize ticket lock */
        lock->owner = 0;
        lock->next = 0;
        smp_wmb();
        return 0;
}

void lock(struct lock *l)
{
        struct lock_impl *lock = (struct lock_impl *)l;
        u32 lockval = 0, newval = 0, ret = 0;

        /**
         * The following asm code means:
         *
         * lockval = atomic_fetch_and_add(lock->next, 1);
         * while(lockval != lock->owner);
         */
        BUG_ON(!lock);
        asm volatile(
                "       prfm    pstl1strm, %3\n"
                "1:     ldaxr   %w0, %3\n"
                "       add     %w1, %w0, #0x1\n"
                "       stxr    %w2, %w1, %3\n"
                "       cbnz    %w2, 1b\n"
                "2:     ldar    %w2, %4\n"
                "       cmp     %w0, %w2\n"
                "       b.ne    2b\n"
                : "=&r"(lockval), "=&r"(newval), "=&r"(ret), "+Q"(lock->next)
                : "Q"(lock->owner)
                : "memory");
}

int try_lock(struct lock *l)
{
        struct lock_impl *lock = (struct lock_impl *)l;
        u32 lockval = 0, newval = 0, ret = 0, ownerval = 0;

        BUG_ON(!lock);
        asm volatile("       prfm    pstl1strm, %4\n"
                     "1:     ldaxr   %w0, %4\n"
                     "       ldar    %w3, %5\n"
                     "       add     %w1, %w0, #0x1\n"
                     "       cmp     %w0, %w3\n"
                     "       b.ne    2f\n" /* fail */
                     "       stxr    %w2, %w1, %4\n"
                     "       cbnz    %w2, 1b\n" /* retry */
                     "       mov     %w2, #0x0\n" /* success */
                     "       dmb     ish\n" /* store -> load/store barrier */
                     "       b       3f\n"
                     "2:     mov     %w2, #0xffffffffffffffff\n" /* fail */
                     "3:\n"
                     : "=&r"(lockval),
                       "=&r"(newval),
                       "=&r"(ret),
                       "=&r"(ownerval),
                       "+Q"(lock->next)
                     : "Q"(lock->owner)
                     : "memory");
        return ret;
}

void unlock(struct lock *l)
{
        struct lock_impl *lock = (struct lock_impl *)l;

        BUG_ON(!lock);
        smp_mb(); /* load, store -> store barrier may use stlr here */

        lock->owner++;
}

int is_locked(struct lock *l)
{
        int ret = 0;
        struct lock_impl *lock = (struct lock_impl *)l;
        ret = lock->owner != lock->next;
        return ret;
}

struct lock big_kernel_lock;

/**
 * Initialization of the big kernel lock
 */
void kernel_lock_init(void)
{
        u32 ret = 0;
        ret = lock_init(&big_kernel_lock);
        BUG_ON(ret != 0);
}

/**
 * Acquire the big kernel lock
 */
void lock_kernel(void)
{
        lock(&big_kernel_lock);
}

/**
 * Release the big kernel lock
 */
void unlock_kernel(void)
{
        BUG_ON(!is_locked(&big_kernel_lock));
        unlock(&big_kernel_lock);
}
