#include <stdio.h>
#include <pthread.h>
#include "cexception.h"
#include "assertest.h"

def_local_exception;

static int
foo(char *name)
{
    printf("come to foo: %s\n", name);
    throw_exception(-1, "Orz");
    return 0;
}

static void *
worker(void *arg)
{
    char *name = (char *) arg;

    try_codes
    {
        foo(name);
    }
    catch_exception
    {
        printf("[%s] throw\n", name);
        assertest(__exception_ret_val == -1, "exception caught");
    }

    return NULL;
}

#define NUM 5

int
main(void)
{
    char *name[] = {"jin", "mu", "shui", "huo", "tu"};

#if 0
    try_codes
    {
        foo();
        printf("reach!\n");
    }
    catch_exception
    {
        printf("error code: %d, message: %s\n", exception_err_code, exception_err_mesg);
    }
#endif

    pthread_t tid[NUM];
    for (int i=0; i<NUM; i++)
    {
        pthread_create(&tid[i], NULL, worker, name[i]);
    }

    for (int i=0; i<NUM; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
