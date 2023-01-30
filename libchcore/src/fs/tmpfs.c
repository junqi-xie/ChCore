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

#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/assert.h>
#include <chcore/internal/server_caps.h>
#include <chcore/fs/defs.h>
#include <string.h>

struct ipc_struct *tmpfs_ipc_struct = NULL;

int alloc_fd()
{
        // 0: stdin 1: stdout 2: stderr
        static int fd = 3;
        return fd++;
}

void connect_tmpfs_server(void)
{
        int tmpfs_cap = __chcore_get_tmpfs_cap();
        chcore_assert(tmpfs_cap >= 0);
        tmpfs_ipc_struct = ipc_register_client(tmpfs_cap);
        chcore_assert(tmpfs_ipc_struct);
}

int fs_open(const char *path, int fd, int mode)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_OPEN;
        strcpy(fr->open.pathname, path);
        fr->open.flags = mode;
        fr->open.new_fd = fd;
        int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}

int fs_read(int fd, size_t size, char *buf)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        int ret, count;

        do {
                count = MIN(size, FS_BUF_SIZE);
                fr->req = FS_REQ_READ;
                fr->read.fd = fd;
                fr->read.count = count;
                ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
                if (ret < 0)
                        break;
                memcpy(buf, ipc_get_msg_data(ipc_msg), ret);
                buf += ret;
                size -= ret;
        } while (size > 0 && ret == count);

        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}

int fs_write(int fd, size_t size, const char *buf)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        int ret, count;

        do {
                count = MIN(size, FS_BUF_SIZE);
                memcpy((void *)fr + sizeof(struct fs_request), buf, count);
                fr->req = FS_REQ_WRITE;
                fr->write.count = count;
                fr->write.fd = fd;
                ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
                if (ret < 0)
                        break;
                buf += ret;
                size -= ret;
        } while (size > 0 && ret == count);

        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}

int fs_close(int fd)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CLOSE;
        fr->close.fd = fd;
        int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}

int fs_creat(const char *path)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, path);
        int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}

int fs_getdents(int fd, size_t size, char *buf)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        int ret, count;

        do {
                count = MIN(size, FS_BUF_SIZE);
                fr->req = FS_REQ_GETDENTS64;
                fr->getdents64.fd = fd;
                fr->getdents64.count = count;
                ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
                if (ret < 0)
                        break;
                memcpy(buf, ipc_get_msg_data(ipc_msg), ret);
                buf += ret;
                size -= ret;
        } while (size > 0 && ret == count);

        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}

int fs_getsize(const char *path)
{
        struct ipc_msg *ipc_msg =
                ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_GET_SIZE;
        strcpy(fr->getsize.pathname, path);
        int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        return ret;
}
