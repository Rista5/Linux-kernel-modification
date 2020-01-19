#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

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
		amma = syscall(336, getpid());
	}
	else 
	{
		int status, wpid;
		mypid = getpid();
		printf("MyPid: %d \t ChildPid: %d\n", mypid, pid);
		signal(SIGALRM, echo_msg);
		signal(SIGCHLD, process_finished);
		
		kill(pid, SIGUSR1);

		amma = syscall(336, pid);
		wpid = wait(&status);

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

		amma = syscall(336, mypid);
		printf("System call sys_signals returned %ld\n", amma);

		signal(SIGALRM, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
	}
	

	return 0;
}
