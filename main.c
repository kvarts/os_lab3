#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>  /* waitpid */
#include <signal.h>

#define OPT_NEED 1
#define OPT_UNKNW 2
#define FILE_NOT_OPEN 3
#define HASH_ERROR 4
#define PTHREAD_ERROR 5
#define MEM_ERROR 6
#define FSTAT 7
#define FORK_ERROR 8
#define WAIT_ERROR 9

#define BUFF_SIZE 255

int flag_wait = 0;
pid_t wait_process = 0;

void signal_handler(int sig)
{
	printf("f = %d, pid = %d\n", flag_wait, wait_process);
	if (flag_wait && wait_process)
		kill(wait_process, SIGKILL);
	
}


int main(int argc, char* argv[])
{	
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);
	
	char buffer[BUFF_SIZE];
	int len;
	while(1) {
		memset(buffer, '\0', BUFF_SIZE);
		terminal_hello();
		len = read(0, buffer, BUFF_SIZE);
		buffer[len-1] = '\0';
		if (is_complete(buffer))
			return EXIT_SUCCESS;
		if (is_ps(buffer)) {
			ps();
			continue;
		}
		command(buffer);
	}
	return EXIT_SUCCESS;
}

int terminal_hello(void)
{
	write(1, "my-terminal->", 13);
	return EXIT_SUCCESS;
}



int command(char *buffer, int len)
{
	char i;
	char *argv[BUFF_SIZE];
	char argc;
	char *pch;
	int status;
	
	/* start parse */
	i = 0;
	pch = strtok(buffer, " ");
	while(pch) {
		argv[i++] = pch;
		pch = strtok(NULL, " ");
	}
	argc = i;
	argv[argc] = 0;
	/*  end parse  */
	
	if (is_nowait_process(argv[argc-1])) {
		argv[argc-1] = 0;
		flag_wait = 0;
	}
	else
		flag_wait = 1;
	
	
	pid_t pid;
	pid = fork();

	switch(pid) {
		case -1: 
			error_exit(FORK_ERROR);
		case 0:
			execvp(argv[0], argv);
		default:
			//printf("wait = %d\n", flag_wait);
			if (flag_wait) {
				wait_process = pid;
				//(void) signal(SIGINT, kill_child);
				(void) waitpid(pid, &status, 0);
			}
	}
	return EXIT_SUCCESS;
}



int is_complete(char *string)
{
	if (strlen(string) != 4)
		return 0;
	return !memcmp(string, "exit", 4);
}

int is_ps(char *string)
{
	if (strlen(string) != 5)
		return 0;
	return !memcmp(string, "my-ps", 5);
}

int is_nowait_process(char *string)
{
	if (strlen(string) != 1)
		return 0;
	return !memcmp(string, "&", 1);
}

int ps(void)
{
	pid_t pid;
	char ppid[10];
	pid = fork();
	switch(pid) {
		case -1: 
			error_exit(FORK_ERROR);
		case 0:
			sprintf(ppid, "%d", getppid());
			execlp("ps", "ps", "-f", "--ppid", ppid, NULL);
		default:
			(void)waitpid(pid, NULL, 0);
	}
	return 0;
}

int error_exit(int id_error)
{
	char text_error[80];
	int i;
	for(i=0; i<80; i++)
		text_error[i]='\0';
	switch (id_error) {
		case OPT_NEED:
			strcpy(text_error, "Error! Need a value of option!\n");
			break;
		case OPT_UNKNW:
			strcpy(text_error, "Error! Unknown an option!\n");
			break;
		case FILE_NOT_OPEN:
			strcpy(text_error, "Error! File doesn`t opened!\n");
			break;
		case HASH_ERROR:
			strcpy(text_error, "Error! Hash doesn`t calculate!\n");
			break;
		case PTHREAD_ERROR:
			strcpy(text_error, "Error! Pthread doesn`t calculate!\n");
			break;
		case MEM_ERROR:
			strcpy(text_error, "Error! Something with memory... hm...\n");
			break;
		case FSTAT:
			strcpy(text_error, "Error! fstat() returned negative value!\n");
			break;
		case WAIT_ERROR:
			strcpy(text_error, "Error! Waitpid function is failes\n");
			break;	
			
	}
	write(0, text_error, 80);
	exit(id_error);
}
