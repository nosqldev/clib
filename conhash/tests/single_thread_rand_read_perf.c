#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "cfhash.h"
#include "timeutil.h"

#define NUM 1000000

struct item_t
{
    int pos;
    int value;
};

HASH_TABLE_DEFINE(intarray, NUM, item_t, pos, value);

static void
hash_reader(void)
{
    hash_item(intarray) item;
    hash_type(intarray) *array_ptr = hash_new(intarray);

    for (int i=0; i<NUM; i++)
    {
        item.pos = i;
        hash_put(intarray, array_ptr, &item);
    }
    pre_timer();
    launch_timer();
    for (int i=0; i<NUM; i++)
    {
        item.pos = rand() % NUM;
        hash_get(intarray, array_ptr, &item.pos);
    }
    stop_timer();
    print_timer();
}

static void
array_reader(void)
{
    static int array[NUM];
    static int array2[NUM];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pre_timer();
    launch_timer();
    for (int i=0; i<NUM; i++)
    {
        pthread_mutex_lock(&lock);
        int n = array[ rand() % NUM ];
        array2[i] = n;
        (void)array2[i];
        pthread_mutex_unlock(&lock);
    }
    stop_timer();
    print_timer();
}

int
main(void)
{
    hash_reader();
    array_reader();

    return 0;
}
