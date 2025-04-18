#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_RECORDS 3
#define NUM_CONN 6

typedef struct {
    int attr[3];
} RecordData;


typedef struct{
    RecordData data;
    pthread_mutex_t lock;
} Record;


typedef struct{
    Record records[NUM_RECORDS];
} Database;

Database db; 

void acquireLock(char *txn, int recId){
    printf("Txn %c wants to acquire lock on record with ID %d\n",*txn,recId);
    pthread_mutex_lock(&db.records[recId].lock);
    printf("Txn %c acquired lock on record with ID %d\n",*txn,recId);
}

void releaseLock(char *txn, int recId){
    pthread_mutex_unlock(&db.records[recId].lock);
    printf("Txn %c released lock on record with ID %d\n",*txn,recId);
}

void initDB(){
    for(int i=0;i<NUM_RECORDS;i++){
        db.records[i].data.attr[0]=i; //id
        db.records[i].data.attr[1]=rand()%20+15; //age
        pthread_mutex_init(&db.records[i].lock,NULL);

    }
}

void *mimic_load(void *arg){
    char *txname=(char*) arg;

    while(1){
        // pick 2 unique records randomly
        int rec1 = rand() % NUM_RECORDS;
        int rec2 = rand() % NUM_RECORDS;

        // cannot acquire the same resource
        if(rec1==rec2) continue;

        // imposing total ordering condition to avoid deadlock, in which if a resource with ID=1 is not available, 
        // it cant acqurie resource with ID=2 as well. ie there is a order in which resources can be acquired (increasing),
        //    else the thread will stop until that resource is free and then acquires all the required resources if avaiable in order 
        if(rec1 > rec2){
            // swap if resource are requested out of order
            int temp=rec2;
            rec2=rec1;
            rec1=temp;
        }

        // acquire lock on both the resources
        acquireLock(txname,rec1);
        acquireLock(txname,rec2);

        // simulate some processing by sleep
        sleep(2);

        // release locks after processing is done
        releaseLock(txname,rec1);
        releaseLock(txname,rec2);

        sleep(1);
    }

    pthread_exit(NULL);
}
int main(){
    srand(time(NULL));
    initDB();

    pthread_t threads[NUM_CONN];

    for(int i=0;i<NUM_CONN;i++){
        char *txn = (char* )malloc(1*sizeof(char));
        txn[0]='A' + i;
        pthread_create(&threads[i], NULL, mimic_load, txn);
    }

    for(int i=0;i<NUM_CONN;i++){
        pthread_join(threads[i],NULL);
    }

    for(int i=0;i<NUM_RECORDS;i++){
        pthread_mutex_destroy(&db.records[i].lock);
    }

    return 0;

}