#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

void proc_1()
{
}
void proc_2()
{
}
void proc_3()
{
}
void proc_4()
{
}
void proc_5()
{
}
void proc_6()
{
}
void proc_7()
{
}

int main()
{
    init();
    info(BEGIN, 1, 0);

    pid_t pid_2;
    pid_2 = fork();
    if (pid_2 == 0)
    {
        info(BEGIN, 2, 0);
        proc_2();
        info(END, 2, 0);
        return 0;
    }

    pid_t pid_3;
    pid_3 = fork();
    if (pid_3 == 0)
    {
        info(BEGIN, 3, 0);
        proc_3();
        info(END, 3, 0);
        return 0;
    }

    pid_t pid_4;
    pid_4 = fork();
    if (pid_4 == 0)
    {
        info(BEGIN, 4, 0);
        pid_t pid_6;
        pid_6 = fork();
        if (pid_6 == 0)
        {
            info(BEGIN, 6, 0);
            pid_t pid_7;
            pid_7 = fork();
            if (pid_7 == 0)
            {
                info(BEGIN, 7, 0);
                proc_7();
                info(END, 7, 0);
                return 0;
            }
            proc_6();
            waitpid(pid_7, NULL, 0);
            info(END, 6, 0);
            return 0;
        }
        proc_4();
        waitpid(pid_6, NULL, 0);
        info(END, 4, 0);
        return 0;
    }

    pid_t pid_5;
    pid_5 = fork();
    if (pid_5 == 0)
    {
        info(BEGIN, 5, 0);
        proc_5();
        info(END, 5, 0);
        return 0;
    }

    proc_1();
    waitpid(pid_2, NULL, 0);
    waitpid(pid_3, NULL, 0);
    waitpid(pid_4, NULL, 0);
    waitpid(pid_5, NULL, 0);
    info(END, 1, 0);
    return 0;
}
