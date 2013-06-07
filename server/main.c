#include "common.h"

int main(void)
{
    struct sockaddr_in sin;
    struct sockaddr_in cin;
    int lfd;//监听套接字
    int cfd;//连接套接字
    socklen_t len = sizeof(struct sockaddr_in);

    char buf[MAX_LINE];
    char str[ADDR_LEN];
    int sock_opt = 1;
    int n;
    pid_t pid;
    int m=0;

    if (init(&sin, &lfd, sock_opt) == -1)
        exit(1);

    printf("waiting connections ...\n");

    while (1)
    {
        if ( (cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1)
        {
            perror("fail to accept");
            exit(1);
        }

        if ( (pid = fork()) < 0)
        {
            perror("fail to fork");
            exit(1);
        }
        else if (pid == 0) //子进程
        {
            close(lfd);//关闭监听套接字
            while (1)
            {
                /****************s接收客户端第一次写的内容************/
                m = read(cfd, buf, MAX_LINE);
                buf[m]='\0';
                if (m<0) exit(1);

                printf("receive from client %s\n",buf);
                if (strstr(buf, "GET") == buf) //以GET开头的命令
                {
                    if (do_put(cfd, &buf[4]) == -1)
                        printf("error occours while putting\n");
                }
                else if (strstr(buf, "PUT") == buf)
                {
                    char *position = strchr(buf,'/');
                    int k=position-buf;
                   // printf("position:%d\n",k);
                    //printf("name:%s\n",&buf[k]);

                    if (do_get(cfd,&buf[k])==-1)
                        printf("error occours while getting\n");
                }

                else if (strstr(buf, "BYE") == buf)
                    break;
                else{
                        printf("wrong command\n");
                        exit(0);
                        }
            }
            close(cfd);
            exit(0);
        }
        else
            close(cfd);
    }
    return 0;
}
