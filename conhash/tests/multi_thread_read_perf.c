#include "cfhash.h"
#include "timeutil.h"

#define NUM 1000000
#define THREAD_NUM 1

struct int_item_t
{
    int key;
    int value;
};

HASH_TABLE_DEFINE(inthashtable, NUM, int_item_t, key, value);

static hash_ref(inthashtable) g_int_ht = NULL;

static void
build_int_hash_table(void)
{
    hash_ref(inthashtable) ht = hash_new(inthashtable);
    hash_item(inthashtable) item;

    for (int i=0; i<NUM; i++)
    {
        item.key = i;
        item.value = i * i;
        assert(hash_put(inthashtable, ht, &item) == 0);
    }

    g_int_ht = ht;
}

static void *
read_int_hash_table(void * arg)
{
    hash_ref(inthashtable) ht = g_int_ht;
    hash_item(inthashtable) item;
    hash_iter(inthashtable) iter;

    assert(ht->occupied_cnt == NUM);
    for (int i=0; i<NUM; i++)
    {
        item.key = rand() % NUM;
        iter = hash_get(inthashtable, ht, &item.key);
        assert(iter->value == iter->key * iter->key);
    }

    return arg;
}

int
main(int argc, char **argv)
{
    int thread_num;
    pthread_t reader[THREAD_NUM];
    pre_timer();

    build_int_hash_table();

    if (argc == 1)
        thread_num = THREAD_NUM;
    else
        thread_num = atoi(argv[1]);

    printf("thread number: %d\n", thread_num);

    launch_timer();
    for (int i=0; i<thread_num; i++)
        pthread_create(&reader[i], NULL, read_int_hash_table, NULL);

    for (int i=0; i<thread_num; i++)
        pthread_join(reader[i], NULL);
    stop_timer();
    print_timer();

    return 0;
}
