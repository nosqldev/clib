/*
 * +-----------------------------------------------------------------------+
 * | Concurrent-FixedSize Hashtable.                                       |
 * | Features:                                                             |
 * |   1. High performance, with concurrent reading / writing.             |
 * |   2. Designed for generic programming.                                |
 * |   3. Read priority.                                                   |
 * |   4. Open addressing.                                                 | 
 * |   5. Traverse with stored order.                                      |
 * |   6. Preallocate memory.                                              |
 * |   7. LRU replacement algorithm supported.                             |
 * |   8. Generic link list is the second usage.                           |
 * |   9. Save & Load.                                                     |
 * +-----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                              |
 * +-----------------------------------------------------------------------+
 *
 * $Id$
 */

#ifndef _CFHASH_H_
#define _CFHASH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include "rwlock.h"
#include "nshash.h"

/* HASH_TABLE_DEFINE(HashName, HashSize, ItemStruct, KeyField, ValueField) */

#define hash_build(hash_name, hash_size) DEF_HASH_BUILD(hash_name)(hash_size)
#define hash_new(hash_name) DEF_HASH_NEW(hash_name)()
#define hash_new2(hash_name, key_length, value_length) DEF_HASH_NEW2(hash_name)(key_length, value_length)
#define hash_set(hash_name, ht, key_ptr, value_ptr) DEF_HASH_SET(hash_name)(ht, key_ptr, value_ptr)
#define hash_set2(hash_name, ht, key_ptr, key_length, value_ptr, value_length) DEF_HASH_SET2(hash_name)(ht, key_ptr, key_length, value_ptr, value_length)
#define hash_put(hash_name, ht, item_ptr) DEF_HASH_PUT(hash_name)(ht, item_ptr)
#define hash_put2(hash_name, ht, item_ptr, key_length, item_length) DEF_HASH_PUT2(hash_name)(ht, item_ptr, key_length, item_length)
#define hash_get(hash_name, ht, key_ptr) DEF_HASH_GET(hash_name)(ht, key_ptr)
#define hash_get2(hash_name, ht, key_ptr, key_length) DEF_HASH_SEARCH(hash_name)(ht, key_ptr, key_length)
#define hash_getval(hash_name, ht, key_ptr) DEF_HASH_GETVAL(hash_name)(ht, key_ptr)
#define hash_del(hash_name, ht, key_ptr) DEF_HASH_DEL(hash_name)(ht, key_ptr)
#define hash_clear(hash_name, ht) DEF_HASH_CLEAR(hash_name)(ht)
#define hash_travel(hash_name, ht, traveler) DEF_HASH_TRAVEL(hash_name)(ht, traveler)
#define hash_destory(hash_name, ht) DEF_HASH_DESTORY(hash_name)(ht)

#define hash_update
#define hash_save
#define hash_load

#define hash_traveler(var_name) struct hash_traveler_t DEF_TRAVELER_VAR_NAME(var_name) = {-1, -1}; struct hash_traveler_t *var_name = &DEF_TRAVELER_VAR_NAME(var_name)
#define hash_traveler_clear(var_name) do { (var_name)->bucket_pos = -1; (var_name)->slide_pos = -1; } while (0)
#define hash_type(hash_name) struct HASH_TABLE_TYPE(hash_name)
#define hash_ref(hash_name) struct HASH_TABLE_TYPE(hash_name) *
#define hash_item(hash_name) DEF_HASH_ITEM(hash_name)
#define hash_iter(hash_name) DEF_HASH_ITER(hash_name)

#define ERR_HASH_TABLE_NOT_FOUND (NULL)
#define ERR_HASH_TABLE_FULL (-1)
#define ERR_HASH_TABLE_PARTIAL_FULL (-2)
#define ERR_HASH_TABLE_NOT_EXISTS (-3)

#define HASH_TABLE_TYPE(n) HashTable_##n##_T
#define DEF_HASH_ITEM(n) Hash_Item_##n##_T
#define DEF_HASH_ITER(n) Hash_Iter_##n##_T
#define STORE_BUCKET_TYPE(n) Store_Bucket_##n##_T
#define LINK_ITEM_TYPE(n) Link_Item_##n##_T
#define DEF_TRAVELER_VAR_NAME(var_name) __Hash_Traveler_##var_name##_ENTITY
#define DEF_HASH_NEW(n) Hash_New_##n##_Func
#define DEF_HASH_NEW2(n) Hash_New2_##n##_Func
#define DEF_HASH_SET(n) Hash_Set_##n##_Func
#define DEF_HASH_SEARCH(n) Hash_Search_##n##_Func
#define DEF_HASH_BUILD(n) Hash_Build_##n##_Func
#define DEF_HASH_PUT(n) Hash_Put_##n##_Func
#define DEF_HASH_PUT2(n) Hash_Put2_##n##_Func
#define DEF_HASH_SET2(n) Hash_Set2_##n##_Func
#define DEF_HASH_TRAVEL(n) Hash_Travel_##n##_Func
#define DEF_HASH_GET(n) Hash_Get_##n##_Func
#define DEF_HASH_GETVAL(n) Hash_Getval_##n##_Func
#define DEF_HASH_DESTORY(n) Hash_Destory_##n##_Func
#define DEF_HASH_CLEAR(n) Hash_Clear_##n##_Func
#define DEF_HASH_DEL(n) Hash_Del_##n##_Func

#define HASH_FUNCTION(buffer, size) nshash1(buffer, size)

#ifndef __offset_of
#define	__offset_of(type, field)	((size_t)(&((type *)0)->field))
#endif

#define SLIDE_CNT 3
#define LOCK_CNT 4

#define FLAG_USE_NEXT_BUCKET (0x1)
#define FLAG_FULL_BUCKET (0x2)

#define TEST_USE_NEXT_BUCKET(f) ((f) & FLAG_USE_NEXT_BUCKET)
#define TEST_FULL_BUCKET(f) ((f) & FLAG_FULL_BUCKET)
#define TEST_USAGE_MAP(map, index) (map[index] == 1)

#define SET_USE_NEXT_BUCKET(f) ((f) = (f) | FLAG_USE_NEXT_BUCKET)
#define SET_FULL_BUCKET(f) ((f) = (f) | FLAG_FULL_BUCKET)
#define SET_USAGE_MAP(map, index) (map[index] = 1)

static const uint32_t conhash_prime_list[] =
{
    0, 13, 37, 101, 1009, 10007, 50021, 100003, 200003, 300007, 400009, 500009,
    600011, 700001, 800011, 900001, 1000003, 1100009, 1200007, 1300021,
    1400017, 1500007, 1600033, 1700021, 1800017, 1900009, 2000003, 2100001,
    2200013, 2300003, 2400001, 2500009, 2600011, 2700023, 2800001, 2900017,
    3000017, 3100007, 3200003, 3300001, 3400043, 3500017, 3600001, 3700001,
    3800021, 3900067, 4000037, 4100011, 4200013, 4300003, 4400021, 4500007,
    4600003, 4700021, 4800007, 4900001, 5000011, 5100071, 5200007, 5300003,
    5400001, 5500003, 5600027, 5700007, 5800019, 5900047, 6000011, 6100001,
    6200003, 6300011, 6400013, 6500003, 6600001, 6700007, 6800033, 6900001,
    7000003, 7100003, 7200007, 7300001, 7400011, 7500013, 7600013, 7700071,
    7800017, 7900001, 8000009, 8100073, 8200007, 8300009, 8400011, 8500007,
    8600023, 8700001, 8800009, 8900029, 9000011, 9100009, 9200017, 9300001,
    9400009, 9500021, 9600037, 9700003, 9800003, 9900047
};

static inline uint32_t
find_good_size(uint32_t wish)
{
    uint32_t size;
    int left;
    int right;
    int mid;

    if (wish < conhash_prime_list[1])
        return conhash_prime_list[1];
    else if (wish > conhash_prime_list[ sizeof(conhash_prime_list)/sizeof(conhash_prime_list[0])-1 ])
        return wish;
    left = 0;
    right = sizeof(conhash_prime_list) / sizeof(conhash_prime_list[0]) - 1;
    for (; ; )
    {
        mid = (left + right) / 2;
        assert(mid-1 > 0);
        assert((size_t)mid < sizeof(conhash_prime_list)/sizeof(conhash_prime_list[0]));
        if ((conhash_prime_list[mid-1] < wish) && (conhash_prime_list[mid] > wish))
        {
            size = conhash_prime_list[mid];
            break;
        }
        else if (conhash_prime_list[mid] < wish)
        {
            left = mid + 1;
        }
        else if (conhash_prime_list[mid] > wish) 
        {
            right = mid - 1;
        }
    }

    return size;
}

static inline uint32_t
Murmur_Hash2(const void *key, size_t len_if)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    const unsigned seed = 0xfeedbeef;
    uint32_t len = (uint32_t)len_if;
    unsigned int h = seed ^ len;

    const unsigned char * data = (const unsigned char *)key;

    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;
        k *= m; 
        k ^= k >> r; 
        k *= m; 
        h *= m; 
        h ^= k;
        data += 4;
        len -= 4;
    }

    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (uint32_t)h;
}

#ifndef _HASH_TRAVELER_T_
#define _HASH_TRAVELER_T_
struct hash_traveler_t
{
    int32_t bucket_pos;
    int16_t slide_pos;
};
#endif /* ! _HASH_TRAVELER_T_ */

#define HASH_TABLE_DEFINE_DATA_STRUCTURE(HashName, ItemStruct)                      \
    typedef struct ItemStruct DEF_HASH_ITEM(HashName);                              \
    typedef struct ItemStruct* DEF_HASH_ITER(HashName);                             \
    struct LINK_ITEM_TYPE(HashName)                                                 \
    {                                                                               \
        int32_t prev_bucket_pos;                                                    \
        int32_t next_bucket_pos;                                                    \
        int16_t prev_slide_pos;                                                     \
        int16_t next_slide_pos;                                                     \
    };                                                                              \
                                                                                    \
    struct STORE_BUCKET_TYPE(HashName)                                              \
    {                                                                               \
        uint32_t used_cnt;                                                          \
        uint8_t flag;                                                               \
        uint8_t usage_map[SLIDE_CNT];                                               \
        struct ItemStruct items[SLIDE_CNT];                                         \
        struct LINK_ITEM_TYPE(HashName) links[SLIDE_CNT];                           \
    };                                                                              \
                                                                                    \
    struct HASH_TABLE_TYPE(HashName)                                                \
    {                                                                               \
        uint32_t size;                      /* predicted size of hash table */      \
        uint32_t buckets_cnt;               /* count of buckets */                  \
        uint32_t occupied_cnt;              /* count of item occupied */            \
        uint16_t key_pos;                   /* offset of key in user defined structure */                       \
        uint16_t value_pos;                 /* offset of value in user defined structure */                     \
        uint32_t key_length;                /* length of key buffer, not used now */                            \
        uint32_t value_length;              /* length of value buffer, not used now */                          \
        uint32_t item_size;                 /* size of item, not used now */                                    \
        int16_t head_slide_pos;             /* pos of hashtable's head item in specified bucket */              \
        int16_t tail_slide_pos;             /* pos of hashtable's tail item in specified bucket */              \
        int32_t head_bucket_pos;            /* bucket of hashtable's head item stored in */                     \
        int32_t tail_bucket_pos;            /* bucket of hashtable's tail item stored in */                     \
        pthread_spinlock_t global_lock;     /* global lock of hash table */                                     \
        struct rwlock_t lock[LOCK_CNT];     /* read-write lock of bucket partition */                           \
        struct STORE_BUCKET_TYPE(HashName) buckets[];    /* array of bucket */                                  \
    };


#define HASH_TABLE_DEFINE(HashName, HashSize, ItemStruct, KeyField, ValueField)     \
    typedef struct ItemStruct DEF_HASH_ITEM(HashName);                              \
    typedef struct ItemStruct* DEF_HASH_ITER(HashName);                             \
    struct LINK_ITEM_TYPE(HashName)                                                 \
    {                                                                               \
        int32_t prev_bucket_pos;                                                    \
        int32_t next_bucket_pos;                                                    \
        int16_t prev_slide_pos;                                                     \
        int16_t next_slide_pos;                                                     \
    };                                                                              \
                                                                                    \
    struct STORE_BUCKET_TYPE(HashName)                                              \
    {                                                                               \
        uint32_t used_cnt;                                                          \
        uint8_t flag;                                                               \
        uint8_t usage_map[SLIDE_CNT];                                               \
        struct ItemStruct items[SLIDE_CNT];                                         \
        struct LINK_ITEM_TYPE(HashName) links[SLIDE_CNT];                           \
    };                                                                              \
                                                                                    \
    struct HASH_TABLE_TYPE(HashName)                                                \
    {                                                                               \
        uint32_t size;                      /* predicted size of hash table */      \
        uint32_t buckets_cnt;               /* count of buckets */                  \
        uint32_t occupied_cnt;              /* count of item occupied */            \
        uint16_t key_pos;                   /* offset of key in user defined structure */                       \
        uint16_t value_pos;                 /* offset of value in user defined structure */                     \
        uint32_t key_length;                /* length of key buffer, not used now */                            \
        uint32_t value_length;              /* length of value buffer, not used now */                          \
        uint32_t item_size;                 /* size of item, not used now */                                    \
        int16_t head_slide_pos;             /* pos of hashtable's head item in specified bucket */              \
        int16_t tail_slide_pos;             /* pos of hashtable's tail item in specified bucket */              \
        int32_t head_bucket_pos;            /* bucket of hashtable's head item stored in */                     \
        int32_t tail_bucket_pos;            /* bucket of hashtable's tail item stored in */                     \
        pthread_spinlock_t global_lock;     /* global lock of hash table */                                     \
        struct rwlock_t lock[LOCK_CNT];     /* read-write lock of bucket partition */                           \
        struct STORE_BUCKET_TYPE(HashName) buckets[];    /* array of bucket */                                  \
    };                                                                                                          \
    static inline int                                                                                   \
    Move_Item_To_Tail_##HashName(hash_type(HashName) *ht, size_t bucket_pos, size_t slide_pos)          \
    {                                                                                                   \
        struct STORE_BUCKET_TYPE(HashName) *prev_bucket_tmp, *next_bucket_tmp, *cur_bucket_tmp;         \
        uint16_t prev_slide_tmp, next_slide_tmp;                                                        \
                                                                                                        \
        if ((bucket_pos == (size_t)ht->tail_bucket_pos) && (slide_pos == (size_t)ht->tail_slide_pos))   \
        {                                                                                               \
            /* if the item is the tail one now, do nothing */                                           \
            goto end;                                                                                   \
        }                                                                                               \
        cur_bucket_tmp = &ht->buckets[bucket_pos];                                                      \
        if (cur_bucket_tmp->links[slide_pos].prev_bucket_pos >= 0)                                      \
        {                                                                                               \
            prev_bucket_tmp = &ht->buckets[ cur_bucket_tmp->links[slide_pos].prev_bucket_pos ];         \
            prev_slide_tmp  = cur_bucket_tmp->links[slide_pos].prev_slide_pos;                          \
            prev_bucket_tmp->links[prev_slide_tmp].next_bucket_pos = cur_bucket_tmp->links[slide_pos].next_bucket_pos;  \
            prev_bucket_tmp->links[prev_slide_tmp].next_slide_pos  = cur_bucket_tmp->links[slide_pos].next_slide_pos;   \
        }                                                                                                               \
        else                                                                                            \
            prev_bucket_tmp = NULL;                                                                     \
        if (cur_bucket_tmp->links[slide_pos].next_bucket_pos >= 0)                                      \
        {                                                                                               \
            next_bucket_tmp = &ht->buckets[ cur_bucket_tmp->links[slide_pos].next_bucket_pos ];         \
            next_slide_tmp  = cur_bucket_tmp->links[slide_pos].next_slide_pos;                          \
            next_bucket_tmp->links[next_slide_tmp].prev_bucket_pos = cur_bucket_tmp->links[slide_pos].prev_bucket_pos;  \
            next_bucket_tmp->links[next_slide_tmp].prev_slide_pos  = cur_bucket_tmp->links[slide_pos].prev_slide_pos;   \
        }                                                                                               \
        else                                                                                            \
            next_bucket_tmp = NULL;                                                                     \
        if ((ht->tail_bucket_pos >= 0) && (ht->tail_slide_pos >= 0))                                    \
        {                                                                                               \
            ht->buckets[ ht->tail_bucket_pos ].links[ ht->tail_slide_pos ].next_bucket_pos = bucket_pos;\
            ht->buckets[ ht->tail_bucket_pos ].links[ ht->tail_slide_pos ].next_slide_pos = slide_pos;  \
            cur_bucket_tmp->links[slide_pos].prev_bucket_pos = ht->tail_bucket_pos;                     \
            cur_bucket_tmp->links[slide_pos].prev_slide_pos  = ht->tail_slide_pos;                      \
        }                                                                                               \
        ht->tail_bucket_pos = bucket_pos;                                                               \
        ht->tail_slide_pos  = slide_pos;                                                                \
        cur_bucket_tmp->links[slide_pos].next_bucket_pos = -1;                                          \
        cur_bucket_tmp->links[slide_pos].next_slide_pos = -1;                                           \
                                                                                                        \
    end:                                                                                                \
        return 0;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    struct HASH_TABLE_TYPE(HashName) *                                              \
    DEF_HASH_NEW2(HashName)(size_t key_length, size_t value_length)                 \
    {                                                                               \
        struct HASH_TABLE_TYPE(HashName) *ht;                                       \
        size_t buckets_cnt;                                                         \
                                                                                    \
        buckets_cnt = find_good_size(ceil(HashSize / SLIDE_CNT * 2));               \
        ht = malloc(sizeof(struct HASH_TABLE_TYPE(HashName)) + buckets_cnt*sizeof(struct STORE_BUCKET_TYPE(HashName)));     \
        ht->size = buckets_cnt * SLIDE_CNT;                                         \
        ht->buckets_cnt = buckets_cnt;                                              \
        ht->occupied_cnt = 0;                                                       \
        ht->key_pos = __offset_of(struct ItemStruct, KeyField);                     \
        ht->key_length = key_length;                                                \
        ht->value_pos = __offset_of(struct ItemStruct, ValueField);                 \
        ht->value_length = value_length;                                            \
        ht->item_size = sizeof(struct ItemStruct);                                  \
        ht->head_slide_pos = -1;                                                    \
        ht->tail_slide_pos = -1;                                                    \
        ht->head_bucket_pos = -1;                                                   \
        ht->tail_bucket_pos = -1;                                                   \
        pthread_spin_init(&ht->global_lock, PTHREAD_PROCESS_PRIVATE);               \
        for (int i=0; i<LOCK_CNT; i++)                                              \
        {                                                                           \
            rwlock_build(&ht->lock[i]);                                             \
        }                                                                           \
        memset(ht->buckets, 0, buckets_cnt*sizeof(struct STORE_BUCKET_TYPE(HashName)));         \
                                                                                    \
        return ht;                                                                  \
    }                                                                               \
                                                                                    \
    int                                                                             \
    DEF_HASH_SET(HashName)(struct HASH_TABLE_TYPE(HashName) *ht, void *key_ptr, void *value_ptr)    \
    {                                                                               \
        uint32_t hashcode;                                                          \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        int ret;                                                                    \
        bool wrapped;                                                               \
        bool only_update;                                                           \
                                                                                    \
        ret = 0;                                                                    \
        wrapped = false;                                                            \
        only_update = false;                                                        \
        pthread_spin_lock(&ht->global_lock);                                        \
        if (ht->occupied_cnt + 1 > ht->size)                                        \
        {                                                                           \
            ret = ERR_HASH_TABLE_FULL;              /* oops! table is full */       \
            pthread_spin_unlock(&ht->global_lock);                                  \
            goto end;                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            pthread_spin_unlock(&ht->global_lock);                                  \
        }                                                                           \
                                                                                    \
        hashcode = HASH_FUNCTION(key_ptr, sizeof(ht->buckets[0].items[0].KeyField));\
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_wrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        while (TEST_FULL_BUCKET(ht->buckets[bucket_pos].flag))  /* find available bucket */     \
        {                                                                           \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                /*+---------------------------------------------------------+       \
                 *| Here needn't to check usage_map, because all the slides |       \
                 *| in this bucket are used.                                |       \
                 *+---------------------------------------------------------+*/     \
                if (memcmp(key_ptr, &ht->buckets[bucket_pos].items[slide_pos].KeyField, sizeof(ht->buckets[0].items[0].KeyField)) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
            SET_USE_NEXT_BUCKET(ht->buckets[bucket_pos].flag);                      \
            /* try next bucket, step = number of locks */                           \
            bucket_pos += LOCK_CNT;                                                 \
            if (bucket_pos >= ht->buckets_cnt)                                      \
            {                                                                       \
                /* wrap to bucket head */                                           \
                bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;                 \
                wrapped = true;                                                     \
            }                                                                       \
            if (wrapped)                                                            \
            {                                                                       \
                if (bucket_pos == hashcode % ht->buckets_cnt)                       \
                {                                                                   \
                    /* free bucket exhausted in this partition */                   \
                    ret = ERR_HASH_TABLE_PARTIAL_FULL;                              \
                    rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);            \
                    goto end;                                                       \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        ssize_t tmp_slide_pos = -1;                                                 \
        for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)     /* travel the slides to find available slide */ \
        {                                                                           \
            if (!TEST_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos))      \
            {                                                                       \
                if (tmp_slide_pos < 0)                                              \
                    tmp_slide_pos = slide_pos;   /* save first available slide pos if nothing was found later */    \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                if (memcmp(key_ptr, &ht->buckets[bucket_pos].items[slide_pos].KeyField, sizeof(ht->buckets[0].items[0].KeyField)) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        if (!only_update)                                                           \
            slide_pos = tmp_slide_pos;                                              \
    found:                                                                          \
        assert(slide_pos < SLIDE_CNT);                                              \
        memcpy(&(ht->buckets[bucket_pos].items[slide_pos].KeyField), key_ptr, sizeof(ht->buckets[0].items[0].KeyField));        \
        memcpy(&(ht->buckets[bucket_pos].items[slide_pos].ValueField), value_ptr, sizeof(ht->buckets[0].items[0].ValueField));  \
        if (!only_update)                                                           \
        {                                                                           \
            ht->buckets[bucket_pos].used_cnt ++;                                    \
            SET_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos);            \
            if (slide_pos == SLIDE_CNT-1)                                           \
            {                                                                       \
                SET_FULL_BUCKET(ht->buckets[bucket_pos].flag); /* set full flag */  \
            }                                                                       \
        }                                                                           \
                                                                                    \
        /* NOTE: Must lock before rwlock_unwrlock to prevent lock contention with hash_clear() / hash_del() */  \
        pthread_spin_lock(&ht->global_lock);                                                                    \
        rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                                                    \
                                                                                                                \
        if (only_update)                                                                                        \
        {                                                                                                       \
            /* need to update the link order if update only */                                                  \
            Move_Item_To_Tail_##HashName(ht, bucket_pos, slide_pos);                                            \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            ht->occupied_cnt ++;                                                                                \
            ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos = ht->tail_bucket_pos;                     \
            ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos = ht->tail_slide_pos;                       \
            ht->buckets[bucket_pos].links[slide_pos].next_bucket_pos = -1;                                      \
            ht->buckets[bucket_pos].links[slide_pos].next_slide_pos = -1;                                       \
            if (ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos >= 0)                                  \
            {                                                                                                   \
                struct STORE_BUCKET_TYPE(HashName) *prev_bucket_tmp;                                            \
                uint16_t prev_slide_tmp;                                                                        \
                                                                                                                \
                prev_bucket_tmp = &ht->buckets[ ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos ];     \
                prev_slide_tmp = ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos;                       \
                prev_bucket_tmp->links[prev_slide_tmp].next_bucket_pos = bucket_pos;                            \
                prev_bucket_tmp->links[prev_slide_tmp].next_slide_pos = slide_pos;                              \
            }                                                                       \
            if (ht->head_bucket_pos < 0)                                            \
            {                                                                       \
                /* this is the first add operation */                               \
                ht->head_bucket_pos = bucket_pos;                                   \
                ht->head_slide_pos = slide_pos;                                     \
            }                                                                       \
            ht->tail_bucket_pos = bucket_pos;                                       \
            ht->tail_slide_pos = slide_pos;                                         \
        }                                                                           \
        pthread_spin_unlock(&ht->global_lock);                                      \
                                                                                    \
    end:                                                                            \
        return ret;                                                                 \
    }                                                                               \
    struct HASH_TABLE_TYPE(HashName) *                                              \
    DEF_HASH_NEW(HashName)()                                                        \
    {                                                                               \
        struct ItemStruct *item = NULL;                                             \
        return DEF_HASH_NEW2(HashName)(sizeof(item->KeyField), sizeof(item->ValueField));    \
    }                                                                               \
    int                                                                             \
    DEF_HASH_PUT(HashName)(struct HASH_TABLE_TYPE(HashName) *ht, struct ItemStruct *item_ptr) \
    {                                                                               \
        uint32_t hashcode;                                                          \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        int ret;                                                                    \
        bool wrapped;                                                               \
        bool only_update;                                                           \
                                                                                    \
        ret = 0;                                                                    \
        wrapped = false;                                                            \
        only_update = false;                                                        \
        pthread_spin_lock(&ht->global_lock);                                        \
        if (ht->occupied_cnt + 1 > ht->size)                                        \
        {                                                                           \
            ret = ERR_HASH_TABLE_FULL;              /* oops! table is full */       \
            pthread_spin_unlock(&ht->global_lock);                                  \
            goto end;                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            pthread_spin_unlock(&ht->global_lock);                                  \
        }                                                                           \
                                                                                    \
        hashcode = HASH_FUNCTION(&item_ptr->KeyField, sizeof(item_ptr->KeyField));  \
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_wrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        while (TEST_FULL_BUCKET(ht->buckets[bucket_pos].flag))  /* find available bucket */     \
        {                                                                           \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                /*+---------------------------------------------------------+       \
                 *| Here needn't to check usage_map, because all the slides |       \
                 *| in this bucket are used.                                |       \
                 *+---------------------------------------------------------+*/     \
                if (memcmp(&item_ptr->KeyField, &ht->buckets[bucket_pos].items[slide_pos].KeyField, sizeof(item_ptr->KeyField)) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
            SET_USE_NEXT_BUCKET(ht->buckets[bucket_pos].flag);                      \
            /* try next bucket, step = number of locks */                           \
            bucket_pos += LOCK_CNT;                                                 \
            if (bucket_pos >= ht->buckets_cnt)                                      \
            {                                                                       \
                /* wrap to bucket head */                                           \
                bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;                 \
                wrapped = true;                                                     \
            }                                                                       \
            if (wrapped)                                                            \
            {                                                                       \
                if (bucket_pos == hashcode % ht->buckets_cnt)                       \
                {                                                                   \
                    /* free bucket exhausted in this partition */                   \
                    ret = ERR_HASH_TABLE_PARTIAL_FULL;                              \
                    rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);            \
                    goto end;                                                       \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        ssize_t tmp_slide_pos = -1;                                                 \
        for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)     /* travel the slides to find available slide */ \
        {                                                                           \
            if (!TEST_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos))      \
            {                                                                       \
                if (tmp_slide_pos < 0)                                              \
                    tmp_slide_pos = slide_pos;   /* save first available slide pos if nothing was found later */    \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                if (memcmp(&item_ptr->KeyField, &ht->buckets[bucket_pos].items[slide_pos].KeyField, sizeof(item_ptr->KeyField)) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        if (!only_update)                                                           \
            slide_pos = tmp_slide_pos;                                              \
    found:                                                                          \
        assert(slide_pos < SLIDE_CNT);                                              \
        memcpy(&(ht->buckets[bucket_pos].items[slide_pos]), item_ptr, sizeof(*item_ptr));                       \
        if (!only_update)                                                           \
        {                                                                           \
            ht->buckets[bucket_pos].used_cnt ++;                                    \
            SET_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos);            \
            if (slide_pos == SLIDE_CNT-1)                                           \
            {                                                                       \
                SET_FULL_BUCKET(ht->buckets[bucket_pos].flag); /* set full flag */  \
            }                                                                       \
        }                                                                           \
        /* NOTE: Must lock before rwlock_unwrlock to prevent lock contention with hash_clear() / hash_del() */  \
        pthread_spin_lock(&ht->global_lock);                                                                    \
        rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                                                    \
                                                                                                                \
        if (only_update)                                                                                        \
        {                                                                                                       \
            /* need to update the link order if update only */                                                  \
            Move_Item_To_Tail_##HashName(ht, bucket_pos, slide_pos);                                            \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            ht->occupied_cnt ++;                                                                                \
            ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos = ht->tail_bucket_pos;                     \
            ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos = ht->tail_slide_pos;                       \
            ht->buckets[bucket_pos].links[slide_pos].next_bucket_pos = -1;                                      \
            ht->buckets[bucket_pos].links[slide_pos].next_slide_pos = -1;                                       \
            if (ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos >= 0)                                  \
            {                                                                                                   \
                struct STORE_BUCKET_TYPE(HashName) *prev_bucket_tmp;                                            \
                uint16_t prev_slide_tmp;                                                                        \
                                                                                                                \
                prev_bucket_tmp = &ht->buckets[ ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos ];     \
                prev_slide_tmp = ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos;                       \
                prev_bucket_tmp->links[prev_slide_tmp].next_bucket_pos = bucket_pos;                            \
                prev_bucket_tmp->links[prev_slide_tmp].next_slide_pos = slide_pos;                              \
            }                                                                       \
            if (ht->head_bucket_pos < 0)                                            \
            {                                                                       \
                /* this is the first add operation */                               \
                ht->head_bucket_pos = bucket_pos;                                   \
                ht->head_slide_pos = slide_pos;                                     \
            }                                                                       \
            ht->tail_bucket_pos = bucket_pos;                                       \
            ht->tail_slide_pos = slide_pos;                                         \
        }                                                                           \
        pthread_spin_unlock(&ht->global_lock);                                      \
                                                                                    \
    end:                                                                            \
        return ret;                                                                 \
    }                                                                               \
    int                                                                             \
    DEF_HASH_PUT2(HashName)(struct HASH_TABLE_TYPE(HashName) *ht, struct ItemStruct *item_ptr, size_t key_length, size_t item_length) \
    {                                                                               \
        uint32_t hashcode;                                                          \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        int ret;                                                                    \
        bool wrapped;                                                               \
        bool only_update;                                                           \
                                                                                    \
        ret = 0;                                                                    \
        wrapped = false;                                                            \
        only_update = false;                                                        \
        pthread_spin_lock(&ht->global_lock);                                        \
        if (ht->occupied_cnt + 1 > ht->size)                                        \
        {                                                                           \
            ret = ERR_HASH_TABLE_FULL;              /* oops! table is full */       \
            pthread_spin_unlock(&ht->global_lock);                                  \
            goto end;                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            pthread_spin_unlock(&ht->global_lock);                                  \
        }                                                                           \
                                                                                    \
        hashcode = HASH_FUNCTION(&item_ptr->KeyField, key_length);                  \
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_wrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        while (TEST_FULL_BUCKET(ht->buckets[bucket_pos].flag))  /* find available bucket */     \
        {                                                                           \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                /*+---------------------------------------------------------+       \
                 *| Here needn't to check usage_map, because all the slides |       \
                 *| in this bucket are used.                                |       \
                 *+---------------------------------------------------------+*/     \
                if (memcmp(&item_ptr->KeyField, &ht->buckets[bucket_pos].items[slide_pos].KeyField, key_length) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
            SET_USE_NEXT_BUCKET(ht->buckets[bucket_pos].flag);                      \
            /* try next bucket, step = number of locks */                           \
            bucket_pos += LOCK_CNT;                                                 \
            if (bucket_pos >= ht->buckets_cnt)                                      \
            {                                                                       \
                /* wrap to bucket head */                                           \
                bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;                 \
                wrapped = true;                                                     \
            }                                                                       \
            if (wrapped)                                                            \
            {                                                                       \
                if (bucket_pos == hashcode % ht->buckets_cnt)                       \
                {                                                                   \
                    /* free bucket exhausted in this partition */                   \
                    ret = ERR_HASH_TABLE_PARTIAL_FULL;                              \
                    rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);            \
                    goto end;                                                       \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        ssize_t tmp_slide_pos = -1;                                                 \
        for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)     /* travel the slides to find available slide */ \
        {                                                                           \
            if (!TEST_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos))      \
            {                                                                       \
                if (tmp_slide_pos < 0)                                              \
                    tmp_slide_pos = slide_pos;   /* save first available slide pos if nothing was found later */    \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                if (memcmp(&item_ptr->KeyField, &ht->buckets[bucket_pos].items[slide_pos].KeyField, key_length) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        if (!only_update)                                                           \
            slide_pos = tmp_slide_pos;                                              \
    found:                                                                          \
        assert(slide_pos < SLIDE_CNT);                                              \
        memcpy(&(ht->buckets[bucket_pos].items[slide_pos]), item_ptr, item_length); \
        if (!only_update)                                                           \
        {                                                                           \
            ht->buckets[bucket_pos].used_cnt ++;                                    \
            SET_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos);            \
            if (slide_pos == SLIDE_CNT-1)                                           \
            {                                                                       \
                SET_FULL_BUCKET(ht->buckets[bucket_pos].flag); /* set full flag */  \
            }                                                                       \
        }                                                                           \
        /* NOTE: Must lock before rwlock_unwrlock to prevent lock contention with hash_clear() / hash_del() */  \
        pthread_spin_lock(&ht->global_lock);                                                                    \
        rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                                                    \
                                                                                                                \
        if (only_update)                                                                                        \
        {                                                                                                       \
            /* need to update the link order if update only */                                                  \
            Move_Item_To_Tail_##HashName(ht, bucket_pos, slide_pos);                                            \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            ht->occupied_cnt ++;                                                                                \
            ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos = ht->tail_bucket_pos;                     \
            ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos = ht->tail_slide_pos;                       \
            ht->buckets[bucket_pos].links[slide_pos].next_bucket_pos = -1;                                      \
            ht->buckets[bucket_pos].links[slide_pos].next_slide_pos = -1;                                       \
            if (ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos >= 0)                                  \
            {                                                                                                   \
                struct STORE_BUCKET_TYPE(HashName) *prev_bucket_tmp;                                            \
                uint16_t prev_slide_tmp;                                                                        \
                                                                                                                \
                prev_bucket_tmp = &ht->buckets[ ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos ];     \
                prev_slide_tmp = ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos;                       \
                prev_bucket_tmp->links[prev_slide_tmp].next_bucket_pos = bucket_pos;                            \
                prev_bucket_tmp->links[prev_slide_tmp].next_slide_pos = slide_pos;                              \
            }                                                                       \
            if (ht->head_bucket_pos < 0)                                            \
            {                                                                       \
                /* this is the first add operation */                               \
                ht->head_bucket_pos = bucket_pos;                                   \
                ht->head_slide_pos = slide_pos;                                     \
            }                                                                       \
            ht->tail_bucket_pos = bucket_pos;                                       \
            ht->tail_slide_pos = slide_pos;                                         \
        }                                                                           \
        pthread_spin_unlock(&ht->global_lock);                                      \
                                                                                    \
    end:                                                                            \
        return ret;                                                                 \
    }                                                                               \
    int                                                                             \
    DEF_HASH_SET2(HashName)(struct HASH_TABLE_TYPE(HashName) *ht, void *key_ptr, size_t key_length, void *value_ptr, size_t value_length) \
    {                                                                               \
        uint32_t hashcode;                                                          \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        int ret;                                                                    \
        bool wrapped;                                                               \
        bool only_update;                                                           \
                                                                                    \
        ret = 0;                                                                    \
        wrapped = false;                                                            \
        only_update = false;                                                        \
        pthread_spin_lock(&ht->global_lock);                                        \
        if (ht->occupied_cnt + 1 > ht->size)                                        \
        {                                                                           \
            ret = ERR_HASH_TABLE_FULL;              /* oops! table is full */       \
            pthread_spin_unlock(&ht->global_lock);                                  \
            goto end;                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            pthread_spin_unlock(&ht->global_lock);                                  \
        }                                                                           \
                                                                                    \
        hashcode = HASH_FUNCTION(key_ptr, key_length);                              \
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_wrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        while (TEST_FULL_BUCKET(ht->buckets[bucket_pos].flag))  /* find available bucket */     \
        {                                                                           \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                /*+---------------------------------------------------------+       \
                 *| Here needn't to check usage_map, because all the slides |       \
                 *| in this bucket are used.                                |       \
                 *+---------------------------------------------------------+*/     \
                if (memcmp(key_ptr, ((void*)&ht->buckets[bucket_pos].items[slide_pos])+ht->key_pos, key_length) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
            SET_USE_NEXT_BUCKET(ht->buckets[bucket_pos].flag);                      \
            /* try next bucket, step = number of locks */                           \
            bucket_pos += LOCK_CNT;                                                 \
            if (bucket_pos >= ht->buckets_cnt)                                      \
            {                                                                       \
                /* wrap to bucket head */                                           \
                bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;                 \
                wrapped = true;                                                     \
            }                                                                       \
            if (wrapped)                                                            \
            {                                                                       \
                if (bucket_pos == hashcode % ht->buckets_cnt)                       \
                {                                                                   \
                    /* free bucket exhausted in this partition */                   \
                    ret = ERR_HASH_TABLE_PARTIAL_FULL;                              \
                    rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);            \
                    goto end;                                                       \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        ssize_t tmp_slide_pos = -1;                                                 \
        for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)     /* travel the slides to find available slide */ \
        {                                                                           \
            if (!TEST_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos))      \
            {                                                                       \
                if (tmp_slide_pos < 0)                                              \
                    tmp_slide_pos = slide_pos;   /* save first available slide pos if nothing was found later */    \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                if (memcmp(key_ptr, ((void*)&ht->buckets[bucket_pos].items[slide_pos])+ht->key_pos, key_length) == 0) \
                {                                                                   \
                    only_update = true;                                             \
                    goto found;                                                     \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        if (!only_update)                                                           \
            slide_pos = tmp_slide_pos;                                              \
    found:                                                                          \
        assert(slide_pos < SLIDE_CNT);                                              \
        memcpy(((void*)&(ht->buckets[bucket_pos].items[slide_pos]))+ht->key_pos, key_ptr, key_length);        \
        memcpy(((void*)&(ht->buckets[bucket_pos].items[slide_pos]))+ht->value_pos, value_ptr, value_length);  \
        if (!only_update)                                                           \
        {                                                                           \
            ht->buckets[bucket_pos].used_cnt ++;                                    \
            SET_USAGE_MAP(ht->buckets[bucket_pos].usage_map, slide_pos);            \
            if (slide_pos == SLIDE_CNT-1)                                           \
            {                                                                       \
                SET_FULL_BUCKET(ht->buckets[bucket_pos].flag); /* set full flag */  \
            }                                                                       \
        }                                                                           \
        /* NOTE: Must lock before rwlock_unwrlock to prevent lock contention with hash_clear() / hash_del() */  \
        pthread_spin_lock(&ht->global_lock);                                                                    \
        rwlock_unwrlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                                                    \
                                                                                                                \
        if (only_update)                                                                                        \
        {                                                                                                       \
            /* need to update the link order if update only */                                                  \
            Move_Item_To_Tail_##HashName(ht, bucket_pos, slide_pos);                                            \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            ht->occupied_cnt ++;                                                                                \
            ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos = ht->tail_bucket_pos;                     \
            ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos = ht->tail_slide_pos;                       \
            ht->buckets[bucket_pos].links[slide_pos].next_bucket_pos = -1;                                      \
            ht->buckets[bucket_pos].links[slide_pos].next_slide_pos = -1;                                       \
            if (ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos >= 0)                                  \
            {                                                                                                   \
                struct STORE_BUCKET_TYPE(HashName) *prev_bucket_tmp;                                            \
                uint16_t prev_slide_tmp;                                                                        \
                                                                                                                \
                prev_bucket_tmp = &ht->buckets[ ht->buckets[bucket_pos].links[slide_pos].prev_bucket_pos ];     \
                prev_slide_tmp = ht->buckets[bucket_pos].links[slide_pos].prev_slide_pos;                       \
                prev_bucket_tmp->links[prev_slide_tmp].next_bucket_pos = bucket_pos;                            \
                prev_bucket_tmp->links[prev_slide_tmp].next_slide_pos = slide_pos;                              \
            }                                                                       \
            if (ht->head_bucket_pos < 0)                                            \
            {                                                                       \
                /* this is the first add operation */                               \
                ht->head_bucket_pos = bucket_pos;                                   \
                ht->head_slide_pos = slide_pos;                                     \
            }                                                                       \
            ht->tail_bucket_pos = bucket_pos;                                       \
            ht->tail_slide_pos = slide_pos;                                         \
        }                                                                           \
        pthread_spin_unlock(&ht->global_lock);                                      \
                                                                                    \
    end:                                                                            \
        return ret;                                                                 \
    }                                                                               \
    struct ItemStruct *                                                             \
    DEF_HASH_SEARCH(HashName)(struct HASH_TABLE_TYPE(HashName) *ht, void *key_ptr, size_t key_length)   \
    {                                                                               \
        uint32_t hashcode;                                                          \
        struct ItemStruct *item_ptr;                                                \
        struct STORE_BUCKET_TYPE(HashName) *bucket_ptr;                             \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        bool wrapped;                                                               \
        bool found;                                                                 \
                                                                                    \
        wrapped = false;                                                            \
        found = false;                                                              \
                                                                                    \
        hashcode = HASH_FUNCTION(key_ptr, key_length);                              \
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_rdlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        for (; ;)                                                                   \
        {                                                                           \
            bucket_ptr = &ht->buckets[bucket_pos];                                  \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                if (!TEST_USAGE_MAP(bucket_ptr->usage_map, slide_pos))              \
                    continue;                                                       \
                item_ptr = &ht->buckets[bucket_pos].items[slide_pos];               \
                if (memcmp(key_ptr, &item_ptr->KeyField, key_length) == 0)          \
                {                                                                   \
                    found = true;                                                   \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
            if (found)                                                              \
            {                                                                       \
                /* found the item */                                                \
                break;                                                              \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                item_ptr = NULL;                                                    \
                /* try next available bucket */                                     \
                if (TEST_USE_NEXT_BUCKET(bucket_ptr->flag))                         \
                    bucket_pos += LOCK_CNT;                                         \
                else                                                                \
                    break;          /* not found */                                 \
                if (bucket_pos >= ht->buckets_cnt)                                  \
                {                                                                   \
                    bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;  /* wrap to bucket head */  \
                    wrapped = true;                                                 \
                }                                                                   \
                if (wrapped)                                                        \
                {                                                                   \
                    if (bucket_pos == hashcode % ht->buckets_cnt)                   \
                    {                                                               \
                        /* not found */                                             \
                        break;                                                      \
                    }                                                               \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        rwlock_unrdlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                        \
                                                                                    \
        return item_ptr;                                                            \
    }                                                                               \
    struct ItemStruct *                                                             \
    DEF_HASH_TRAVEL(HashName)(struct HASH_TABLE_TYPE(HashName) *ht, struct hash_traveler_t *traveler)   \
    {                                                                                                   \
        struct ItemStruct *item_ptr;                                                                    \
        if ((traveler->bucket_pos < 0) || (traveler->bucket_pos >= (int32_t)ht->buckets_cnt) ||         \
            (traveler->slide_pos < 0) || (traveler->slide_pos >= SLIDE_CNT))                            \
        {                                                                           \
            traveler->bucket_pos = ht->head_bucket_pos;                             \
            traveler->slide_pos  = ht->head_slide_pos;                              \
            item_ptr = &ht->buckets[traveler->bucket_pos].items[traveler->slide_pos];                   \
            goto end;                                                               \
        }                                                                           \
        int next_bucket_pos = ht->buckets[traveler->bucket_pos].links[traveler->slide_pos].next_bucket_pos;     \
        int next_slide_pos  = ht->buckets[traveler->bucket_pos].links[traveler->slide_pos].next_slide_pos;      \
        traveler->bucket_pos = next_bucket_pos;                                     \
        traveler->slide_pos  = next_slide_pos;                                      \
        if ((next_bucket_pos < 0) || (next_slide_pos < 0))                          \
        {                                                                           \
            hash_traveler_clear(traveler);                                          \
            item_ptr = NULL;                                                        \
            goto end;                                                               \
        }                                                                           \
        item_ptr = &ht->buckets[traveler->bucket_pos].items[traveler->slide_pos];   \
    end:                                                                            \
        return item_ptr;                                                            \
    }                                                                               \
    struct ItemStruct *                                                             \
    DEF_HASH_GET(HashName)(hash_type(HashName) *ht, void *key_ptr)                  \
    {                                                                               \
        uint32_t hashcode;                                                          \
        struct ItemStruct *item_ptr;                                                \
        struct STORE_BUCKET_TYPE(HashName) *bucket_ptr;                             \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        bool wrapped;                                                               \
        bool found;                                                                 \
                                                                                    \
        wrapped = false;                                                            \
        found = false;                                                              \
                                                                                    \
        hashcode = HASH_FUNCTION(key_ptr, sizeof(item_ptr->KeyField));              \
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_rdlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        for (; ;)                                                                   \
        {                                                                           \
            bucket_ptr = &ht->buckets[bucket_pos];                                  \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                if (!TEST_USAGE_MAP(bucket_ptr->usage_map, slide_pos))              \
                    continue;                                                       \
                item_ptr = &ht->buckets[bucket_pos].items[slide_pos];               \
                if (memcmp(key_ptr, &item_ptr->KeyField, sizeof(item_ptr->KeyField)) == 0)          \
                {                                                                   \
                    found = true;                                                   \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
            if (found)                                                              \
            {                                                                       \
                /* found the item */                                                \
                break;                                                              \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                item_ptr = NULL;                                                    \
                /* try next available bucket */                                     \
                if (TEST_USE_NEXT_BUCKET(bucket_ptr->flag))                         \
                    bucket_pos += LOCK_CNT;                                         \
                else                                                                \
                    break;          /* not found */                                 \
                if (bucket_pos >= ht->buckets_cnt)                                  \
                {                                                                   \
                    bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;  /* wrap to bucket head */  \
                    wrapped = true;                                                 \
                }                                                                   \
                if (wrapped)                                                        \
                {                                                                   \
                    if (bucket_pos == hashcode % ht->buckets_cnt)                   \
                    {                                                               \
                        /* not found */                                             \
                        break;                                                      \
                    }                                                               \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        rwlock_unrdlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                        \
                                                                                    \
        return item_ptr;                                                            \
    }                                                                               \
    void *                                                                          \
    DEF_HASH_GETVAL(HashName)(hash_type(HashName) *ht, void *key_ptr)               \
    {                                                                               \
        uint32_t hashcode;                                                          \
        struct ItemStruct *item_ptr;                                                \
        struct STORE_BUCKET_TYPE(HashName) *bucket_ptr;                             \
        size_t bucket_pos;                                                          \
        size_t slide_pos;                                                           \
        bool wrapped;                                                               \
        bool found;                                                                 \
                                                                                    \
        wrapped = false;                                                            \
        found = false;                                                              \
                                                                                    \
        hashcode = HASH_FUNCTION(key_ptr, sizeof(item_ptr->KeyField));              \
        bucket_pos = hashcode % ht->buckets_cnt;                                    \
                                                                                    \
        rwlock_rdlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                          \
        for (; ;)                                                                   \
        {                                                                           \
            bucket_ptr = &ht->buckets[bucket_pos];                                  \
            for (slide_pos=0; slide_pos<SLIDE_CNT; slide_pos++)                     \
            {                                                                       \
                if (!TEST_USAGE_MAP(bucket_ptr->usage_map, slide_pos))              \
                    continue;                                                       \
                item_ptr = &ht->buckets[bucket_pos].items[slide_pos];               \
                if (memcmp(key_ptr, &item_ptr->KeyField, sizeof(item_ptr->KeyField)) == 0)          \
                {                                                                   \
                    found = true;                                                   \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
            if (found)                                                              \
            {                                                                       \
                /* found the item */                                                \
                break;                                                              \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                item_ptr = NULL;                                                    \
                /* try next available bucket */                                     \
                if (TEST_USE_NEXT_BUCKET(bucket_ptr->flag))                         \
                    bucket_pos += LOCK_CNT;                                         \
                else                                                                \
                    break;          /* not found */                                 \
                if (bucket_pos >= ht->buckets_cnt)                                  \
                {                                                                   \
                    bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;  /* wrap to bucket head */  \
                    wrapped = true;                                                 \
                }                                                                   \
                if (wrapped)                                                        \
                {                                                                   \
                    if (bucket_pos == hashcode % ht->buckets_cnt)                   \
                    {                                                               \
                        /* not found */                                             \
                        break;                                                      \
                    }                                                               \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        rwlock_unrdlock(&ht->lock[ bucket_pos % LOCK_CNT ]);                        \
                                                                                    \
        if (item_ptr)                                                               \
            return &item_ptr->ValueField;                                           \
        else                                                                        \
            return NULL;                                                            \
    }                                                                               \
                                                                                    \
    struct HASH_TABLE_TYPE(HashName) *                                              \
    DEF_HASH_BUILD(HashName)(size_t hash_size)                                      \
    {                                                                               \
        struct HASH_TABLE_TYPE(HashName) *ht;                                       \
        size_t buckets_cnt;                                                         \
                                                                                    \
        buckets_cnt = find_good_size(ceil(hash_size / SLIDE_CNT * 2));              \
        ht = malloc(sizeof(struct HASH_TABLE_TYPE(HashName)) + buckets_cnt*sizeof(struct STORE_BUCKET_TYPE(HashName)));     \
        ht->size = buckets_cnt * SLIDE_CNT;                                         \
        ht->buckets_cnt = buckets_cnt;                                              \
        ht->occupied_cnt = 0;                                                       \
        ht->key_pos = __offset_of(struct ItemStruct, KeyField);                     \
        ht->key_length = sizeof(ht->buckets[0].items[0].KeyField);                  \
        ht->value_pos = __offset_of(struct ItemStruct, ValueField);                 \
        ht->value_length = sizeof(ht->buckets[0].items[0].ValueField);              \
        ht->item_size = sizeof(struct ItemStruct);                                  \
        ht->head_slide_pos = -1;                                                    \
        ht->tail_slide_pos = -1;                                                    \
        ht->head_bucket_pos = -1;                                                   \
        ht->tail_bucket_pos = -1;                                                   \
        pthread_spin_init(&ht->global_lock, PTHREAD_PROCESS_PRIVATE);               \
        for (int i=0; i<LOCK_CNT; i++)                                              \
        {                                                                           \
            rwlock_build(&ht->lock[i]);                                             \
        }                                                                           \
        memset(ht->buckets, 0, buckets_cnt*sizeof(struct STORE_BUCKET_TYPE(HashName)));         \
                                                                                    \
        return ht;                                                                  \
    }                                                                               \
    int                                                                             \
    DEF_HASH_DESTORY(HashName)(hash_type(HashName) *ht)                             \
    {                                                                               \
        free(ht);                                                                   \
        return 0;                                                                   \
    }                                                                               \
    int                                                                             \
    DEF_HASH_CLEAR(HashName)(hash_type(HashName) *ht)                               \
    {                                                                               \
        for (int i=0; i<LOCK_CNT; i++)                                              \
        {                                                                           \
            rwlock_wrlock(&ht->lock[i]);                                            \
        }                                                                           \
        pthread_spin_lock(&ht->global_lock);                                        \
        ht->occupied_cnt = 0;                                                       \
        ht->head_slide_pos = ht->tail_slide_pos = -1;                               \
        ht->head_bucket_pos = ht->tail_bucket_pos = -1;                             \
        memset(ht->buckets, 0, ht->buckets_cnt*sizeof(struct STORE_BUCKET_TYPE(HashName)));         \
        pthread_spin_unlock(&ht->global_lock);                                      \
        for (int i=0; i<LOCK_CNT; i++)                                              \
        {                                                                           \
            rwlock_unwrlock(&ht->lock[i]);                                          \
        }                                                                           \
        return 0;                                                                   \
    }                                                                               \
    int                                                                             \
    DEF_HASH_DEL(HashName)(hash_type(HashName) *ht, void *key_ptr)                  \
    {                                                                               \
        hash_iter(HashName) item_ptr;                                               \
        int32_t curr_bucket_pos, prev_bucket_pos, next_bucket_pos;                  \
        int16_t curr_slide_pos, prev_slide_pos, next_slide_pos;                     \
        uint32_t hashcode;                                                          \
        struct STORE_BUCKET_TYPE(HashName) *curr_bucket_ptr = NULL;                 \
        struct STORE_BUCKET_TYPE(HashName) *prev_bucket_ptr = NULL;                 \
        struct STORE_BUCKET_TYPE(HashName) *next_bucket_ptr = NULL;                 \
        bool wrapped;                                                               \
        bool found;                                                                 \
        int ret;                                                                    \
                                                                                    \
        wrapped = false;                                                            \
        found = false;                                                              \
        ret = 0;                                                                    \
                                                                                    \
        hashcode = HASH_FUNCTION(key_ptr, sizeof(item_ptr->KeyField));              \
        curr_bucket_pos = (int32_t)(hashcode % ht->buckets_cnt);                    \
                                                                                    \
        rwlock_wrlock(&ht->lock[ curr_bucket_pos % LOCK_CNT ]);                     \
        for (; ;)                                                                   \
        {                                                                           \
            curr_bucket_ptr = &ht->buckets[curr_bucket_pos];                        \
            for (curr_slide_pos=0; curr_slide_pos<SLIDE_CNT; curr_slide_pos++)      \
            {                                                                       \
                if (!TEST_USAGE_MAP(curr_bucket_ptr->usage_map, curr_slide_pos))    \
                    continue;                                                       \
                item_ptr = &ht->buckets[curr_bucket_pos].items[curr_slide_pos];     \
                if (memcmp(key_ptr, &item_ptr->KeyField, sizeof(item_ptr->KeyField)) == 0)          \
                {                                                                   \
                    found = true;                                                   \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
            if (found)                                                              \
            {                                                                       \
                /* found the item */                                                \
                break;                                                              \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                item_ptr = NULL;                                                    \
                /* try next available bucket */                                     \
                if (TEST_USE_NEXT_BUCKET(curr_bucket_ptr->flag))                    \
                    curr_bucket_pos += LOCK_CNT;                                    \
                else                                                                \
                    break;          /* not found */                                 \
                if (curr_bucket_pos >= (int32_t)ht->buckets_cnt)                    \
                {                                                                   \
                    curr_bucket_pos = hashcode % ht->buckets_cnt % LOCK_CNT;  /* wrap to bucket head */  \
                    wrapped = true;                                                 \
                }                                                                   \
                if (wrapped)                                                        \
                {                                                                   \
                    if (curr_bucket_pos == (int32_t)(hashcode % ht->buckets_cnt))   \
                    {                                                               \
                        /* not found */                                             \
                        break;                                                      \
                    }                                                               \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        if (!found)                                                                 \
        {                                                                           \
            ret = ERR_HASH_TABLE_NOT_EXISTS;                                        \
            goto end;                                                               \
        }                                                                           \
        curr_bucket_ptr->usage_map[curr_slide_pos] = 0;                             \
        prev_bucket_pos = curr_bucket_ptr->links[curr_slide_pos].prev_bucket_pos;   \
        prev_slide_pos = curr_bucket_ptr->links[curr_slide_pos].prev_slide_pos;     \
        next_bucket_pos = curr_bucket_ptr->links[curr_slide_pos].next_bucket_pos;   \
        next_slide_pos = curr_bucket_ptr->links[curr_slide_pos].next_slide_pos;     \
        if ((prev_bucket_pos >= 0) && (prev_bucket_pos < (int32_t)ht->buckets_cnt)) \
        {                                                                           \
            if (prev_bucket_pos % LOCK_CNT != curr_bucket_pos % LOCK_CNT)           \
            {                                                                       \
                rwlock_wrlock(&ht->lock[ prev_bucket_pos % LOCK_CNT ]);             \
            }                                                                       \
            prev_bucket_ptr = &ht->buckets[prev_bucket_pos];                        \
        }                                                                           \
        if ((next_bucket_pos >= 0) && (next_bucket_pos < (int32_t)ht->buckets_cnt)) \
        {                                                                           \
            if (next_bucket_pos % LOCK_CNT != curr_bucket_pos % LOCK_CNT)           \
            {                                                                       \
                rwlock_wrlock(&ht->lock[ next_bucket_pos % LOCK_CNT ]);             \
            }                                                                       \
            next_bucket_ptr = &ht->buckets[next_bucket_pos];                        \
        }                                                                           \
        if ((next_bucket_pos >= 0) && (next_bucket_pos < (int32_t)ht->buckets_cnt) && (prev_bucket_ptr != NULL)) \
        {                                                                                               \
            prev_bucket_ptr->links[prev_slide_pos].next_bucket_pos = next_bucket_pos;                   \
            prev_bucket_ptr->links[prev_slide_pos].next_slide_pos  = next_slide_pos;                    \
        }                                                                                               \
        if ((prev_bucket_pos >= 0) && (prev_bucket_pos < (int32_t)ht->buckets_cnt) && (next_bucket_ptr != NULL)) \
        {                                                                                               \
            next_bucket_ptr->links[next_slide_pos].prev_bucket_pos = prev_bucket_pos;                   \
            next_bucket_ptr->links[next_slide_pos].prev_slide_pos  = prev_slide_pos;                    \
        }                                                                                               \
        pthread_spin_lock(&ht->global_lock);                                                            \
        ht->occupied_cnt --;                                                                            \
        if ((ht->head_bucket_pos == curr_bucket_pos) && (ht->head_slide_pos == curr_slide_pos))         \
        {                                                                                               \
            /* if the item to be deleted is at the head of the link list */                             \
            if ((next_bucket_pos >= 0) && (next_bucket_pos < (int32_t)ht->buckets_cnt))                 \
            {                                                                                           \
                ht->head_bucket_pos = next_bucket_pos;                                                  \
                ht->head_slide_pos  = next_slide_pos;                                                   \
                next_bucket_ptr->links[next_slide_pos].prev_bucket_pos = -1;                            \
                next_bucket_ptr->links[next_slide_pos].prev_slide_pos = -1;                             \
            }                                                                                           \
        }                                                                                               \
        if ((ht->tail_bucket_pos == curr_bucket_pos) && (ht->tail_slide_pos == curr_slide_pos))         \
        {                                                                                               \
            /* if the item to be deleted is at the end of the link list */                              \
            if ((prev_bucket_pos >= 0) && (prev_bucket_pos < (int32_t)ht->buckets_cnt))                 \
            {                                                                                           \
                ht->tail_bucket_pos = prev_bucket_pos;                                                  \
                ht->tail_slide_pos  = prev_slide_pos;                                                   \
                prev_bucket_ptr->links[prev_slide_pos].next_bucket_pos = -1;                            \
                prev_bucket_ptr->links[prev_slide_pos].next_slide_pos = -1;                             \
            }                                                                                           \
        }                                                                                               \
        if (ht->occupied_cnt == 0)                                                                      \
        {                                                                                               \
            ht->head_bucket_pos = ht->tail_bucket_pos = -1;                                             \
            ht->head_slide_pos = ht->tail_slide_pos = -1;                                               \
        }                                                                                               \
        pthread_spin_unlock(&ht->global_lock);                                                          \
        if ((prev_bucket_pos >= 0) && (prev_bucket_pos < (int32_t)ht->buckets_cnt)) \
        {                                                                           \
            if (prev_bucket_pos % LOCK_CNT != curr_bucket_pos % LOCK_CNT)           \
            {                                                                       \
                rwlock_unwrlock(&ht->lock[ prev_bucket_pos % LOCK_CNT ]);           \
            }                                                                       \
        }                                                                           \
        if ((next_bucket_pos >= 0) && (next_bucket_pos < (int32_t)ht->buckets_cnt)) \
        {                                                                           \
            if (next_bucket_pos % LOCK_CNT != curr_bucket_pos % LOCK_CNT)           \
            {                                                                       \
                rwlock_unwrlock(&ht->lock[ next_bucket_pos % LOCK_CNT ]);           \
            }                                                                       \
        }                                                                           \
        rwlock_unwrlock(&ht->lock[ curr_bucket_pos % LOCK_CNT ]);                   \
    end:                                                                            \
        return ret;                                                                 \
    }                                                                               \

#endif /* ! _CONHASH_H_ */

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
