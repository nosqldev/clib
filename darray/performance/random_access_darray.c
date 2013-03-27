/* © Copyright 2010 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * | random access performance of darray                                  |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2010-04-13 18:19                                            |
 * +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include "darray.h"
#include "timeutil.h"

struct item_t
{
    int id;
};

darray_new_ptr(garray);
#define NUM (1000000)
static int pos[NUM] = {0};
static int count;
static int native_array[NUM] = {0};
static int *malloc_array;

/* {{{ static void load_access_pos(char *filename) */
static void
load_access_pos(char *filename)
{
    FILE *fp;
    int i = 0;
    char buffer[1024];
    fp = fopen(filename, "r");
    while (fgets(buffer, sizeof buffer, fp) != NULL)
    {
        pos[i++] = atoi(buffer) % NUM;
        assert(pos[i-1] < NUM);
    }
    fclose(fp);
    printf("loaded pos array's size: %u\n", i);
    count = i;
}
/* }}} */

/* {{{ static void init_array(void) */
static void
init_array(void)
{
    int i;
    struct item_t *ptr;
    struct rusage r;

    memset(native_array, 0, sizeof native_array);
    getrusage(RUSAGE_SELF, &r);
    printf("[memory usage init] rss = %ldK\n", r.ru_maxrss);
    malloc_array = malloc(sizeof(int) * NUM);
    memset(malloc_array, 0, sizeof(int)*NUM);
    getrusage(RUSAGE_SELF, &r);
    printf("[memory usage after malloc] rss = %ldK\n", r.ru_maxrss);
    for (i=0; i<count; i++)
    {
        ptr = darray_addr(garray, i);
        ptr->id = random();
        native_array[i] = ptr->id;
        malloc_array[i] = ptr->id;
    }
    getrusage(RUSAGE_SELF, &r);
    printf("[memory usage after darray] rss = %ldK\n", r.ru_maxrss);
}
/* }}} */

/* {{{ static void validity_check(void) */
/** 
 * @brief check whether the darray equal to the native_array after init_array()
 */
static void
validity_check(void)
{
    struct item_t *ptr;
    for (int i=0; i<count; i++)
    {
        ptr = darray_addr(garray, i);
        assert(ptr->id == native_array[i]);
    }
}
/* }}} */

/* {{{ static inline void rand_read(void) */
static inline void
rand_read(void)
{
    struct item_t *ptr;
    for (int i=0; i<count; i++)
    {
        ptr = darray_addr(garray, pos[i]);
        (void)ptr;
    }
}
/* }}} */

/* {{{ static inline void rand_read_native_array(void) */
static inline void
rand_read_native_array(void)
{
    int num;
    for (int i=0; i<count; i++)
    {
        num = native_array[pos[i]];
        (void)num;
    }
}
/* }}} */

/* {{{ static inline void rand_read_malloc_array(void) */
static inline void
rand_read_malloc_array(void)
{
    int num;
    for (int i=0; i<count; i++)
    {
        num = malloc_array[ pos[i] ];
        (void)num;
    }
}
/* }}} */

/* {{{ void usage(void) */
void
usage(void)
{
    printf("usage:\n"
           "arg 1: filename of access pos array\n");
    exit(0);
}
/* }}} */

/* {{{ static inline void seq_read(void) */
static inline void
seq_read(void)
{
    struct item_t *ptr;
    /*uint64_t sum = 0;*/
    for (int i=0; i<count; i++)
    {
        ptr = darray_addr(garray, i);
        /*sum += ptr->id;*/
        (void)ptr;
    }
}
/* }}} */

/* {{{ static inline void seq_read_native_array(void) */
static inline void
seq_read_native_array(void)
{
    int num;
    for (int i=0; i<count; i++)
    {
        num = native_array[i];
        (void)num;
    }
}
/* }}} */

/* {{{ static inline void seq_read_malloc_array(void) */
static inline void
seq_read_malloc_array(void)
{
    int num;
    for (int i=0; i<count; i++)
    {
        num = malloc_array[i];
        (void)num;
    }
}
/* }}} */

int
main(int argc, char **argv)
{
    pre_timer();
    darray_new2(array, item_t, 1000);
    garray = array;

    if (!((argc == 2) && (access(argv[1], F_OK) == 0)))
    {
        usage();
    }

    load_access_pos(argv[1]);
    init_array();

    printf("[node size] %d\n", array->node_size);

    launch_timer();
    validity_check();
    stop_timer();
    print_timer("- validity_check - ");

    launch_timer();
    rand_read();
    stop_timer();
    print_timer("\n- rand read darray - ");

    launch_timer();
    rand_read_native_array();
    stop_timer();
    print_timer("- rand read native static array - ");

    launch_timer();
    rand_read_malloc_array();
    stop_timer();
    print_timer("- rand read malloc array - ");

    launch_timer();
    seq_read();
    stop_timer();
    print_timer("\n- seq read darray - ");

    launch_timer();
    seq_read_native_array();
    stop_timer();
    print_timer("- seq read native static array - ");

    launch_timer();
    seq_read_malloc_array();
    stop_timer();
    print_timer("- seq read malloc array - ");

    return 0;
}

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
