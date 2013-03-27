#ifndef _LEAK_DECTOR_H_
#define _LEAK_DECTOR_H_

/* © Copyright 2010 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * | leak detector                                                        |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2010-04-26 15:12                                            |
 * +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define NUM (10240)

struct alloc_info
{
    int alloc_times;
    int free_times;
    int end_pos;
    int curr_pos;
    int last_free_id;
};

struct alloc_node
{
    void *ptr;
    size_t size;
    int prev_free_id;
};

static struct alloc_info g_alloc_info = {.alloc_times=0, .free_times=0, .end_pos=0, .curr_pos=0, .last_free_id=-1};
static struct alloc_node g_alloc_node[] =
{
    [0 ... NUM] = {NULL, 0, -1}
};

/* {{{ void * my_alloc(size_t size) */
static inline void *
my_alloc(size_t size)
{
    void *ptr;

    assert(g_alloc_info.curr_pos <= g_alloc_info.end_pos);
    ptr = malloc(size);
    g_alloc_node[ g_alloc_info.curr_pos ].ptr = ptr;
    g_alloc_node[ g_alloc_info.curr_pos ].size = size;
    if (g_alloc_info.curr_pos == g_alloc_info.end_pos)
    {
        assert(g_alloc_info.alloc_times - g_alloc_info.free_times == g_alloc_info.end_pos);
        g_alloc_info.curr_pos ++;
        g_alloc_info.end_pos ++;
        g_alloc_info.last_free_id = -1;
    }
    else
    {
        assert(g_alloc_info.alloc_times >= g_alloc_info.free_times);
        if (g_alloc_node[ g_alloc_info.curr_pos ].prev_free_id == -1)
        {
            g_alloc_info.curr_pos = g_alloc_info.end_pos;
            g_alloc_info.last_free_id = -1;
        }
        else
        {
            int tmp_pos = g_alloc_info.curr_pos;
            g_alloc_info.curr_pos = g_alloc_node[ tmp_pos ].prev_free_id;
            //g_alloc_info.last_free_id = g_alloc_node[ g_alloc_info.curr_pos ].prev_free_id;
            g_alloc_info.last_free_id = g_alloc_info.curr_pos;
            g_alloc_node[ tmp_pos ].prev_free_id = -1;
        }
    }
    assert(g_alloc_info.curr_pos < NUM);
    assert(g_alloc_info.curr_pos >= 0);
    g_alloc_info.alloc_times ++;

    return ptr;
}
/* }}} */

/* {{{ void my_free(void *ptr) */
static inline void
my_free(void *ptr)
{
    bool found = false;

    assert(g_alloc_info.alloc_times > g_alloc_info.free_times);
    for (int i=0; i<g_alloc_info.end_pos; i++)
    {
        if (g_alloc_node[i].ptr == ptr)
        {
            found = true;
            free(ptr);
            g_alloc_node[i].ptr = NULL;
            g_alloc_node[i].size = 0;
            g_alloc_node[i].prev_free_id = g_alloc_info.last_free_id;
            g_alloc_info.curr_pos = i;
            g_alloc_info.last_free_id = i;
            break;
        }
    }

    assert(found == true);
    g_alloc_info.free_times ++;
}
/* }}} */

#endif /* ! _LEAK_DECTOR_H_ */

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
