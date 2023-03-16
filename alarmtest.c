#include "types.h"
#include "stat.h"
#include "user.h"

void periodic();

int
main(int argc, char *argv[])
{
    int i;
    printf(1, "alarmtest starting\n");
    alarm(10, periodic);
    // crash kernel
    // asm("mov 0x80104e74, %esp");
    asm("mov 0x8dfbefac, %esp");
    for(i = 0; i < 25*5000000; i++) {
        if ((i % 250000) == 0)
            write(2, ".", 1);
    }
    exit();
}

void
periodic()
{
    printf(1, "alarm!\n");
}
