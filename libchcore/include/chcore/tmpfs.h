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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

int alloc_fd();
void connect_tmpfs_server();

int fs_open(const char *path, int fd, int mode);
int fs_read(int fd, size_t size, char *buf);
int fs_write(int fd, size_t size, const char *buf);
int fs_close(int fd);
int fs_creat(const char *path);
int fs_unlink(const char *path);
int fs_mkdir(const char *path);
int fs_rmdir(const char *path);
int fs_getdents(int fd, size_t size, char *buf);
int fs_getsize(const char *path);

#ifdef __cplusplus
}
#endif
