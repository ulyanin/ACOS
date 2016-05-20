#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 48
#define NUM_OF_THREADS 8

static long long generation = 0;
int num = 0;
pthread_t thread[NUM_OF_THREADS];
pthread_t thread_init;
int end[NUM_OF_THREADS];
//int global_step = 0;
char map[2][SIZE][SIZE];
unsigned long long mask = 0;
int was_printed = 0;
int was_initialized = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER,
                mutex_init = PTHREAD_MUTEX_INITIALIZER;
int temp[NUM_OF_THREADS];
pthread_barrier_t barrier_life_game_step;
pthread_barrier_t barrier_life_game_step_end;
pthread_barrier_t barrier_initialization;

int add(int x)
{
    if (x == SIZE - 1)
        return 0;
    else
        return x + 1;
}

int sub(int x)
{
    if (x == 0)
        return SIZE - 1;
    else
        return x - 1;
}

int alive(int step, int x, int y)
{
    int al = 0;
    if (map[step][sub(x)][sub(y)] == 1)
        ++al;
    if (map[step][sub(x)][y] == 1)
        ++al;
    if (map[step][sub(x)][add(y)] == 1)
        ++al;
    if (map[step][x][sub(y)] == 1)
        ++al;
    if (map[step][x][add(y)] == 1)
        ++al;
    if (map[step][add(x)][sub(y)] == 1)
        ++al;
    if (map[step][add(x)][y] == 1)
        ++al;
    if (map[step][add(x)][add(y)] == 1)
        ++al;
    return al;
}

void recalc(int step, int x, int y)
{
    if (map[step][x][y] == 0)
    {
        if (alive(step, x, y) == 3)
            map[1 ^ step][x][y] = 1;
        else
            map[1 ^ step][x][y] = 0;
    }
    if (map[step][x][y] == 1)
    {
        if (alive(step, x, y) == 3 || alive(step, x, y) == 2)
            map[1 ^ step][x][y] = 1;
        else
            map[1 ^ step][x][y] = 0;
    }
}

void print_map(int step)
{
    system("clear");
    printf("GENERETION #%d\n", step);
    int i, j;
    for (i = 0; i < SIZE; i++){
        for (j = 0; j < SIZE; j++)
            printf("%s ", map[(step & 1) ^ 1][i][j] ? "■" : ".");
        printf("\n");
    }
}

int check_if_game_over()
{
    int i;
    for (i = 0; i < NUM_OF_THREADS; i++) {
        if (end[i] == 1) {
            return 0;
        }
    }
    return 1;
}

void try_print_map(int step)
{
    pthread_mutex_lock(&lock);
    if (!was_printed) {
        was_initialized = 0;
        print_map(step);
        was_printed = 1;
    }
    pthread_mutex_unlock(&lock);
}

void initizlize_variables()
{
    pthread_mutex_lock(&mutex_init);
    if (!was_initialized) {
        was_printed = 0;
        int i;
        for (i = 0; i < NUM_OF_THREADS; ++i) {
            end[i] = 0;
        }
        was_initialized = 1;
    }
    pthread_mutex_unlock(&mutex_init);
}


void* life_1(void* arg)
{
    int life_game_step;
    for (life_game_step = 0; ;life_game_step++) {
        initizlize_variables();
        pthread_barrier_wait(&barrier_initialization);
        int i, j;
        int id = *(int*)arg;
        for (i = id; i < id + (SIZE / NUM_OF_THREADS); i++) {
            for (j = 0; j < SIZE; j++)
            {
                recalc(life_game_step & 1, i, j);
                if (map[0][i][j] != map[1][i][j])
                    end[(id * NUM_OF_THREADS) / SIZE] = 1;
                //printf("%d\n",     end[(id * NUM_OF_THREADS) / SIZE]);
            }
        }

        pthread_barrier_wait(&barrier_life_game_step);
        try_print_map(life_game_step);
        if (0 & check_if_game_over()) {
            printf("GAME OVER DETECTED\n");
            return NULL;
        }
        pthread_barrier_wait(&barrier_life_game_step_end);
        usleep(10000);
    }
    return NULL;
}

int main(){
        int status;
    int i, j;/*
    FILE* game = fopen("game_of_life", "r");
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
        {
            c = fgetc(game);
            if (c == 10)
                c = fgetc(game);
            if (c != EOF)
                map[0][i][j] = c - '0';
        }*/
    /*map[0][10][10] = 1;
    map[0][10][11] = 1;
    map[0][11][10] = 1;
    map[0][11][11] = 1;

    map[0][10][21] = 1;
    map[0][11][21] = 1;
    map[0][9][21] = 1;

    map[0][8][22] = 1;
    map[0][12][22] = 1;
    map[0][7][23] = 1;
    map[0][13][23] = 1;
    map[0][8][24] = 1;
    map[0][12][24] = 1;

    map[0][10][25] = 1;
    map[0][11][25] = 1;
    map[0][9][25] = 1;

    map[0][10][26] = 1;
    map[0][11][26] = 1;
    map[0][9][26] = 1;

    map[0][9][31] = 1;
    map[0][8][31] = 1;
    map[0][7][31] = 1;

    map[0][9][32] = 1;
    map[0][10][32] = 1;
    map[0][7][32] = 1;
    map[0][6][32] = 1;

    map[0][9][33] = 1;
    map[0][10][33] = 1;
    map[0][7][33] = 1;
    map[0][6][33] = 1;

    map[0][9][34] = 1;
    map[0][10][34] = 1;
    map[0][8][34] = 1;
    map[0][7][34] = 1;
    map[0][6][34] = 1;

    map[0][10][35] = 1;
    map[0][11][35] = 1;
    map[0][6][35] = 1;
    map[0][5][35] = 1;

    map[0][7][40] = 1;
    map[0][6][40] = 1;

    map[0][8][44] = 1;
    map[0][9][44] = 1;
    map[0][8][45] = 1;
    map[0][9][45] = 1;*/
    srand(time(NULL));
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            map[0][i][j] = rand() % 2;
    pthread_barrier_init(&barrier_life_game_step, NULL, NUM_OF_THREADS);
    pthread_barrier_init(&barrier_life_game_step_end, NULL, NUM_OF_THREADS);
    pthread_barrier_init(&barrier_initialization, NULL, NUM_OF_THREADS);
    int k;
    for (k = 0; k < NUM_OF_THREADS; k++){
        temp[k] = k * (SIZE / NUM_OF_THREADS);
        status = pthread_create(&thread[k], NULL, life_1, &temp[k]);
        if (status != 0)
            perror("error pthread creating\n");
    }
    for (k = 0; k < NUM_OF_THREADS; ++k) {
        pthread_join(thread[k], NULL);
    }
    return 0;
}

