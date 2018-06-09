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
	int numRead;
	char ch;
	char path[1024];
	struct sigaction sa;

	strcpy(path,argv[1]);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		return 1;
	}

	fd = open(path, O_RDONLY);
	if (fd == -1)
		perror("Open");

	int count = 1;
	while((numRead = read(fd, &ch, sizeof(char))) != 0)
	{
		if (numRead == -1)
			perror("read");
		
		if(ch == 10)
			count++;
	}

	printf("%d\n", count);

	close(fd);

	return 0;
}