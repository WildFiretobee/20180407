#include "factory.h"

int recv_n(int,char*,int);

void tran_file(int new_fd)
{
	signal(SIGPIPE,SIG_IGN);
	train d;
	char filename[128];

	int ret=recv_n(new_fd,filename,strlen(filename));//接收文件名	
	if(-1==ret)
	{
		perror("recv_n");
		goto end;
	}
	
	strcpy(d.buf,filename);
	d.len=strlen(d.buf);
	
	int fd;
	fd=open(filename,O_RDONLY);
	if(-1==fd)
	{
		perror("open");
		goto end;
	}
	struct stat buf;
	ret=fstat(fd,&buf);
	if(-1==ret)
	{
		perror("fstat");
		goto end;
	}
	d.len=sizeof(off_t);//发送文件长度
	memcpy(d.buf,&buf.st_size,d.len);
	send_n(new_fd,(char*)&d,4+d.len);
	
	while((d.len=read(fd,d.buf,sizeof(d.buf)))>0)//发送文件内容
	{
		ret=send_n(new_fd,(char*)&d,4+d.len);
		if(-1==ret)
		{
			goto end;
		}
	}
	d.len=0;
	send_n(new_fd,(char*)&d,4);//告诉客户端文件发送结束
end:
	close(new_fd);
}
