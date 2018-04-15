#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__
#include "head.h"
typedef struct tag_node
{
	int new_fd;
	int sfd;
	struct tag_node *pNext;
}node_t,*pnode_t;
typedef struct{
	pnode_t que_head,que_tail;
	int que_capacity;
	int que_size;
	pthread_mutex_t que_mutex;
}que_t,*pque_t;
void que_init(pque_t,int);
void que_set(pque_t,pnode_t);
void que_get(pque_t,pnode_t*);
#endif

