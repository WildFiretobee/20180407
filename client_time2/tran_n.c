#include "func.h"

int send_n(int sfd,char* buf,int len)
{
	int total=0;
	int ret;
	while(total<len)
	{
		ret=send(sfd,buf+total,len-total,0);
		total=total+ret;
	}
	return 0;
}


int recv_n(int sfd,char* buf,int len)
{
	int total=0;
	int ret;
	while(total<len)
	{
		ret=recv(sfd,buf+total,len-total,0);
		if(0==ret)
		{
			printf("server close\n");
			return -1;
		}
		total=total+ret;
	}
	return 0;
}
