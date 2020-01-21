// MDULE LOCATION
//  /sys/module/<module_name>/parameters
// example: echo "1">/sys/module/simple/parameters/debug

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PID_LOCATION "/sys/module/signal_module/parameters/pid"
#define BUFF_LEN 100
int main()
{
    int fd;
    char buffer[BUFF_LEN];
    int read_count;

    fd = open(PID_LOCATION, O_RDONLY);

    read_count = read(fd, buffer, BUFF_LEN);
    if (read_count > 0)
    {
        printf("%s", buffer);
    }
    else
    {
        printf("Nothing to print\n");
    }

    close(fd);
}