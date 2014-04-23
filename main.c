#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>  /* waitpid */
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 255
#define MAX_ARGC 64
#define STR_SH_LINE "sh-> "

int flag_wait = 1;
char buffer[BUF_SIZE];

void int_handler(int sig)
{
	write(1,"\n", 1);
	write(1, STR_SH_LINE, sizeof(STR_SH_LINE));
	memset(buffer, '/0', BUF_SIZE);
	return;
}

void chld_handler(int sig)
{
	int status;
	pid_t w = wait(&status);
	if (w == -1)
		return;
	printf("Procces by PID = [%d] ", w);
	if (WIFEXITED(status)) 
		printf("exited, status=%d\n", WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("killed by signal %d\n", WTERMSIG(status));
	else if (WIFSTOPPED(status))
		printf("stopped by signal %d\n", WSTOPSIG(status));
	write(1, STR_SH_LINE, sizeof(STR_SH_LINE));
}

int main(int argc, char* argv[])
{	
	(void)signal(SIGINT, int_handler);
	(void)signal(SIGCHLD, chld_handler);
	write(1, "Hello! Welcome to mini shell. Enter \"q\" to exit\n", 48);
	while(1) {
		int len;
		write(1, STR_SH_LINE, sizeof(STR_SH_LINE));
		len = read(0, buffer, BUF_SIZE);
		buffer[len-1] = '\0';
		if(len == 1)
			continue;
		if (strlen(buffer) == 1 && buffer[0] == 'q')
			_exit(EXIT_SUCCESS);
		command();
	}
	_exit(EXIT_SUCCESS);
}

int command(void)
{
	char *argv[MAX_ARGC];
	char argc;
	
	char last_simbol = buffer[strlen(buffer)-1];
	if (last_simbol == '&') {
		buffer[strlen(buffer)-1] = '\0';
		flag_wait = 0;
	}
	else
		flag_wait = 1;
	
	/* start parse */
	char i = 0;
	char *pch = strtok(buffer, " ");
	while(pch) {
		argv[i] = (char *) malloc (sizeof(char) * strlen(pch));
		if (!argv[i])
			error_exit(errno);
		strcpy(argv[i++], pch);
		pch = strtok(NULL, " ");
	}
	argc = i;
	argv[argc] = 0;
	/*  end parse  */

	sigset_t sigmask;
	if ((sigemptyset(&sigmask) == -1) || (sigaddset(&sigmask, SIGINT) == -1))
			error_exit(errno);
	

	pid_t pid;
	pid = fork();

	switch(pid) {
		case -1: 
			error_exit(errno);
		case 0:
			if(!flag_wait) {
				if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1)
					error_exit(errno);
				printf("\tNew proccess with PID = [%d]\n", getpid());
			}
			execvp(argv[0], argv);
			perror("Exec error!\n");
			_exit(EXIT_FAILURE);
		default:
			if (flag_wait) {
				int status;
				(void) waitpid(pid, &status, 0);
			}
			memset(buffer, '/0', BUF_SIZE);
			sleep(1);
			break;
	}
	return EXIT_SUCCESS;
}

int error_exit(int id_error)
{
	perror("Error! Comment: ");
	perror(strerror(id_error));
	exit(id_error);
}
