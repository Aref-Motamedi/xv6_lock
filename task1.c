#include "types.h"
#include "user.h"

#define NCHILD 10

int main ()
{
    int proid;
    ticketlockInit();

    proid = fork();
    int parent = proid;
    for(int i = 1; i< NCHILD; i++)
    {
       if(proid < 0)
        {
            printf(1, "fork failed\n");
            exit();
        }
        else if(proid > 0)
        {
            proid = fork();
        }
    }
    if(proid < 0)
    {
        printf(1, "fork failed\n");
        exit();
    }
    else if(proid == 0)
    {
        printf(1, "child adding to shared counter\n");
        ticketlockTest();
        // rwtest(1);
        // rwtest(1);
    }
    else
    {
        if  (proid == parent){
            sleep(4000);
        }
        for(int i = 0 ; i < NCHILD; i++)
            wait();
        
        printf(1, "user program finished\n");
        printf(2, "ticket lock value: %d\n", ticketlockTest()-1);
    }
    exit();

    return 0;    
}