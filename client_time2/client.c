#include "func.h"
//服务器断开，客户端正常退出是这个


#define SERVER_PORT    12345
#define BUF_SIZE 4096
#define FILE_SIZE 4096
#define CWD_SIZE 4096

int getfile(int s,char file_name[FILE_SIZE]);
int sendfile(int s,char file_name[FILE_SIZE]);
int pwdser(int s);
int dirser(int s);
int cdser(int s,char file_name[FILE_SIZE]);
int helpcom(int s);
char buf[BUF_SIZE];
char cwd_buf[CWD_SIZE];
int length;
int offset;

int send_n(int sfd,char* buf,int len);

int recv_n(int sfd,char* buf,int len);
int removefile(int sfd,char *file_name);
void get_salt(char*,char*);

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		printf("./server IP PORT\n");
		return -1;
	}

	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	char *orders,*p,*words=" \n";
	char order_name[FILE_SIZE];
	char file_name[FILE_SIZE];

	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));//将端口转换为网络字节序
	ser.sin_addr.s_addr=inet_addr(argv[1]);//将点分十进制的ip地址转为32位的网络字节序
	int ret;
	ret=connect(sfd,(struct sockaddr*)&ser,sizeof(ser));
	if(-1==ret)
	{
		perror("connect");
		return -1;
	}
	int len;
	//登录用户
	struct spwd *sp;
	char uname[128]={0};
	char *passwd;
	char salt[1024]={0};
login:
	printf("请输入用户:\n");

	int	red=read(STDIN_FILENO,buf,sizeof(buf));
	strncpy(uname,buf,strlen(buf)-1);
	if(red<=0)
	{
		printf("read red=%d\n",red);
		return -1;
	}

	if(strcmp(uname,"")!=0){	
		passwd=getpass("请输入密码：");
	}else{
		goto login;
	}

	ret=send(sfd,uname,sizeof(uname),0);
	if(-1==ret)
	{
		perror("send");
		goto end;
	}
	ret=recv(sfd,salt,sizeof(salt),0);
	if(-1==ret)
	{
		perror("recv_salt");
		return -1;
	}
	char pass[1024]={0};	
	strcpy(pass,crypt(passwd,salt));
	ret=send(sfd,pass,sizeof(pass),0);
	if(-1==ret)
	{
		perror("send2");
		goto end;
	}
	bzero(buf,sizeof(buf));
	ret=recv(sfd,buf,sizeof(buf),0);//接收验证
	if(-1==ret)
	{
		perror("recv");
		return -1;
	}
	if(strcmp(buf,"ok")==0)
	{
		printf("登陆成功!\n");
	}else{
		printf("请重新输入!\n");
		goto login;
	}
	fd_set rdset;

	while(1)
	{
		FD_ZERO(&rdset);
		FD_SET(STDIN_FILENO,&rdset);
		FD_SET(sfd,&rdset);
		bzero(file_name, FILE_SIZE);
		bzero(order_name, FILE_SIZE);

		ret=select(sfd+1,&rdset,NULL,NULL,NULL);
		if(ret>0)
		{
again:		if(FD_ISSET(sfd,&rdset))//sfd可读
			{
				bzero(buf,sizeof(buf));
				ret=recv(sfd,buf,sizeof(buf),0);
				if(-1==ret)
				{
					perror("recv");
					return -1;
				}else if(0==ret)
				{
					printf("byebye\n");
					break;
				}
				printf("%s\n",buf);

			}
			if(FD_ISSET(0,&rdset))//标准输入可读
			{
				int flag=1;
				fgets(order_name,FILE_SIZE,stdin);

				orders=strtok(order_name,words);
				if(p=strtok(NULL,words))
					strcpy(file_name,p);
				if(strcmp(orders,"get")==0&&*file_name!='\0'){

					flag=getfile(sfd,file_name);}
				else if(strcmp(orders,"put")==0&&*file_name!='\0')
					flag=sendfile(sfd,file_name);
				else if(strcmp(orders,"cd")==0&&*file_name!='\0')
					flag=cdser(sfd,file_name);
				else if(strcmp(orders,"pwd")==0&&*file_name=='\0')
					flag=pwdser(sfd);
				else if(strcmp(orders,"ls")==0&&*file_name=='\0')
					flag=dirser(sfd);
				else if(strcmp(orders,"remove")==0&&*file_name!='\0')
					flag=removefile(sfd,file_name);
				else if((strcmp(orders,"help")==0||strcmp(orders,"?")==0)&&*file_name=='\0')
					flag=helpcom(sfd);
				else 
				{	
					printf("You wrote a wrong order, please write again!\n");

				}
				if(flag==0)
				{
					goto again;
				}
			}

		}
	}
end:	
	close(sfd);
	return 0;
}


int getfile(int s,char file_name[FILE_SIZE])
{
	char order_name[FILE_SIZE]="get&";
	int lengthgth = 0; 
	int write_lengthgth;
	strcat(order_name,file_name);

	bzero(buf,sizeof(buf));
	strncpy(buf, order_name, strlen(order_name)>BUF_SIZE?BUF_SIZE:strlen(order_name));
	if(send(s,order_name,strlen(order_name),0)<0)
	{
		printf("Send orders Failed\n");
		return 0;
	}
	bzero(cwd_buf,sizeof(cwd_buf));
	length=strlen(file_name);
	for(offset=length-1;offset>=0 && file_name[offset]!='/';offset--);
	offset++;
	getcwd(cwd_buf,sizeof(cwd_buf));
	strcat(cwd_buf,"/");
	strcat(cwd_buf,(char*)file_name+offset);
	bzero(buf,sizeof(buf));

	float f=0;
	int len=0;
	off_t f_size;
	int ret;
	ret=recv_n(s,(char*)&len,sizeof(len));
	if(-1==ret)
	{
		printf("server close\n");
		return 0;
	}
	printf("len=%d\n",len);
	ret= recv_n(s,(char*)&f_size,len);//接文件长度
	if(-1==ret)
	{	printf("server close\n");
		return 0;
	}
	printf("f_size=%ld\n",f_size);

	int fd= open(cwd_buf,O_RDWR|O_CREAT,0666);    
	if(-1==fd)
	{
		printf("Can not open '%s'\n", file_name);
		return 0;
	}
	time_t now,last;
	now=time(NULL);
	last=now;
	do{
		ret=recv_n(s,(char*)&len,sizeof(len));
		if(-1==ret)
		{
			printf("%5.2f%s\n",f/f_size*100,"%");
			break;
		}
		else if(len>0)
		{
			f=f+len;
			time(&now);
			if(now-last>=1)
			{
				printf("%5.2f%s\r",f/f_size*100,"%");
				fflush(stdout);
				last=now;
			}
			ret=recv_n(s,buf,len);
			write(fd,buf,len);
			if(-1==ret)
			{
				printf("%5.2f%s\n",f/f_size*100,"%");
				break;
			}
		}else{
			printf("          \r");
			printf("%d%s\n",100,"%");
			break;
		}
	}while(1);
	close(fd);

	return 1;
}

int sendfile(int s,char file_name[FILE_SIZE])
{
	char order_name[FILE_SIZE]="put&";
	int read_lengthgth;
	bzero(buf,BUF_SIZE);
	length=strlen(file_name);
	for(offset=length-1;offset>=0 && file_name[offset]!='/';offset--);
	offset++;	
	strcat(order_name,(char*)file_name+offset);
	bzero(buf,BUF_SIZE);
	strncpy(buf, order_name, strlen(order_name)>BUF_SIZE?BUF_SIZE:strlen(order_name));
	if(send(s,buf,BUF_SIZE,0)<0)
	{
		printf("Send order Failed\n");
		return 0;
	}
	FILE * fp = fopen(file_name,"r");
	if(NULL == fp)
		printf("File not found\n");
	else
	{
		bzero(buf, BUF_SIZE);
		while( (read_lengthgth = fread(buf,sizeof(char),BUF_SIZE,fp))>0)
		{
			if(send(s,buf,read_lengthgth,0)<0)
			{
				printf("Send '%s' Failed\n", file_name);
				return 0;
			}
			bzero(buf, BUF_SIZE);
		}
		fclose(fp);

		//    	send_n(s,buf,sizeof(buf));

		printf("Send '%s' Success!\n",file_name);
	}
	return 1;
}


int pwdser(int s)
{
	char pwdorder[]="pwd&";
	int lengthgth;
	if(send(s,pwdorder,sizeof(pwdorder),0)<0)
	{
		printf("Send orders Failed\n");
		exit(1);
	}
	bzero(buf,BUF_SIZE);
	lengthgth=recv(s,buf,BUF_SIZE,0);
	if(lengthgth < 0)
	{
		printf("Can not get server's dir!\n");
		exit(1);
	}
	printf("pwd:%s\n",buf);
	return 1;
}

int dirser(int s)
{
	char dirorder[]="ls&";
	int lengthgth=0;
	if(send(s,dirorder,sizeof(dirorder),0)<0)
	{
		printf("Send orders Failed!\n");
		return 0;
	}
	bzero(buf,BUF_SIZE);

	while(lengthgth=recv(s,buf,strlen(buf),0))
	{
		if(lengthgth<0)
		{
			printf("Error in getting server's dir!\n");
			return 0;
		}
		printf("%10s\n",buf);
		bzero(buf,BUF_SIZE);
	}	
	return 1;
}


int cdser(int s,char file_name[FILE_SIZE])
{
	char order_name[FILE_SIZE]="cd&";
	strcat(order_name,file_name);
	bzero(buf,BUF_SIZE);
	strncpy(buf, order_name, strlen(order_name)>BUF_SIZE?BUF_SIZE:strlen(order_name));
	if(send(s,buf,BUF_SIZE,0)<0)
	{
		printf("Send orders Failed\n");
		return 0;
	}
	return 1;
}


int helpcom(int s)
{

	char helporder[]="help&";
	if(send(s,helporder,sizeof(helporder),0)<0)
	{
		printf("Send orders Failed!\n");
		return 0;
	}
	printf("get      :get file from the server\n");
	printf("put      :send file to the server\n");
	printf("pwd      :the current directory of the server\n");
	printf("ls       :list the files in the current directory of the server\n");	
	printf("cd       :change the current directory of the server\n");		
	printf("?, help  :list all orderss\n");

	return 1;
}

int removefile(int s,char file_name[FILE_SIZE])
{
	char order_name[FILE_SIZE]="remove&";
	int lengthgth = 0; 
	int write_lengthgth;
	strcat(order_name,file_name);

	bzero(buf,BUF_SIZE);
	strncpy(buf, order_name, strlen(order_name)>BUF_SIZE?BUF_SIZE:strlen(order_name));

	if(send(s,buf,BUF_SIZE,0)<0)
	{
		printf("Send orders Failed\n");
		return 0;
	}
	recv(s,buf,strlen(buf),0);
	if(strcmp(buf,"ok")==0){
		printf("Remove success\n");
	}
	return 1;

}

void get_salt(char* salt,char* ciph)
{
	int i,j;
	for(i=0,j=0;ciph[i]!=0&&j!=3;i++)
	{
		if('$'==ciph[i])
		{
			j++;
		}
	}
	strncpy(salt,ciph,i-1);
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
