#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <limits.h>

long long int number_in_circle = 0;
long long int number_of_tosses_per_thread = -1;
int thread_count = -1; 
pthread_mutex_t mut;

int get_max_threads()
{
	int max_string_size = 10; //including null char
	FILE *fp;
	char* ret;
	int max = -1;
	char str[max_string_size];
	fp = fopen("/proc/sys/kernel/threads-max","r");
	if(NULL == fp)
	{
		printf("error opening file thread max\n");
	}
	else
	{
		ret = fgets(str, max_string_size, fp);
		if (NULL == ret)
		{
			printf("file read error\n");
		}
		else
		{
			max = atoi(str);
		}
	}
	return max;
}
double grabRand() 
{
	double div = RAND_MAX / 2;
	return -1 + (rand() / div);
}
void *monty(void* _)
{
	double distance_squared,x,y;
	long long int toss;
	long long int mycircleCount =0;
	for (toss = 0; toss < number_of_tosses_per_thread; toss++)
	{
		x = grabRand();
		y = grabRand();
		distance_squared = x*x + y*y;
		if (distance_squared <= 1)
		{
			mycircleCount++;
		}
	}
	pthread_mutex_lock(&mut);
	number_in_circle+=mycircleCount; 
	pthread_mutex_unlock(&mut);
	return NULL;
}
int main(int argc, char *argv[])
{
	int i, ret;
	int max_threads;
	int thread_count;
	double number_of_tosses;
	double pi_estimate;
	pthread_t* threads;
	srand(time(NULL)); //void return;

	if (3 != argc)
	{
		printf("I want 2 positional arguments : thread count & tosses per thread\n");
		return 1;
	}
	max_threads = get_max_threads();
	if(1 > max_threads)
	{
		printf("Are you on a posix complicant system?\n");
	}
	thread_count = atoi(argv[1]);
	if(1 > thread_count || thread_count>max_threads)
	{
		printf("Must supply an integer thread count between 1 and your system's max inclusivly\n");
		return 2;
	}
	number_of_tosses_per_thread = atoi(argv[2]);
	//billion is too much for cloudland, I tryed it and killed the program after 24 hours. Will recompile for stampede
	if(1 > number_of_tosses_per_thread || number_of_tosses_per_thread>1000000000)
	{
		printf("Supply a toss_per_thread between 1 and a billion inclusivly\n");
		return 3;
	}
	if(0!=pthread_mutex_init(&mut, NULL))
	{
		printf("mutex creation fail\n");
		return 4;
	}
	threads = malloc(thread_count * sizeof(pthread_t));
	if (NULL == threads)
	{
		printf("pthread malloc failure\n");
		return 5;
	}
	for(i=0;i<thread_count;i++)
	{
		ret = pthread_create(&threads[i], NULL, *monty, NULL);
		if(0!=ret)
		{
			printf("thread creation fail\n");
			return 6;
		}
	}
	for(i=0;i<thread_count;i++)
	{
		ret = pthread_join(threads[i],NULL);
		if(0!=ret)
		{
			printf("thread join fail\n");
			return 7;
		}
	}
	ret = pthread_mutex_destroy(&mut);	
	if(0!=ret)
	{
		printf("mutex destroy fail\n");
		return 8;
	}
	free(threads); //void return
	//printf("%llu\n",number_in_circle);
	//printf("%llu\n",thread_count*number_of_tosses_per_thread);
	number_of_tosses = thread_count*number_of_tosses_per_thread;
	pi_estimate = 4*number_in_circle/number_of_tosses;
	printf("An estimation of PI : %f (try using more (tosses*threads) for a more accurate estimation)\n",pi_estimate);
	return 0;
}