#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "slab.h"
#include "rwlock.h"

struct rwlock_t *lock;

static void *
reader(void *arg)
{
    long id = (long)arg;
    struct timeval t;

    gettimeofday(&t, NULL);
    printf("reader[%ld](%ld.%06ld): begin\n", id, t.tv_sec, t.tv_usec);

    rwlock_rdlock(lock);
    gettimeofday(&t, NULL);
    printf("\nreader[%ld](%ld.%06ld): got lock\n", id, t.tv_sec, t.tv_usec);
    sleep(1);
    rwlock_unrdlock(lock);

    gettimeofday(&t, NULL);
    printf("reader[%ld](%ld.%06ld): release read lock\n", id, t.tv_sec, t.tv_usec);

    return NULL;
}

static void *
writer(void *arg)
{
    long id = (long)arg;
    struct timeval t;

    gettimeofday(&t, NULL);
    printf("writer[%ld](%ld.%06ld): begin\n", id, t.tv_sec, t.tv_usec);

    rwlock_wrlock(lock);
    gettimeofday(&t, NULL);
    printf("\nwriter[%ld](%ld.%06ld): got lock\n", id, t.tv_sec, t.tv_usec);
    sleep(1);
    rwlock_unwrlock(lock);

    gettimeofday(&t, NULL);
    printf("writer[%ld](%ld.%06ld): release write lock\n", id, t.tv_sec, t.tv_usec);

    return NULL;
}

int
main(void)
{
    pthread_t tid[1024];

    slab_init(0, 1.125);

    lock = rwlock_init(_slab_alloc);

    pthread_create(&tid[0], NULL, reader, (void*)0);
    pthread_create(&tid[1], NULL, reader, (void*)1);
    pthread_create(&tid[2], NULL, writer, (void*)2);
    pthread_create(&tid[3], NULL, writer, (void*)3);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);
    pthread_join(tid[3], NULL);

    slab_stats();

    rwlock_destory(lock, _slab_free);

    slab_stats();

    return 0;
}
