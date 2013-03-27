#include <unistd.h>
#include "cfhash.h"
#include "timeutil.h"

struct item_t
{
    int64_t id;
    int64_t value;
};

#define NUM 4000000


HASH_TABLE_DEFINE(testhash, NUM, item_t, id, value)

int
main(void)
{
    struct item_t i;
    struct item_t *ptr;
    struct STORE_BUCKET_TYPE(testhash) *bucket_ptr;
    hash_type(testhash) *ht = hash_new(testhash);

    pre_timer();
    launch_timer();
    for (int nn=0; nn<NUM; nn++)
    {
        i.id = i.value = rand();

        hash_put(testhash, ht, &i);
    }
    stop_timer();
    print_timer();

    launch_timer();
    for (int nn=0; nn<NUM; nn++)
    {
        i.id = rand() % 10000;
        ptr = hash_get2(testhash, ht, &i.id, 8);
        (void)ptr;
        /*
         *if (ptr)
         *   printf("%ld %ld\n", ptr->id, ptr->value);
         */
    }
    stop_timer();
    print_timer();

    printf("sizeof = %zu, size: %u, buckets_cnt: %u, occupied_cnt: %u\n",
          sizeof(*ht), ht->size, ht->buckets_cnt, ht->occupied_cnt);
    printf("sizeof(bucket) = %zu, item = %zu, link = %zu\n", sizeof(struct STORE_BUCKET_TYPE(testhash)), sizeof(bucket_ptr->items), sizeof(bucket_ptr->links));

    return 0;
}
