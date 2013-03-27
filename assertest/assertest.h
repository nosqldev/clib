#ifndef _ASSERTEST_H_
#define _ASSERTEST_H_
/* © Copyright 2013 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * | assert implementation                                                |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2013-02-22 16:37                                            |
 * +----------------------------------------------------------------------+
 */
#include <stdarg.h>
#include <string.h>

#define FAILED_PROMPT "\033[0;30;31m[FAILED]\033[0m\t"
#define PASSED_PROMPT "\t\033[0;30;32mPASSED\033[0m\t"

static inline int
assertest(int exp, const char *fmt, ...)
{
    va_list ap;
    char buffer[4096] = {0};

    va_start(ap, fmt);
    if (exp == 0)
    {
        strcpy(buffer, FAILED_PROMPT);
        vsnprintf(buffer + strlen(FAILED_PROMPT), sizeof(buffer) - strlen(buffer), fmt, ap);
    }
    else
    {
        strcpy(buffer, PASSED_PROMPT);
        vsnprintf(buffer + strlen(PASSED_PROMPT), sizeof(buffer) - strlen(buffer), fmt, ap);
    }
    va_end(ap);

    printf("%s\n", buffer);

    return 0;
}

#endif /* ! _ASSERTEST_H_ */
/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
