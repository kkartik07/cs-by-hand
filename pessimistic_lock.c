#include <stdio.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t mu=PTHREAD_MUTEX_INITIALIZER;

// global variable to update
int count=0;
void inc(){
    count++;
}
void* inc_thread(void* arg) {
    // increment count
    pthread_mutex_lock(&mu);
    count++;
    pthread_mutex_unlock(&mu);
    return NULL;
}

// function to update the global variable sequentailly
void update_count_seq(int n){
    for(int i=0;i<n;i++){
        inc();
    }
}

// function to update the global variable parallely
void update_count_par(int n){
    pthread_t threads[n];     // Array of thread identifiers
    int thread_ids[n];        // Arguments to pass to threads

    for(int i=0;i<n;i++){
        thread_ids[i]=i;
        if (pthread_create(&threads[i], NULL, (void*)inc_thread, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }
}
int main(){
    clock_t begin=clock();
    update_count_seq(100000);
    clock_t end = clock();
    double elapsed = (double)(end - begin)/CLOCKS_PER_SEC;
    printf("Seq took %.20f seconds\n", elapsed);
    printf("Final value of count is %d\n\n", count);

    // reset count value
    count=0;

    begin=clock();
    update_count_par(100000);
    end = clock();
    double elapsedp = (double)(end - begin)/CLOCKS_PER_SEC;
    printf("Parallel took %.20f seconds\n", elapsedp);
    printf("Final value of count is %d\n", count);

    return 0;
}