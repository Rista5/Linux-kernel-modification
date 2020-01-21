#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define MODULE_LOCATION "/proc/sig_mod"
#define BUFF_LEN 100
#define SEC 3
// SIGALRM: 	 14
// SIGCHLD: 	 17


void echo_msg()
{
	printf("Alarm handler\n");
}

void smt()
{
	sleep(0);
	return;
}

void sleep_hand()
{
	sleep(SEC);
	exit(0);
}

void process_finished()
{
	printf("Child process has finished\n");
}

void print_signals(int pid)
{
    int fd;
    char buffer[BUFF_LEN];
    int read_count, write_count;
    int len;
    fd = open(MODULE_LOCATION, O_WRONLY);

    if(fd < 0)
    {
        printf("Error opening pid file descriptor\n");
		printf("error code : %d\n", fd);
		return;
    }

    sprintf(buffer, "%d", pid);
    len = strlen(buffer);

    write_count = write(fd, buffer, len);
    if (write_count > 0)
    {
		printf("Wrote to pid -> %s\n", buffer);
    }
    else
    {
        printf("Couldnt print to pid location\n");
        return;
    }
    close(fd);

    fd = open(MODULE_LOCATION, O_RDONLY);
    if(fd < 0)
    {
        printf("Error opening module file descriptor\n");
    }

	read_count = read(fd, buffer, BUFF_LEN);
    while(read_count)
    {
        printf("%.*s", read_count, buffer);
        printf("\n");
		read_count = read(fd, buffer, BUFF_LEN);
    }
    close(fd);
}

int main()
{
	int pid;
	int mypid;
	long int amma;
	printf("SIGALRM: \t %d\n", SIGALRM);
	printf("SIGCHLD: \t %d\n", SIGCHLD);

	pid = fork();
	if (pid == 0)
	{
		signal(SIGALRM, smt);
		signal(SIGUSR1, smt);
		alarm(1);
		pause();
		// amma = syscall(336, getpid());
	}
	else 
	{
		int status, wpid;
		mypid = getpid();
		printf("MyPid: %d \t ChildPid: %d\n", mypid, pid);
		signal(SIGALRM, echo_msg);
		signal(SIGCHLD, process_finished);
		
		kill(pid, SIGUSR1);

		// amma = syscall(336, pid);
		wpid = wait(NULL);

		printf("Before first ALARM\n");
		alarm(SEC);
		printf("First ALARM\n");
		pause();
		printf("After first ALARM\n");


		printf("Before second ALARM\n");
		alarm(SEC);
		printf("Second ALARM\n");
		pause();
		printf("After second ALARM\n");

		// amma = syscall(336, mypid);
        print_signals(mypid);

		signal(SIGALRM, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
	}

	return 0;
}
