#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

char currentDir[1024];

struct command
{
	char command1[30],
		 command2[30],
		 path1[1024],
		 path2[1024],
		 file1[1024],
		 file2[1024];
	int pipeSign,
		redir_in,
		redir_out;
};

static void handler(int sig)
{
	printf("\n-- CTRL-C --\n");
	exit(0);
}

void help();
void pwd();
void cd(const char *path);
void ls();
void cat(char *path);
void getCommand(struct command *cmd);
int indexOf(char ch, char **str, int size);
void wc(char *path);


int main(int argc, char const *argv[])
{
	pid_t pid;
	int isExit = 0;
	char path[1024];
	struct command *cmd;
	int pipeFd[2];
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		return 1;
	}

	if (pipe(pipeFd) == -1)
	{
		perror("pipe");
		return 1;
	}

	//USAGE
	if(argc != 1)
	{
		printf("USAGE: ./shell\n");
		help();
		return 1;
	}
	pwd();

	while(!isExit)
	{
		printf("%s$ ", currentDir);
		cmd = malloc(sizeof(struct command));
		getCommand(cmd);



		if(strcmp(cmd->command1,"ls") == 0)
		{
			pid = fork();
			if(pid == -1)
			{
				perror("Fork");
			}
			if(pid == 0)
			{
				ls();
				printf("dödü\n");
				exit(0);
			}
			else
				wait(NULL);
		}
		else if(strcmp(cmd->command1,"pwd") == 0)
		{
			pwd();
			printf("%s\n", currentDir);
		}
		else if(strcmp(cmd->command1,"cd") == 0)
		{
			cd(cmd->path1);
		}
		else if(strcmp(cmd->command1,"help") == 0)
		{
			printf("help\n");
			help();
		}
		else if(strcmp(cmd->command1,"cat") == 0)
		{
			printf("cat\n");
			pid = fork();
			if(pid == -1)
			{
				perror("Fork");
			}
			if(pid == 0)
			{
				cat(cmd->path1);
				exit(0);
			}
			else
				wait(NULL);
			
		}
		else if(strcmp(cmd->command1,"wc") == 0)
		{
			pid = fork();
			if(pid == -1)
			{
				perror("Fork");
			}
			if(pid == 0)
			{
				wc(cmd->path1);
				exit(0);
			}
			else
				wait(NULL);
		}
		else if(strcmp(cmd->command1,"exit") == 0)
		{
			isExit = 1;
		}
		else
		{
			help();
		}

	}

	free(cmd);

	return 0;
}

void getCommand(struct command *cmd)
{
	char *line;
	size_t size = 1024;
	size_t numRead;
	

	/* <READ LINE> */
	line = malloc(size * sizeof(char));
	numRead = getline(&line,&size,stdin);
	memcpy( line, line, numRead-1 );
	line[numRead-1] = '\0';
	/* </READ LINE> */
	
	/* <PARS LINE> */
	char **temp = malloc(size * sizeof(char*));
	char **commands = malloc(size * sizeof(char*));
	for (int i = 0; i < size; ++i)
	{
		temp[i] = malloc(size*sizeof(char));
		commands[i] = malloc(size*sizeof(char));
	}
	int count = 0, start = 0;
	for (int i = 0; i < strlen(line); ++i)
	{
		if(line[i] == ' ' || line[i] == '|' || line[i] == '>' || line[i] == '<')
		{
			start = 0;
			count++;
			temp[count][0] = line[i];
			count++;
		}
		else
		{
			temp[count][start] = line[i];
			start++;
		}
	}
	count++;

	int c = 0;
	for (int i = 0; i < count; ++i)
	{
		if(!(strlen(temp[i]) == 0 || temp[i][0] == ' '))
		{
			strcpy(commands[c],temp[i]);
			c++;
		}
	}
	/* </PARS LINE> */

	free(line);

	/* <COMMAND ANALYZE> */
	int index;
	if((index = indexOf('>',commands,c)) != -1)
	{
		cmd->redir_out = 1;
		strcpy(cmd->file1,commands[index+1]);
		
	}
	if((index = indexOf('<',commands,c)) != -1)
	{
		cmd->redir_in = 1;
		strcpy(cmd->file2,commands[index+1]);
	}
	if((index = indexOf('|',commands,c)) != -1)
	{
		if(index == 1)
		{
			strcpy(cmd->command1,commands[0]);
			cmd->pipeSign = 1;
			strcpy(cmd->command2,commands[2]);
			if(c > 3)
			{
				if(commands[4][0] != '<' && commands[4][0] != '>')
				{
					strcpy(cmd->path2,commands[3]);
				}
			}
		}
		if(index == 2)
		{
			strcpy(cmd->command1,commands[0]);
			strcpy(cmd->path1,commands[1]);
			cmd->pipeSign = 1;
			strcpy(cmd->command2,commands[3]);
			if(c > 4)
			{
				if(commands[5][0] != '<' && commands[5][0] != '>')
				{
					strcpy(cmd->path2,commands[4]);
				}
			}
		}
	}
	else
	{
		strcpy(cmd->command1,commands[0]);
		if(c > 1)
		{
			strcpy(cmd->path1,commands[1]);
		}
			
	}
	/* </COMMAND ANALYZE> */
	/*printf("-----\n");
	printf("c:%s %s \n",cmd->command1,cmd->command2);
	printf("p:%s %s \n",cmd->path1,cmd->path2);
	printf("f:%s %s \n",cmd->file1,cmd->file2);
	printf("-----\n");*/


	for (int i = 0; i < size; ++i)
	{
		free(commands[i]);
		free(temp[i]);
	}
	free(commands);
	free(temp);
}

int indexOf(char ch, char **str,int size)
{
	int i = 0;

	
	for (int i = 0; i < size; ++i)
	{
		if(str[i][0] == ch)
			return i;
	}
	
	return -1;
}

void pwd()
{
	if (getcwd(currentDir, sizeof(currentDir)) == NULL)
		perror("getcwd() error");
}

void cd(const char *path)
{
	char nextPath[1024];

	strcpy(nextPath,currentDir);
	strcat(nextPath,"/");
	strcat(nextPath,path);

	if(chdir(nextPath) == -1)
		perror("cd-error");

	strcpy(currentDir,nextPath);
}

void ls()
{
	char *args[]={"./ls",currentDir,NULL};
	execvp(args[0],args);
}

void help()
{
	printf("Supported <commands>:\n");
	printf("ls\n");
	printf("pwd\n");
	printf("cd <path>\n");
	printf("help\n");
	printf("cat <filename>\n");
	printf("wc <filename>\n");
	printf("exit\n");
}

void cat(char *path)
{
	char *args[]={"./cat",path,NULL};
	execvp(args[0],args);
}

void wc(char *path)
{
	char *args[]={"./wc",path,NULL};
	execvp(args[0],args);
}