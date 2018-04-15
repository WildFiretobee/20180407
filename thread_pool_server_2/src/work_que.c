#include "work_que.h"

void que_init(pque_t pq,int cap)
{
	pq->que_capacity=cap;
	pq->que_size=0;
	pthread_mutex_init(&pq->que_mutex,NULL);
}
//将节点放入队列
void que_set(pque_t pq,pnode_t pnew)
{
	if(!pq->que_head)
	{
		pq->que_head=pnew;
		pq->que_tail=pnew;
	}else{
		pq->que_tail->pNext=pnew;
		pq->que_tail=pnew;
	}
	pq->que_size++;
}

void que_get(pque_t pq,pnode_t* p)
{
	*p=pq->que_head;
	pq->que_head=pq->que_head->pNext;
	pq->que_size--;
}