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

#include <chcore/assert.h>
#include <chcore/ipc.h>
#include <chcore/procm.h>
#include <chcore/thread.h>
#include <chcore/fs/defs.h>
#include <chcore/tmpfs.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define SERVER_READY_FLAG(vaddr) (*(int *)(vaddr))
#define SERVER_EXIT_FLAG(vaddr)  (*(int *)((u64)vaddr + 4))

/* fs_server_cap in current process; can be copied to others */
int fs_server_cap;
extern struct ipc_struct *tmpfs_ipc_struct;

#define BUFLEN 4096
static char path[BUFLEN] = "/";

static void get_path(char *pathbuf, char *cmdline)
{
        pathbuf[0] = '\0';
        while (*cmdline == ' ')
                cmdline++;
        if (*cmdline != '/')
                strcpy(pathbuf, path);
        if (*cmdline != '\0')
                strcat(pathbuf, cmdline);
        else if (strlen(pathbuf) != 1)
                pathbuf[strlen(pathbuf) - 1] = '\0';
}

/* Retrieve the entry name from one dirent */
static void get_dent_name(struct dirent *p, char name[])
{
        int len;
        len = p->d_reclen - sizeof(p->d_ino) - sizeof(p->d_off)
              - sizeof(p->d_reclen) - sizeof(p->d_type);
        memcpy(name, p->d_name, len);
        name[len - 1] = '\0';
}

int do_complement(char *buf, char *complement, int complement_time)
{
        int ret;
        char pathbuf[BUFLEN];
        char name[BUFLEN];
        char scan_buf[BUFLEN];
        struct dirent *p;
        int i, len;

        get_path(pathbuf, buf);
        i = strlen(pathbuf) - 1;
        while (pathbuf[i] != '/')
                --i;
        if (i == 0) {
                strcpy(buf, pathbuf + 1);
                pathbuf[1] = '\0';
        } else {
                strcpy(buf, pathbuf + i + 1);
                pathbuf[i] = '\0';
        }
        len = strlen(buf);

        int fd = alloc_fd();
        ret = fs_open(pathbuf, fd, O_RDONLY);
        ret = fs_getdents(fd, BUFLEN, scan_buf);
        for (i = 0; i < ret; i += p->d_reclen) {
                p = (struct dirent *)(scan_buf + i);
                get_dent_name(p, name);
                if (*name != '.' && strncmp(buf, name, len) == 0) {
                        if (complement_time == 0) {
                                strcpy(complement, name + len);
                                return 0;
                        }
                        --complement_time;
                }
        }
        ret = fs_close(fd);
        return -1;
}

extern char getch();

// read a command from stdin leading by `prompt`
// put the commond in `buf` and return `buf`
// What you typed should be displayed on the screen
char *readline(const char *prompt)
{
        static char buf[BUFLEN];

        int i = 0, j = 0;
        signed char c = 0;
        int ret = 0;
        char complement[BUFLEN];
        int complement_time = 0;

        if (prompt != NULL)
                printf("%s", prompt);
        complement[0] = '\0';

        while (1) {
                __chcore_sys_yield();
                c = getch();

                /* Fill buf and handle tabs with do_complement(). */
                if (c < 0)
                        return NULL;
                else if (c == '\t') {
                        buf[i] = '\0';
                        if (complement_time == 0) {
                                j = i;
                                while (j > 0 && buf[j] != ' ')
                                        --j;
                                if (buf[j] == ' ')
                                        ++j;
                        }

                        ret = do_complement(
                                buf + j, complement, complement_time);

                        strcpy(buf + i, complement);
                        printf("\n%s%s", prompt, buf);
                        ++complement_time;
                } else {
                        complement_time = 0;
                        if (*complement) {
                                i += strlen(complement);
                                complement[0] = '\0';
                        }
                        putc(c);
                        if (c == '\r' || c == '\n')
                                break;
                        buf[i++] = c;
                }
        }
        buf[i] = '\0';
        return buf;
}

int do_cd(char *cmdline)
{
        cmdline += 2;
        while (*cmdline == ' ')
                cmdline++;
        if (*cmdline == '\0')
                return 0;
        else if (*cmdline != '/') {
                strcat(path, cmdline);
                strcat(path, "/");
        } else
                strcpy(path, cmdline);
        return 0;
}

int do_ls(char *cmdline)
{
        int ret;
        char pathbuf[BUFLEN];
        char name[BUFLEN];
        char scan_buf[BUFLEN];
        struct dirent *p;
        int i;

        cmdline += 2;
        get_path(pathbuf, cmdline);

        int fd = alloc_fd();
        ret = fs_open(pathbuf, fd, O_RDONLY);
        if (ret < 0) {
                printf("[Shell] No such directory\n");
                return ret;
        }

        ret = fs_getdents(fd, BUFLEN, scan_buf);
        if (ret < 0) {
                printf("[Shell] Not a directory\n");
                return ret;
        }
        for (i = 0; i < ret; i += p->d_reclen) {
                p = (struct dirent *)(scan_buf + i);
                get_dent_name(p, name);
                if (*name != '.')
                        printf("%s ", name);
        }
        printf("\n");
        ret = fs_close(fd);
        return ret;
}

int do_cat(char *cmdline)
{
        int ret;
        char pathbuf[BUFLEN];

        cmdline += 3;
        get_path(pathbuf, cmdline);

        int file_size = fs_getsize(pathbuf);
        char buf[file_size];

        int file_fd = alloc_fd();
        ret = fs_open(pathbuf, file_fd, O_RDONLY);
        if (ret < 0) {
                printf("[Shell] No such file\n");
                return ret;
        }
        ret = fs_read(file_fd, file_size, buf);
        if (ret < 0) {
                printf("[Shell] Permission denied\n");
                return ret;
        }
        printf("%s", buf);
        ret = fs_close(file_fd);
        return ret;
}

int do_echo(char *cmdline)
{
        cmdline += 4;
        while (*cmdline == ' ')
                cmdline++;
        printf("%s", cmdline);
        return 0;
}

int do_top()
{
        __chcore_sys_top();
        return 0;
}

void do_clear(void)
{
        putc(12);
        putc(27);
        putc('[');
        putc('2');
        putc('J');
}

int builtin_cmd(char *cmdline)
{
        int ret, i;
        char cmd[BUFLEN];
        for (i = 0; cmdline[i] != ' ' && cmdline[i] != '\0'; i++)
                cmd[i] = cmdline[i];
        cmd[i] = '\0';
        if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit")) {
                __chcore_sys_thread_exit();
        } else if (!strcmp(cmd, "cd")) {
                ret = do_cd(cmdline);
                return !ret ? 1 : -1;
        } else if (!strcmp(cmd, "ls")) {
                ret = do_ls(cmdline);
                return !ret ? 1 : -1;
        } else if (!strcmp(cmd, "echo")) {
                ret = do_echo(cmdline);
                return !ret ? 1 : -1;
        } else if (!strcmp(cmd, "cat")) {
                ret = do_cat(cmdline);
                return !ret ? 1 : -1;
        } else if (!strcmp(cmd, "clear")) {
                do_clear();
                return 1;
        } else if (!strcmp(cmd, "top")) {
                ret = do_top();
                return !ret ? 1 : -1;
        }
        return 0;
}

int run_cmd(char *cmdline)
{
        char pathbuf[BUFLEN];
        int cap = fs_server_cap;

        get_path(pathbuf, cmdline);
        int ret = chcore_procm_spawn(pathbuf, &cap);
        return ret;
}

void connect_fs(void)
{
        // register fs client
        connect_tmpfs_server();
}
