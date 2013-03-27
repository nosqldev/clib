#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>

#ifndef _CEXCEPTION_H_
#define _CEXCEPTION_H_

#define ERR_MESG_LEN 32

struct exception_info_t
{
    int err_code;
    char err_mesg[ ERR_MESG_LEN ];
};

typedef struct exception_t
{
    jmp_buf jmp_buf_env;
    struct exception_info_t info;
} exception_t;

#define exception_err_code ((__local_exception_ptr).info.err_code)
#define exception_err_mesg ((__local_exception_ptr).info.err_mesg)

#define def_local_exception __thread exception_t __local_exception_ptr

#define try_codes  \
    int __exception_ret_val = 0;    \
    __exception_ret_val = setjmp( *(jmp_buf *)(__local_exception_ptr).jmp_buf_env );  \
    if (__exception_ret_val == 0)

#define catch_exception   else

#define throw_exception(err_code_value, err_mesg_value, ...) do {   \
    (__local_exception_ptr).info.err_code = err_code_value;    \
    if (err_mesg_value != NULL) snprintf((__local_exception_ptr).info.err_mesg, ERR_MESG_LEN, err_mesg_value, ##__VA_ARGS__);  \
    else (__local_exception_ptr).info.err_mesg[0] = '\0';      \
    (__local_exception_ptr).info.err_mesg[ ERR_MESG_LEN-1 ] = '\0';          \
    longjmp((__local_exception_ptr).jmp_buf_env, -1);  \
} while(0)

#endif /* ! _AS_EXCEPTION_H_ */

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
