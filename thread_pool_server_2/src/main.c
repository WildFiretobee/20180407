#include "factory.h"
void tran_conf(char* line,char* arg);

void* thread_handle(void* p)
{
	pfac pf=(pfac)p;
	pque_t pq=&pf->que;
	pnode_t pcur;
	while(1)
	{
		pthread_mutex_lock(&pq->que_mutex);
		if(!pq->que_size)
		{
			pthread_cond_wait(&pf->cond,&pq->que_mutex);
		}
		que_get(pq,&pcur);
		pthread_mutex_unlock(&pq->que_mutex);

		tran_chat(pcur->new_fd,pcur->sfd);

		free(pcur);
	}
}
int main(int argc,char* argv[])
{
	if(argc!=2)
	{
		printf("./server path\n");
		return -1;
	}
	fac f;//主要数据结构
	bzero(&f,sizeof(f));
	char ip[128]="";
	char port[128]="";
	char thread_num[128]="";
	char thread_capacity[128]="";

	FILE *fp_conf=fopen(argv[1],"r");//从文件中读取数据
	if(NULL==fp_conf)
	{
		perror("fopen");
		return -1;
	}
	char line[128];
	
	bzero(line,sizeof(line));
	fgets(line,128,fp_conf);
	line[strlen(line)-1]='\0';
	tran_conf(line,ip);

	bzero(line,sizeof(line));
	fgets(line,128,fp_conf);
	line[strlen(line)-1]='\0';
	tran_conf(line,port);

	bzero(line,sizeof(line));
	fgets(line,128,fp_conf);
	line[strlen(line)-1]='\0';
	tran_conf(line,thread_num);

	bzero(line,sizeof(line));
	fgets(line,128,fp_conf);
	line[strlen(line)-1]='\0';
	tran_conf(line,thread_capacity);

	printf("%s %s %s %s\n",ip,port,thread_num,thread_capacity);
	f.pthread_num=atoi(thread_num);
	int capacity=atoi(thread_capacity);
	factory_init(&f,thread_handle,capacity);
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	int reuse=1;
	int ret;
	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	if(-1==ret)
	{
		perror("setsockopt");
		return -1;
	}
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(port));//将端口转换为网络字节序
	ser.sin_addr.s_addr=inet_addr(ip);//将点分十进制的ip地址转为32位的网络字节序
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(ser));
	if(-1==ret)
	{
		perror("bind");
		return -1;
	}

	factory_start(&f);
	listen(sfd,100);	//激活
	int new_fd;
	pnode_t pnew;
	pque_t pq=&f.que;
	while(1)
	{
		new_fd=accept(sfd,NULL,NULL);
		pnew=(pnode_t)calloc(1,sizeof(node_t));
		pnew->new_fd=new_fd;
		pnew->sfd=sfd;

		pthread_mutex_lock(&pq->que_mutex);
		que_set(pq,pnew);
		pthread_mutex_unlock(&pq->que_mutex);

		pthread_cond_signal(&f.cond);//来一个任务让条件变量成立一次
	}
}

void tran_conf(char* line,char* arg)
{
	char* ptr = strchr(line, '=');
	if(ptr == NULL)
	{
		perror("strchr");
		return;
	}
	strcpy(arg, ptr + 1);
}
