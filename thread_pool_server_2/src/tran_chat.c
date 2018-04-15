#include "factory.h"

#define SERVER_PORT    12345
#define BUF_SIZE 4096
#define FILE_SIZE 4096
#define CWD_SIZE 4096

#define SERVER_PORT    12345
#define BUF_SIZE 4096
#define FILE_SIZE 4096
#define CWD_SIZE 4096
#define QUEUE_SIZE 10

int getfile(int new_sock,char file_name[FILE_SIZE]);
int sendfile(int new_sock,char file_name[FILE_SIZE]);
int removefile(int s,char file_name[FILE_SIZE]);
int quitser();
int cdser(char file_name[FILE_SIZE]);
int pwdser(int new_sock);
int dirser(int new_sock);
int	login_check(char*,char*,char*);
int history_client(char*);

char buf[BUF_SIZE];
char cwd_buf[CWD_SIZE];

void tran_chat(int new_sock,int s)
{

	char *orders,*chp,*words="&";

	char file_name[FILE_SIZE];

	//登录	
	char uname[128]={0};
	char pass[1024]={0};
	char passwd[1024]={0};
	char salt[512]={0};
	char flag[128]={0};

	int loginret;
login:	
	loginret=recv(new_sock,uname,sizeof(uname),0);
	if(-1==loginret)
	{
		perror("recv");
		goto end;
	}

	login_check(uname,salt,pass);//根据用户名查询盐值和密文
	loginret=send(new_sock,salt,sizeof(salt),0);
	if(-1==loginret)
	{
		perror("send_salt");
		goto end;
	}
	loginret=recv(new_sock,passwd,sizeof(passwd),0);
	if(-1==loginret)
	{
		perror("recv2");
		goto end;
	}
	if(strcmp(pass,passwd)==0)//密文比较
	{
		loginret=send(new_sock,"ok",2,0);
		if(-1==loginret)
		{
			perror("send1");
			goto end;
		}
		time_t t;
		t=time(NULL);
		char stime[128]={0};
		strncpy(stime,ctime(&t),strlen(ctime(&t)));

		history_client(uname);
		strcpy(flag,"/040/");
		history_client(flag);
		history_client(stime);
	}else{
		loginret=send(new_sock,"worry",5,0);
		if(-1==loginret)
		{
			perror("send1");
			goto end;
		}
		goto login;
	}
	//接收命令

	while (1) 
	{
again:       
		bzero(buf, sizeof(buf));
		int length = recv(new_sock,buf,sizeof(buf),0);
		if (length < 0)
		{
			printf("Can not recieve data from client!\n");
			break;
		}
		if(strcmp(buf,"")==0){
			goto again;
		}
		time_t t2;
		t2=time(NULL);
		char stime2[128]={0};
		strncpy(stime2,ctime(&t2),strlen(ctime(&t2))-1);

		history_client(flag);
		history_client(buf);
		history_client(flag);
		history_client(ctime(&t2));

		chp=buf; 
		orders=strsep(&chp,words);
		char *strsep(char **stringp, const char *delim);

		if(chp=strsep(&chp,words));
		strcpy(file_name,chp);
		if(strcmp(orders,"quit")==0)
			quitser();
		else if(strcmp(orders,"cd")==0)
			cdser(file_name);
		else if(strcmp(orders,"pwd")==0)
			pwdser(new_sock);
		else if(strncmp(orders,"ls",2)==0)
			dirser(new_sock);
		else if(strcmp(orders,"help")==0){}
		else if(strcmp(orders,"get")==0&&*chp!='\0')
			getfile(new_sock,file_name);
		else if(strcmp(orders,"put")==0&&*chp!='\0')
			sendfile(new_sock,file_name);
		else if(strcmp(orders,"remove")==0&&*chp!='\0')
			removefile(new_sock,file_name);
		
	}
end:
	close(new_sock);
	close(s);
	return ;
}

int getfile(int new_sock,char file_name[FILE_SIZE])
{
	train d;

	int fd = open(file_name,O_RDONLY);
	if(-1 == fd)
	{
		printf("'%s' is not found!\n", file_name);
		return 0;
	}
	struct stat buff;
	int rett=fstat(new_sock,&buff);
	if(-1==rett)
	{
		perror("fstat");
		return 0;
	}
	d.len=sizeof(off_t);//发送文件长度
	memcpy(d.buf,&buff.st_size,d.len);
	send_n(new_sock,(char*)&d,4+d.len);

	while( (d.len = read(fd,d.buf,sizeof(d.buf)))>0)
	{
			int yyy=send_n(new_sock,(char*)&d,4+d.len);
			if(-1==yyy)
			{
				printf("Can not send '%s' to client\n", file_name);
				return 0;
			}
		}
		d.len=0;
		send_n(new_sock,(char*)&d,4);

		printf("Send '%s' to client success!\n",file_name);

	return 0;
}

int sendfile(int new_sock,char file_name[FILE_SIZE])
{
	int length;
	int write_length;
	bzero(buf,BUF_SIZE);
	length = recv(new_sock,buf,BUF_SIZE,0);
	if(length>0)
	{
		FILE * fp = fopen(file_name,"w");    
		if(NULL == fp )
		{
			printf("'%s' Can not open\n", file_name);
			exit(1);
		}
		do{
			if(length < 0)
			{
				printf("Can not recieve data from client!\n");
				break;
			}
			write_length = fwrite(buf,sizeof(char),strlen(buf),fp);
			if (write_length<strlen(buf))
			{
				printf("can not write '%s'\n", file_name);
				break;
			}
			bzero(buf,BUF_SIZE);    
		}while( length = recv(new_sock,buf,strlen(buf),0));
		fclose(fp);

		printf("Receive '%s' from client!\n",file_name); 

	}
	return 0;
}

int pwdser(int new_sock)
{
	bzero(cwd_buf,CWD_SIZE);//将内存区域置0
	getcwd(cwd_buf,sizeof(cwd_buf));	//获取当前工作目录绝对路径
	send( new_sock,cwd_buf,sizeof(cwd_buf),0);
	return 0;
}

int dirser(int new_sock)
{
	DIR *dirp;
	struct dirent *dirr;
	bzero(cwd_buf,CWD_SIZE); //将内存区域置0
	getcwd(cwd_buf,sizeof(cwd_buf));	//获取当前工作目录绝对路径
	if((dirp=opendir(cwd_buf))==NULL)	//打开指定目录，和open类似
		printf("Error in openning directory\n");
	else
	{
		bzero(buf,BUF_SIZE);
		while((dirr=readdir(dirp))!=NULL)	//读取目录
		{	
			strcpy(buf,dirr->d_name);	//拷贝一个文件名
			if(send(new_sock,buf,BUF_SIZE,0)<0)
			{
				printf("Error in sending directory\n");
				break;
			}
			bzero(buf,BUF_SIZE);
		}
	}	
	return 0;
}

int cdser(char file_name[FILE_SIZE])
{
	chdir(file_name);	//改变当前目录
	return 0;
}

int quitser()
{
	printf("A client quit!\n");
	return 0;
}

int removefile(int new_sock,char file_name[FILE_SIZE])
{
	char buff[128]="ok";

	if(remove(file_name)== 0){

		send(new_sock,buff,strlen(buff),0);
	}
	printf("Removed %s.\n", file_name);
	return 0;
}


int	login_check(char *uname,char* salt,char* pass)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="root";
	char* database="login";//要访问的数据库名称
	char query[300]="select * from login where uname='";
	sprintf(query,"%s%s%s",query,uname,"'");

	strcpy(query,"select salt from login");
	char query2[128]={0};
	strcpy(query2,"select passwd from login");
	int t,r,t2,r2;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
	}
	t=mysql_query(conn,query);
	if(t)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}else{
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{	
				for(t=0;t<mysql_num_fields(res);t++)
				{
					strcpy(salt,row[t]);
				}
				printf("\n");
			}
		}
		mysql_free_result(res);
	}
	t2=mysql_query(conn,query2);
	if(t2)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}else{
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{	
				for(t=0;t<mysql_num_fields(res);t++)
				{
					strcpy(pass,row[t]);
				}
			}

		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return 0;

}

int history_client(char *order)
{
	int length;
	int fp = open("history",O_RDWR|O_CREAT,0666);    
	if(-1== fp )
	{
		printf("file Can not open\n");
		return -1;
	}
	lseek(fp,0,SEEK_END);
	length = write(fp,order,strlen(order));
	if (length<=0)
	{
		printf("can not write \n");
		return -1;
	}
	close(fp);
	return 0;
}
