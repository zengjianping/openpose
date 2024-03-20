#include "openpose/core/ctpl_stl.hpp"


namespace ctpl {

//thread_pool gThreadPool(4);

}

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#ifndef _WIN32
void set_thread_affinity(pthread_t tid, int cpu_bind) {
    int s, j;
    cpu_set_t cpuset;

    if (cpu_bind <= 0) {
        return;
    }

    // Set affinity mask to include CPUs 4 to 7
    CPU_ZERO(&cpuset);
    for (j = 0; j < 32; j++) {
		if (cpu_bind & (1<<j))
        	CPU_SET(j, &cpuset);
	}

    s = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        handle_error_en(s, "pthread_setaffinity_np failed!");

    // Check the actual affinity mask assigned to the thread
    s = pthread_getaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        handle_error_en(s, "pthread_getaffinity_np failed!");

    printf("Thread %ld run on: \n", tid);
    for (j = 0; j < CPU_SETSIZE; j++)
        if (CPU_ISSET(j, &cpuset))
            printf("    CPU %d\n", j);
}

#else

static void set_thread_affinity(pthread_t tid, int cpu_bind) {
}

#endif

void set_this_thread_affinity(int cpu_bind) {
    set_thread_affinity(pthread_self(), cpu_bind);
}

