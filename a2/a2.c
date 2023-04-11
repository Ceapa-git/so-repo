#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
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

typedef struct
{
    int id;
    int *proc_5_thread_2_start;
    int *proc_5_thread_1_end;
    sem_t *sem;
} proc_5_thread_data;
void *proc_5_thread(void *arg)
{
    int id = ((proc_5_thread_data *)arg)->id;
    int *proc_5_thread_2_start = ((proc_5_thread_data *)arg)->proc_5_thread_2_start;
    int *proc_5_thread_1_end = ((proc_5_thread_data *)arg)->proc_5_thread_1_end;
    sem_t *sem = ((proc_5_thread_data *)arg)->sem;
    free(arg);

    if (id == 1)
    {
        int ok=0;
        while (!ok)
        {
            sem_wait(sem);
            ok = *proc_5_thread_2_start;
            sem_post(sem);
        }
    }

    info(BEGIN, 5, id);
    if (id == 2)
    {
        sem_wait(sem);
        *proc_5_thread_2_start = 1;
        sem_post(sem);
        int ok = 0;
        while (!ok)
        {
            sem_wait(sem);
            ok = *proc_5_thread_1_end;
            sem_post(sem);
        }
    }
    info(END, 5, id);

    if (id == 1)
    {
        sem_wait(sem);
        *proc_5_thread_1_end = 1;
        sem_post(sem);
    }
    return NULL;
}
void proc_5()
{
    pthread_t threads[5];
    sem_t sem;
    sem_init(&sem, 0, 1);
    int proc_5_thread_2_start = 0;
    int proc_5_thread_1_end = 0;

    for (int i = 0; i < 5; i++)
    {
        proc_5_thread_data *data = malloc(sizeof(proc_5_thread_data));
        data->id = i + 1;
        data->proc_5_thread_2_start = &proc_5_thread_2_start;
        data->proc_5_thread_1_end = &proc_5_thread_1_end;
        data->sem = &sem;

        pthread_create(&threads[i], NULL, proc_5_thread, data);
    }
    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem);
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
