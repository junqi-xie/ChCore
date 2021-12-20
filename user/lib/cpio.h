#pragma once

#include <lib/type.h>

/* Spec from https://man.openbsd.org/FreeBSD-12.0/cpio.5 */

/* Value for the field `c_magic'.  */
#define MAGIC	"070701"

/* Values for c_mode, OR'd together: */

#define C_IRUSR		000400
#define C_IWUSR		000200
#define C_IXUSR		000100
#define C_IRGRP		000040
#define C_IWGRP		000020
#define C_IXGRP		000010
#define C_IROTH		000004
#define C_IWOTH		000002
#define C_IXOTH		000001

#define C_ISUID		004000
#define C_ISGID		002000
#define C_ISVTX		001000

#define C_ISBLK		060000
#define C_ISCHR		020000
#define C_ISDIR		040000
#define C_ISFIFO	010000
#define C_ISSOCK	0140000
#define C_ISLNK		0120000
#define C_ISCTG		0110000
#define C_ISREG		0100000

struct cpio_newc_header {
	char c_magic[6];
	char c_ino[8];
	char c_mode[8];
	char c_uid[8];
	char c_gid[8];
	char c_nlink[8];
	char c_mtime[8];
	char c_filesize[8];
	char c_devmajor[8];
	char c_devminor[8];
	char c_rdevmajor[8];
	char c_rdevminor[8];
	char c_namesize[8];
	char c_check[8];
};

struct cpio_header {
	char c_magic[6];
	u64 c_ino;
	u64 c_mode;
	u64 c_uid;
	u64 c_gid;
	u64 c_nlink;
	u64 c_mtime;
	u64 c_filesize;
	u64 c_devmajor;
	u64 c_devminor;
	u64 c_rdevmajor;
	u64 c_rdevminor;
	u64 c_namesize;
	u64 c_check;
};

#define cpio_fatal(fmt, ...) do { \
	fprintf(stderr, "CPIO tool fatal error: " fmt, ##__VA_ARGS__); \
	exit(-1); \
} while (0)

#define cpio_fatal_e(fmt, ...) do { \
	fprintf(stderr, "CPIO tool fatal error (%s): " fmt, \
		strerror(errno), ##__VA_ARGS__); \
	exit(-1); \
} while (0)

#define ALIGN4_UP(x) ((((u64)x) & (~3llu)) + ((!!(((u64)x) & 3)) << 2))

struct cpio_file {
	struct cpio_header header;
	const char *name;
	const char *data;
	struct cpio_file *next;
};

struct {
	struct cpio_file head;
	struct cpio_file *tail;
} g_files;

void cpio_init_g_files(void);
int cpio_extract_file(const void *addr, const char *dirat);
void cpio_extract(const void *addr, const char *dirat);
void *cpio_extract_single(const void *addr, const char *target,
			  void *(*cpio_single_file_filler) (const void *start,
							    size_t size,
							    void *data),
			  void *data);
