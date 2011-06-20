#ifndef __SIMPLEAV_CORE_UTIL_SAQUEUE_H_DEFINED__
#define __SIMPLEAV_CORE_UTIL_SAQUEUE_H_DEFINED__

typedef struct _SAQNode {
     void *data;
     struct _SAQNode *next;
} _SAQNode;

typedef struct SAQContext {
     _SAQNode *head, *tail;
     int nb;
} SAQContext;

SAQContext *SAQ_init(void);

int SAQ_push(SAQContext *, void *);

void *SAQ_pop(SAQContext *);

#endif
