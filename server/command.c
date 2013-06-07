#include "common.h"

int init(struct sockaddr_in *sin, int *lfd, int sock_opt)
{
    int tfd;

    bzero(sin, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = INADDR_ANY;
    sin->sin_port = htons(PORT);

    if ( (tfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("fail to creat socket");
        return -1;
    }

    //socketclose之后不会立即关闭，而经历time_wait，为了在这个时间段内重用，则采用下面设置
    setsockopt(tfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));

    if ( (bind(tfd, (struct sockaddr *)sin, sizeof(struct sockaddr_in))) == -1)
    {
        perror("fail to bind");
        return -1;
    }

    if ( (listen(tfd, 20)) == -1)
    {
        perror("fail to listen");
        return -1;
    }

    *lfd = tfd;

    return 0;
}


int do_put(int cfd, char *file)
{
    struct stat statbuf;
    int n, fd;
    int res = -1;
    char buf[1024];

    if ( (fd = open(file, O_RDONLY)) == -1)
    {
        write(cfd, "ERR open server file\n", strlen("ERR open server file\n"));
        return -1;
    }

    fstat(fd, &statbuf);

    if (!S_ISREG(statbuf.st_mode))
    {
        write(cfd, "ERR: server file should be a regular\n",
              strlen("ERR: server file should be a regular\n"));
        close(fd);
        return -1;
    }
/*****************第一次向客户端发送内容******************************/
    sprintf(buf, "OK %ld\n", statbuf.st_size);
    write(cfd, buf, strlen(buf)); //服务器向客户端发送“OK 字节数“命令

/****************接收客户端的第二次写的内容**************************/
    n = read(cfd, buf, MAX_LINE);//接收客户端发送的“RDY“字符串，之后双方进行数据传输
    buf[n]='\0';
    //printf("%s\n",buf);

/********************文件传输***************************************/
    while (1)
    {
        n = read(fd, buf, MAX_LINE);
        if (n > 0)
            write(cfd, buf, n);
        else if (n == 0)
        {
            printf("file transfer OK\n");
            break;
        }
        else
        {
            perror("fail to read");
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}


int do_get(int cfd, char *file)
{
    struct stat statbuf;
    int n, fd;
    char buf[1024];
    int len;

    if ( (fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
    {
        printf("cannot open the file\n");
        write(cfd, "ERR open server file\n", strlen("ERR open server file\n"));
        close(fd);
        return -1;

    }

    if ( (fstat(fd, &statbuf)) == -1)
    {
        printf("cannot get the stat of file\n");
        write(cfd, "ERR stat server file\n", strlen("ERR stat server file\n"));
        close(fd);
        return -1;
    }

    if (!S_ISREG(statbuf.st_mode))
    {
        write(cfd, "ERR server path should be a regular file\n",
              strlen("ERR server path should be a regular file\n"));
        close(fd);
        return -1;
    }
/***************服务器端第二次读**************************/
n = read(cfd,buf,MAX_LINE);
buf[n]='\0';
len = atoi(&buf[5]);
//printf("%s size:%d\n",buf,len);

/****************服务器第一次写*********************************/
    write(cfd, "server is OK", strlen("server is OK"));

/******************文件传输*********************************/
//确定接收文件的大小,大小由之前双方协商得出，保证写入固定大小的字节
    char buf1[1000];
    while ((n = read(cfd,buf1,MAX_LINE))>0)
    {
        //printf("%s\n",buf1);
        if (write(fd,buf1,n)!=n)
        {
            printf("write error\n");
            return -1;
        }
	len-=n;
	if(len ==0)
	{
		fsync(fd);
		printf("file transfer OK\n");
		break;
	}
    }
    if (n<0)
    {
        printf("read error\n");
        return -1;
    }
    close(fd);
    return 0;
}
