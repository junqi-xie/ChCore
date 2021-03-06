#include <print.h>
#include <syscall.h>
#include <launcher.h>
#include <defs.h>
#include <bug.h>
#include <fs_defs.h>
#include <ipc.h>
#include <string.h>
#include <proc.h>

#define SERVER_READY_FLAG(vaddr) (*(int *)(vaddr))
#define SERVER_EXIT_FLAG(vaddr)  (*(int *)((u64)vaddr+ 4))

extern ipc_struct_t *tmpfs_ipc_struct;
static ipc_struct_t ipc_struct;
static int tmpfs_scan_pmo_cap, tmpfs_read_pmo_cap;

/* fs_server_cap in current process; can be copied to others */
int fs_server_cap;

#define BUFLEN	4096
static char path[BUFLEN] = "/";

static int get_path(char *pathbuf, char *cmdline)
{
	pathbuf[0] = '\0';
	while (*cmdline == ' ')
		cmdline++;
	if (*cmdline == '\0')
		return -1;
	else if (*cmdline != '/')
		strcpy(pathbuf, path);
	strcat(pathbuf, cmdline);
	return 0;
}

static int do_complement(char *buf, char *complement, int complement_time)
{
	int ret, start = 0;
	char pathbuf[BUFLEN];
	char strbuf[256];
	void *vp;
	struct dirent *p;
	int i, len;

	ret = get_path(pathbuf, buf);
	printf("\n%s", pathbuf);
	if (ret < 0) {
		printf("[Shell] No input file\n");
		return ret;
	}
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

	do {
		ret = fs_scan(pathbuf, &tmpfs_scan_pmo_cap, start);
		if (ret < 0) {
			printf("[Shell] No such directory\n");
			return ret;
		}

		vp = (void*)TMPFS_SCAN_BUF_VADDR;
		start += ret;
		for (i = 0; i < ret; ++i) {
			p = vp;
			strcpy(strbuf, p->d_name);
			if (strncmp(buf, strbuf, len) == 0) {
				if (complement_time == 0) {
					strcpy(complement, strbuf + len);
					return 0;
				}
				--complement_time;
			}
			vp += p->d_reclen;
		}
	} while (ret != 0);

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

	if (prompt != NULL) {
		printf("%s", prompt);
	}

	while (1) {
		c = getch();
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

			ret = do_complement(buf + j, complement, complement_time);
			if (ret < 0)
				break;

			strcpy(buf + i, complement);
			printf("\n%s%s", prompt, buf);
			++complement_time;
		} else {
			complement_time = 0;
			usys_putc(c);
			if (c == '\n')
				break;
			buf[i] = c;
			++i;
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
	}
	else
		strcpy(path, cmdline);
	return 0;
}

int do_top()
{
	usys_top();
	return 0;
}

int do_ls(char *cmdline)
{
	int ret, start = 0;
	char pathbuf[BUFLEN];
	char strbuf[256];
	void *vp;
	struct dirent *p;
	int i;

	cmdline += 2;
	ret = get_path(pathbuf, cmdline);
	if (ret < 0) {
		strcpy(pathbuf, path);
	}
	i = strlen(pathbuf) - 1;
	if (i != 0 && pathbuf[i] == '/') {
		pathbuf[i] = '\0';
	}

	do {
		ret = fs_scan(pathbuf, &tmpfs_scan_pmo_cap, start);
		if (ret < 0) {
			printf("[Shell] No such directory\n");
			return ret;
		}

		vp = (void *)TMPFS_SCAN_BUF_VADDR;
		start += ret;
		for (i = 0; i < ret; ++i) {
			p = vp;
			strcpy(strbuf, p->d_name);
			if (strbuf[0] != '.') {
				printf("%s\n", strbuf);
			}
			vp += p->d_reclen;
		}
	} while (ret != 0);

	return 0;
}

int do_cat(char *cmdline)
{
	int ret;
	char pathbuf[BUFLEN];

	cmdline += 3;
	ret = get_path(pathbuf, cmdline);
	if (ret < 0) {
		printf("[Shell] No input file\n");
		return ret;
	}

	ret = fs_read(pathbuf, &tmpfs_read_pmo_cap);
	if (ret < 0) {
		printf("[Shell] No such file\n");
		return ret;
	}
	printf("%s\n", (const char *)TMPFS_READ_BUF_VADDR);

	return 0;
}

int do_echo(char *cmdline)
{
	cmdline += 4;
	while (*cmdline == ' ')
		cmdline++;
	printf("%s\n", cmdline);
	return 0;
}

void do_clear(void)
{
	usys_putc(12);
	usys_putc(27);
	usys_putc('[');
	usys_putc('2');
	usys_putc('J');
}

int builtin_cmd(char *cmdline)
{
	int ret, i;
	char cmd[BUFLEN];
	for (i = 0; cmdline[i] != ' ' && cmdline[i] != '\0'; i++)
		cmd[i] = cmdline[i];
	cmd[i] = '\0';
	if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit")) {
		usys_exit(0);
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
	struct user_elf user_elf;
	int ret;
	int caps[1];

	ret = get_path(pathbuf, cmdline);
	if (ret < 0) {
		printf("[Shell] No input file\n");
		return ret;
	}

	ret = fs_readelf(pathbuf, &tmpfs_read_pmo_cap, &user_elf);
	if (ret < 0) {
		printf("[Shell] No such binary\n");
		return ret;
	}

	caps[0] = fs_server_cap;
	return launch_process_with_pmos_caps(&user_elf, NULL, NULL,
					     NULL, 0, caps, 1, 0);
}

static int
run_cmd_from_kernel_cpio(const char *filename, int *new_thread_cap,
			 struct pmo_map_request *pmo_map_reqs,
			 int nr_pmo_map_reqs)
{
	struct user_elf user_elf;
	int ret;

	ret = readelf_from_kernel_cpio(filename, &user_elf);
	if (ret < 0) {
		printf("[Shell] No such binary in kernel cpio\n");
		return ret;
	}
	return launch_process_with_pmos_caps(&user_elf, NULL, new_thread_cap,
					     pmo_map_reqs, nr_pmo_map_reqs,
					     NULL, 0, 0);
}

void boot_fs(void)
{
	int ret = 0;
	int info_pmo_cap;
	int tmpfs_main_thread_cap;
	struct pmo_map_request pmo_map_requests[1];

	/* create a new process */
	printf("Booting fs...\n");
	/* prepare the info_page (transfer init info) for the new process */
	info_pmo_cap = usys_create_pmo(PAGE_SIZE, PMO_DATA);
	fail_cond(info_pmo_cap < 0, "usys_create_ret ret %d\n", info_pmo_cap);

	ret = usys_map_pmo(SELF_CAP,
			   info_pmo_cap, TMPFS_INFO_VADDR, VM_READ | VM_WRITE);
	fail_cond(ret < 0, "usys_map_pmo ret %d\n", ret);

	SERVER_READY_FLAG(TMPFS_INFO_VADDR) = 0;
	SERVER_EXIT_FLAG(TMPFS_INFO_VADDR) = 0;

	/* We also pass the info page to the new process  */
	pmo_map_requests[0].pmo_cap = info_pmo_cap;
	pmo_map_requests[0].addr = TMPFS_INFO_VADDR;
	pmo_map_requests[0].perm = VM_READ | VM_WRITE;
	ret = run_cmd_from_kernel_cpio("/tmpfs.srv", &tmpfs_main_thread_cap,
				       pmo_map_requests, 1);
	fail_cond(ret != 0, "create_process returns %d\n", ret);

	fs_server_cap = tmpfs_main_thread_cap;

	while (SERVER_READY_FLAG(TMPFS_INFO_VADDR) != 1)
		usys_yield();

	/* register IPC client */
	tmpfs_ipc_struct = &ipc_struct;
	ret = ipc_register_client(tmpfs_main_thread_cap, tmpfs_ipc_struct);
	fail_cond(ret < 0, "ipc_register_client failed\n");

	tmpfs_scan_pmo_cap = usys_create_pmo(PAGE_SIZE, PMO_DATA);
	fail_cond(tmpfs_scan_pmo_cap < 0, "usys create_ret ret %d\n",
		  tmpfs_scan_pmo_cap);

	ret = usys_map_pmo(SELF_CAP,
			   tmpfs_scan_pmo_cap,
			   TMPFS_SCAN_BUF_VADDR, VM_READ | VM_WRITE);
	fail_cond(ret < 0, "usys_map_pmo ret %d\n", ret);

	tmpfs_read_pmo_cap = usys_create_pmo(SIZE_2M, PMO_DATA);
	fail_cond(tmpfs_read_pmo_cap < 0, "usys create_ret ret %d\n",
		  tmpfs_read_pmo_cap);

	ret = usys_map_pmo(SELF_CAP,
			   tmpfs_read_pmo_cap,
			   TMPFS_READ_BUF_VADDR, VM_READ | VM_WRITE);
	fail_cond(ret < 0, "usys_map_pmo ret %d\n", ret);

	printf("fs is UP.\n");
}
