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
	int fd;
	int isEof = 0;
	int numRead;
	char ch;
	char path[1024];
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

	fd = open(path, O_RDONLY);
	if (fd == -1)
		perror("Open");

	while((numRead = read(fd, &ch, sizeof(char))) != 0)
	{
		if (numRead == -1)
			perror("read");
		
		printf("%c", ch);
	}

	close(fd);

	return 0;
}