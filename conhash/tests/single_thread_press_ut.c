#include "timeutil.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "uthash.h"

struct item_t
{
    int32_t id;
    int64_t value;
    UT_hash_handle hh;
};

#define NUM 1000000

int
main(void)
{
    struct item_t *head = NULL;
    struct item_t *ptr;

    pre_timer();
    launch_timer();
    for (int i=0; i<NUM; i++)
    {
        ptr = malloc(sizeof(struct item_t));
        ptr->id = rand();
        ptr->value = ptr->id;
        HASH_ADD_INT(head, id, ptr);
    }
    stop_timer();
    print_timer();

    launch_timer();
    for (int i=0; i<NUM; i++)
    {
        int32_t nid = rand();
        HASH_FIND_INT(head, &nid, ptr);
        /*
         *if (ptr)
         *    printf("%d %ld\n", ptr->id, ptr->value);
         *else
         *    printf("%d not found\n", nid);
         */
    }
    stop_timer();
    print_timer();

    /*pause();*/

    return 0;
}
