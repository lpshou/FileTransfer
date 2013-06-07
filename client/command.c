#include "common.h"
//完成客户端和服务器的连接
int do_connect(char *ip,struct sockaddr_in *sin, int *sock_fd)
{
    int sfd;

    bzero(sin, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;

    if (inet_pton(AF_INET, ip, &sin->sin_addr) == -1)
    {
        perror("wrong format of ip address");
        return -1;
    }

    sin->sin_port = htons(PORT);
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("fail to creat socket");
        return -1;
    }

    if (connect(sfd, (struct sockaddr *)sin, sizeof(struct sockaddr_in)) == -1)
    {
        perror("fail to connect");
        return -1;
    }
    *sock_fd = sfd;
    return 0;
}

//完成客户端上传文件操作，src是源文件，dst是目的路径,文件名保持不变
int do_get(const char *src, const char *dst, int sock_fd)
{
    char *dst_file;
    char *p;
    struct stat statbuf;
    int n, fd,len;
    char buf[MAX_LINE];
    if (src == NULL || dst == NULL)
    {
        printf("wrong command\n");
        return -1;
    }

    //在本地打开一个文件
    if ( (dst_file = (char *)malloc(strlen(dst) + strlen(src))) == NULL)
    {
        perror("fail to malloc");
        return -1;
    }
    strcpy(dst_file, dst);
    if (dst_file[strlen(dst_file) - 1] != '/')
        strcat(dst_file, "/");
    p = rindex(src, '/');
    strcat(dst_file, p + 1);

    if ((fd = open(dst_file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
    {
        perror("fail to open dst-file");
        close(fd);
    }

    fstat(fd, &statbuf); //获取在本地创建的文件信息
    if (!S_ISREG(statbuf.st_mode))
    {
        perror("dst-file should be a regular file");
        close(fd);
        free(dst_file);
        return -1;
    }

    /***************************文件传输前第1次写*********************/
    sprintf(buf, "GET %s", src);
    write(sock_fd, buf, strlen(buf));//向服务器端发送“GET 文件名“命令

    /**************************文件传输前第1次读***********************/
    n = read(sock_fd, buf, MAX_LINE);//接收服务器端的"OK 字节数“
    buf[n]='\0';
   // printf("%s",buf);
    if (buf[0] == 'E')//判断接收到的是以“E“开头的字符串,则表明服务器端出错
    {
        printf("%s\n",buf);
        close(fd);
        free(dst_file);
        return -1;
    }
    len = atoi(&buf[3]);//如果接收到的不是以"E"开头的字符窜，则获取其中字节数

    /********************文件传输前第2次写********************************/
    write(sock_fd, "RDY", 3); //向服务器端发送“RDY"字符串,之后双方进行数据传输

    /**************************文件传输*********************************/
    //确定接收文件的大小,大小由之前双方协商得出，保证写入固定大小的字节
    while ((n=read(sock_fd,buf,MAX_LINE))>0)
    {
        if (write(fd,buf,n)!=n)
            perror("write error");
        len-=n;
        if (len == 0)
        {
                fsync(fd);
            printf("get file from server succeed\n");
            break;
        }
    }
    if (n<0)
        perror("read error");

    close(fd);
    free(dst_file);
    return 0;
}
//完成客户端下载文件操作，src是源文件，dst是目的路径,文件名保持不变
int do_put(const char *src, const char *dst, int sock_fd)
{
    char *dst_file;
    struct stat statbuf;
    int n, fd;
    int res = -1;
    char buf[MAX_LINE];
    char *p;

    if (src == NULL || dst == NULL)
    {
        printf("wrong command\n");
        return -1;
    }
    if ((dst_file = malloc(strlen(dst)+strlen(src)+2)) == NULL)
    {
        perror("fail to malloc");
        return -1;
    }

    strcpy(dst_file, dst);
    if (dst_file[strlen(dst_file)-1] != '/')
        strcat(dst_file, "/");
    p = rindex(src, '/');
    strcat(dst_file, p + 1);

    //printf("name: %s\n",dst_file);

    if ((fd = open(src, O_RDONLY)) == -1)
    {
        perror("fail to open src-file");
        close(fd);
        free(dst_file);
        return -1;
    }

    fstat(fd, &statbuf);
    if (!S_ISREG(statbuf.st_mode))
    {
        perror("src-file should be a regular file\n");
        free(dst_file);
        return -1;
        //goto end2;
    }
 /**************************客户端第1次向服务器端发送“PUT 文件名“命令****************/
    sprintf(buf, "PUT %s",dst_file);
    write(sock_fd, buf, strlen(buf)); //将要上传文件的大小,文件名发送给服务器
// statbuf.st_size,
 /**************************客户端第2次向服务器端发送“SIZE 文件大小“命令****************/
 sprintf(buf,"SIZE %ld",statbuf.st_size);
 write(sock_fd,buf,strlen(buf));



 /**************************客户端第1次读取服务器端命令******************************/
    n = read(sock_fd, buf, MAX_LINE);//获取服务器反馈的信息
    buf[n]='\0';
    //printf("%s\n",buf);
    if (buf[0]=='E')//如果服务器反馈字符串中首字符为E，表示服务器端出错
    {
        printf("%s\n",buf);
        close(fd);
        free(dst_file);
    }
    /**************************文件传输*********************************/
    char buf1[MAX_LENGTH];
    while ((n = read(fd,buf1,MAX_LINE))>0)
    {
           // printf("%s\n",buf1);
        if (write(sock_fd,buf1,n)!=n)
        {
            perror("write error");
            return -1;
        }
    }
    if (n == 0)

        printf("put the file to server succeed\n");
    if (n<0)
    {
        perror("read error");
        return -1;
    }
    close(fd);
    free(dst_file);

    return 0;
}

int do_bye(int sock_fd)
{
    char buf[MAX_LINE];

    sprintf(buf, "BYE");
    if (write(sock_fd, buf, strlen(buf)+1) == -1)
        return -1;

    return 0;
}

