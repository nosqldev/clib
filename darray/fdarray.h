/* © Copyright 2010 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * |  Fast Dynamic Array                                                  |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2010-04-14 17:26                                            |
 * +----------------------------------------------------------------------+
 */

#ifndef _DARRAY_H_
#define _DARRAY_H_

#ifdef __cplusplus
extern "C"
{
#endif 

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "slab.h"

/*
 * SUPPORTED MACRO DEFINITIONS
 * ---------------------------
 * MUL_THREAD
 * DARRAY_DEBUG_MODE
 * mem_alloc
 * mem_free
 * DARRAY_NODE_SIZE
 */

// {{{ #################### SYSTEM DEFINITION ####################

#ifdef DARRAY_DEBUG_MODE
  #define DEBUG_PUTS(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PUTS(...) ((void)0)
#endif /* DARRAY_DEBUG_MODE */

#ifndef mem_alloc
  #ifdef SLAB_ALLOC
    #define mem_alloc slab_alloc
  #else
    #define mem_alloc malloc
  #endif
#endif

#ifndef mem_free
  #ifdef SLAB_ALLOC
    #define mem_free slab_free
  #else
    #define mem_free free
  #endif
#endif

#define DARRAY_NODE_SIZE 128

struct darray_common_head
{
    uint32_t node_count;
    uint32_t capacity;
    uint32_t setsize; /* set as many as you want, just used for pass sizeof the darray */
    struct darray_common_node * first_node_ptr;
    struct darray_common_node * last_node_ptr;
    uint32_t node_size;
    uint32_t item_size;
    uint32_t cache_node_subscript;
    uint32_t cache_item_subscript;
    struct darray_common_node * cache_node_ptr;
    void * cache_item_ptr;
    pthread_rwlock_t lock;
};

typedef struct darray_common_head * darray;

struct darray_common_node
{
    uint32_t id;
    uint32_t offset;
    struct darray_common_node *pre_node;
    struct darray_common_node *next_node;
    unsigned char item_array[];
};

// }}}

// {{{ #################### FORWARD DECLARATION #################### 
static inline uint32_t min_in_3(uint32_t a, uint32_t b, uint32_t c) __attribute__((always_inline));
static inline struct darray_common_node * darray_find_node(struct darray_common_head *head_ptr, uint32_t nth) __attribute__((always_inline));
static inline int darray_extend_node(struct darray_common_head *head_ptr, size_t sizeof_node) __attribute__((always_inline));
// }}}

// {{{ #################### INTERFACE ####################

/** 
 * @brief Alloc a dynamic-array object.
 * 
 * @param head_ptr Pointer to the new dynamic-array.
 * @param item_struct Structure of the item in the dynamic-array.
 * 
 */
#define darray_new(head_ptr, item_struct)   \
    struct darray_common_head *head_ptr = mem_alloc(sizeof(struct darray_common_head)); \
    assert(head_ptr != NULL);                   \
    bzero(head_ptr, sizeof(struct darray_common_head)); \
    pthread_rwlock_init(&head_ptr->lock, NULL);         \
    do {    \
        struct darray_common_node *node_ptr = mem_alloc(sizeof(struct darray_common_node) + (sizeof(struct item_struct)*DARRAY_NODE_SIZE)); \
        assert(node_ptr != NULL);   \
        bzero(node_ptr, sizeof(struct darray_common_node)); \
        head_ptr->node_count = 1;   \
        head_ptr->capacity = DARRAY_NODE_SIZE;  \
        head_ptr->node_size = DARRAY_NODE_SIZE; \
        head_ptr->item_size = sizeof(struct item_struct);   \
        head_ptr->first_node_ptr = node_ptr;    \
        head_ptr->last_node_ptr = node_ptr;     \
        head_ptr->cache_node_subscript = 0;     \
        head_ptr->cache_item_subscript = 0;     \
        head_ptr->cache_node_ptr = head_ptr->first_node_ptr;  \
        head_ptr->cache_item_ptr = &(node_ptr->item_array[0]);  \
        head_ptr->setsize = 0;                  \
        node_ptr->id = 0;                       \
        node_ptr->offset = 0;                   \
    } while (0)

#define darray_new2(head_ptr, item_struct, my_node_size)   \
    struct darray_common_head *head_ptr = mem_alloc(sizeof(struct darray_common_head)); \
    assert(head_ptr != NULL);                   \
    bzero(head_ptr, sizeof(struct darray_common_head)); \
    pthread_rwlock_init(&head_ptr->lock, NULL);         \
    do {    \
        DEBUG_PUTS("alloc size: %zu\n", sizeof(struct item_struct)*my_node_size);   \
        struct darray_common_node *node_ptr = mem_alloc(sizeof(struct darray_common_node) + (sizeof(struct item_struct)*my_node_size)); \
        assert(node_ptr != NULL);   \
        bzero(node_ptr, sizeof(struct darray_common_node)); \
        head_ptr->node_count = 1;   \
        head_ptr->capacity = my_node_size;  \
        head_ptr->node_size = my_node_size; \
        head_ptr->item_size = sizeof(struct item_struct);   \
        head_ptr->first_node_ptr = node_ptr;    \
        head_ptr->last_node_ptr = node_ptr;     \
        head_ptr->cache_node_subscript = 0;     \
        head_ptr->cache_item_subscript = 0;     \
        head_ptr->cache_node_ptr = head_ptr->first_node_ptr;  \
        head_ptr->cache_item_ptr = &(node_ptr->item_array[0]);  \
        head_ptr->setsize = 0;                  \
        node_ptr->id = 0;                       \
        node_ptr->offset = 0;                   \
    } while (0)

/** 
 * @brief Just declare a new pointer to the dynamic-array.
 * 
 * @param head_ptr Pointer to be declared.
 * 
 */
#define darray_new_ptr(head_ptr)  \
    struct darray_common_head *head_ptr = NULL

/** 
 * @brief Lock the dynamic-array exclusively.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * 
 */
#ifdef MUL_THREAD
  #define darray_exlock(head_ptr) do { assert(head_ptr!=NULL); pthread_rwlock_wrlock(&(head_ptr->lock)); } while (0)
#else
  #define darray_exlock(head_ptr) do {} while (0)
#endif


/** 
 * @brief Use a shared lock on the dynamic-array.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * 
 */
#ifdef MUL_THREAD
  #define darray_shlock(head_ptr) do { assert(head_ptr!=NULL); pthread_rwlock_rdlock(&(head_ptr->lock)); } while (0)
#else
  #define darray_shlock(head_ptr) do {} while (0)
#endif

/** 
 * @brief Release lock.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * 
 */
#ifdef MUL_THREAD
  #define darray_unlock(head_ptr) do { assert(head_ptr!=NULL); pthread_rwlock_unlock(&(head_ptr->lock)); } while (0)
#else
  #define darray_unlock(head_ptr) do { } while (0)
#endif

/** 
 * @brief Destroy the dynamic-array.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * 
 */
#define darray_free(head_ptr) do {  \
        assert(head_ptr != NULL);   \
        darray_exlock(head_ptr);    \
        struct darray_common_node *p = (struct darray_common_node *)(head_ptr->first_node_ptr);    \
        struct darray_common_node *pp = p;                          \
        while (pp) {                                                \
            p = pp->next_node;                                      \
            mem_free(pp);                                           \
            pp = p;                                                 \
        }                                                           \
        head_ptr->node_count = head_ptr->capacity = head_ptr->cache_item_subscript = head_ptr->cache_node_subscript = 0; \
        head_ptr->first_node_ptr = head_ptr->last_node_ptr = head_ptr->cache_node_ptr = NULL;   \
        head_ptr->cache_item_ptr = NULL;                            \
        darray_unlock(head_ptr);                                    \
        pthread_rwlock_destroy(&(head_ptr->lock));                  \
        mem_free(head_ptr);                                         \
    } while (0)

/** 
 * @brief Obtain the size of the dynamic-array allocated.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * 
 * @return Size of the dynamic-array.
 */
#define darray_size(head_ptr) (head_ptr->capacity)

#define darray_setsize(head_ptr) (head_ptr->setsize)

#define _darray_addr(head_ptr, index, item_ptr) do {     \
        assert(head_ptr != NULL);                       \
        struct darray_common_node *node_ptr;            \
        int offset_in_node = index % head_ptr->node_size;   \
        darray_shlock(head_ptr);                        \
        if ((index >= 0) && (index < (int)head_ptr->capacity)) { \
            if (index == (int)head_ptr->cache_item_subscript) {  \
                item_ptr = (void *)head_ptr->cache_item_ptr;   \
                DEBUG_PUTS("cache_item_ptr\n"); \
            } else {                                        \
                node_ptr = darray_find_node(head_ptr, index);   \
                item_ptr = (void *)&(node_ptr->item_array[head_ptr->item_size * offset_in_node]);  \
                head_ptr->cache_item_ptr = item_ptr;        \
                head_ptr->cache_item_subscript = index;     \
                if (node_ptr == NULL) item_ptr = NULL;      \
            }                                           \
        } else if (index >= (int)head_ptr->capacity) {       \
            int gap = (int)ceil((index + 1 - head_ptr->capacity)*1.0 / (head_ptr->node_size*1.0));   \
            DEBUG_PUTS("Extend: %d\n", gap);   \
            for (; gap > 0; --gap) {                    \
                darray_extend_node(head_ptr, sizeof(struct darray_common_node)+head_ptr->node_size*head_ptr->item_size); \
            }                                           \
            item_ptr = (void *)&(head_ptr->last_node_ptr->item_array[head_ptr->item_size * (index % head_ptr->node_size)]); \
            head_ptr->cache_item_subscript = index;     \
            head_ptr->cache_item_ptr = item_ptr;        \
        }                                               \
        else {                                          \
            item_ptr = NULL;                            \
        }                                               \
        darray_unlock(head_ptr);                        \
    } while (0)

/** 
 * @brief Use index to address the dynamic-array, memory will be allocated
 *        automatically if it doesn't exists.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * @param index Index to be addressed.
 * @param item_ptr Pointer to the item.
 * 
 */
static void *
darray_addr(struct darray_common_head *head_ptr, int index)
{
    void *item_ptr;
    
    _darray_addr(head_ptr, index, item_ptr);

    return item_ptr;
}

/** 
 * @brief Push back into dynamic-array.
 *        USE CAREFULLY.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * @param item_ptr Pointer to the item to be push_back.
 * 
 * @return Index of new item.
 */
static inline int
darray_push(struct darray_common_head *head_ptr, void *item_ptr)
{
    int index;
    void *ptr;

    if (darray_setsize(head_ptr) == 0)
    {
        index = 0;
    }
    else
    {
        index = head_ptr->cache_item_subscript + 1;
    }
    ptr = darray_addr(head_ptr, index);

    memcpy(ptr, (void*)item_ptr, head_ptr->item_size);
    darray_setsize(head_ptr) ++;

    return index;
}

/** 
 * @brief Shrink the dynamic-array.
 * 
 * @param head_ptr Pointer to the dynamic-array.
 * @param nsize New size of the dynamic-array.
 * 
 * @return 
 */
static inline int
darray_shrink_size(struct darray_common_head *head_ptr, size_t nsize)
{
    struct darray_common_node *node_ptr;
    size_t free_node_count;

    if (darray_size(head_ptr) > nsize)
    {
        free_node_count = head_ptr->node_count - (size_t)ceil((nsize*1.0)/(head_ptr->node_size*1.0));
        darray_exlock(head_ptr);
        for (; free_node_count > 0; --free_node_count)
        {
            node_ptr = head_ptr->last_node_ptr;
            head_ptr->last_node_ptr = node_ptr->pre_node;
            head_ptr->last_node_ptr->next_node = NULL;
            head_ptr->node_count --;
            head_ptr->capacity -= head_ptr->node_size;
            mem_free(node_ptr);
        }
        head_ptr->cache_node_subscript = 0;
        head_ptr->cache_node_ptr = head_ptr->first_node_ptr;
        head_ptr->cache_item_subscript = 0;
        head_ptr->cache_item_ptr = &(head_ptr->first_node_ptr->item_array[0]);
        darray_unlock(head_ptr);
    }

    return 0;
}

// }}}

// {{{ #################### INTERNAL IMPLEMENTATION ####################
static inline uint32_t
min_in_3(uint32_t a, uint32_t b, uint32_t c)
{
    a = ((a < b) ? a : b);
    a = ((a < c) ? a : c);

    return a;
}

static inline struct darray_common_node *
darray_find_node(struct darray_common_head *head_ptr, uint32_t nth)
{
    int32_t node_id, item_id;
    uint32_t a, b, c, gap;
    struct darray_common_node *node_ptr, *base_ptr;

    node_id = nth / head_ptr->node_size;
    item_id = nth % head_ptr->node_size;

    if (node_id == (int)head_ptr->cache_node_subscript)
    {
        return head_ptr->cache_node_ptr;
    }

    a = node_id;
    b = abs(node_id - (int)head_ptr->cache_node_subscript);
    c = abs(node_id - ((int)head_ptr->node_count-1));
    gap = min_in_3(a, b, c);

    if ((int)gap == node_id)
        base_ptr = head_ptr->first_node_ptr;
    else if (gap == b)
        base_ptr = head_ptr->cache_node_ptr;
    else
        base_ptr = head_ptr->last_node_ptr;

    node_ptr = base_ptr;
    if (node_id > (int)base_ptr->id)  /* Backward search */
    {
        //DEBUG_PUTS("[Backward search: from %d to %d]\n", base_ptr->id, node_id);
        while ((gap>0) && (node_ptr!=NULL))
        {
            node_ptr = node_ptr->next_node;
            --gap;
        }
    }
    else if (node_id < (int)base_ptr->id)   /* Forward search */
    {
        //DEBUG_PUTS("[Forward search: from %d to %d]\n", base_ptr->id, node_id);
        while ((gap>0) && (node_ptr!=NULL))
        {
            node_ptr = node_ptr->pre_node;
            --gap;
        }
    }
    else    /* Just U!!! */
    {
        /* Nothing to do here */
        //DEBUG_PUTS("[Lucky]\n");
    }
    if (node_ptr != NULL)
    {
        head_ptr->cache_node_ptr = node_ptr;
        head_ptr->cache_node_subscript = node_id;
    }

    return node_ptr;
}

static inline int
darray_extend_node(struct darray_common_head *head_ptr, size_t sizeof_node)
{
    struct darray_common_node *last_node, *new_node;

    last_node = head_ptr->last_node_ptr;
    DEBUG_PUTS("extend size: %zu\n", sizeof_node);
    new_node = (struct darray_common_node *)mem_alloc(sizeof_node);
    bzero(new_node, sizeof(struct darray_common_node));
    head_ptr->last_node_ptr = new_node;
    last_node->next_node = new_node;
    head_ptr->node_count++;
    head_ptr->capacity += head_ptr->node_size;
    new_node->pre_node = last_node;
    new_node->next_node = NULL;
    new_node->id = last_node->id + 1;
    new_node->offset = last_node->offset + head_ptr->node_size;
    head_ptr->cache_node_subscript = new_node->id;
    head_ptr->cache_node_ptr = new_node;

    return 0;
}

static inline void
display_darray(struct darray_common_head *head_ptr)
{
    struct darray_common_node *node_ptr;
    puts("================================================================================");
    puts("\t----- head info -----");
    printf("\tnode_count: %d\tcapacity: %d\tnode_size: %d\tsetsize: %d\n",
           head_ptr->node_count, head_ptr->capacity, head_ptr->node_size, head_ptr->setsize);
    printf("\tcache_node_subscript: %d\tcache_item_subscript: %d\n", head_ptr->cache_node_subscript, head_ptr->cache_item_subscript);
    printf("\tfirst_node_ptr: %p\tlast_node_ptr: %p\tcache_node_ptr: %p\tcache_item_ptr: %p\n",
           head_ptr->first_node_ptr, head_ptr->last_node_ptr, head_ptr->cache_node_ptr, head_ptr->cache_item_ptr);
    puts("\t----- node info -----");
    node_ptr = head_ptr->first_node_ptr;
    while (node_ptr != NULL)
    {
        printf("\tid: %u\toffset: %u\tthis: %p\tpre_node: %p\tnext_node: %p\n",
               node_ptr->id, node_ptr->offset, node_ptr, node_ptr->pre_node, node_ptr->next_node);
        node_ptr = node_ptr->next_node;
    }
    puts("================================================================================");
}

// }}}

#ifdef __cplusplus
}
#endif 

#endif /* ! _DARRAY_H_ */

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
