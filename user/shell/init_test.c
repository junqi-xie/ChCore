#include "init.h"

#include <print.h>
#include <launcher.h>
#include <syscall.h>
#include <string.h>

/* blocking getchar */
char getch()
{
	static int i = 0;
	char *cmd1 = "echo ls tt yy \nbsd\t\t\ncmd\t\n";
	if (i == strlen(cmd1)) {
		return -1;
	}
	return cmd1[i++];
}

static void test_readline()
{
	readline("> ");
}

int main()
{
	printf("Start init_test.\n");
	usys_fs_load_cpio(CPIO_BIN);

	boot_fs();

	test_readline();
	builtin_cmd("echo abc123XYZ");
	builtin_cmd("ls");
	builtin_cmd("ls tar");
	builtin_cmd("cd tar");
	builtin_cmd("cat cat_test.txt");
	test_readline();
	test_readline();

	run_cmd("/ipc_mem.bin");
	printf("\n");
	builtin_cmd("top");

	printf("\nEnd init_test.\n");
	return 0;
}
