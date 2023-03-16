# HW6: multithreaded programming
To avoid this sequence of events, insert lock and unlock statements in put and get so that the number of keys missing is always 0. The relevant pthread calls are (for more see the manual pages, man pthread):

```c
pthread_mutex_t lock;     // declare a lock
pthread_mutex_init(&lock, NULL);   // initialize the lock
pthread_mutex_lock(&lock);  // acquire lock
pthread_mutex_unlock(&lock);  // release lock
```

We don't need to lock when getting since there is no critical area when running `get` function.
To avoid keys missing, we just need wrap a lock around `put` function:

```c
// declare a lock as global variable
pthread_mutex_t lock;


// init lock at main function
pthread_mutex_init(&lock, NULL);

// wrap lock at put fucntion
for (i = 0; i < b; i++) {
  // printf("%d: put %d\n", n, b*n+i);
  pthread_mutex_lock(&lock);
  put(keys[b*n + i], n);
  pthread_mutex_unlock(&lock);
}
```

We get the output when running command `./a.out 2`

```shell
$ ./a.out 2
1: put time = 0.012621
0: put time = 0.012677
0: get time = 5.493438
0: 0 keys missing
1: get time = 5.501054
1: 0 keys missing
completion time = 5.513869
```

Comparing with single thread:

```shell
$ ./a.out 1
0: put time = 0.010118
0: get time = 5.603231
0: 0 keys missing
completion time = 5.613539
```

> Test your code first with 1 thread, then test it with 2 threads. Is it correct (i.e. have you eliminated missing keys?)? Is the two-threaded version faster than the single-threaded version? 

Yes, it does faster than the single-threaded version

> Modify your code so that get operations run in parallel while maintaining correctness. (Hint: are the locks in get necessary for correctness in this application?) 

We can let each thread only gets the part in which it inserts.
Origin code:

```c
t0 = now();
for (i = 0; i < NKEYS; i++) {
struct entry *e = get(keys[i]);
if (e == 0) k++;
}
t1 = now();
```

Modified code:

```c
for (i = 0; i < b; i++) {
  struct entry *e = get(keys[b*n + i]);
  if (e == 0) k++;
}
```

Performance:

```shell
# 2 thread
$ ./a.out 2
1: put time = 0.013604
0: put time = 0.013658
0: get time = 2.660065
0: 0 keys missing
1: get time = 2.666521
1: 0 keys missing
completion time = 2.680332

# 1 thread
$ ./a.out 1
0: put time = 0.008785
0: get time = 5.413762
0: 0 keys missing
completion time = 5.422745
```

> Modify your code so that some put operations run in parallel while maintaining correctness. (Hint: would a lock per bucket work?) 

Each bucket is independent, so it is allright that we can lock each bucket with one lock. As a result, threads could run parallel when accessing different buckets.

```c
// declare a lock array
pthread_mutex_t locks[NBUCKET];

// init each lock in the array
for (i = 0; i < NBUCKET; ++i) {
  pthread_mutex_init(&locks[i], NULL);
}

// lock when accessing a bucket
pthread_mutex_lock(&locks[key%NBUCKET]);
insert(key, value, &table[i], table[i]);
pthread_mutex_unlock(&locks[key%NBUCKET]);
```

Result:

```shell
$ ./a.out 2
0: put time = 0.008910
1: put time = 0.008913
0: get time = 2.748055
0: 0 keys missing
1: get time = 2.772845
1: 0 keys missing
completion time = 2.781917

$ ./a.out 1
0: put time = 0.009049
0: get time = 5.321198
0: 0 keys missing
completion time = 5.330456
```
