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

#include "shell.h"

#include <stdio.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/fs/defs.h>
#include <chcore/assert.h>

// get a character from standard input
char getch()
{
        char c;
        c = getc();
        return c;
}

#ifdef SHELL_TEST
void shell_test();
#endif

int main(int argc, char *argv[])
{
        char *buf;
        int ret = 0;

        connect_fs();

#ifdef SHELL_TEST
        shell_test();
#endif

        printf("Hello from ChCore Shell!\n");

        while (1) {
                printf("\n");
                buf = readline("$ ");
                if (buf == NULL) {
                        printf("buf is NULL\n");
                        __chcore_sys_thread_exit();
                }
                if (buf[0] == 0)
                        continue;
                if (builtin_cmd(buf))
                        continue;
                if ((ret = run_cmd(buf)) < 0) {
                        printf("Cannot run %s, ERROR %d\n", buf, ret);
                }
        }

        return 0;
}
