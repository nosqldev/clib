clib
====

Including some basic libraries for C developing.

assertest
---------
Only a header file, to provide a simple unit test tools for tiny project.

Also used in this repo.

bsdqueue
--------
This is a single linked list in fact, which is migrated from kernel of FreeBSD.

However, I rewritten the interfaces to for using easily.

I cannot find where the testsuites for this lib are.

cexception
----------
An implementation of exception in C achieved through setjmp(3) & longjmp(3).

This lib is thread-safe by using of Thread Local Storage.

conhash
-------
A high performance of hash table for concurrent operations(read & write).

This is half done since delete/update is not finished.

Refer to the testsuites for how to use it.

darray (Dynamic Array)
----------------------
This is a fast dynamic array implementation.

You are free to access any postion in the array without realloc manually.

Data items are grouped by memory block first, then linked these blocks.

Have a look at the testsuites for more information.

log
---
A lightweight implementation of log library.

slab
----
A simple memory allocator implemented of slab algorithm.

Implementing a memory allocator can accelerate the speed of complex system as
well as debug the memory operations.

The interfaces are the same as malloc(3)/free(3), so it's easy to replace them.

However, it's a simple implementation so that not considering NUMA.

timeutil
--------
Providing some time datatype related functions.
