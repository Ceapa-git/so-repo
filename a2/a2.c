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
        int ok = 0;
        while (!ok)
        {
            sem_wait(sem);
            ok = *proc_5_thread_2_start;
            sem_post(sem);
        }
    }
    if (id == 3)
    {
        sem_t *sem_proc_7_thread_4_end = sem_open("/sem_proc_7_thread_4_end", 0);
        sem_wait(sem_proc_7_thread_4_end);
        sem_close(sem_proc_7_thread_4_end);
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
    if (id == 3)
    {
        sem_t *sem_proc_5_thread_3_end = sem_open("/sem_proc_5_thread_3_end", 0);
        sem_post(sem_proc_5_thread_3_end);
        sem_close(sem_proc_5_thread_3_end);
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

typedef struct
{
    int id;
    int *proc_6_thread_13_pending;
    sem_t *sem_sync;
    sem_t *sem_13;
} proc_6_thread_data;
void *proc_6_thread(void *arg)
{
    int id = ((proc_6_thread_data *)arg)->id;
    int *proc_6_thread_13_pending = ((proc_6_thread_data *)arg)->proc_6_thread_13_pending;
    sem_t *sem_sync = ((proc_6_thread_data *)arg)->sem_sync;
    sem_t *sem_13 = ((proc_6_thread_data *)arg)->sem_13;
    free(arg);

    int ok = 0;
    sem_wait(sem_13);
    ok = *proc_6_thread_13_pending;
    if (ok < 0)
    {
        sem_post(sem_13);
        sem_wait(sem_sync);
        info(BEGIN, 6, id);
    }
    else
    {
        if (id == 13)
        {
            sem_wait(sem_sync);
            info(BEGIN, 6, id);
            sem_post(sem_13);
            do
            {
                sem_wait(sem_13);
                ok = *proc_6_thread_13_pending;
                sem_post(sem_13);
            } while (ok < 4);
        }
        else if (ok < 4)
        {
            *proc_6_thread_13_pending = ok + 1;
            sem_wait(sem_sync);
            info(BEGIN, 6, id);
            sem_post(sem_13);
        }
        else
        {
            sem_post(sem_13);
            while (ok >= 0)
            {
                sem_wait(sem_13);
                ok = *proc_6_thread_13_pending;
                sem_post(sem_13);
            }
            sem_wait(sem_sync);
            info(BEGIN, 6, id);
        }
    }
    if (id != 13 && ok >= 0)
    {
        while (ok >= 0)
        {
            sem_wait(sem_13);
            ok = *proc_6_thread_13_pending;
            sem_post(sem_13);
        }
    }
    info(END, 6, id);
    if (id == 13)
    {
        sem_wait(sem_13);
        *proc_6_thread_13_pending = -1;
        sem_post(sem_13);
    }
    sem_post(sem_sync);

    return NULL;
}
void proc_6()
{
    pthread_t threads[42];
    sem_t sem_sync;
    sem_init(&sem_sync, 0, 5);
    sem_t sem_13;
    sem_init(&sem_13, 0, 1);
    sem_t sem_remaining;
    sem_init(&sem_remaining, 0, 1);
    int proc_6_thread_13_pending = 0;

    for (int i = 0; i < 42; i++)
    {
        proc_6_thread_data *data = malloc(sizeof(proc_6_thread_data));
        data->id = i + 1;
        data->proc_6_thread_13_pending = &proc_6_thread_13_pending;
        data->sem_sync = &sem_sync;
        data->sem_13 = &sem_13;

        pthread_create(&threads[i], NULL, proc_6_thread, data);
    }
    for (int i = 0; i < 42; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem_sync);
    sem_destroy(&sem_13);
    sem_destroy(&sem_remaining);
}

typedef struct
{
    int id;
} proc_7_thread_data;
void *proc_7_thread(void *arg)
{
    int id = ((proc_7_thread_data *)arg)->id;
    free(arg);

    if (id == 6)
    {
        sem_t *sem_proc_5_thread_3_end = sem_open("/sem_proc_5_thread_3_end", 0);
        sem_wait(sem_proc_5_thread_3_end);
        sem_close(sem_proc_5_thread_3_end);
    }

    info(BEGIN, 7, id);
    info(END, 7, id);

    if (id == 4)
    {
        sem_t *sem_proc_7_thread_4_end = sem_open("/sem_proc_7_thread_4_end", 0);
        sem_post(sem_proc_7_thread_4_end);
        sem_close(sem_proc_7_thread_4_end);
    }

    return NULL;
}
void proc_7()
{
    pthread_t threads[6];
    for (int i = 0; i < 6; i++)
    {
        proc_7_thread_data *data = malloc(sizeof(proc_7_thread_data));
        data->id = i + 1;

        pthread_create(&threads[i], NULL, proc_7_thread, data);
    }
    for (int i = 0; i < 6; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main()
{
    init();
    info(BEGIN, 1, 0);

    sem_t *sem_proc_7_thread_4_end = sem_open("/sem_proc_7_thread_4_end", O_CREAT | O_EXCL, 0666, 0);
    if (sem_proc_7_thread_4_end == SEM_FAILED)
    {
        sem_unlink("/sem_proc_7_thread_4_end");
        sem_proc_7_thread_4_end = sem_open("/sem_proc_7_thread_4_end", O_CREAT | O_EXCL, 0666, 0);
        if (sem_proc_7_thread_4_end == SEM_FAILED)
        {
            printf("Error creating semaphore\n");
            return -1;
        }
    }
    sem_close(sem_proc_7_thread_4_end);
    sem_t *sem_proc_5_thread_3_end = sem_open("/sem_proc_5_thread_3_end", O_CREAT | O_EXCL, 0666, 0);
    if (sem_proc_5_thread_3_end == SEM_FAILED)
    {
        sem_unlink("/sem_proc_5_thread_3_end");
        sem_proc_5_thread_3_end = sem_open("/sem_proc_5_thread_3_end", O_CREAT | O_EXCL, 0666, 0);
        if (sem_proc_5_thread_3_end == SEM_FAILED)
        {
            printf("Error creating semaphore\n");
            return -1;
        }
    }
    sem_close(sem_proc_5_thread_3_end);

    pid_t pid_2;
    pid_2 = fork();
    if (pid_2 < 0)
    {
        printf("Error forking\n");
        return -1;
    }
    if (pid_2 == 0)
    {
        info(BEGIN, 2, 0);
        info(END, 2, 0);
        return 0;
    }

    pid_t pid_3;
    pid_3 = fork();
    if (pid_3 < 0)
    {
        printf("Error forking\n");
        return -1;
    }
    if (pid_3 == 0)
    {
        info(BEGIN, 3, 0);
        info(END, 3, 0);
        return 0;
    }

    pid_t pid_4;
    pid_4 = fork();
    if (pid_4 < 0)
    {
        printf("Error forking\n");
        return -1;
    }
    if (pid_4 == 0)
    {
        info(BEGIN, 4, 0);
        pid_t pid_6;
        pid_6 = fork();
        if (pid_6 < 0)
        {
            printf("Error forking\n");
            return -1;
        }
        if (pid_6 == 0)
        {
            info(BEGIN, 6, 0);
            pid_t pid_7;
            pid_7 = fork();
            if (pid_7 < 0)
            {
                printf("Error forking\n");
                return -1;
            }
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
        waitpid(pid_6, NULL, 0);
        info(END, 4, 0);
        return 0;
    }

    pid_t pid_5;
    pid_5 = fork();
    if (pid_5 < 0)
    {
        printf("Error forking\n");
        return -1;
    }
    if (pid_5 == 0)
    {
        info(BEGIN, 5, 0);
        proc_5();
        info(END, 5, 0);
        return 0;
    }

    waitpid(pid_2, NULL, 0);
    waitpid(pid_3, NULL, 0);
    waitpid(pid_4, NULL, 0);
    waitpid(pid_5, NULL, 0);

    info(END, 1, 0);
    sem_unlink("/sem_proc_7_thread_4_end");
    sem_unlink("/sem_proc_5_thread_3_end");
    return 0;
}
