#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "robot.h"

#define NUM_ROBOTS 3
#define NUM_TASKS 15

int main(){
    Warehouse wh;
    initWarehouse(&wh);

    srand(time(NULL));

    // Create tasks
    for(int i=0;i<NUM_TASKS;i++){
        Task t;
        t.id=i;
        t.pickupX=rand()%5;
        t.pickupY=rand()%5;
        t.dropX=rand()%5;
        t.dropY=rand()%5;
        t.priority=rand()%5+1;
        t.deadline=rand()%10+5;

        pushTask(&wh,t);
    }

    pthread_t threads[NUM_ROBOTS];
    Robot robots[NUM_ROBOTS];

    clock_t start=clock();

    for(int i=0;i<NUM_ROBOTS;i++){
        robots[i].id=i+1;
        robots[i].wh=&wh;

        pthread_create(&threads[i],NULL,robotFunc,&robots[i]);
    }

    sleep(10); // simulation time

    clock_t end=clock();

    double timeTaken=(double)(end-start)/CLOCKS_PER_SEC;

    printf("\n=== PERFORMANCE REPORT ===\n");
    printf("Tasks completed: %d\n",wh.totalTasksCompleted);
    printf("Time: %.2f sec\n",timeTaken);
    printf("Throughput: %.2f tasks/sec\n",wh.totalTasksCompleted/timeTaken);

    if(wh.totalTasksCompleted>0)
        printf("Avg wait: %.2f\n",wh.totalWaitTime/wh.totalTasksCompleted);

    printf("Zone congestion: %d\n",wh.zoneBlockCount);

    return 0;
}
