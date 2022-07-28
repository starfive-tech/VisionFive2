#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <pthread.h>

volatile int msg_done = 0;
int active = 1;

void mailbox_irq_handler(int val)
{
    printf("mailbox msg recv %d\n",val);
    msg_done = 1;
}

void init_signal(int fd)
{
    int flags = 0, ret = 0;

    signal(SIGIO, mailbox_irq_handler);

    ret = fcntl(fd, F_SETOWN, getpid());
    printf("mailbox fcntl F_SETOWN ret: %d\n",ret);

    flags = fcntl(fd, F_GETFL);

    fcntl(fd, F_SETFL, flags | FASYNC);
}

void * mailbox_user_read(void *arg)
{
    char buf[8] = {0};
    int fp = open("/sys/kernel/debug/soc:mailbox_client@0/message", O_RDWR);

    while(active) {
        read(fp, buf, sizeof(char));
        printf("user recv:%c\n", buf[0]);
    }
    close(fp);
    return NULL;
}

int main(int argv,char **argc)
{
    int fd = 0, i = 0;
    char buf_tx[8];

    fd = open("/sys/kernel/debug/soc:mailbox_client@0/message", O_RDWR);
    if (fd < 0) {
        printf("open device error\n");
        return 1;
    }

    pthread_t thread_read;
    if (pthread_create(&thread_read, NULL, mailbox_user_read, NULL) != 0) {
        printf("pthread_create error\n");
        close(fd);
        return 0;
    }

    buf_tx[0] = 'a';
    do {
        write(fd, buf_tx, sizeof(char));
        printf("user send:%c\n", buf_tx[0]);
        sleep(1);

        buf_tx[0]++;
        i++;
    } while (i < 2);
    active = 0;
    close(fd);

    return 0;
}
