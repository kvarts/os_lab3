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

void int_handler(int sig)
{
	write(1,"\n", 1);
	write(1, STR_SH_LINE, sizeof(STR_SH_LINE));
	return;
}

void chld_handler(int sig)
{
	
}

int main(int argc, char* argv[])
{	
	(void)signal(SIGINT, int_handler);
	(void)signal(SIGCHLD, chld_handler);
	char buffer[BUF_SIZE];
	int len;
	write(1, "Hello! Welcome to mini shell. Enter \"q\" to exit\n", 48);
	while(1) {
		memset(buffer, '\0', BUF_SIZE);
		write(1, STR_SH_LINE, sizeof(STR_SH_LINE));
		len = read(0, buffer, BUF_SIZE);
		buffer[len-1] = '\0';
		if(len == 1)
			continue;
		if (is_exit(buffer))
			_exit(EXIT_SUCCESS);
		command(buffer);
	}
	_exit(EXIT_SUCCESS);
}

int command(char *buffer, int len)
{
	char *argv[MAX_ARGC];
	char argc;
	
	char last_simbol = buffer[strlen(buffer)-1];
	if (last_simbol == '&') {
		buffer[strlen(buffer)-1] = '\0';
		flag_wait = 0;
	}
	
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
	if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1)
			error_exit(errno);

	pid_t pid;
	pid = fork();

	switch(pid) {
		case -1: 
			error_exit(errno);
		case 0:
			if(flag_wait)
				if (sigprocmask(SIG_UNBLOCK, &sigmask, NULL) == -1)
					error_exit(errno);
			printf("\tProcces forked.\n\tNew PID = [%d]\n", getpid());
			execvp(argv[0], argv);
			error_exit(errno);
		default:
			if (flag_wait) {
				int status;
				(void) waitpid(pid, &status, 0);
			}
			sleep(1);
			break;
	}
	return EXIT_SUCCESS;
}

int is_exit(char *string)
{
	if (strlen(string) != 1)
		return 0;
	return !memcmp(string, "q", 1);
}

int error_exit(int id_error)
{
	perror("Error! Comment: ");
	perror(strerror(id_error));
	exit(id_error);
}
