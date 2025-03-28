/*
** EPITECH PROJECT, 2025
** PipeViewer
** File description:
** main
*/

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <bits/sigaction.h>

#define BUFF_BYTES_LEN 4    // 4096

int fd = -1;
ssize_t lastAlarmBytes = 0;
ssize_t wroteBytes = 0;
ssize_t fileSize = 0;

void displayTime(void)
{
    struct timespec preciseTime;
    struct tm brokenLocalTime;
    char buffer[32];

    clock_gettime(CLOCK_REALTIME, &preciseTime);
    localtime_r(&preciseTime.tv_sec, &brokenLocalTime);
    strftime(buffer, 50, "%Y-%m-%d %H:%M:%S", &brokenLocalTime);
    fprintf(stderr, "%s.%06lu:", buffer, preciseTime.tv_nsec / 1000);
}

void displayProgress(void)
{
    double progress = 0;

    fprintf(stderr, " Progress %lu b / %lu b ", wroteBytes, fileSize);
    if (fileSize != 0) {
        progress = ((double)wroteBytes / (double)fileSize) * 100;
        fprintf(stderr, "(%.02f %%).", progress);
    } else
        fprintf(stderr, "(inf %%).");
    fprintf(stderr, " Bitrate: %lu bytes/sec.", wroteBytes - lastAlarmBytes);
}

void signalHandler(int signalNum, siginfo_t *info, void *context)
{
    (void) info;
    (void) context;
    switch (signalNum) {
        case SIGALRM:
            if (sigaction(SIGALRM, NULL, NULL) == 0) {
                displayTime();
                displayProgress();
                fprintf(stderr, "\n");
                lastAlarmBytes = wroteBytes;
                alarm(1);
            }
            break;
        case SIGINT:
            if (sigaction(SIGINT, NULL, NULL) == 0) {
                close(fd);
                fprintf(stderr, "Done!\n");
                exit(EXIT_SUCCESS);
            }
            break;
    }
}

int signalSetup(void)
{
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &signalHandler;
    if (sigaction(SIGALRM, &act, NULL) != 0) {
        perror(strerror(errno));
        return 84;
    }
    if (sigaction(SIGINT, &act, NULL) != 0) {
        perror(strerror(errno));
        return 84;
    }
    return 0;
}

int openFile(const char *filepath)
{
    struct stat sb;

    if (stat(filepath, &sb) != 0) {
        perror("stat");
        return fd;
    }
    fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return fd;
    }
    fileSize = sb.st_size;
    return fd;
}

int readFile()
{
    ssize_t readRet = -1;

    void *buffer[BUFF_BYTES_LEN];

    readRet = read(fd, buffer, BUFF_BYTES_LEN);
    while (readRet != 0) {
        if (readRet == (ssize_t) -1) {
            perror("read");
            return 84;
        }
        if (write(STDOUT_FILENO, buffer, BUFF_BYTES_LEN) < 0){
            perror("write");
            return 84;
        }
        wroteBytes += readRet;
        readRet = read(fd, buffer, BUFF_BYTES_LEN);
    }
    displayTime();
    displayProgress();
    fprintf(stderr, "\n");
    fprintf(stderr, "Done !\n");
    close(fd);
    return 0;
}

int main(int ac, char **av)
{
    if (signalSetup() != 0)
        return 84;
    alarm(1);
    if (ac < 2)
        return 84;
    fd = openFile(av[1]);
    if (fd < 0)
        return 84;
    return readFile();
}
