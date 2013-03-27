/* © Copyright 2010 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * | test darray memory leak                                              |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2010-04-21 11:12                                            |
 * +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "leakdetector.h"
#include "CUnit/Basic.h"

#define mem_alloc my_alloc
#define mem_free my_free

#include "darray.h"

struct item_t
{
    int id;
};

/* {{{ static void darray_shrink_size_test(void) */
static void
darray_shrink_size_test(void)
{
    struct item_t *item_ptr;
    darray_new2(array, item_t, 8);
    CU_ASSERT(g_alloc_info.curr_pos == 2);
    CU_ASSERT(g_alloc_info.alloc_times == 2);
    CU_ASSERT(g_alloc_info.end_pos == 2);

    item_ptr = darray_addr(array, 63);
    (void)item_ptr;
    CU_ASSERT(g_alloc_info.curr_pos == g_alloc_info.end_pos);
    CU_ASSERT(g_alloc_info.alloc_times == 9);
    darray_shrink_size(array, 8);
    CU_ASSERT(g_alloc_info.curr_pos == 2);
    CU_ASSERT(g_alloc_info.alloc_times - g_alloc_info.free_times == 2);
    CU_ASSERT(g_alloc_info.last_free_id == 2);
    CU_ASSERT(g_alloc_node[2].prev_free_id == 3);
    CU_ASSERT(g_alloc_node[3].prev_free_id == 4);
    CU_ASSERT(g_alloc_node[4].prev_free_id == 5);
    CU_ASSERT(g_alloc_node[5].prev_free_id == 6);
    CU_ASSERT(g_alloc_node[6].prev_free_id == 7);
    CU_ASSERT(g_alloc_node[7].prev_free_id == 8);
    CU_ASSERT(g_alloc_node[8].prev_free_id == -1);

    item_ptr = darray_addr(array, 23);
    CU_ASSERT(g_alloc_info.curr_pos == 4);
    CU_ASSERT(g_alloc_node[4].prev_free_id == 5);
    CU_ASSERT(g_alloc_node[3].prev_free_id == -1);
    CU_ASSERT(g_alloc_info.last_free_id == 4);

    item_ptr = darray_addr(array, 31);
    CU_ASSERT(g_alloc_info.curr_pos == 5);
    CU_ASSERT(g_alloc_node[5].prev_free_id = 6);
    CU_ASSERT(g_alloc_node[4].prev_free_id = -1);
    CU_ASSERT(g_alloc_info.last_free_id == 5);

    item_ptr = darray_addr(array, 39);
    CU_ASSERT(g_alloc_info.curr_pos == 6);
    CU_ASSERT(g_alloc_node[5].prev_free_id = -1);
    CU_ASSERT(g_alloc_node[6].prev_free_id = 7);
    CU_ASSERT(g_alloc_info.last_free_id == 6);

    darray_shrink_size(array, 9);
    CU_ASSERT(g_alloc_info.curr_pos == 3);
    CU_ASSERT(g_alloc_node[3].prev_free_id == 4);
    CU_ASSERT(g_alloc_node[4].prev_free_id == 5);
    CU_ASSERT(g_alloc_node[5].prev_free_id == 6);
    CU_ASSERT(g_alloc_node[6].prev_free_id == 7);
    CU_ASSERT(g_alloc_node[7].prev_free_id == 8);
    CU_ASSERT(g_alloc_node[8].prev_free_id == -1);
    CU_ASSERT(g_alloc_info.alloc_times - g_alloc_info.free_times == 3);

    darray_shrink_size(array, 0);
    CU_ASSERT(g_alloc_info.curr_pos == 2);

    darray_free(array);
    CU_ASSERT(g_alloc_info.curr_pos == 0);
    CU_ASSERT(g_alloc_info.alloc_times == g_alloc_info.free_times);
}
/* }}} */

int
main(void)
{
    /* {{{ init CU suite memory_leak */
    CU_pSuite pSuite = NULL;
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();
    pSuite = CU_add_suite("memory_leak", NULL, NULL);
    if (pSuite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /* }}} */

    /* {{{ CU_add_test: darray_shrink_size_test */
    if (CU_add_test(pSuite, "darray_shrink_size_test", darray_shrink_size_test) == NULL)
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

    return 0;
}

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
