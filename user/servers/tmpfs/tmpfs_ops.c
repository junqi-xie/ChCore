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

#include "cpio.h"
#include <chcore/fs/defs.h>
#include <chcore/ipc.h>
#include <chcore/memory.h>
#include <chcore/assert.h>
#include <stdio.h>
#include <string.h>
#include <libc/sys/stat.h>
#include "wrapper.h"
#include "vnode.h"
#include "tmpfs.h"

int tmpfs_open(const char *path, int flags, int mode, unsigned long *vnode_id,
               size_t *vnode_size, int *vnode_type, void **vnode_private)
{
        struct inode *inode;
        int ret = -ENOENT;

        BUG_ON(!path);
        BUG_ON(*path != '/');

        inode = tfs_open_path((const char *)path);
        if (inode) {
                *vnode_id = (unsigned long)inode;
                *vnode_type = inode->type == FS_REG ? FS_NODE_REG : FS_NODE_DIR;
                *vnode_size = inode->size;
                *vnode_private = inode;
                get_inode(inode);
                ret = 0;
        }
        return ret;
}

int tmpfs_read(void *operator, unsigned long offset, size_t size, char *buf)
{
        struct inode *inode = (struct inode *)operator;
        int ret = 0;
        if (inode)
                ret = tfs_file_read(inode, offset, buf, size);
        return ret;
}

int tmpfs_write(void *operator, unsigned long offset, size_t size,
                const char *buf)
{
        struct inode *inode = (struct inode *)operator;
        int ret = 0;
        if (inode)
                ret = tfs_file_write(inode, offset, buf, size);
        return ret;
}

int tmpfs_close(void *operator, bool is_dir)
{
        int ret = 0;
        ret = put_inode((struct inode *)operator);
        return ret;
}

int tmpfs_creat(const char *path)
{
        struct inode *dirat = NULL;
        const char *leaf = path;
        int err;

        BUG_ON(!path);
        BUG_ON(*path != '/');

        err = tfs_namex(&dirat, &leaf, 1);
        if (!err)
                err = tfs_creat(dirat, leaf, strlen(leaf));
        return err;
}

int tmpfs_unlink(const char *path, int flags)
{
        struct inode *dirat = NULL;
        const char *leaf = path;
        int err;

        BUG_ON(!path);
        BUG_ON(*path != '/');

        err = tfs_namex(&dirat, &leaf, 0);
        if (!err)
                err = tfs_remove(dirat, leaf, strlen(leaf));
        return err;
}

int tmpfs_rmdir(const char *path, int flags)
{
        return tmpfs_unlink(path, flags);
}

int tmpfs_mkdir(const char *path, mode_t mode)
{
        struct inode *dirat = NULL;
        const char *leaf = path;
        int err;

        BUG_ON(!path);
        BUG_ON(*path != '/');

        err = tfs_namex(&dirat, &leaf, 1);
        if (!err)
                err = tfs_mkdir(dirat, leaf, strlen(leaf));
        return err;
}

int tmpfs_getdents(void *operator, unsigned long offset, size_t size, char *buf)
{
        struct inode *inode = (struct inode *)operator;
        int ret = 0, read_bytes;
        if (inode) {
                if (inode->type == FS_DIR) {
                        ret = tfs_scan(
                                inode, offset, buf, buf + size, &read_bytes);
                        ret = read_bytes;
                } else
                        ret = -ENOTDIR;
        } else
                ret = -ENOENT;
        return ret;
}

vaddr_t tmpfs_get_page_addr(void *operator, size_t offset)
{
        struct inode *inode;
        void *page;
        int page_no;

        inode = (struct inode *)operator;
        page_no = offset / PAGE_SIZE;
        page = radix_get(&inode->data, page_no);

        return (vaddr_t)page;
}

int tmpfs_getsize(char *path)
{
        struct inode *inode;
        int ret = -ENOENT;

        BUG_ON(!path);
        BUG_ON(*path != '/');

        inode = tfs_open_path((const char *)path);
        if (inode) {
                return inode->size;
        }
        return -1;
}

struct fs_server_ops server_ops = {
        .open = tmpfs_open,
        .read = tmpfs_read,
        .write = tmpfs_write,
        .close = tmpfs_close,
        .creat = tmpfs_creat,
        .unlink = tmpfs_unlink,
        .mkdir = tmpfs_mkdir,
        .rmdir = tmpfs_rmdir,
        .getdents64 = tmpfs_getdents,
        .getsize = tmpfs_getsize,
};