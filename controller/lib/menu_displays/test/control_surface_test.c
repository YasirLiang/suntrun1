#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 

extern void *thread_control_surface(void *arg);

void main(int argc, char * argv)
{
    pthread_t th;  
    int ret;  
    void *arg;
    ret = pthread_create(&th, NULL, thread_control_surface, NULL);  
    if (ret != 0) {  
        printf( "Create thread_control_surface error!\n");  
        exit(1); 
    } 
    pthread_join( th, &arg);
}

