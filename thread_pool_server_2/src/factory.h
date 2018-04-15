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
	int start_flag;//�߳�������־
	pfunc thread_func;//�̺߳���
}fac,*pfac;
typedef struct{
	int len;//��������
	char buf[1000];
}train;

void factory_init(pfac,pfunc,int);
void factory_start(pfac);

void tran_file(int);
void tran_chat(int,int);

int login_check(char*,char*,char*);
int send_n(int sfd,char* buf,int len);

#endif
