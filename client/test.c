//#include "common.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define COMMAND_LINE 256
#define MAX_ARG 10
#define MAX_LENGTH 64
#define NAME_LEN 256
#define MAX_LINE 1024
int del_blank(int pos, char *cline)
{
    while (cline[pos] != '\0' && (cline[pos] == ' ' || cline[pos] == '\t' || cline[pos]=='\n'))
        pos++;
    return pos;
}
int get_arg(char *arg, int pos, char *cline)
{
    int i = 0;
    while (cline[pos] != '\0' && cline[pos] != ' ' && cline[pos] != '\t' && cline[pos]!='\n')
    {
        arg[i++] = cline[pos++];
    }
    arg[i]='\0';
    return pos;
}
struct command_line
{
    char *name;
    char *argv[MAX_ARG];
};


int split(struct command_line * command, char cline[])
{
    int i;
    int pos = 0;

    pos = del_blank(pos, cline);

    i = 0;
    while (cline[pos] != '\0')
    {
        if ((command->argv[i] = (char *)malloc(MAX_LENGTH)) == NULL)
        {
            perror("fail to malloc");
            return -1;
        }

        pos = get_arg(command->argv[i], pos, cline);
        //printf("argv[%d]:%s\n",i,command->argv[i]);

        i++;
        pos = del_blank(pos, cline);
    }

    command->argv[i] = NULL;
    command->name = command->argv[0];

    return i;
}
int main(void)
{
        int i;
    char cline[COMMAND_LINE];
    struct command_line command;
    //strcpy(cline,"get haha  test");
    fgets(cline, MAX_LINE, stdin);
    int k = split(&command,cline);
    printf("name:%s\n",command.name);
    for (i=0;i<k;++i)
        printf("argv[%d]:%s\n",i,command.argv[i]);

}
