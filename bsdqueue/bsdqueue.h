/*
 * +-----------------------------------------------------------------------+
 * | Generic Single-Linked-List and Double-Linked-List                     |
 * |                                                                       |
 * | Migrates from FreeBSD 6.2 Kernel Source Code.                         |
 * +-----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                              |
 * +-----------------------------------------------------------------------+
 *
 * $Id: bsdqueue.h,v 1.5 2008/10/27 07:42:34 jingmi Exp $
 */

#ifndef _BSDQUEUE_H_
#define	_BSDQUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif 

#include <fcntl.h>
#include <unistd.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <pthread.h>

#ifndef __offset_of
#define	__offset_of(type, field)	((size_t)(&((type *)0)->field))
#endif

#ifndef mem_calloc
  #ifdef SLAB_CALLOC
    #define mem_calloc slab_calloc
  #else
    #define mem_calloc malloc
  #endif
#endif

#ifndef mem_free
  #ifdef SLAB_FREE
    #define mem_free slab_free
  #else
    #define mem_free free
  #endif
#endif

/* ==================== USER INTERFACE ==================== */

/* single linked list */
#define list_def(head_struct, item_struct, head) \
    STAILQ_HEAD(head_struct, item_struct) head = STAILQ_HEAD_INITIALIZER(head); \
    STAILQ_INIT(&head)
#define list_new(head_ptr, head_struct) \
    struct head_struct *head_ptr = mem_calloc(sizeof(struct head_struct));  \
    STAILQ_INIT(head_ptr)

#define list_size(head_ptr) ((*(head_ptr)).size)

#define list_next_ptr(item_struct)   STAILQ_ENTRY(item_struct) LINK_NEXT_PTR

#define list_push(head_ptr, item_ptr)  STAILQ_INSERT_TAIL(head_ptr, item_ptr, LINK_NEXT_PTR)
#define list_unshift(head_ptr, item_ptr) STAILQ_INSERT_HEAD(head_ptr, item_ptr, LINK_NEXT_PTR)
#define list_insert(head_ptr, pos_ptr, item_ptr) STAILQ_INSERT_AFTER(head_ptr, pos_ptr, item_ptr, LINK_NEXT_PTR)
#define list_shift(head_ptr, item_ptr)    do {    \
    item_ptr = list_first(head_ptr);    \
    list_remove_head(head_ptr);         \
} while (0)
#define list_pop(head_ptr, item_ptr, item_struct)    do {    \
    item_ptr = list_last(head_ptr, item_struct);    \
    list_remove(head_ptr, item_ptr, item_struct);   \
} while (0)

#define list_first(head_ptr) STAILQ_FIRST(head_ptr)
#define list_last(head_ptr, item_struct) STAILQ_LAST(head_ptr, item_struct, LINK_NEXT_PTR)

#define list_foreach(head_ptr, item_ptr)    STAILQ_FOREACH(item_ptr, head_ptr, LINK_NEXT_PTR)
#define list_foreach_safe(head_ptr, item_ptr, temp_ptr) STAILQ_FOREACH_SAFE(item_ptr, head_ptr, LINK_NEXT_PTR, temp_ptr)
#define list_next(item_ptr) STAILQ_NEXT(item_ptr, LINK_NEXT_PTR)
#define list_addr(head_ptr, n, item_ptr)    do { \
    unsigned int m = (unsigned int)(n + 1);      \
    if (m > list_size(head_ptr)) {  \
        item_ptr = NULL;    break;  \
    }                               \
    item_ptr = list_first(head_ptr);    \
    while (--m != 0) {           \
        item_ptr = list_next(item_ptr); \
    }   \
} while (0)

#define list_empty(head_ptr) STAILQ_EMPTY(head_ptr)

#define list_cat(head1, head2)  STAILQ_CONCAT(head1, head2)

#define list_remove(head_ptr, item_ptr, item_struct) STAILQ_REMOVE(head_ptr, item_ptr, item_struct, LINK_NEXT_PTR)
#define list_remove_head(head_ptr)  STAILQ_REMOVE_HEAD(head_ptr, LINK_NEXT_PTR)
#define list_destroy(head_ptr, item_ptr, item_struct) do{   \
    list_remove(head_ptr, item_ptr, item_struct);   \
    mem_free(item_ptr);                        \
} while (0)
#define list_destroy_head(head_ptr)    do{ \
    void *first_ptr = (void *)list_first(head_ptr);  \
    list_remove_head(head_ptr);             \
    mem_free(first_ptr);               \
} while (0)
#define list_destroy_with_head(head_ptr) do {    \
    while (list_first(head_ptr) != NULL)    {           \
        list_destroy_head(head_ptr);                    \
    }                                                   \
    mem_free(head_ptr);                                \
    head_ptr = NULL;                                    \
} while (0)
#define list_destroy_without_head(head_ptr) do {    \
    while (list_first(head_ptr) != NULL)    {           \
        list_destroy_head(head_ptr);                    \
    }                                                   \
} while (0)

/** 
 * @brief Dump list to datafile.
 * 
 * @param head_ptr Head of the list.
 * @param item_ptr Temporary pointer to a node.
 * @param get_node_size_fun Function to get every size of node.
 * @param filename Name of file to store list.
 * @param retval 0 means write successfully
 *               1 means Open file error
 *               2 means write(2) head_ptr error
 *               3 means writev(2) node error
 *               4 means close(2) error
 * 
 */
#define list_dump_file(head_ptr, item_ptr, get_node_size_fun, filename, retval)   do  {   \
    struct iovec iov[64];                                                      \
    int fd, n;                                                                  \
    retval = fd = n = 0;                                                        \
    if ((fd = open(filename, O_EXCL|O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {  \
        retval = 1; /* Open file error */                                       \
        break;                                                                  \
    }                                                                           \
    if (write(fd, head_ptr, sizeof(*head_ptr)) < 0) {                           \
        retval = 2;                                                             \
        close(fd);                                                              \
        break;                                                                  \
    }                                                                           \
    item_ptr = list_first(head_ptr);                                            \
    while (item_ptr != NULL) {                                                  \
        for (n=0; n<(int)(sizeof(iov)/sizeof(iov[0])); n++) {                   \
            if (item_ptr == NULL) break;                                        \
            iov[n].iov_base = item_ptr;                                         \
            iov[n].iov_len = get_node_size_fun(item_ptr);                       \
            item_ptr = list_next(item_ptr);                                     \
        }                                                                       \
        if (writev(fd, iov, n) < 0) {                                           \
            retval = 3;                                                         \
            break;                                                              \
        }                                                                       \
    }                                                                           \
    if (close(fd) < 0) { retval = 4; break; }                                   \
} while (0)

/** 
 * @brief Restore list from data file.
 * 
 * @param head_ptr Head of the list.
 * @param item_struct Struct of the node.
 * @param get_node_fun Function to read one node.
 * @param filename Name of data file.
 * @param retval 0 means ok;
 *               1 means cannot open file;
 *               2 means cannot read head info;
 *               3 means cannot close file.
 * 
 */
#define list_restore_file(head_ptr, item_struct, get_node_fun, filename, retval)  do { \
    int fd, n;                                                           \
    struct item_struct *item_ptr;                                        \
    retval = fd = n = 0;                                                 \
    if ((fd=open(filename, O_RDONLY)) < 0) { retval = 1; break; }        \
    if (read(fd, head_ptr, sizeof(*head_ptr)) != sizeof(*head_ptr)) {    \
        retval = 2; close(fd); break;                                    \
    }                                                                    \
    STAILQ_INIT(head_ptr);                                               \
    while ((item_ptr=get_node_fun(fd)) != NULL) {                        \
        list_push(head_ptr, item_ptr);                                   \
    }                                                                    \
    if (close(fd) < 0) { retval = 3; break; }                            \
} while (0)

#define list_dump_mem()

#define list_restore_mem()

/* ==================== SYSTEM IMPLEMENTATION ==================== */
#define	QUEUE_MACRO_DEBUG 0
#if QUEUE_MACRO_DEBUG
/* Store the last 2 places the queue element or head was altered */
struct qm_trace {
	char * lastfile;
	int lastline;
	char * prevfile;
	int prevline;
};

#define	TRACEBUF	struct qm_trace trace;
#define	TRASHIT(x)	do {(x) = (void *)-1;} while (0)

#define	QMD_TRACE_HEAD(head) do {					\
	(head)->trace.prevline = (head)->trace.lastline;		\
	(head)->trace.prevfile = (head)->trace.lastfile;		\
	(head)->trace.lastline = __LINE__;				\
	(head)->trace.lastfile = __FILE__;				\
} while (0)

#define	QMD_TRACE_ELEM(elem) do {					\
	(elem)->trace.prevline = (elem)->trace.lastline;		\
	(elem)->trace.prevfile = (elem)->trace.lastfile;		\
	(elem)->trace.lastline = __LINE__;				\
	(elem)->trace.lastfile = __FILE__;				\
} while (0)

#else
#define	QMD_TRACE_ELEM(elem)
#define	QMD_TRACE_HEAD(head)
#define	TRACEBUF
#define	TRASHIT(x)
#endif	/* QUEUE_MACRO_DEBUG */

/*
 * Singly-linked List declarations.
 */
#define	SLIST_HEAD(name, type)						\
struct name {								\
	struct type *slh_first;	/* first element */			\
}

#define	SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define	SLIST_ENTRY(type)						\
struct {								\
	struct type *sle_next;	/* next element */			\
}

/*
 * Singly-linked List functions.
 */
#define	SLIST_EMPTY(head)	((head)->slh_first == NULL)

#define	SLIST_FIRST(head)	((head)->slh_first)

#define	SLIST_FOREACH(var, head, field)					\
	for ((var) = SLIST_FIRST((head));				\
	    (var);							\
	    (var) = SLIST_NEXT((var), field))

#define	SLIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = SLIST_FIRST((head));				\
	    (var) && ((tvar) = SLIST_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	SLIST_FOREACH_PREVPTR(var, varp, head, field)			\
	for ((varp) = &SLIST_FIRST((head));				\
	    ((var) = *(varp)) != NULL;					\
	    (varp) = &SLIST_NEXT((var), field))

#define	SLIST_INIT(head) do {						\
	SLIST_FIRST((head)) = NULL;					\
} while (0)

#define	SLIST_INSERT_AFTER(slistelm, elm, field) do {			\
	SLIST_NEXT((elm), field) = SLIST_NEXT((slistelm), field);	\
	SLIST_NEXT((slistelm), field) = (elm);				\
} while (0)

#define	SLIST_INSERT_HEAD(head, elm, field) do {			\
	SLIST_NEXT((elm), field) = SLIST_FIRST((head));			\
	SLIST_FIRST((head)) = (elm);					\
} while (0)

#define	SLIST_NEXT(elm, field)	((elm)->field.sle_next)

#define	SLIST_REMOVE(head, elm, type, field) do {			\
	if (SLIST_FIRST((head)) == (elm)) {				\
		SLIST_REMOVE_HEAD((head), field);			\
	}								\
	else {								\
		struct type *curelm = SLIST_FIRST((head));		\
		while (SLIST_NEXT(curelm, field) != (elm))		\
			curelm = SLIST_NEXT(curelm, field);		\
		SLIST_NEXT(curelm, field) =				\
		    SLIST_NEXT(SLIST_NEXT(curelm, field), field);	\
	}								\
} while (0)

#define	SLIST_REMOVE_HEAD(head, field) do {				\
	SLIST_FIRST((head)) = SLIST_NEXT(SLIST_FIRST((head)), field);	\
} while (0)

/*
 * Singly-linked Tail queue declarations.
 */
#define	STAILQ_HEAD(name, type)						\
struct name {								\
	struct type *stqh_first;/* first element */			\
	struct type **stqh_last;/* addr of last next element */		\
    pthread_mutex_t lock;                                       \
    unsigned int size;                                         \
}

#define	STAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).stqh_first, PTHREAD_MUTEX_INITIALIZER, 0,}

#define	STAILQ_ENTRY(type)						\
struct {								\
	struct type *stqe_next;	/* next element */			\
}

/*
 * Singly-linked Tail queue functions.
 */
#define	STAILQ_CONCAT(head1, head2) do {				\
	if (!STAILQ_EMPTY((head2))) {					\
		*(head1)->stqh_last = (head2)->stqh_first;		\
		(head1)->stqh_last = (head2)->stqh_last;		\
        (head1)->size += (head2)->size;         \
		STAILQ_INIT((head2));					\
	}								\
} while (0)

#define	STAILQ_EMPTY(head)	((head)->stqh_first == NULL)

#define	STAILQ_FIRST(head)	((head)->stqh_first)

#define	STAILQ_FOREACH(var, head, field)				\
	for((var) = STAILQ_FIRST((head));				\
	   (var);							\
	   (var) = STAILQ_NEXT((var), field))


#define	STAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = STAILQ_FIRST((head));				\
	    (var) && ((tvar) = STAILQ_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	STAILQ_INIT(head) do {						\
	STAILQ_FIRST((head)) = NULL;					\
	(head)->stqh_last = &STAILQ_FIRST((head));			\
    (head)->lock = PTHREAD_MUTEX_INITIALIZER;           \
    (head)->size = 0;                               \
} while (0)

#define	STAILQ_INSERT_AFTER(head, tqelm, elm, field) do {		\
	if ((STAILQ_NEXT((elm), field) = STAILQ_NEXT((tqelm), field)) == NULL)\
		(head)->stqh_last = &STAILQ_NEXT((elm), field);		\
	STAILQ_NEXT((tqelm), field) = (elm);				\
    (head)->size ++;                           \
} while (0)

#define	STAILQ_INSERT_HEAD(head, elm, field) do {			\
	if ((STAILQ_NEXT((elm), field) = STAILQ_FIRST((head))) == NULL)	\
		(head)->stqh_last = &STAILQ_NEXT((elm), field);		\
	STAILQ_FIRST((head)) = (elm);					\
    (head)->size ++;                           \
} while (0)

#define	STAILQ_INSERT_TAIL(head, elm, field) do {			\
	STAILQ_NEXT((elm), field) = NULL;				\
	*(head)->stqh_last = (elm);					\
	(head)->stqh_last = &STAILQ_NEXT((elm), field);			\
    (head)->size ++;                           \
} while (0)

#define	STAILQ_LAST(head, type, field)					\
	(STAILQ_EMPTY((head)) ?						\
		NULL :							\
	        ((struct type *)					\
		((char *)((head)->stqh_last) - __offset_of(struct type, field))))

#define	STAILQ_NEXT(elm, field)	((elm)->field.stqe_next)

#define	STAILQ_REMOVE(head, elm, type, field) do {			\
	if (STAILQ_FIRST((head)) == (elm)) {				\
		STAILQ_REMOVE_HEAD((head), field);			\
	}								\
	else {								\
		struct type *curelm = STAILQ_FIRST((head));		\
		while (STAILQ_NEXT(curelm, field) != (elm))		\
			curelm = STAILQ_NEXT(curelm, field);		\
		if ((STAILQ_NEXT(curelm, field) =			\
		     STAILQ_NEXT(STAILQ_NEXT(curelm, field), field)) == NULL)\
			(head)->stqh_last = &STAILQ_NEXT((curelm), field);\
        (head)->size --;                                        \
	}								\
} while (0)

#define	STAILQ_REMOVE_HEAD(head, field) do {				\
	if ((STAILQ_FIRST((head)) =					\
	     STAILQ_NEXT(STAILQ_FIRST((head)), field)) == NULL)		\
		(head)->stqh_last = &STAILQ_FIRST((head));		\
        (head)->size --;                       \
} while (0)

#define	STAILQ_REMOVE_HEAD_UNTIL(head, elm, field) do {			\
	if ((STAILQ_FIRST((head)) = STAILQ_NEXT((elm), field)) == NULL)	\
		(head)->stqh_last = &STAILQ_FIRST((head));		\
} while (0)

/*
 * List declarations.
 */
#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}

#define	LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define	LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;	/* next element */			\
	struct type **le_prev;	/* address of previous next element */	\
}

/*
 * List functions.
 */

#define	LIST_EMPTY(head)	((head)->lh_first == NULL)

#define	LIST_FIRST(head)	((head)->lh_first)

#define	LIST_FOREACH(var, head, field)					\
	for ((var) = LIST_FIRST((head));				\
	    (var);							\
	    (var) = LIST_NEXT((var), field))

#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = LIST_FIRST((head));				\
	    (var) && ((tvar) = LIST_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	LIST_INIT(head) do {						\
	LIST_FIRST((head)) = NULL;					\
} while (0)

#define	LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if ((LIST_NEXT((elm), field) = LIST_NEXT((listelm), field)) != NULL)\
		LIST_NEXT((listelm), field)->field.le_prev =		\
		    &LIST_NEXT((elm), field);				\
	LIST_NEXT((listelm), field) = (elm);				\
	(elm)->field.le_prev = &LIST_NEXT((listelm), field);		\
} while (0)

#define	LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	LIST_NEXT((elm), field) = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &LIST_NEXT((elm), field);		\
} while (0)

#define	LIST_INSERT_HEAD(head, elm, field) do {				\
	if ((LIST_NEXT((elm), field) = LIST_FIRST((head))) != NULL)	\
		LIST_FIRST((head))->field.le_prev = &LIST_NEXT((elm), field);\
	LIST_FIRST((head)) = (elm);					\
	(elm)->field.le_prev = &LIST_FIRST((head));			\
} while (0)

#define	LIST_NEXT(elm, field)	((elm)->field.le_next)

#define	LIST_REMOVE(elm, field) do {					\
	if (LIST_NEXT((elm), field) != NULL)				\
		LIST_NEXT((elm), field)->field.le_prev = 		\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = LIST_NEXT((elm), field);		\
} while (0)

/*
 * Tail queue declarations.
 */
#define	TAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
	TRACEBUF							\
}

#define	TAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define	TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
	TRACEBUF							\
}

/*
 * Tail queue functions.
 */
#define	TAILQ_CONCAT(head1, head2, field) do {				\
	if (!TAILQ_EMPTY(head2)) {					\
		*(head1)->tqh_last = (head2)->tqh_first;		\
		(head2)->tqh_first->field.tqe_prev = (head1)->tqh_last;	\
		(head1)->tqh_last = (head2)->tqh_last;			\
		TAILQ_INIT((head2));					\
		QMD_TRACE_HEAD(head1);					\
		QMD_TRACE_HEAD(head2);					\
	}								\
} while (0)

#define	TAILQ_EMPTY(head)	((head)->tqh_first == NULL)

#define	TAILQ_FIRST(head)	((head)->tqh_first)

#define	TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))

#define	TAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = TAILQ_FIRST((head));				\
	    (var) && ((tvar) = TAILQ_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = TAILQ_LAST((head), headname);			\
	    (var);							\
	    (var) = TAILQ_PREV((var), headname, field))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)	\
	for ((var) = TAILQ_LAST((head), headname);			\
	    (var) && ((tvar) = TAILQ_PREV((var), headname, field), 1);	\
	    (var) = (tvar))

#define	TAILQ_INIT(head) do {						\
	TAILQ_FIRST((head)) = NULL;					\
	(head)->tqh_last = &TAILQ_FIRST((head));			\
	QMD_TRACE_HEAD(head);						\
} while (0)

#define	TAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if ((TAILQ_NEXT((elm), field) = TAILQ_NEXT((listelm), field)) != NULL)\
		TAILQ_NEXT((elm), field)->field.tqe_prev = 		\
		    &TAILQ_NEXT((elm), field);				\
	else {								\
		(head)->tqh_last = &TAILQ_NEXT((elm), field);		\
		QMD_TRACE_HEAD(head);					\
	}								\
	TAILQ_NEXT((listelm), field) = (elm);				\
	(elm)->field.tqe_prev = &TAILQ_NEXT((listelm), field);		\
	QMD_TRACE_ELEM(&(elm)->field);					\
	QMD_TRACE_ELEM(&listelm->field);				\
} while (0)

#define	TAILQ_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	TAILQ_NEXT((elm), field) = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &TAILQ_NEXT((elm), field);		\
	QMD_TRACE_ELEM(&(elm)->field);					\
	QMD_TRACE_ELEM(&listelm->field);				\
} while (0)

#define	TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if ((TAILQ_NEXT((elm), field) = TAILQ_FIRST((head))) != NULL)	\
		TAILQ_FIRST((head))->field.tqe_prev =			\
		    &TAILQ_NEXT((elm), field);				\
	else								\
		(head)->tqh_last = &TAILQ_NEXT((elm), field);		\
	TAILQ_FIRST((head)) = (elm);					\
	(elm)->field.tqe_prev = &TAILQ_FIRST((head));			\
	QMD_TRACE_HEAD(head);						\
	QMD_TRACE_ELEM(&(elm)->field);					\
} while (0)

#define	TAILQ_INSERT_TAIL(head, elm, field) do {			\
	TAILQ_NEXT((elm), field) = NULL;				\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &TAILQ_NEXT((elm), field);			\
	QMD_TRACE_HEAD(head);						\
	QMD_TRACE_ELEM(&(elm)->field);					\
} while (0)

#define	TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define	TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define	TAILQ_REMOVE(head, elm, field) do {				\
	if ((TAILQ_NEXT((elm), field)) != NULL)				\
		TAILQ_NEXT((elm), field)->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else {								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
		QMD_TRACE_HEAD(head);					\
	}								\
	*(elm)->field.tqe_prev = TAILQ_NEXT((elm), field);		\
	TRASHIT((elm)->field.tqe_next);					\
	TRASHIT((elm)->field.tqe_prev);					\
	QMD_TRACE_ELEM(&(elm)->field);					\
} while (0)


#ifdef _KERNEL

/*
 * XXX insque() and remque() are an old way of handling certain queues.
 * They bogusly assumes that all queue heads look alike.
 */

struct quehead {
	struct quehead *qh_link;
	struct quehead *qh_rlink;
};

#ifdef __CC_SUPPORTS___INLINE

static __inline void
insque(void *a, void *b)
{
	struct quehead *element = (struct quehead *)a,
		 *head = (struct quehead *)b;

	element->qh_link = head->qh_link;
	element->qh_rlink = head;
	head->qh_link = element;
	element->qh_link->qh_rlink = element;
}

static __inline void
remque(void *a)
{
	struct quehead *element = (struct quehead *)a;

	element->qh_link->qh_rlink = element->qh_rlink;
	element->qh_rlink->qh_link = element->qh_link;
	element->qh_rlink = 0;
}

#else /* !__CC_SUPPORTS___INLINE */

void	insque(void *a, void *b);
void	remque(void *a);

#endif /* __CC_SUPPORTS___INLINE */

#endif /* _KERNEL */

#ifdef __cplusplus
}
#endif 

#endif /* !_SYS_QUEUE_H_ */

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
