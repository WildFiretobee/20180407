#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
typedef void* (*pfunc)(void*);
typedef struct{
	pthread_t *pthid;//�߳�id�洢����ʼ��ַ
	pthread_cond_t cond;
	que_t que;
	int pthread_num;//�߳���Ŀ
	pfunc thread_func;//�̺߳���
}fac,*pfac;
#endif