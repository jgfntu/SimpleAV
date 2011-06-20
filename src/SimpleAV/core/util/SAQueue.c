#include "SimpleAV/core/util/SAQueue.h"

#include <stdlib.h>

SAQContext *SAQ_init(void)
{
     SAQContext *p = (SAQContext *)malloc(sizeof(SAQContext));
     if(p == NULL)
          return p;
     
     p->head = p->tail = NULL;
     p->nb = 0;
     return p;
}

int SAQ_push(SAQContext *saq_ptr, void *data)
{
     _SAQNode *node_ptr = (_SAQNode *)malloc(sizeof(_SAQNode));
     if(node_ptr == NULL)
          return -1;
     
     node_ptr->next = NULL;
     node_ptr->data = data;
     if(saq_ptr->tail != NULL)
          saq_ptr->tail->next = node_ptr;
     saq_ptr->tail = node_ptr;
     if(saq_ptr->head == NULL)
          saq_ptr->head = node_ptr;
     saq_ptr->nb++;
     return 0;
}

void *SAQ_pop(SAQContext *saq_ptr)
{
     if(saq_ptr->nb == 0)
          return NULL;
     
     _SAQNode *head_ptr = saq_ptr->head;
     void *data = head_ptr->data;

     if(saq_ptr->nb == 1)
          saq_ptr->tail = NULL;
     saq_ptr->head = head_ptr->next;

     saq_ptr->nb--;
     
     free(head_ptr);
     return data;
}
