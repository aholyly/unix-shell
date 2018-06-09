#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static void handler(int sig)
{
	printf("\n-- CTRL-C --\n");
}

int main(int argc, char const *argv[])
{
	struct dirent *direntp;
	DIR *dirp;
	pid_t pid;
	
	char path[1024];
	int redir_in;
	int redir_out;
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		return 1;
	}

	strcpy(path,argv[1]);

	if ((dirp = opendir(path)) != NULL)
	{
		while ((direntp = readdir(dirp)) != NULL)
		{
			printf("%s\n", direntp->d_name);
		}
	}

	exit(0);

	return 0;
}