#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 8000

static long long generation = 0;
int num = 0;
int step = 0;
int SIZE = 500;
char map[2][MAX_SIZE][MAX_SIZE];
unsigned long long mask = 0;

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

int alive(int x, int y)
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

void recalc(int x, int y)
{
    if (map[step][x][y] == 0)
    {
        if (alive(x, y) == 3)
            map[1 - step][x][y] = 1;
        else
            map[1 - step][x][y] = 0;
    }
    if (map[step][x][y] == 1)
    {
        if (alive(x, y) == 3 || alive(x, y) == 2)
            map[1 - step][x][y] = 1;
        else
            map[1 - step][x][y] = 0;
    }
}

int main(int argc, char ** argv)
{
    if (argc < 2) {
        printf("bad params: ./life SIZE steps_amount freq\n");
        return 0;
    }
    int freq = 100;
    int generations_amount = 100;
    if (argc > 1) {
        SIZE = atoi(argv[1]);
    }
    if (argc > 2) {
        generations_amount = atoi(argv[2]);
    }
    if (argc > 3) {
        freq = atoi(argv[3]);
        if (freq == 0)
            freq = 1;
    }
    int i, j;
    /*FILE* game = fopen("game_of_life", "r");
    int c;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
        {
            c = fgetc(game);
            if (c == 10)
                c = fgetc(game);
            if (c != EOF)
                map[0][i][j] = c - '0';
        }
    }*/
    srand(time(NULL));
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            map[0][i][j] = rand() % 2;
    for (; generation < generations_amount; ++generation, step ^= 1)
    {
        if (generation % freq == 0) {
            system("clear");
            printf("GENERETION #%lld\n", generation);
            int f_size = SIZE < 40 ? SIZE : 40;
            for (i = 0; i < f_size; i++){
                for (j = 0; j < f_size; j++)
                    printf("%s ", map[step][i][j] ? "■" : ".");
                printf("\n");
            }
        }
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                recalc(i, j);
            }
        }

    }
    return 0;
}

