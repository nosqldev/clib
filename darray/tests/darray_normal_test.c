/* © Copyright 2010 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * |  normal test of darray                                               |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2010-04-13 12:14                                            |
 * +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "darray.h"
#include "CUnit/Basic.h"

struct item_t
{
    int id;
    char buffer[32];
};

darray_new_ptr(garray);

#define NUM 1024

/* {{{ static void push_test(void) */
static void
push_test(void)
{
    struct item_t item;
    struct item_t *ptr;
    int ret;


    for (int i=0; i<NUM; i++)
    {
        item.id = i;
        snprintf(item.buffer, sizeof item.buffer, "%d", item.id);
        ret = darray_push(garray, &item);
        CU_ASSERT(ret == i);
        if (ret != i)
        {
           printf("========== %d, %d\n", ret, i);
        }
    }
    CU_ASSERT(darray_setsize(garray) == NUM);

    for (int i=0; i<10; i++)
    {
        ptr = darray_addr(garray, i);
        CU_ASSERT(ptr->id == i);
    }
}
/* }}} */

/* {{{ static void set_test(void) */
static void
set_test(void)
{
    struct item_t *ptr;
    int *array;

    array = malloc(sizeof(int) * NUM);
    for (int i=0; i<NUM; i++)
    {
        array[i] = random();
    }

    for (int i=0; i<NUM; i++)
    {
        ptr = darray_addr(garray, i);
        ptr->id = array[i];
    }

    for (int i=0; i<NUM; i++)
    {
        ptr = darray_addr(garray, i);
        CU_ASSERT(ptr->id == array[i]);
    }

    free(array);
}
/* }}} */

/* {{{ static void shrink_test(void) */
static void
shrink_test(void)
{
    struct item_t *item;
    struct item_t it;

    darray_new2(array, item_t, 32);
    for (int i=0; i<100; i++)
    {
        item = darray_addr(array, i);
        item->id = i * i;
    }
    CU_ASSERT(array->node_count == (100/32 + 1));

    darray_setsize(array) = 100;
    for (int i=0; i<100; i++)
    {
        it.id = i * i;
        darray_push(array, &it);
    }
    CU_ASSERT(array->node_count == (200/32 + 1));
    CU_ASSERT(array->capacity == array->node_count * array->node_size);

    darray_shrink_size(array, 96);
    CU_ASSERT(array->node_count == (96/32));

    darray_shrink_size(array, 90);
    CU_ASSERT(array->node_count == (90/32+1));
    CU_ASSERT(array->first_node_ptr->next_node->next_node->next_node == NULL);
    CU_ASSERT(array->last_node_ptr == array->first_node_ptr->next_node->next_node);
    CU_ASSERT(array->node_count == 3);

    darray_shrink_size(array, 32);
    CU_ASSERT(array->first_node_ptr->next_node == NULL);
    CU_ASSERT(array->last_node_ptr == array->first_node_ptr);
    CU_ASSERT(array->node_count == 1);
    CU_ASSERT(array->capacity == 32);
    CU_ASSERT(array->cache_node_subscript == 0);
    CU_ASSERT(array->cache_item_subscript == 0);
    CU_ASSERT(array->cache_node_ptr == array->first_node_ptr);
    CU_ASSERT(array->cache_item_ptr == &(array->first_node_ptr->item_array[0]));

    darray_shrink_size(array, 99);
    CU_ASSERT(array->first_node_ptr->next_node == NULL);
    CU_ASSERT(array->last_node_ptr == array->first_node_ptr);
    CU_ASSERT(array->node_count == 1);
    CU_ASSERT(array->capacity == 32);
}
/* }}} */

int
main(void)
{
    /* {{{ init CU suite normal_test */
    CU_pSuite pSuite = NULL;
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();
    pSuite = CU_add_suite("normal_test", NULL, NULL);
    if (pSuite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /* }}} */

    darray_new(array, item_t);
    garray = array;
    
    /* {{{ CU_add_test: push_test */
    if (CU_add_test(pSuite, "push_test", push_test) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /* }}} */

    /* {{{ CU_add_test: set_test */
    if (CU_add_test(pSuite, "set_test", set_test) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /* }}} */

    /* {{{ CU_add_test: shrink_test */
    if (CU_add_test(pSuite, "shrink_test", shrink_test) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /* }}} */

    /* {{{ CU run & cleanup */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    /* }}} */

    return CU_get_error();
}

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
