#include "dl_syscalls.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

static struct timespec usec_to_timespec(unsigned long usec)
{
	struct timespec ts;

	ts.tv_sec = usec / 1000000;
	ts.tv_nsec = (usec % 1000000) * 1000;

	return ts;
}

static int64_t timespec_to_nsec(struct timespec *ts)
{
	return ts->tv_sec * 1E9 + ts->tv_nsec;
}

/*
 * SCHED_DEADLINE system call
 */
int sched_setattr(pid_t pid,
		const struct sched_attr *attr,
		unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
		struct sched_attr *attr,
		unsigned int size,
		unsigned int flags)
{
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

int set_sched_dl(long period, long exec)
{	
	pid_t pid;
	struct sched_attr dl_attr;
	struct timespec dl_period, dl_exec;
	int ret;
	unsigned int flags = 0;

	pid = getpid();
	dl_period = usec_to_timespec(period);
	dl_exec = usec_to_timespec(exec);
	dl_attr.size = sizeof(dl_attr);
	dl_attr.sched_flags = 0;
	dl_attr.sched_policy = SCHED_DEADLINE;
	dl_attr.sched_runtime = timespec_to_nsec(&dl_exec) + (timespec_to_nsec(&dl_exec) / 100) * 5;
	dl_attr.sched_deadline = timespec_to_nsec(&dl_period);
	dl_attr.sched_period = timespec_to_nsec(&dl_period);


	ret = sched_setattr(pid, &dl_attr, flags);
	if (ret != 0)
	{
		perror("sched_setattr");
	}

	return ret;

}

void* thread_routine(void* param)
{
	printf("loop in thread\n");
	while(1)
	{
		sleep(1);
	}
}

int main(int argc, char* argv[])
{
	int ret;
	long period, exec;
	char* token;
	pthread_t thread;
	struct rlimit limit;
	if (argc == 2)
	{
		token = strtok(argv[1], ":");
		period = strtol(token, NULL, 10);
		token = strtok(NULL, ":");
		exec = strtol(token, NULL, 10);

		ret = set_sched_dl(period, exec);

		if (ret != 0)
		{
			exit(1);
		}

		ret = getrlimit(RLIMIT_NPROC, &limit);
		if (ret != 0)
		{
			perror("getrlimit");
			exit(1);
		}
		printf("RLIMIT_NPROC current = %d, max = %d\n", (int)limit.rlim_cur, (int)limit.rlim_max);

			ret = pthread_create(&thread, NULL, thread_routine, NULL);
		if (ret != 0)
		{
			errno = ret;
			perror("pthread_create");
			exit(1);
		}
		ret = pthread_join(thread, NULL);
		if (ret != 0)
		{
			errno = ret;
			perror("pthread_join");
			exit(1);
		}

	}
	else
	{
		printf("usage: pthread_deadline <period>:<exec>\n");
	}
}
