
#ifndef _NSHASH_H_
#define _NSHASH_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

uint32_t nshash1(const void *buf, size_t len);
uint32_t nshash2(const void *buf, size_t len);

#endif /* ! _NSHASH_H_ */
