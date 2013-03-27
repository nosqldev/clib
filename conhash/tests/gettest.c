#include "cfhash.h"

struct item_t
{
    int id;
    int value;
};

HASH_TABLE_DEFINE(gettest, 50, item_t, id, value);

int
main(void)
{
    struct item_t i, *ptr;

    hash_type(gettest) *ht = hash_build(gettest, 7);

    for (int n=0; n<15; n++)
    {
        i.id = n;
        i.value = n * n;
        assert(hash_put(gettest, ht, &i) == 0);
    }

    for (int n=0; n<20; n++)
    {
        ptr = hash_get(gettest, ht, &n);
        if (ptr)
            printf("[hash_get] %d\t%d\n", ptr->id, ptr->value);
        else
            printf("[hash_get] %d: not found\n", n);
    }
    for (int n=0; n<20; n++)
    {
        ptr = hash_get2(gettest, ht, &n, 4);
        if (ptr)
            printf("[hash_get2] %d\t%d\n", ptr->id, ptr->value);
        else
            printf("[hash_get2] %d: not found\n", n);
    }
    for (int n=0; n<20; n++)
    {
        int *int_ptr;
        int_ptr = hash_getval(gettest, ht, &n);
        if (int_ptr)
            printf("[hash_getval] %d\t%d\n", n, *int_ptr);
        else
            printf("[hash_getval] %d: not found\n", n);
    }

    printf("size: %u, buckets_cnt: %u, occupied_cnt: %u\n",
            ht->size, ht->buckets_cnt, ht->occupied_cnt);

    return 0;
}
