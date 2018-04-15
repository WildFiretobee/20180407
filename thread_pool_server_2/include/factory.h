#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
typedef void* (*pfunc)(void*);
typedef struct{
	pthread_t *pthid;//线程id存储的起始地址
	pthread_cond_t cond;
	que_t que;
	int pthread_num;//线程数目
	pfunc thread_func;//线程函数
}fac,*pfac;
#endif