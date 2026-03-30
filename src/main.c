#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 1000
#define THREADS 3

typedef struct
{
  int id;
  int count;
  pthread_barrier_t *barrier;
  struct timespec start;
  struct timespec end;
  double duration;
} ThreadData;

int cmp_threads(const void *a, const void *b)
{
  ThreadData *tdata1 = (ThreadData *)a;
  ThreadData *tdata2 = (ThreadData *)b;

  if (tdata1->duration < tdata2->duration)
    return -1;
  if (tdata1->duration > tdata2->duration)
    return 1;
  return 0;
}

void prt_tdata(ThreadData *tdata)
{
  printf("THREAD %d, duration: %f ns\n", tdata->id, tdata->duration);
}

void *race(void *arg)
{
  ThreadData *tdata = (ThreadData *)arg; // arg points to provided data on pthread_create
  pthread_barrier_wait(tdata->barrier);
  clock_gettime(CLOCK_MONOTONIC, &tdata->start);
  tdata->count = 0;
  for (int i = 0; i < MAX; i++)
  {
    tdata->count += 1;
    printf("THREAD %d: %d\n", tdata->id, i);
  }

  clock_gettime(CLOCK_MONOTONIC, &tdata->end);
  tdata->duration = (tdata->end.tv_sec - tdata->start.tv_sec) * 1000000000 + (tdata->end.tv_nsec - tdata->start.tv_nsec);
  return NULL;
}

int main()
{
  pthread_t threads[THREADS];
  ThreadData data[THREADS];

  pthread_barrier_t barrier;
  pthread_barrier_init(&barrier, NULL, THREADS);

  for (int i = 0; i < THREADS; i++)
  {
    ThreadData *tdata = &data[i];
    pthread_t *thread = &threads[i];

    tdata->id = i;
    tdata->barrier = &barrier;
    pthread_create(thread, NULL, race, tdata);
  }

  for (int i = 0; i < THREADS; i++)
  {
    ThreadData *tdata = &data[i];
    pthread_t thread = threads[i];
    pthread_join(thread, NULL);
  }

  qsort(data, THREADS, sizeof(ThreadData), cmp_threads);

  for (int i = 0; i < THREADS; i++)
  {
    ThreadData *tdata = &data[i];
    prt_tdata(tdata);
  }

  pthread_barrier_destroy(&barrier);

  return 0;
}