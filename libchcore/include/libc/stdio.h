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

#include <chcore/console.h>
#include <chcore/tmpfs.h>

#define printf chcore_console_printf
#define cgetc  chcore_console_getc
#define putc   chcore_console_putc
#define getc   chcore_console_getc

typedef struct FILE {
        int fd;
        int mode;
} FILE;

FILE *fopen(const char *filename, const char *mode);
int fprintf(FILE *f, const char *fmt, ...);
size_t fwrite(const void *src, size_t size, size_t nmemb, FILE *f);
size_t fread(void *dst, size_t size, size_t nmemb, FILE *f);
int fclose(FILE *f);
