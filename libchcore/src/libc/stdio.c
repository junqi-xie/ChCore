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

#include <chcore/console.h>
#include <chcore/ipc.h>
#include <chcore/procm.h>
#include <chcore/types.h>
#include <chcore/fs/defs.h>
#include <chcore/tmpfs.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <stdio.h>
#include <string.h>

#define FILE_BUF_LEN 1024

extern struct ipc_struct *tmpfs_ipc_struct;

FILE *fopen(const char *filename, const char *mode)
{
        FILE *f = malloc(sizeof(FILE));
        int file_fd = alloc_fd();
        f->fd = file_fd;
        f->mode = *mode == 'w' ? O_WRONLY : O_RDONLY;
        int ret = fs_open(filename, file_fd, f->mode);
        if (ret == -ENOENT && f->mode == O_WRONLY) {
                ret = fs_creat(filename);
                if (ret < 0) {
                        return NULL;
                }
                ret = fs_open(filename, file_fd, f->mode);
                if (ret < 0) {
                        return NULL;
                }
        } else if (ret < 0) {
                return NULL;
        }
        return f;
}

size_t fwrite(const void *src, size_t size, size_t nmemb, FILE *f)
{
        size_t len = size * nmemb;
        return fs_write(f->fd, len, src);
}

size_t fread(void *dst, size_t size, size_t nmemb, FILE *f)
{
        size_t len = size * nmemb;
        return fs_read(f->fd, len, dst);
}

int fclose(FILE *f)
{
        int ret = fs_close(f->fd);
        if (ret < 0) {
                return ret;
        }
        free(f);
        return 0;
}

int fprintf(FILE *f, const char *fmt, ...)
{
        char buf[FILE_BUF_LEN];
        char *p = buf;
        va_list va;
        va_start(va, fmt);
        int ret = simple_vsprintf(&p, fmt, va);
        va_end(va);
        if (ret < 0) {
                return ret;
        }
        ret = fwrite(buf, sizeof(char), ret, f);
        return ret;
}
