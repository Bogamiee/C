// main.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "myls.h"

int CAPACITY = 1024;
int terminalWidth;

int main(int argc, char *argv[])
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    terminalWidth = w.ws_col;

    return runLs(argc, (const char **)argv);
}
